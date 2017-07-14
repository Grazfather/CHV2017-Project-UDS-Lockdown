#include <avr/wdt.h>
#include <SPI.h>
#include <EEPROM.h>
#include <CANBus.h>
#include <MessageQueue.h>

CANBus busses[] = {
    CANBus(CAN1SELECT, CAN1RESET, 1, "Bus 1"),
    CANBus(CAN2SELECT, CAN2RESET, 2, "Bus 2"),
    CANBus(CAN3SELECT, CAN3RESET, 3, "Bus 3")
};

extern "C" {
#include <isotp.h>
#include <isotp_posix.h>

#include "tp_link_canbustriple.h"

void println(char*s) {
	Serial.println(s);
}
}

struct tp_link_handle send_link;
struct tp_link_handle recv_link;
struct isotp_handle send_isotp_h;
struct isotp_handle recv_isotp_h;

char sendbuf[20] = {0};
char recvbuf[20] = {0};

void setup()
{
    delay(1);

    Serial.begin(115200); // USB
    Serial1.begin(57600); // UART

    /*
    *  Power LED
    */
    DDRE |= B00000100;
    PORTE |= B00000100;

    /*
    *  BLE112 Init
    */
    pinMode(BT_SLEEP, OUTPUT);
    digitalWrite(BT_SLEEP, HIGH); // Keep BLE112 Awake

    /*
    *  Boot LED
    */
    pinMode(BOOT_LED, OUTPUT);

    pinMode(CAN1INT_D, INPUT);
    pinMode(CAN1RESET, OUTPUT);
    pinMode(CAN1SELECT, OUTPUT);

    digitalWrite(CAN1RESET, LOW);


    // Init the sending node
    // -- Init tp link layer
    struct tp_link_config link_cfg = {
        .busId = 1,
        .tx_id = 40,
        .rx_id = 20,
    };
    tp_link_handle_init(&send_link, &link_cfg);

    // -- Init iso-tp layer
    struct isotp_config isotp_cfg = {
        .link = &send_link, .use_padding = true, .padding_byte = 0x55,
    };
    isotp_init(&send_isotp_h, &isotp_cfg);

    // Init the receiving node
    // -- Init tp link layer
    link_cfg.busId = 1; // TODO: Can we re-use the same tranceiver like this?
    link_cfg.tx_id = 20;
    link_cfg.rx_id = 40;
    tp_link_handle_init(&recv_link, &link_cfg);

    // -- Init iso-tp layer
    isotp_cfg.link = &recv_link;
    isotp_init(&recv_isotp_h, &isotp_cfg);

    ((long int*)sendbuf)[0] = 0x41414141;
    ((long int*)sendbuf)[1] = 0x42424242;
    ((long int*)sendbuf)[2] = 0x43434343;
    ((long int*)sendbuf)[3] = 0x44444444;
    ((long int*)sendbuf)[4] = 0x45454545;
}


/*
*  Main Loop
*/

void loop()
{
    int len = 20;
    int sent = 0;
    int recvd = 0;
    memset(recvbuf, 0, 20);


    if ((sent = isotp_send(&send_isotp_h, (char*)sendbuf, (size_t)len)) != len)
    {
	 Serial.print("Send failed!: ");
	 Serial.println(sent);
    }
    Serial.print("Sent ");
    Serial.print(sent);
    Serial.println(" bytes");

    delay(5000);
    ((long int*)sendbuf)[0] += 1;
    ((long int*)sendbuf)[5] += 1;
} // End loop()


