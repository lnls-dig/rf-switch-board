#include "mbed.h"
#include "EthernetInterface.h"

#define BUFSIZE 140
#define SERVER_PORT 6791

// MBED Leds
DigitalOut led1(LED1);
DigitalOut led2(LED2);
DigitalOut led3(LED3);
DigitalOut led4(LED4);

DigitalOut S1_C1(p17);
DigitalOut S1_C2(p16);
DigitalOut S2_C1(p14);
DigitalOut S2_C2(p13);

// Analog in to read temp
AnalogIn   ain(p20);

DigitalOut LedY(p30); // Yellow led of the Ethernet connector. LVTTLIndicate transmiting data
DigitalOut LedG(p29); // Green led of the Ethernet connector. LVTTLIndicate active connection

int main(void)
{
    char buf[BUFSIZE];
    int s;
    Ethernet *cable = new Ethernet;
    float temp;

    S1_C1 = 0;
    S1_C2 = 0;
    S2_C1 = 0;
    S2_C2 = 0;
    

    // Ethernet initialization

    //EthernetInterface eth;
    static const char* mbedIp       = "10.0.18.119" ;  //IP
    static const char* mbedMask     = "255.255.255.0";  // Mask
    static const char* mbedGateway  = "10.0.18.1";    //Gateway
    EthernetInterface::init(mbedIp,mbedMask,mbedGateway); //Use  these parameters for static IP
    //EthernetInterface::init(); //Use DHCP

    TCPSocketConnection client;
    TCPSocketServer server;

    printf("\n Trying to establish connection...\n");
    while (EthernetInterface::connect(5000)) {
        printf("\n Connection failure\n");
    }
    printf("\nConnected\n");

    while (true) {

        server.bind(SERVER_PORT);
        server.listen();

        printf("    IP Address is %s\n", EthernetInterface::getIPAddress());
        printf("    MAC Address is %s\n", EthernetInterface::getMACAddress());
        printf("    port %d\n", SERVER_PORT);
        printf("\n Wait for new client connection...\n");
        server.accept(client);
        client.set_blocking(false, 1500); // Timeout after (1.5)s

        printf("Connection from client: %s\n", client.get_address());
        printf("port: %d\n", client.get_port());
        while (client.is_connected()) {
            s = client.receive(buf, BUFSIZE);
            Thread::wait(10);
            if (s <= 0) {
                LedG = 0;
                continue;
            }
            LedG = 1;
            if (buf[0] == ':' && buf[1] == 'S' && buf[2] == 'C' && buf[3] == '1' && buf[4] == '0' && buf[5] == '!')
            {
                S1_C1 = 0; S1_C2 = 0; led1=0; led2=0;
            }
            else if (buf[0] == ':' && buf[1] == 'S' && buf[2] == 'C' && buf[3] == '1' && buf[4] == '1' && buf[5] == '!')
                {S1_C1 = 1; S1_C2 = 0; led1=1; led2=0;}
            else if (buf[0] == ':' && buf[1] == 'S' && buf[2] == 'C' && buf[3] == '1' && buf[4] == '2' && buf[5] == '!')
                {S1_C1 = 0; S1_C2 = 1; led1=0; led2=1;}

            if (buf[0] == ':' && buf[1] == 'S' && buf[2] == 'C' && buf[3] == '2' && buf[4] == '0' && buf[5] == '!')
            {
                S2_C1 = 0; S2_C2 = 0; led3=0; led4=0;
            }
            else if (buf[0] == ':' && buf[1] == 'S' && buf[2] == 'C' && buf[3] == '2' && buf[4] == '1' && buf[5] == '!')
                {S2_C1 = 1; S2_C2 = 0; led3=1; led4=0;}
            else if (buf[0] == ':' && buf[1] == 'S' && buf[2] == 'C' && buf[3] == '2' && buf[4] == '2' && buf[5] == '!')
                {S2_C1 = 0; S2_C2 = 1; led3=0; led4=1;}


             

 
            /*
            printf("\nMsg enviada de %d bytes: ",s);
            for (int i = 0; i < s; i++)
                printf("%x ",(char*)response.data[i]);
            printf("\n");
            */

            if (s < 0)
                error("ERROR writing to socket");

            //echo_command((char*)buf);

        }
        //printf("\nDisconnected: %d\n",eth.disconnect());
        client.close();
        server.close();
        if (!cable->link()) {
            EthernetInterface::disconnect();
            printf("\n Trying to establish connection...\n");
            while (EthernetInterface::connect(5000)) {
                printf("\n Connection failure\n");
            }
            printf("\nConnected\n");
        }
        printf("\nClient disconnected\n");
    }

}
