
#include "ethernet.h"

byte ethernet::mac[] = {ETH_MACADDR};
EthernetClient ethernet::client;

void ethernet::init()
{
    Serial.println("Begin Ethernet");

    Ethernet.init(5);
 
    if (Ethernet.begin(ethernet::mac)) // Dynamic IP setup
    { 
        Serial.println("DHCP OK!");
    }
    else
    {
        Serial.println("Failed to configure Ethernet using DHCP");
        // Check for Ethernet hardware present
        if (Ethernet.hardwareStatus() == EthernetNoHardware) 
        {
          Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
          while (true) 
          {
            delay(1); // do nothing, no point running without Ethernet hardware
          }
        }
        if (Ethernet.linkStatus() == LinkOFF) 
        {
          Serial.println("Ethernet cable is not connected.");
        }
 
        IPAddress ip(ETH_IPADDR);
        IPAddress dns(ETH_DNS);
        IPAddress gw(ETH_GW);
        IPAddress sn(ETH_IPMASK);
        Ethernet.begin(mac, ip, dns, gw, sn);
        Serial.println("STATIC OK!");
    }

    delay(5000); 
 
    Serial.print("Local IP : ");
    Serial.println(Ethernet.localIP());
    Serial.print("Subnet Mask : ");
    Serial.println(Ethernet.subnetMask());
    Serial.print("Gateway IP : ");
    Serial.println(Ethernet.gatewayIP());
    Serial.print("DNS Server : ");
    Serial.println(Ethernet.dnsServerIP());
 
    Serial.println("Ethernet Successfully Initialized");
}