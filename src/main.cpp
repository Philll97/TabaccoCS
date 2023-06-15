
#include <Arduino.h>
#include <HardwareSerial.h>
#include <SPI.h>
#include <Ethernet.h>
#include <MQTT.h>
 
#include "defines.h"
#include "ethernet.h"
#include "mqtt.h"
#include "uart.h"
#include "timer.h"
#include  "release_content.h"

machine_tasks current_task;
release_content_sub_tasks release_content_sub_task; 
release_content_sub_sub_tasks release_content_sub_sub_task; 
check_if_empty_sub_tasks check_if_empty_sub_task;
peripherie_command current_command;

uint8_t health_check_cnt;
void loop2(void* pvParameters)
{
  uint8_t task_id = *((uint8_t*)pvParameters);
  while(1)
  {
    Serial.print("Loop: ");
    Serial.println(task_id); 
    delay(1000);
  }
}

uint8_t task_id;
uint8_t task1_id = 0;
uint8_t tube_nr = 39;
std::vector<uint8_t> task_ids;
ReleaseContent newReleas = ReleaseContent(tube_nr);
int cnt = 0;

void setup() 
{
    Serial.begin(115200);
    uart::init();
    delay(1000);
    //ethernet::init();
    //mqtt::init(&ethernet::client, MQTT_BROKER);

    current_task = machine_tasks::idle;

    delay(5000);
    Serial.println("Try start process");
    newReleas.start();
}
 
void loop() 
{
  // --------- UART ------------
  if(uart::state == communication_states::idle)
  {
    if(newReleas.check_uart_send_flag())
    {
      uart::send(newReleas.get_uart_command());
      newReleas.reset_uart_send_flag();
      uart::state = communication_states::waiting_for_msg;
    }
  }
  else if(uart::state == communication_states::waiting_for_msg)
  {
    if(uart::timeout_reached())
    {
      peripherie_reply reply;
      reply.state = peripherie_states::message_error;
      newReleas.set_uart_recieve_flag(reply);
      uart::state = communication_states::idle;
    }
  }
  else if(uart::state == communication_states::new_msg)
  {
    if(uart::reply.address == 0x46)
    {
      newReleas.set_uart_recieve_flag(uart::reply);
    }
    uart::state = communication_states::idle;
  }
  else if(uart::state == communication_states::error)
  {
    peripherie_reply reply;
      reply.state = peripherie_states::message_error;
      newReleas.set_uart_recieve_flag(reply);
      uart::state = communication_states::idle;
  }
  /*
    mqtt::loop();

    if(timer::check() && uart::state == communication_states::idle && mqtt::state == communication_states::idle)
    {
      switch(current_task)
      {
        case machine_tasks::idle:
          mqtt::state = communication_states::waiting_for_msg;
          break;
        
        case machine_tasks::health_check:
          current_command.address = health_check_cnt + I2C_START_ADDR - 1;
          Serial.println(current_command.address, HEX);
          current_command.command_type = peripherie_command_types::check_communication;

          uart::send(current_command);
          uart::state = communication_states::waiting_for_msg;
          break;
        case machine_tasks::release_content:
          switch(release_content_sub_task)
          {
            case release_content_sub_tasks::put_tube_in_standby:
              switch(release_content_sub_sub_task)
              {
                case release_content_sub_sub_tasks::set_output_A:
                  current_command.command_type = peripherie_command_types::output_on;
                  current_command.data1 = 1;

                  uart::send(current_command);
                  uart::state = communication_states::waiting_for_msg;
                  break;  
                case release_content_sub_sub_tasks::reset_output_B:
                  current_command.command_type = peripherie_command_types::output_off;
                  current_command.data1 = 2;

                  uart::send(current_command);
                  uart::state = communication_states::waiting_for_msg;
                  break;    
              }
              break;
            case release_content_sub_tasks::check_if_ready:
              current_command.command_type = peripherie_command_types::read_digital_inputs;

              uart::send(current_command);
              uart::state = communication_states::waiting_for_msg;
              break;
            case release_content_sub_tasks::eject_product:
              switch(release_content_sub_sub_task)
              {
                case release_content_sub_sub_tasks::reset_output_A:
                  current_command.command_type = peripherie_command_types::output_off;
                  current_command.data1 = 1;

                  uart::send(current_command);
                  uart::state = communication_states::waiting_for_msg;
                  break;   
                case release_content_sub_sub_tasks::set_output_B:
                  current_command.command_type = peripherie_command_types::output_on;
                  current_command.data1 = 2;

                  uart::send(current_command);
                  uart::state = communication_states::waiting_for_msg;
                  break;   
              }
              
              break;
            case release_content_sub_tasks::check_if_finished:
              current_command.command_type = peripherie_command_types::read_digital_inputs;

              uart::send(current_command);
              uart::state = communication_states::waiting_for_msg;
              break;
            case release_content_sub_tasks::reset_pusher_position:
              switch(release_content_sub_sub_task)
              {
                case release_content_sub_sub_tasks::set_output_A:
                  current_command.command_type = peripherie_command_types::output_on;
                  current_command.data1 = 1;

                  uart::send(current_command);
                  uart::state = communication_states::waiting_for_msg;
                  break;
                case release_content_sub_sub_tasks::reset_output_B:
                  current_command.command_type = peripherie_command_types::output_off;
                  current_command.data1 = 2;

                  uart::send(current_command);
                  uart::state = communication_states::waiting_for_msg;
                  break;     
              }              
              break;
            case release_content_sub_tasks::check_if_empty:
              current_command.command_type = peripherie_command_types::read_digital_inputs;

              uart::send(current_command);
              uart::state = communication_states::waiting_for_msg;
              break;        
            case release_content_sub_tasks::reset_output_A:
              current_command.command_type = peripherie_command_types::output_off;
              current_command.data1 = 1;

              uart::send(current_command);
              uart::state = communication_states::waiting_for_msg;
              break; 
          }
          break;
        
        case machine_tasks::check_if_emtpy:
          switch (check_if_empty_sub_task)
          {
            case check_if_empty_sub_tasks::set_output_A:
              current_command.command_type = peripherie_command_types::output_on;
              current_command.data1 = 1;

              uart::send(current_command);
              uart::state = communication_states::waiting_for_msg;
              break;
            case check_if_empty_sub_tasks::get_digital_state:
              current_command.command_type = peripherie_command_types::read_digital_inputs;

              uart::send(current_command);
              uart::state = communication_states::waiting_for_msg;
              break;
            case check_if_empty_sub_tasks::reset_output_A:
              current_command.command_type = peripherie_command_types::output_off;
              current_command.data1 = 1;

              uart::send(current_command);
              uart::state = communication_states::waiting_for_msg;
              break;
            default:
              break;
          }
          break;
        case machine_tasks::set_i2c_address:
          break;
        case machine_tasks::error:
          break;
        default:
          break;
      }
    }

    if(mqtt::state == communication_states::new_msg)
    {
      Serial.println("Mqtt message recieved");
      if(mqtt::message.hasPropertyEqual(JSON_KEY_ACKN, JSON_VAL_REQ))
      {
        if(mqtt::message.hasPropertyEqual(JSON_KEY_COMMAND, JSON_VAL_HEALTH_CHECK))
        {
          Serial.println("Task: health check");
          health_check_cnt = 1;
          mqtt::reply = JSON.parse("");
          mqtt::reply = nullptr;
          mqtt::reply = mqtt::message;
          mqtt::reply[JSON_KEY_FAILED_COUNT] = 0;
          mqtt::reply[JSON_KEY_ACKN] = JSON_VAL_ACKN;
          current_task = machine_tasks::health_check;
          mqtt::send_acknowledge();
        }
        else if(mqtt::message.hasPropertyEqual(JSON_KEY_COMMAND, JSON_VAL_SET_I2C_ADDRESS))
        {
          Serial.println("Task: set i2c address");
          if(mqtt::message.hasOwnProperty(JSON_KEY_TUBE_NR))
          {
            uint8_t tube_nr = (uint8_t) mqtt::message[JSON_KEY_TUBE_NR];

            if(tube_nr > 0 && tube_nr <= MAX_TUBES)
            {
              Serial.print("Tube-Nr.: ");
              Serial.println(std::to_string(tube_nr).c_str());
              current_command.address = I2C_CONFIG_ADDR;
              current_command.data1 = tube_nr + I2C_START_ADDR - 1; // -1 because tube_nr start with 1
              current_command.command_type = peripherie_command_types::set_i2c_address;

              uart::send(current_command);
              uart::state = communication_states::waiting_for_msg;
              current_task = machine_tasks::set_i2c_address; 
              mqtt::send_acknowledge();
            }
            else
            {
              Serial.print("Error: invalid tube number");
              mqtt::reply = JSON.parse("");
              mqtt::reply = nullptr;
              mqtt::reply = mqtt::message;
              mqtt::reply[JSON_KEY_ERROR] = JSON_VAL_TUBE_NR_WRONG;
              mqtt::reply[JSON_KEY_ACKN] = JSON_VAL_ACKN;
              mqtt::send_reply();
            }

          }
        }
        else if(mqtt::message.hasPropertyEqual(JSON_KEY_COMMAND, JSON_VAL_CHECK_IF_EMPTY))
        {
          Serial.println("Task: check if empty");
          if(mqtt::message.hasOwnProperty(JSON_KEY_TUBE_NR))
          {
            uint8_t tube_nr = (uint8_t) mqtt::message[JSON_KEY_TUBE_NR];

            if(tube_nr > 0 && tube_nr <= MAX_TUBES)
            {
              Serial.print("Tube-Nr.: ");
              Serial.println(std::to_string(tube_nr).c_str());
              current_command.address = tube_nr + I2C_START_ADDR - 1; // -1 because tube_nr start with 1
              current_task = machine_tasks::check_if_emtpy; 
              check_if_empty_sub_task = check_if_empty_sub_tasks::set_output_A;
              mqtt::send_acknowledge();
            }
            else
            {
              Serial.print("Error: invalid tube number");
              mqtt::reply = JSON.parse("");
              mqtt::reply = nullptr;
              mqtt::reply = mqtt::message;
              mqtt::reply[JSON_KEY_ERROR] = JSON_VAL_TUBE_NR_WRONG;
              mqtt::reply[JSON_KEY_ACKN] = JSON_VAL_ACKN;
              mqtt::send_reply();
            }
          }
        }
        else if(mqtt::message.hasPropertyEqual(JSON_KEY_COMMAND, JSON_VAL_RELEASE_CONTENT))
        {
          Serial.println("Task: release content");
          if(mqtt::message.hasOwnProperty(JSON_KEY_TUBE_NR))
          {
            uint8_t tube_nr = (uint8_t) mqtt::message[JSON_KEY_TUBE_NR];

            if(tube_nr > 0 && tube_nr <= MAX_TUBES)
            {
              Serial.print("Tube-Nr.: ");
              Serial.println(std::to_string(tube_nr).c_str());
              current_command.address = tube_nr + I2C_START_ADDR - 1; // -1 because tube_nr start with 1
              current_task = machine_tasks::release_content; 
              release_content_sub_task = release_content_sub_tasks::put_tube_in_standby;
              release_content_sub_sub_task = release_content_sub_sub_tasks::set_output_A;
              mqtt::send_acknowledge();
            }
            else
            {
              Serial.print("Error: invalid tube number");
              mqtt::reply = JSON.parse("");
              mqtt::reply = nullptr;
              mqtt::reply = mqtt::message;
              mqtt::reply[JSON_KEY_ERROR] = JSON_VAL_TUBE_NR_WRONG;
              mqtt::reply[JSON_KEY_ACKN] = JSON_VAL_ACKN;
              mqtt::send_reply();
            }
          }
        }
        else
        {
          Serial.print("Wrong command: ");
          Serial.println(mqtt::message[JSON_KEY_COMMAND]);
          mqtt::reply = JSON.parse("");
          mqtt::reply = nullptr;
          mqtt::reply = mqtt::message;
          mqtt::reply[JSON_KEY_ERROR] = JSON_VAL_COMMAND_ERR;
          mqtt::reply[JSON_KEY_ACKN] = JSON_VAL_ACKN;
          mqtt::send_reply();
        }
      }
      mqtt::state = communication_states::idle;
    }
    if(uart::state == communication_states::new_msg)
    {
      Serial.println("New uart message");

      switch(uart::reply.command_type)
      {
        case peripherie_command_types::check_communication:
        {
          Serial.println("Command: check communication");
          if(uart::reply.state != peripherie_states::ready)
          {
            mqtt::reply[JSON_KEY_FAILED_TUBES][(int) mqtt::reply[JSON_KEY_FAILED_COUNT]] = health_check_cnt;
            mqtt::reply[JSON_KEY_FAILED_COUNT] = ((int) mqtt::reply[JSON_KEY_FAILED_COUNT]) + 1;
          }
          health_check_cnt++;

          if(health_check_cnt > MAX_TUBES)
          {
            mqtt::send_reply();
            current_task = machine_tasks::idle;
          }

          break;
        }
        case peripherie_command_types::set_i2c_address:
        {
          Serial.println("Command: set i2c address");
          mqtt::reply = JSON.parse("");
          mqtt::reply = nullptr;
          mqtt::reply = mqtt::message;
          if(uart::reply.state == peripherie_states::command_understood)
            mqtt::reply[JSON_KEY_ERROR] = JSON_VAL_NO_ERROR;
          else
            mqtt::reply[JSON_KEY_ERROR] = JSON_VAL_UART_ERR;

          mqtt::reply[JSON_KEY_ACKN] = JSON_VAL_ACKN;
          mqtt::send_reply();
          break;
        }
        case peripherie_command_types::read_digital_inputs:
        {
          Serial.println("Command: digital read");
          if(uart::reply.state == peripherie_states::ready)
          {
            if(current_task == machine_tasks::check_if_emtpy)
            {              
              mqtt::reply = JSON.parse("");
              mqtt::reply = nullptr;
              mqtt::reply = mqtt::message;
              if(uart::reply.data1 == 1)
                mqtt::reply[JSON_KEY_ERROR] = JSON_VAL_TUBE_EMPTY;
              else
                mqtt::reply[JSON_KEY_ERROR] = JSON_VAL_NO_ERROR;

              mqtt::reply[JSON_KEY_ACKN] = JSON_VAL_ACKN;
              mqtt::send_reply();
              check_if_empty_sub_task = check_if_empty_sub_tasks::reset_output_A;
            }
            else if(current_task == machine_tasks::release_content)
            {
              if(release_content_sub_task == release_content_sub_tasks::check_if_ready)
              {
                if(uart::reply.data1 == 0)
                {
                  if(uart::reply.data2 == 0) // position reset still ongoing pause for 1 Second
                  {
                      timer::start(500);
                  }
                  else
                  {
                    release_content_sub_task = release_content_sub_tasks::eject_product;
                    release_content_sub_sub_task = release_content_sub_sub_tasks::reset_output_A;
                  }
                }
                else
                {
                  mqtt::reply = JSON.parse("");
                  mqtt::reply = nullptr;
                  mqtt::reply = mqtt::message;
                  mqtt::reply[JSON_KEY_ERROR] = JSON_VAL_TUBE_EMPTY;
                  mqtt::reply[JSON_KEY_RELEASE_SUCCESS] = false;
                  mqtt::reply[JSON_KEY_ACKN] = JSON_VAL_ACKN;

                  mqtt::send_reply();
                  release_content_sub_task = release_content_sub_tasks::reset_output_A;
                }
              }
              else if(release_content_sub_task == release_content_sub_tasks::check_if_finished)
              {
                  if(uart::reply.data2 == 0) // position reset still ongoing pause for 1 Second
                  {
                      timer::start(500);
                  }
                  else
                  {
                    release_content_sub_task = release_content_sub_tasks::reset_pusher_position;
                    release_content_sub_sub_task = release_content_sub_sub_tasks::reset_output_B;
                  }
              }
              else if(release_content_sub_task == release_content_sub_tasks::check_if_empty)
              {
                mqtt::reply = JSON.parse("");
                mqtt::reply = nullptr;
                mqtt::reply = mqtt::message;
                mqtt::reply[JSON_KEY_RELEASE_SUCCESS] = true;
                if(uart::reply.data1 == 1)
                  mqtt::reply[JSON_KEY_ERROR] = JSON_VAL_TUBE_EMPTY;
                else
                  mqtt::reply[JSON_KEY_ERROR] = JSON_VAL_NO_ERROR;

                mqtt::reply[JSON_KEY_ACKN] = JSON_VAL_ACKN;
                mqtt::send_reply();
                release_content_sub_task = release_content_sub_tasks::reset_output_A;
              }
            }
          }
          else
          {
            Serial.println("Error try to resend");
          }
          break;
        }
        case peripherie_command_types::output_on:
        {
          Serial.println("Command: output on");
          if(uart::reply.state == peripherie_states::ready)
          {
            if(current_task == machine_tasks::release_content)
            {
              switch(release_content_sub_task)
              {
                case release_content_sub_tasks::put_tube_in_standby:
                  if(release_content_sub_sub_task == release_content_sub_sub_tasks::set_output_A)
                  {
                    release_content_sub_task = release_content_sub_tasks::check_if_ready;
                  }
                  break;
                case release_content_sub_tasks::eject_product:
                  if(release_content_sub_sub_task == release_content_sub_sub_tasks::set_output_B)
                  {
                    release_content_sub_task = release_content_sub_tasks::check_if_finished;
                    timer::start(1000);
                  }
                  break;
                case release_content_sub_tasks::reset_pusher_position:
                  if(release_content_sub_sub_task == release_content_sub_sub_tasks::set_output_A)
                  {
                    release_content_sub_task = release_content_sub_tasks::check_if_empty;
                    timer::start(500);
                  }
                  break;
                default:
                  break;
              } 
            }
            else if(current_task == machine_tasks::check_if_emtpy)
            {
              check_if_empty_sub_task = check_if_empty_sub_tasks::get_digital_state;
            }
          }
          else
          {
            Serial.println("Error try to resend");
          }
          break;
        }
        case peripherie_command_types::output_off:
        {
          Serial.println("Command: output off");
          if(uart::reply.state == peripherie_states::ready)
          {
            if(current_task == machine_tasks::release_content)
            {
              switch(release_content_sub_task)
              {
                case release_content_sub_tasks::put_tube_in_standby:
                case release_content_sub_tasks::reset_pusher_position:
                  if(release_content_sub_sub_task == release_content_sub_sub_tasks::reset_output_B)
                  {
                    release_content_sub_sub_task = release_content_sub_sub_tasks::set_output_A;
                  }
                  break;
                case release_content_sub_tasks::eject_product:
                  if(release_content_sub_sub_task == release_content_sub_sub_tasks::reset_output_A)
                  {
                    release_content_sub_sub_task = release_content_sub_sub_tasks::set_output_B;
                  }
                  break;
                case release_content_sub_tasks::reset_output_A:
                  release_content_sub_sub_task = release_content_sub_sub_tasks::none;
                  release_content_sub_task = release_content_sub_tasks::none;
                  current_task = machine_tasks::idle;
                default:
                  break;
              }
            }
            else if(current_task == machine_tasks::check_if_emtpy)
            {
              check_if_empty_sub_task = check_if_empty_sub_tasks::none;
              current_task = machine_tasks::idle;
            }
          }
          else
          {
            Serial.println("Error try to resend");
          }
          break;
        }
        default:
          Serial.println("No command found");
          switch(current_task)
          {
            case machine_tasks::health_check:
            {
              mqtt::reply[JSON_KEY_FAILED_TUBES][(int) mqtt::reply[JSON_KEY_FAILED_COUNT]] = health_check_cnt;
              mqtt::reply[JSON_KEY_FAILED_COUNT] = ((int) mqtt::reply[JSON_KEY_FAILED_COUNT]) + 1;           
              health_check_cnt++;

              if(health_check_cnt > MAX_TUBES)
              {
                mqtt::send_reply();
                current_task = machine_tasks::idle;
              }
              break;
            }
          }
          break;
      }
      uart::state = communication_states::idle;
    }
    else if(uart::state == communication_states::error)
    {
      uart::state = communication_states::idle;
      mqtt::state = communication_states::waiting_for_msg;
    }*/
}
