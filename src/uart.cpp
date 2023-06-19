#include "uart.h"

HardwareSerial uart::SerialPort(1); // use UART1
communication_states uart::state;
peripherie_reply uart::reply;
bool uart::ready;
long uart::timestamp;
std::shared_ptr<Task> uart::cur_sender;

void uart::init()
{
    SerialPort.begin(UART_BAUDRATE, UART_CONFIG, UART_RX_PIN, UART_TX_PIN);
    SerialPort.setRxTimeout(UART_RX_TIMEOUT);
    state = communication_states::idle;
    ready = true;
    timestamp = 0;
}

void onReceiveFunction(void) {
    // This is a callback function that will be activated on UART RX events
    size_t available = uart::SerialPort.available();
    Serial.printf("onReceive Callback:: There are %d bytes available: ", available);
    if(available == UART_MSG_SIZE)
    {
        uint8_t data_in[UART_MSG_SIZE];
        for(int i = 0; i < available; i++)
        {
            data_in[i] = uart::SerialPort.read();
            Serial.print(data_in[i], HEX);
            Serial.print(" | ");
        }
        Serial.println();
        if(data_in[5] == data_in[0] + data_in[1] + data_in[2] + data_in[3] + data_in[4])
        {
            uart::reply.address = data_in[0];
            uart::reply.command_type = static_cast<peripherie_command_types>(data_in[1]);
            uart::reply.state = static_cast<peripherie_states>(data_in[2]);
            uart::reply.data1 = data_in[3];
            uart::reply.data2 = data_in[4];
            uart::state = communication_states::new_msg;

        }
        else
        {
            Serial.println("Communication Error: Wrong checksum");
            uart::state = communication_states::error;
        }
    }
    else
    {
        Serial.println("Communication Error: Wrong message size");
        uart::state = communication_states::error;
    }

    uart::SerialPort.onReceive(NULL);
}

void uart::send(peripherie_command command)
{
    uint8_t data_out[UART_MSG_SIZE];
    data_out[0] = command.address;
    data_out[1] = static_cast<uint8_t>(command.command_type);
    data_out[2] = command.data1;
    data_out[3] = command.data2;
    data_out[4] = command.data3;
    data_out[5] = data_out[0] + data_out[1] + data_out[2] + data_out[3] + data_out[4];
    
    SerialPort.flush(); // wait Serial FIFO to be empty and then spend almost no time processing it
    SerialPort.setRxFIFOFull(UART_MSG_SIZE);
    SerialPort.onReceive(onReceiveFunction, true);
    
    SerialPort.write(data_out, UART_MSG_SIZE);
    timestamp = millis();
}

bool uart::timeout_reached()
{
    long now = millis();
    if(now > timestamp + UART_RX_TIMEOUT * 1000)
        return true;
    
    return false;
}