
#include <Arduino.h>
#include <HardwareSerial.h>
#include <SPI.h>
#include <Ethernet.h>
#include <MQTT.h>
#include <memory>
 
#include "defines.h"
#include "ethernet.h"
#include "mqtt.h"
#include "uart.h"
#include "timer.h"
#include "command.h"

std::vector<Command> v_commands;

void setup() 
{
    Serial.begin(115200);
    uart::init();
    delay(1000);
    ethernet::init();
    mqtt::init(&ethernet::client, MQTT_BROKER);
    delay(5000);


}
 
void loop() 
{
  if(!v_commands.empty())
  {
    // --------- Command --------
    if(v_commands.front().check_if_all_tasks_finished())
    {
      v_commands.front().send_reply();
      v_commands.erase(v_commands.begin());
    }

    // --------- UART ------------
    if(uart::state == communication_states::idle)
    {
      v_commands.front().send_next_uart_msg();
    }
    else if((uart::state == communication_states::waiting_for_msg && uart::timeout_reached()) || uart::state == communication_states::error)
    {
      v_commands.front().set_uart_recieve_flag(uart::cur_sender, true);
    }
    else if(uart::state == communication_states::new_msg)
    {
      v_commands.front().set_uart_recieve_flag(uart::cur_sender, false);
    }
  }
  
  // --------- MQTT ------------
  mqtt::loop();
  if(mqtt::state == communication_states::new_msg)
  {
    Serial.println("Mqtt message recieved");
    if(mqtt::message.hasPropertyEqual(JSON_KEY_ACKN, JSON_VAL_REQ))
    {
      if(mqtt::message.hasPropertyEqual(JSON_KEY_TOPIC, JSON_VAL_SET_I2C_ADDRESS))
      {
        Serial.println("Command: set I2C address");
        if(mqtt::message.hasOwnProperty(JSON_KEY_DATA))
        {          
          Command new_command = Command(modul_command_types::set_i2c_address, mqtt::message);
          v_commands.push_back(new_command);
        }
        else
        {
          Serial.print("Command Error: ");
          Serial.println(mqtt::message[JSON_KEY_TOPIC]);
          mqtt::reply = JSON.parse("");
          mqtt::reply = nullptr;
          mqtt::reply = mqtt::message;
          mqtt::reply[JSON_KEY_ERROR] = JSON_VAL_COMMAND_ERR;
          mqtt::reply[JSON_KEY_ACKN] = JSON_VAL_ACKN;
          mqtt::send_reply();
        }
      }
      else if(mqtt::message.hasPropertyEqual(JSON_KEY_TOPIC, JSON_VAL_HEALTH_CHECK))
      {
        Serial.println("Command: health check");

        Command new_command = Command(modul_command_types::health_check, mqtt::message);
        v_commands.push_back(new_command);
      }
      else if(mqtt::message.hasPropertyEqual(JSON_KEY_TOPIC, JSON_VAL_CHECK_IF_EMPTY))
      {
        Serial.println("Command: check if empty");

        Command new_command = Command(modul_command_types::check_if_emtpy, mqtt::message);
        v_commands.push_back(new_command);
      }
      else if(mqtt::message.hasPropertyEqual(JSON_KEY_TOPIC, JSON_VAL_RELEASE_CONTENT))
      {
        Serial.println("Command: release content");
        if(mqtt::message.hasOwnProperty(JSON_KEY_DATA))
        {
          Command new_command = Command(modul_command_types::release_content, mqtt::message);
          v_commands.push_back(new_command);
        }
        else
        {
          Serial.print("Command Error: ");
          Serial.println(mqtt::message[JSON_KEY_TOPIC]);
          mqtt::reply = JSON.parse("");
          mqtt::reply = nullptr;
          mqtt::reply = mqtt::message;
          mqtt::reply[JSON_KEY_ERROR] = JSON_VAL_COMMAND_ERR;
          mqtt::reply[JSON_KEY_ACKN] = JSON_VAL_ACKN;
          mqtt::send_reply();
        }
      }
      else
      {
        Serial.print("Wrong command: ");
        Serial.println(mqtt::message[JSON_KEY_TOPIC]);
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
  else
  {
    mqtt::check_buffer();
  }
}
