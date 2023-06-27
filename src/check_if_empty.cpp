#include "check_if_empty.h"
#include "uart.h"
#include "mqtt.h"

CheckIfEmpty::CheckIfEmpty(uint8_t tube_nr, uint16_t stack_size, uint8_t priority, BaseType_t core_id) : Task("check_if_empty", stack_size, priority, core_id)
{
    this->str_status = TASK_STATE_CREATED;
    this->ui8_tube_nr = tube_nr;
    this->log(str_status.c_str());
}

CheckIfEmpty::~CheckIfEmpty()
{}

void CheckIfEmpty::perform_command()
{
    this->log("Start check if empty");
    // create peripherie command and set up address for the tube
    this->s_cur_command.address = this->ui8_tube_nr + I2C_START_ADDR - 1;
    uint8_t state = 0;
    uint8_t failed_com_count = 0;

    this->log("put tube in standby");
    this->str_status = "put_in_standby";
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
        switch(state)
        {
            case 0: // turn off output B
            {
                this->log("turn off output B");

                // setup command
                this->s_cur_command.command_type = peripherie_command_types::output_off;
                this->s_cur_command.data1 = 2;
                this->s_cur_command.data2 = 0;
                this->s_cur_command.data3 = 0;

                //set send flag
                this->b_uart_send_flag = true;
                // suspend task till reply arrives
                vTaskSuspend(NULL);

                // check if reply is good
                if(this->s_cur_reply.command_type == peripherie_command_types::output_off && this->s_cur_reply.state == peripherie_states::ready)
                {
                    failed_com_count = 0;
                    state = 1;
                    
                }
                else
                    failed_com_count++;

                break;
            }
            case 1: // turn on output A
            {
                this->log("turn on output A");

                // setup command
                this->s_cur_command.command_type = peripherie_command_types::output_on;
                this->s_cur_command.data1 = 1;
                this->s_cur_command.data2 = 0;
                this->s_cur_command.data3 = 0;

                //set send flag
                this->b_uart_send_flag = true;
                // suspend task till reply arrives
                vTaskSuspend(NULL);

                // check if reply is good
                if(this->s_cur_reply.command_type == peripherie_command_types::output_on && this->s_cur_reply.state == peripherie_states::ready)
                {
                    failed_com_count = 0;
                    this->str_status = "check_if_empyt";
                    state = 2;
                }
                else
                    failed_com_count++;

                break;
            }
            case 2: // check if empty
            {
                this->log("check if empty");

                // setup command
                this->s_cur_command.command_type = peripherie_command_types::read_digital_inputs;
                this->s_cur_command.data1 = 0;
                this->s_cur_command.data2 = 0;
                this->s_cur_command.data3 = 0;

                //set send flag
                this->b_uart_send_flag = true;
                // suspend task till reply arrives
                vTaskSuspend(NULL);

                // check if reply is good
                if(this->s_cur_reply.command_type == peripherie_command_types::read_digital_inputs && this->s_cur_reply.state == peripherie_states::ready)
                {
                    failed_com_count = 0;
                    if(this->s_cur_reply.data1 == 1) 
                    {
                        this->str_error = JSON_VAL_TUBE_EMPTY;
                        this->log("Tube emtpy");
                    }
                    
                    state = 30;
                }
                else
                    failed_com_count++;

                break;
            }
            case 30: // turn off output A
            {
                this->log("turn off output A");

                // setup command
                this->s_cur_command.command_type = peripherie_command_types::output_off;
                this->s_cur_command.data1 = 1;
                this->s_cur_command.data2 = 0;
                this->s_cur_command.data3 = 0;

                //set send flag
                this->b_uart_send_flag = true;
                // suspend task till reply arrives
                vTaskSuspend(NULL);

                // check if reply is good
                if(this->s_cur_reply.command_type == peripherie_command_types::output_off && this->s_cur_reply.state == peripherie_states::ready)
                {
                    failed_com_count = 0;
                    this->log("Finished");
                    this->str_status = TASK_STATE_FINISHED;
                    return;
                }
                else
                    failed_com_count++;

                break;
            }
        }
    }
}

modul_command_types CheckIfEmpty::get_command()
{
    return modul_command_types::check_if_emtpy;
}

std::string CheckIfEmpty::get_status()
{
    return this->str_status;
}

std::string CheckIfEmpty::get_error()
{
    return this->str_error;
}

uint8_t CheckIfEmpty::get_tube_nr()
{
    return this->ui8_tube_nr;
}

void CheckIfEmpty::set_uart_recieve_flag(peripherie_reply msg)
{
    this->s_cur_reply = msg;
    vTaskResume(this->get_handle());
}

bool CheckIfEmpty::check_uart_send_flag()
{
    return this->b_uart_send_flag;
}

peripherie_command CheckIfEmpty::get_uart_command()
{
    return this->s_cur_command;
}

void CheckIfEmpty::reset_uart_send_flag()
{
    this->b_uart_send_flag = false;
}

void CheckIfEmpty::log(std::string msg)
{    
    std::string log_str = "[CheckIfEmpty(" + std::to_string(this->ui8_tube_nr) + ")]: " + msg.c_str();
	Serial.println(log_str.c_str());
}