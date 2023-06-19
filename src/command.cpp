#include "command.h"

Command::Command(machine_command_types command, JSONVar mqtt_msg)
{
    this->e_command = command;
    this->js_mqtt_command = mqtt_msg;
    this->waiting_tube_nrs.clear();


    this->js_mqtt_reply = JSON.parse("");
    this->js_mqtt_reply = nullptr;
    this->js_mqtt_reply[JSON_KEY_COMMAND] = JSON_VAL_RELEASE_CONTENT;
    this->js_mqtt_reply[JSON_KEY_TUBE_NRS] = JSON.parse("[]");
    this->js_mqtt_reply[JSON_KEY_ACKN] = JSON_VAL_ACKN;

    setup_tasks();
}

Command::~Command()
{}

void Command::setup_tasks()
{
    switch(this->e_command)
    {
        case machine_command_types::release_content:
        {
            std::vector<uint8_t> started_tube_nrs;
            JSONVar tube_nrs = this->js_mqtt_command[JSON_KEY_TUBE_NRS];
            for (int i = 0; i < tube_nrs.length(); i++) {
                uint8_t tube_nr = (uint8_t) tube_nrs[i];
                Serial.println(tube_nr);
                if(std::find(started_tube_nrs.begin(), started_tube_nrs.end(), tube_nr) == started_tube_nrs.end())
                {
                    Serial.println("create task");
                    auto newTask = std::make_shared<ReleaseContent>(tube_nr);
                    newTask->start();

                    this->v_tasks.push_back(newTask);
                    started_tube_nrs.push_back(tube_nr);
                    delay(100);
                }
                else
                {
                    Serial.println("wait");
                    this->waiting_tube_nrs.push_back(tube_nr);
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
        case machine_command_types::release_content:
        {
            for(int i = 0; i < this->v_tasks.size(); i++)
            { 
                std::shared_ptr<ReleaseContent> release = std::static_pointer_cast<ReleaseContent>(this->v_tasks.at(i));
                if((release->get_status() == "finished" || release->get_error() == "com_failed") && !release->checked_after_finished)
                {
                    release->checked_after_finished = true;
                    
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
                else if(release->get_status() != "finished")
                    finished = false;
            }
            break;
        }
    }
    return finished;
}

xTaskHandle Command::get_task_with_send_flag()
{
    switch(this->e_command)
    {
        case machine_command_types::release_content:
        {
            for(int i = 0; i < this->v_tasks.size(); i++)
            {
                if((std::static_pointer_cast<ReleaseContent>(this->v_tasks.at(i)))->check_uart_send_flag())
                    return this->v_tasks.at(i)->get_handle();
            }
            break;
        }
    }
    return nullptr;
}

bool Command::send_next_uart_msg()
{
    switch(this->e_command)
    {
        case machine_command_types::release_content:
        {
            for(int i = 0; i < this->v_tasks.size(); i++)
            {
                std::shared_ptr<ReleaseContent> release = std::static_pointer_cast<ReleaseContent>(this->v_tasks.at(i));
                if(release->check_uart_send_flag())
                {
                    uart::cur_sender = release;
                    release->log("Send command");
                    uart::send(release->get_uart_command());
                    release->reset_uart_send_flag();
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
        case machine_command_types::release_content:
        {
            for(int i = 0; i < this->v_tasks.size(); i++)
            {
                std::shared_ptr<ReleaseContent> release = std::static_pointer_cast<ReleaseContent>(this->v_tasks.at(i));
                if(cur_sender == release)
                {
                    if(error)
                    {
                        release->log("Communication failed");
                        peripherie_reply reply;
                        reply.state = peripherie_states::message_error;
                        release->set_uart_recieve_flag(reply);
                    }
                    else
                        release->set_uart_recieve_flag(uart::reply);
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