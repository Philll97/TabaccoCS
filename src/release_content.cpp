#include "release_content.h"
#include "uart.h"
#include "mqtt.h"

ReleaseContent::ReleaseContent(uint8_t tube_nr, uint16_t stack_size, uint8_t priority, BaseType_t core_id) : Task("release_content", stack_size, priority, core_id)
{
    this->str_status = "Task created";
    this->ui8_tube_nr = tube_nr;
    this->log(str_status.c_str());
}

ReleaseContent::~ReleaseContent()
{}

void ReleaseContent::perform_command()
{
    this->log("Start Release Content");
    // create peripherie command and set up address for the tube
    this->s_cur_command.address = this->ui8_tube_nr + I2C_START_ADDR - 1;
    uint8_t state = 0;
    uint8_t failed_com_count = 0;

    this->log("put tube in standby");
    this->str_status = "check_if_ready";
    this->str_error = "no_error";
    while(1)
    {
        if(failed_com_count == UART_MAX_RETRY)
        {
            this->str_error = "com_failed";
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
                    state = 2;
                }
                else
                    failed_com_count++;

                break;
            }
            case 2: // check if ready
            {
                this->log("check if ready");

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
                    if(this->s_cur_reply.data1 == 0) 
                    {
                        if(this->s_cur_reply.data2 == 0) // position reset still ongoing pause for 1 Second
                        {
                            delay(500);
                        }
                        else
                        {
                            this->log("Tube ready -> eject product");
                            this->str_status = "eject_product";
                            state = 10;
                        }
                    }
                    else
                    {
                        this->str_error = "tube_empty";
                        this->log("Tube empty");
                        state = 30;
                    }
                }
                else
                    failed_com_count++;

                break;
            }
            case 10: // turn off output A
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
                    state = 11;
                }
                else
                    failed_com_count++;

                break;
            }
            case 11: // turn on output B
            {
                this->log("turn on output B");

                // setup command
                this->s_cur_command.command_type = peripherie_command_types::output_on;
                this->s_cur_command.data1 = 2;
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
                    delay(2000);
                    state = 12;
                }
                else
                    failed_com_count++;

                break;
            }
            case 12: // check if finished
            {
                this->log("check if finished");

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
                    if(this->s_cur_reply.data2 == 0) // position reset still ongoing pause for 0.5 Second
                    {
                        delay(500);
                    }
                    else
                    {
                        state = 20;
                        this->log("product ejected -> check if emptied");
                        this->str_status = "check_if_emptied";
                    }
                }
                else
                    failed_com_count++;

                break;
            }
            case 20: // turn off output B
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
                    state = 21;
                }
                else
                    failed_com_count++;

                break;
            }
            case 21: // turn on output A
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
                    state = 22;
                }
                else
                    failed_com_count++;

                break;
            }
            case 22: // check if emptied
            {
                this->log("check if emptied");

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
                        this->str_error = "tube_emptied";
                        this->log("Tube emptied");
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
                    this->str_status = "finished";
                    return;
                }
                else
                    failed_com_count++;

                break;
            }
        }
    }
}

machine_command_types ReleaseContent::get_command()
{
    return machine_command_types::release_content;
}

std::string ReleaseContent::get_status()
{
    return this->str_status;
}

uint8_t ReleaseContent::get_tube_nr()
{
    return this->ui8_tube_nr;
}

void ReleaseContent::set_uart_recieve_flag(peripherie_reply msg)
{
    this->s_cur_reply = msg;
    vTaskResume(this->get_handle());
}

bool ReleaseContent::check_uart_send_flag()
{
    return this->b_uart_send_flag;
}

peripherie_command ReleaseContent::get_uart_command()
{
    return this->s_cur_command;
}

void ReleaseContent::reset_uart_send_flag()
{
    this->b_uart_send_flag = false;
}

void ReleaseContent::log(std::string msg)
{    
    std::string log_str = "[ReleaseContent(" + std::to_string(this->ui8_tube_nr) + ")]: " + msg.c_str();
	Serial.println(log_str.c_str());
}