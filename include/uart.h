#ifndef UART_HEADER
#define UART_HEADER

#include <HardwareSerial.h>
#include <Arduino.h>
#include "defines.h"

namespace uart
{
    extern HardwareSerial SerialPort;
    extern communication_states state;
    extern peripherie_reply reply;

    void init();
    void send(peripherie_command command);    
}

#endif