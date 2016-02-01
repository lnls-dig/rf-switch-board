#include "mbed.h"
#include "EthernetInterface.h"
#include "lpc_phy.h"

#define BUFSIZE 140
#define SERVER_PORT 6791

#define DP8_SPEED10MBPS    (1 << 1)   /**< 1=10MBps speed */
#define DP8_VALID_LINK     (1 << 0)   /**< 1=Link active */

bool get_eth_link_status(void)
{
    return (lpc_mii_read_data() & DP8_VALID_LINK) ? true : false;
}

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
    //float temp;

    S1_C1 = 0;
    S1_C2 = 0;
    S2_C1 = 0;
    S2_C2 = 0;


    // Ethernet initialization

#if defined(ETH_DHCP)
    EthernetInterface::init(); //Use DHCP
#else
#if defined(ETH_FIXIP)
    EthernetInterface::init(ETH_IP,ETH_MASK,ETH_GATEWAY); //Use  these parameters for static IP
#endif
#endif

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
        if (!get_eth_link_status()) {
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
