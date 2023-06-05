#ifndef ETHERNET_HEADER
#define ETHERNET_HEADER

#include <Arduino.h>
#include <SPI.h>
#include <Ethernet.h>
#include "defines.h"

namespace ethernet
{
    extern byte mac[];
    extern EthernetClient client;

    void init();
}

#endif

