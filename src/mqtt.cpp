#include "mqtt.h"


MQTTClient mqtt::client;
communication_states mqtt::state;
JSONVar mqtt::message;
JSONVar mqtt::reply;

void recieved(String &topic, String &payload)
{
    if(mqtt::state == communication_states::idle)
    {
        Serial.println("incoming: " + topic + " - " + payload);
        JSONVar msg = JSON.parse(payload);

        // JSON.typeof(jsonVar) can be used to get the type of the variable
        if (JSON.typeof(msg) == "undefined") {
            Serial.println("Parsing input failed!");
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
    while (!client.connect("arduino", "public", "public")) 
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
    Serial.println("\nreconnected!");
    while (!mqtt::client.connect("arduino", "public", "public")) 
    {
        delay(10);
    }

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

void mqtt::send_acknowledge()
{
    message[JSON_KEY_ACKN] = JSON_VAL_ACKN;
    client.publish(MQTT_PUB_TOPIC, JSON.stringify(message).c_str());
}
