#include "mqtt.h"
#include <cstdlib>

MQTTClient mqtt::client(1024);
communication_states mqtt::state;
JSONVar mqtt::message;
JSONVar mqtt::reply;
std::vector<mqtt::BufferElement> msg_buffer;

void recieved(String &topic, String &payload)
{
    if(mqtt::state == communication_states::idle)
    {
        Serial.println("incoming: " + topic + " - " + payload);
        JSONVar msg = JSON.parse(payload);

        // JSON.typeof(jsonVar) can be used to get the type of the variable
        if (JSON.typeof(msg) == "undefined") 
        {
            Serial.println("Parsing input failed!");
            return;
        }
        if((uint8_t) msg[JSON_KEY_DEVICE_NR] != DEVICE_NR || (uint8_t) msg[JSON_KEY_DEVICE_TYPE] != DEVICE_TYPE)
        {
            Serial.println("Ignore Message: Not for this machine");
            return;
        }
        if(msg.hasPropertyEqual(JSON_KEY_ACKN, JSON_VAL_ACKN) && msg.hasOwnProperty(JSON_KEY_MESSAGE_ID))
        {
            Serial.println("Acknowledgement");
            int msg_id = (int) msg[JSON_KEY_MESSAGE_ID];
            auto it = std::find_if(msg_buffer.begin(), msg_buffer.end(), [msg_id](mqtt::BufferElement &obj) 
                {
                    return (int) obj.request[JSON_KEY_MESSAGE_ID] == msg_id;
                });

            if (it != msg_buffer.end())
            {
                msg_buffer.erase(it);
            }
            return;
        }
        mqtt::message = nullptr;
        mqtt::message = msg;
        mqtt::state = communication_states::new_msg;
    }
}


void mqtt::init(EthernetClient* eth_client, String broker_address)
{
    client.begin(broker_address.c_str(), *eth_client);
    client.onMessage(recieved);
    Serial.print("\nconnecting...");
    while (!client.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) 
    {
        Serial.print(".");
        delay(1000);
    }

    Serial.println("\nconnected!");
    mqtt::client.subscribe(MQTT_SUB_TOPIC);
    state = communication_states::idle;
}

void reconnect()
{
    while (!mqtt::client.connect(MQTT_CLIENT_ID, MQTT_USERNAME, MQTT_PASSWORD)) 
    {
        delay(10);
    }
    Serial.println("\nreconnected!");

    mqtt::client.subscribe(MQTT_SUB_TOPIC);
}

void mqtt::loop()
{
    client.loop();
    if (!client.connected()) 
    {
        reconnect();
    }
}

void mqtt::send_reply()
{
    client.publish(MQTT_PUB_TOPIC, JSON.stringify(reply).c_str());
}

void mqtt::send_msg(JSONVar msg)
{
    msg[JSON_KEY_SENDER] = MQTT_SUB_TOPIC;
    msg[JSON_KEY_RECEIVER] = MQTT_PUB_TOPIC;
    msg[JSON_KEY_MESSAGE_ID] = rand() * -1;
    msg[JSON_KEY_DEVICE_NR] = DEVICE_NR;
    msg[JSON_KEY_DEVICE_TYPE] = DEVICE_TYPE;
    client.publish(MQTT_PUB_TOPIC, JSON.stringify(msg).c_str());
    BufferElement new_req;
    new_req.request = msg;
    new_req.timestamp = millis();
    msg_buffer.push_back(new_req);
}

void mqtt::send_acknowledge()
{
    JSONVar ackn = message;
    ackn[JSON_KEY_SENDER] = (const char*) message[JSON_KEY_RECEIVER];
    ackn[JSON_KEY_RECEIVER] = (const char*) message[JSON_KEY_SENDER];
    ackn[JSON_KEY_ACKN] = JSON_VAL_ACKN;
    client.publish(MQTT_PUB_TOPIC, JSON.stringify(ackn).c_str());
}

void mqtt::check_buffer()
{
    while(1)
    {
        auto it = std::find_if(msg_buffer.begin(), msg_buffer.end(), [](mqtt::BufferElement &obj)
        {
            return obj.timestamp + MQTT_REQ_TIMEOUT * 1000 < millis();
        });

        if(it != msg_buffer.end())
        {
            msg_buffer.erase(it);
            //! TODO error handling timeout
        }
        else
        {
            return;
        }

    }
}
