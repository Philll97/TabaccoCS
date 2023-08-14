#ifndef MQTT_HEADER
#define MQTT_HEADER


#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include <MQTT.h>
#include <vector>
#include "defines.h"
#include "timer.h"

namespace mqtt
{
    struct BufferElement
    {
        JSONVar request;
        long timestamp;
    };

    extern MQTTClient client;
    extern communication_states state;
    extern JSONVar message;
    extern JSONVar reply;

    void init(EthernetClient* eth_client, String broker_address);
    void loop();
    /* void send(machine_reply_empty reply);
    void send(machine_reply_health_check reply);
    void send(machine_reply_release_content reply);
    void send(machine_reply_set_i2c_address reply); */
    void send_reply();
    void send_msg(JSONVar msg);
    void send_acknowledge();

    void check_buffer();
}

#endif