#ifndef COMMAND_HEADER
#define COMMAND_HEADER

#include <Arduino.h>
#include <memory>
#include "defines.h"
#include "task.h"
#include "release_content.h"
#include "com_check.h"
#include "check_if_empty.h"
#include "set_i2c_address.h"
#include "uart.h"
#include "mqtt.h"

class Command
{
    private:
        machine_command_types e_command;
        std::vector<std::shared_ptr<Task>> v_tasks;
        std::vector<uint8_t> waiting_tube_nrs;
        JSONVar js_mqtt_command;
        JSONVar js_mqtt_reply;

        void setup_tasks();

    public:
        Command(machine_command_types command, JSONVar mqtt_msg);
        ~Command();

        bool check_if_all_tasks_finished();
        bool send_next_uart_msg();
        bool set_uart_recieve_flag(std::shared_ptr<Task> cur_sender, bool error);
        void send_reply();

};

#endif