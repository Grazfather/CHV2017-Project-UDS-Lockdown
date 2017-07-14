#include <CANBus.h>
#include <MessageQueue.h>

#include <tp_link.h>

#define READ_BUFFER_SIZE 20
#define WRITE_BUFFER_SIZE 20
Message readBuffer[READ_BUFFER_SIZE];
Message writeBuffer[WRITE_BUFFER_SIZE];
MessageQueue readQueue(READ_BUFFER_SIZE, readBuffer);
MessageQueue writeQueue(WRITE_BUFFER_SIZE, writeBuffer);


/*
*  Load CAN Controller buffer and set send flag
*/
bool sendMessage(Message msg, CANBus * bus)
{
    int ch = bus->getNextTxBuffer();
    if (ch < 0 || ch > 2) return false; // All TX buffers full

    digitalWrite(BOOT_LED, HIGH);
    bus->loadFullFrame(ch, msg.length, msg.frame_id, msg.frame_data );
    bus->transmitBuffer(ch);
    digitalWrite(BOOT_LED, LOW );

    return true;
}

bool readMsgFromBuffer(CANBus * bus, byte bufferId, byte rx_status)
{
    if (readQueue.isFull()) return false;
    Message msg;
    msg.busStatus = rx_status;
    msg.busId = bus->busId;
    msg.dispatch = false;
    bus->readFullFrame(bufferId, &msg.length, msg.frame_data, &msg.frame_id );
    return readQueue.push(msg);
}


/*
*  Read Can Controller Buffer
*/
void readBus(CANBus * bus)
{
    byte rx_status = 0x3;
    bool bufferAvailable = true;
    while((rx_status & 0x3) && bufferAvailable) {
        rx_status = bus->readStatus();
        if (rx_status & 0x1)
            bufferAvailable = readMsgFromBuffer(bus, 0, rx_status);
        if ((rx_status & 0x2) && bufferAvailable)
            bufferAvailable = readMsgFromBuffer(bus, 1, rx_status);
    }
}


struct tp_link_config
{
    unsigned int busId;
    unsigned short tx_id;
    unsigned short rx_id;
};

struct tp_link_handle
{
    CANBus *bus;
    unsigned short tx_id;
    unsigned short rx_id;
};


// TODO: Move to a C file
bool tp_link_handle_init(struct tp_link_handle *handle,
                         const struct tp_link_config *cfg)
{
    CANBus *bus = &busses[cfg->busId - 1];

    bus->begin();
    bus->setClkPre(1);
    bus->baudConfig(500);
    bus->setRxInt(true); // TODO: Are we sure?
    bus->bitModify(RXB0CTRL, 0x04, 0x04); // Set buffer rollover enabled
    bus->disableFilters(); // TODO: Enable filtering on tx/rd id
    bus->setMode(NORMAL);

    handle->bus = bus;
    handle->tx_id = cfg->tx_id;
    handle->rx_id = cfg->rx_id;
}

/*
 * Send a frame with the given link handle. Returns true on success.
 */
bool tp_link_send(struct tp_link_handle *handle,
                  const struct tp_link_frame *frame)
{
    Message msg;
    msg.length = frame->len;
    msg.frame_id = handle->tx_id;
    memcpy(msg.frame_data, frame->data, msg.length);

    sendMessage(msg, handle->bus);
    return true;
}

/*
 * Receive a frame with the given link handle. Returns true on success.
 */
bool tp_link_recv(struct tp_link_handle *handle, struct tp_link_frame *frame)
{
    //readBus(handle->bus);
    //if (readQueue.isEmpty()) {
        //return false;
    //}

    // Gross :( Keep trying for 100ms
    int tries = 0;
    while (readQueue.isEmpty() || tries < 10) {
            readBus(handle->bus);
            delay(10);
            tries++;
    }

    Message msg = readQueue.pop();
    memcpy(frame->data, msg.frame_data, TP_LINK_MAX_LEN);
    return true;
}

/*
 * Get the current time in milliseconds, with respect to any reference.
 */
uint32_t tp_get_time_ms(void)
{
    return millis();
}

