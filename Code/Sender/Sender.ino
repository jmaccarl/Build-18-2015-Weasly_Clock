#include <XBee.h>

XBee xbee = XBee()

uint_8 payload[] = { 'a'};

XBeeAddress64 addr64 = XBeeAddress64(0x0013a200, 0x403e0f30);

Tx64Request tx64 = Tx64Request(addr64, payload, sizeof(payload));

void setup()
{
    xbee.begin(9600);
}

void loop()
{
    xbee.send(tx64);
    delay(50);
}
