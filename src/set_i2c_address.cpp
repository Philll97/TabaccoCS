#include "set_i2c_address.h"
#include "uart.h"
#include "mqtt.h"

SetI2CAddress::SetI2CAddress(uint8_t tube_nr, uint16_t stack_size, uint8_t priority, BaseType_t core_id) : Task("com_check", stack_size, priority, core_id)
{
    this->str_status = TASK_STATE_CREATED;
    this->ui8_tube_nr = tube_nr;
    this->log(str_status.c_str());
}

SetI2CAddress::~SetI2CAddress()
{}

void SetI2CAddress::perform_command()
{
    this->log("Set I2C address");
    // create peripherie command and set up address for the tube
    this->s_cur_command.address = I2C_CONFIG_ADDR;
    uint8_t failed_com_count = 0;

    this->str_status = "set_i2c_address";
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

        // setup command
        this->s_cur_command.command_type = peripherie_command_types::set_i2c_address;
        this->s_cur_command.data1 = this->ui8_tube_nr + I2C_START_ADDR - 1;
        this->s_cur_command.data2 = 0;
        this->s_cur_command.data3 = 0;

        //set send flag
        this->b_uart_send_flag = true;
        // suspend task till reply arrives
        vTaskSuspend(NULL);
        
        // check if reply is good
        if(this->s_cur_reply.command_type == peripherie_command_types::set_i2c_address)
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

modul_command_types SetI2CAddress::get_command()
{
    return modul_command_types::set_i2c_address;
}

std::string SetI2CAddress::get_status()
{
    return this->str_status;
}

std::string SetI2CAddress::get_error()
{
    return this->str_error;
}

uint8_t SetI2CAddress::get_tube_nr()
{
    return this->ui8_tube_nr;
}

void SetI2CAddress::set_uart_recieve_flag(peripherie_reply msg)
{
    this->s_cur_reply = msg;
    vTaskResume(this->get_handle());
}

bool SetI2CAddress::check_uart_send_flag()
{
    return this->b_uart_send_flag;
}

peripherie_command SetI2CAddress::get_uart_command()
{
    return this->s_cur_command;
}

void SetI2CAddress::reset_uart_send_flag()
{
    this->b_uart_send_flag = false;
}

void SetI2CAddress::log(std::string msg)
{    
    std::string log_str = "[SetI2CAddress(" + std::to_string(this->ui8_tube_nr) + ")]: " + msg.c_str();
	Serial.println(log_str.c_str());
}