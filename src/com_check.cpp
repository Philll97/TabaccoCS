#include "com_check.h"
#include "uart.h"
#include "mqtt.h"

ComCheck::ComCheck(uint8_t tube_nr, uint16_t stack_size, uint8_t priority, BaseType_t core_id) : Task("com_check", stack_size, priority, core_id)
{
    this->str_status = TASK_STATE_CREATED;
    this->ui8_tube_nr = tube_nr;
    this->log(str_status.c_str());
}

ComCheck::~ComCheck()
{}

void ComCheck::perform_command()
{
    this->log("Check communication");
    // create peripherie command and set up address for the tube
    this->s_cur_command.address = this->ui8_tube_nr + I2C_START_ADDR - 1;
    uint8_t failed_com_count = 0;

    this->str_status = "check_communication";
    this->str_error = JSON_VAL_NO_ERROR;
    while(1)
    {
        if(failed_com_count == UART_MAX_RETRY)
        {
            this->str_error = JSON_VAL_UART_ERR;
            this->str_status = TASK_STATE_FINISHED;
            this->log("Communication error");
            return;
        }

        this->log("Send com check");

        // setup command
        this->s_cur_command.command_type = peripherie_command_types::check_communication;
        this->s_cur_command.data1 = 0;
        this->s_cur_command.data2 = 0;
        this->s_cur_command.data3 = 0;

        //set send flag
        this->b_uart_send_flag = true;
        // suspend task till reply arrives
        vTaskSuspend(NULL);
        
        // check if reply is good
        if(this->s_cur_reply.command_type == peripherie_command_types::check_communication)
        {
            switch(this->s_cur_reply.state)
            {
                case peripherie_states::ready:
                case peripherie_states::command_understood:
                    this->str_status = TASK_STATE_FINISHED;
                    return;
                case peripherie_states::not_configured:
                    this->str_status = TASK_STATE_FINISHED;
                    this->str_error = JSON_VAL_TUBE_NOT_CONF;
                    return;
                case peripherie_states::command_error:
                case peripherie_states::message_error:
                    failed_com_count++;
                    break;
            }            
        }
        else
            failed_com_count++;

    }
}

modul_command_types ComCheck::get_command()
{
    return modul_command_types::health_check;
}

std::string ComCheck::get_status()
{
    return this->str_status;
}

std::string ComCheck::get_error()
{
    return this->str_error;
}

uint8_t ComCheck::get_tube_nr()
{
    return this->ui8_tube_nr;
}

void ComCheck::set_uart_recieve_flag(peripherie_reply msg)
{
    this->s_cur_reply = msg;
    vTaskResume(this->get_handle());
}

bool ComCheck::check_uart_send_flag()
{
    return this->b_uart_send_flag;
}

peripherie_command ComCheck::get_uart_command()
{
    return this->s_cur_command;
}

void ComCheck::reset_uart_send_flag()
{
    this->b_uart_send_flag = false;
}

void ComCheck::log(std::string msg)
{    
    std::string log_str = "[ComCheck(" + std::to_string(this->ui8_tube_nr) + ")]: " + msg.c_str();
	Serial.println(log_str.c_str());
}