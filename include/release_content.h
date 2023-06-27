#ifndef RELEASE_CONTENT_HEADER
#define RELEASE_CONTENT_HEADER

#include <Arduino.h>
#include "defines.h"
#include "task.h"

class ReleaseContent : public Task
{
    private:
        uint8_t ui8_tube_nr;
        std::string str_status;
        std::string str_error;
        bool b_uart_send_flag;
        peripherie_reply s_cur_reply;
        peripherie_command s_cur_command;

    public:
        ReleaseContent(uint8_t tube_nr, uint16_t stack_size = 5000, uint8_t priority = 10, BaseType_t core_id = 0);
        ~ReleaseContent();

        void perform_command();
        modul_command_types get_command();
        std::string get_status();
        std::string get_error();
        uint8_t get_tube_nr();

        void set_uart_recieve_flag(peripherie_reply msg);
        bool check_uart_send_flag();
        void reset_uart_send_flag();
        peripherie_command get_uart_command();
        void log(std::string msg);

};

#endif