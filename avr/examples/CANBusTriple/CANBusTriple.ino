#include <avr/wdt.h>
#include <SPI.h>
#include <EEPROM.h>
#include <CANBus.h>
#include <MessageQueue.h>

CANBus busses[] = {
    CANBus(CAN1SELECT, CAN1RESET, 1, "Bus 1"),
};

extern "C" {
#include <isotp.h>
#include <isotp_posix.h>
#include <uds_hook.h>
#include <uds_security.h>
#include <uds_server.h>

void println(char*s) { Serial.println(s); Serial.flush(); }
void hexd(char*s, int l)
{
	for (int i = 0; i < l; i++)
		Serial.print(s[i] & 0xFF, HEX);
	Serial.print("\n");
}

#include "tp_link_canbustriple.h"
#include "my_uds_server.h"

}

struct tp_link_handle can_link;
struct isotp_handle send_isotp_h;

void setup()
{
    Serial.begin(115200); // USB

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

    // Get some entropy
    // TODO: Get better entropy, this sucks
    randomSeed(analogRead(0));

    // Init tp link layer
    struct tp_link_config link_cfg = {
        .busId = 1,
        .tx_id = 20,
        .rx_id = 40,
    };
    tp_link_handle_init(&can_link, &link_cfg);

    // Init iso-tp layer
    struct isotp_config isotp_cfg = {
        .link = &can_link,
	.use_padding = true,
	.padding_byte = 0x55,
    };
    isotp_cfg.separation_time = 10,
    isotp_cfg.timeout_ms = 100;
    isotp_init(&send_isotp_h, &isotp_cfg);

    // Init uds server
    uds_server_init(&isotp_cfg);
}


/*
*  Main Loop
*/

void loop()
{
    uds_server_run();
}
