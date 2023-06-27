#include "command.h"

Command::Command(modul_command_types command, JSONVar mqtt_msg)
{
    this->e_command = command;
    this->js_mqtt_command = mqtt_msg;
    this->waiting_tube_nrs.clear();


    this->js_mqtt_reply = JSON.parse("");
    this->js_mqtt_reply = nullptr;

    setup_tasks();
}

Command::~Command()
{}

void Command::setup_tasks()
{
    switch(this->e_command)
    {
        case modul_command_types::set_i2c_address:
        {
            this->js_mqtt_reply[JSON_KEY_COMMAND] = JSON_VAL_HEALTH_CHECK;
            this->js_mqtt_reply[JSON_KEY_TUBE_NR] = (uint8_t) this->js_mqtt_command[JSON_KEY_TUBE_NR];
            this->js_mqtt_reply[JSON_KEY_ACKN] = JSON_VAL_ACKN;

            uint8_t tube_nr = (uint8_t) this->js_mqtt_command[JSON_KEY_TUBE_NR];
            if(tube_nr > 0 && tube_nr <= TUBE_CNT)
            {
                auto newTask = std::make_shared<SetI2CAddress>(tube_nr);
                newTask->start();

                this->v_tasks.push_back(newTask);
                delay(10);
            }
            else
            {
                this->js_mqtt_reply[JSON_KEY_ERROR] = JSON_VAL_TUBE_NR_WRONG;
            }
            break;
        }
        case modul_command_types::health_check:
        {
            this->js_mqtt_reply[JSON_KEY_COMMAND] = JSON_VAL_HEALTH_CHECK;
            this->js_mqtt_reply[JSON_KEY_FAILED_TUBES] = JSON.parse("[]");
            this->js_mqtt_reply[JSON_KEY_ACKN] = JSON_VAL_ACKN;
            for(int i = 1; i <= TUBE_CNT; i++)
            {
                auto newTask = std::make_shared<ComCheck>(i);
                newTask->start();

                this->v_tasks.push_back(newTask);
                delay(10);
            }
            break;
        }
        case modul_command_types::check_if_emtpy:
        {
            this->js_mqtt_reply[JSON_KEY_COMMAND] = JSON_VAL_CHECK_IF_EMPTY;
            this->js_mqtt_reply[JSON_KEY_TUBE_NRS] = JSON.parse("[]");
            this->js_mqtt_reply[JSON_KEY_ACKN] = JSON_VAL_ACKN;
            std::vector<uint8_t> started_tube_nrs;
            JSONVar tube_nrs = this->js_mqtt_command[JSON_KEY_TUBE_NRS];
            for (int i = 0; i < tube_nrs.length(); i++) 
            {
                uint8_t tube_nr = (uint8_t) tube_nrs[i];
                if(tube_nr > 0 && tube_nr <= TUBE_CNT)
                {   
                    if(std::find(started_tube_nrs.begin(), started_tube_nrs.end(), tube_nr) == started_tube_nrs.end()) // ignore if a tube number is called twice
                    {
                        auto newTask = std::make_shared<CheckIfEmpty>(tube_nr);
                        newTask->start();

                        this->v_tasks.push_back(newTask);
                        started_tube_nrs.push_back(tube_nr);
                        delay(10);
                    }
                }
                else
                {
                    JSONVar task_reply;
                    task_reply[JSON_KEY_TUBE_NR] = tube_nr;                    
                    task_reply[JSON_KEY_ERROR] = JSON_VAL_TUBE_NR_WRONG;

                    if(this->js_mqtt_reply[JSON_KEY_TUBE_NRS].length() == -1)
                        this->js_mqtt_reply[JSON_KEY_TUBE_NRS][0] = task_reply;
                    else
                        this->js_mqtt_reply[JSON_KEY_TUBE_NRS][this->js_mqtt_reply[JSON_KEY_TUBE_NRS].length()] = task_reply;
                }
            }
            break;
        }
        case modul_command_types::release_content:
        {
            this->js_mqtt_reply[JSON_KEY_COMMAND] = JSON_VAL_RELEASE_CONTENT;
            this->js_mqtt_reply[JSON_KEY_TUBE_NRS] = JSON.parse("[]");
            this->js_mqtt_reply[JSON_KEY_ACKN] = JSON_VAL_ACKN;
            std::vector<uint8_t> started_tube_nrs;
            JSONVar tube_nrs = this->js_mqtt_command[JSON_KEY_TUBE_NRS];
            for (int i = 0; i < tube_nrs.length(); i++) 
            {
                uint8_t tube_nr = (uint8_t) tube_nrs[i];
                if(tube_nr > 0 && tube_nr <= TUBE_CNT)
                { 
                    if(std::find(started_tube_nrs.begin(), started_tube_nrs.end(), tube_nr) == started_tube_nrs.end()) // put tube number on the wating list if it is already called
                    {
                        auto newTask = std::make_shared<ReleaseContent>(tube_nr);
                        newTask->start();

                        this->v_tasks.push_back(newTask);
                        started_tube_nrs.push_back(tube_nr);
                        delay(10);
                    }
                    else
                    {
                        this->waiting_tube_nrs.push_back(tube_nr);
                    }
                }
                else
                {
                    JSONVar task_reply;
                    task_reply[JSON_KEY_TUBE_NR] = tube_nr;             
                    task_reply[JSON_KEY_RELEASE_SUCCESS] = false;       
                    task_reply[JSON_KEY_ERROR] = JSON_VAL_TUBE_NR_WRONG;

                    if(this->js_mqtt_reply[JSON_KEY_TUBE_NRS].length() == -1)
                        this->js_mqtt_reply[JSON_KEY_TUBE_NRS][0] = task_reply;
                    else
                        this->js_mqtt_reply[JSON_KEY_TUBE_NRS][this->js_mqtt_reply[JSON_KEY_TUBE_NRS].length()] = task_reply;
                }

            }
            break;
        }
    }
}

bool Command::check_if_all_tasks_finished()
{
    bool finished = true;
    switch(this->e_command)
    {
        case modul_command_types::set_i2c_address:
        {
            std::shared_ptr<SetI2CAddress> cur_task = std::static_pointer_cast<SetI2CAddress>(this->v_tasks.front());
            if(cur_task->get_status() == TASK_STATE_FINISHED)
            {      
                this->js_mqtt_reply[JSON_KEY_ERROR] = cur_task->get_error().c_str();

                this->v_tasks.front() = nullptr;
                this->v_tasks.clear();
            }
            else
                finished = false;
            break;
        }
        case modul_command_types::health_check:
        {
            for(int i = 0; i < this->v_tasks.size(); i++)
            { 
                std::shared_ptr<ComCheck> com_check = std::static_pointer_cast<ComCheck>(this->v_tasks.at(i));
                if(com_check->get_status() == TASK_STATE_FINISHED)
                {    
                    if(com_check->get_error() != JSON_VAL_NO_ERROR)
                    {                
                        JSONVar task_reply;
                        task_reply[JSON_KEY_TUBE_NR] = com_check->get_tube_nr();                        
                        task_reply[JSON_KEY_ERROR] = com_check->get_error().c_str();

                        if(this->js_mqtt_reply[JSON_KEY_FAILED_TUBES].length() == -1)
                            this->js_mqtt_reply[JSON_KEY_FAILED_TUBES][0] = task_reply;
                        else
                            this->js_mqtt_reply[JSON_KEY_FAILED_TUBES][this->js_mqtt_reply[JSON_KEY_FAILED_TUBES].length()] = task_reply;
                    }

                    this->v_tasks.at(i) = nullptr;
                    this->v_tasks.erase(this->v_tasks.begin() + i);
                    i--;
                }
                else
                    finished = false;
            }
            break;
        }
        case modul_command_types::check_if_emtpy:
        {
            for(int i = 0; i < this->v_tasks.size(); i++)
            { 
                std::shared_ptr<CheckIfEmpty> check_if_empty = std::static_pointer_cast<CheckIfEmpty>(this->v_tasks.at(i));
                if(check_if_empty->get_status() == TASK_STATE_FINISHED)
                {                    
                    JSONVar task_reply;
                    task_reply[JSON_KEY_TUBE_NR] = check_if_empty->get_tube_nr();                    
                    task_reply[JSON_KEY_ERROR] = check_if_empty->get_error().c_str();

                    if(this->js_mqtt_reply[JSON_KEY_TUBE_NRS].length() == -1)
                        this->js_mqtt_reply[JSON_KEY_TUBE_NRS][0] = task_reply;
                    else
                        this->js_mqtt_reply[JSON_KEY_TUBE_NRS][this->js_mqtt_reply[JSON_KEY_TUBE_NRS].length()] = task_reply;

                    this->v_tasks.at(i) = nullptr;
                    this->v_tasks.erase(this->v_tasks.begin() + i);
                    i--;
                }
                else
                    finished = false;
            }
            break;
        }    
        case modul_command_types::release_content:
        {
            for(int i = 0; i < this->v_tasks.size(); i++)
            { 
                std::shared_ptr<ReleaseContent> release = std::static_pointer_cast<ReleaseContent>(this->v_tasks.at(i));
                if(release->get_status() == TASK_STATE_FINISHED)
                {                    
                    JSONVar task_reply;
                    task_reply[JSON_KEY_TUBE_NR] = release->get_tube_nr();
                    if(release->get_error() == JSON_VAL_NO_ERROR || release->get_error() == JSON_VAL_TUBE_EMPTIED)
                        task_reply[JSON_KEY_RELEASE_SUCCESS] = true;
                    else
                        task_reply[JSON_KEY_RELEASE_SUCCESS] = false;
                    
                    task_reply[JSON_KEY_ERROR] = release->get_error().c_str();

                    if(this->js_mqtt_reply[JSON_KEY_TUBE_NRS].length() == -1)
                        this->js_mqtt_reply[JSON_KEY_TUBE_NRS][0] = task_reply;
                    else
                        this->js_mqtt_reply[JSON_KEY_TUBE_NRS][this->js_mqtt_reply[JSON_KEY_TUBE_NRS].length()] = task_reply;

                    if(std::find(this->waiting_tube_nrs.begin(), this->waiting_tube_nrs.end(), release->get_tube_nr()) != this->waiting_tube_nrs.end())
                    {
                        finished = false;
                        this->waiting_tube_nrs.erase(std::find(this->waiting_tube_nrs.begin(), this->waiting_tube_nrs.end(), release->get_tube_nr()));
                        auto newTask = std::make_shared<ReleaseContent>(release->get_tube_nr());
                        newTask->start();

                        this->v_tasks.push_back(newTask);
                        delay(100);
                    }

                    this->v_tasks.at(i) = nullptr;
                    this->v_tasks.erase(this->v_tasks.begin() + i);
                    i--;
                }
                else
                    finished = false;
            }
            break;
        }
    }
    return finished;
}

bool Command::send_next_uart_msg()
{
    switch(this->e_command)
    {
        case modul_command_types::set_i2c_address:
        {
            for(int i = 0; i < this->v_tasks.size(); i++)
            {
                std::shared_ptr<SetI2CAddress> cur_task = std::static_pointer_cast<SetI2CAddress>(this->v_tasks.at(i));
                if(cur_task->check_uart_send_flag())
                {
                    uart::cur_sender = cur_task;
                    cur_task->log("Send command");
                    uart::send(cur_task->get_uart_command());
                    cur_task->reset_uart_send_flag();
                    uart::state = communication_states::waiting_for_msg;
                    return true;
                }
            }
            break;
        }
        case modul_command_types::health_check:
        {
            for(int i = 0; i < this->v_tasks.size(); i++)
            {
                std::shared_ptr<ComCheck> cur_task = std::static_pointer_cast<ComCheck>(this->v_tasks.at(i));
                if(cur_task->check_uart_send_flag())
                {
                    uart::cur_sender = cur_task;
                    cur_task->log("Send command");
                    uart::send(cur_task->get_uart_command());
                    cur_task->reset_uart_send_flag();
                    uart::state = communication_states::waiting_for_msg;
                    return true;
                }
            }
            break;
        }
        case modul_command_types::check_if_emtpy:
        {
            for(int i = 0; i < this->v_tasks.size(); i++)
            {
                std::shared_ptr<CheckIfEmpty> cur_task = std::static_pointer_cast<CheckIfEmpty>(this->v_tasks.at(i));
                if(cur_task->check_uart_send_flag())
                {
                    uart::cur_sender = cur_task;
                    cur_task->log("Send command");
                    uart::send(cur_task->get_uart_command());
                    cur_task->reset_uart_send_flag();
                    uart::state = communication_states::waiting_for_msg;
                    return true;
                }
            }
            break;
        }
        case modul_command_types::release_content:
        {
            for(int i = 0; i < this->v_tasks.size(); i++)
            {
                std::shared_ptr<ReleaseContent> cur_task = std::static_pointer_cast<ReleaseContent>(this->v_tasks.at(i));
                if(cur_task->check_uart_send_flag())
                {
                    uart::cur_sender = cur_task;
                    cur_task->log("Send command");
                    uart::send(cur_task->get_uart_command());
                    cur_task->reset_uart_send_flag();
                    uart::state = communication_states::waiting_for_msg;
                    return true;
                }
            }
            break;
        }
    }

    return false;
}

bool Command::set_uart_recieve_flag(std::shared_ptr<Task> cur_sender, bool error)
{
    switch(this->e_command)
    {
        case modul_command_types::set_i2c_address:
        {
            std::shared_ptr<SetI2CAddress> cur_task = std::static_pointer_cast<SetI2CAddress>(this->v_tasks.front());
            if(cur_sender == cur_task)
            {
                if(error)
                {
                    cur_task->log("Communication failed");
                    peripherie_reply reply;
                    reply.state = peripherie_states::message_error;
                    cur_task->set_uart_recieve_flag(reply);
                }
                else
                    cur_task->set_uart_recieve_flag(uart::reply);
                uart::state = communication_states::idle;
                return true;
            }
            break;
        }
        case modul_command_types::health_check:
        {
            for(int i = 0; i < this->v_tasks.size(); i++)
            {
                std::shared_ptr<ComCheck> cur_task = std::static_pointer_cast<ComCheck>(this->v_tasks.at(i));
                if(cur_sender == cur_task)
                {
                    if(error)
                    {
                        cur_task->log("Communication failed");
                        peripherie_reply reply;
                        reply.state = peripherie_states::message_error;
                        cur_task->set_uart_recieve_flag(reply);
                    }
                    else
                        cur_task->set_uart_recieve_flag(uart::reply);
                    uart::state = communication_states::idle;
                    return true;
                }
            }
            break;
        }
        case modul_command_types::check_if_emtpy:
        {
            for(int i = 0; i < this->v_tasks.size(); i++)
            {
                std::shared_ptr<CheckIfEmpty> cur_task = std::static_pointer_cast<CheckIfEmpty>(this->v_tasks.at(i));
                if(cur_sender == cur_task)
                {
                    if(error)
                    {
                        cur_task->log("Communication failed");
                        peripherie_reply reply;
                        reply.state = peripherie_states::message_error;
                        cur_task->set_uart_recieve_flag(reply);
                    }
                    else
                        cur_task->set_uart_recieve_flag(uart::reply);
                    uart::state = communication_states::idle;
                    return true;
                }
            }
            break;
        }
        case modul_command_types::release_content:
        {
            for(int i = 0; i < this->v_tasks.size(); i++)
            {
                std::shared_ptr<ReleaseContent> cur_task = std::static_pointer_cast<ReleaseContent>(this->v_tasks.at(i));
                if(cur_sender == cur_task)
                {
                    if(error)
                    {
                        cur_task->log("Communication failed");
                        peripherie_reply reply;
                        reply.state = peripherie_states::message_error;
                        cur_task->set_uart_recieve_flag(reply);
                    }
                    else
                        cur_task->set_uart_recieve_flag(uart::reply);
                    uart::state = communication_states::idle;
                    return true;
                }
            }
            break;
        }
    }

    return false;
}

void Command::send_reply()
{
    mqtt::reply = this->js_mqtt_reply;
    Serial.println(JSON.stringify(this->js_mqtt_reply).c_str());
    mqtt::send_reply();
}