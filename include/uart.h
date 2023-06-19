#ifndef UART_HEADER
#define UART_HEADER

#include <HardwareSerial.h>
#include <Arduino.h>
#include <memory>
#include "defines.h"
#include "task.h"

namespace uart
{
    extern HardwareSerial SerialPort;
    extern communication_states state;
    extern peripherie_reply reply;
    extern bool ready;
    extern long timestamp;
    extern std::shared_ptr<Task> cur_sender;

    void init();
    void send(peripherie_command command);
    bool timeout_reached();
}

#endif