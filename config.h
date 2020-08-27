#define config
#include <lmic.h>
#include <hal/hal.h>
#include <string.h>

#define CHANNEL 8

SSD1306 display(OLED_I2C_ADDR, OLED_SDA, OLED_SCL);

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in arduino-lmic/project_config/lmic_project_config.h,
// otherwise the linker will complain)
void os_getArtEui(u1_t *buf) {}
void os_getDevEui(u1_t *buf) {}
void os_getDevKey(u1_t *buf) {}

//Aquí se guarda el tamaño de la ultima respuesta, cada loop se restablece a 0
int dataLength = 0;
//Contador de envíos
uint32_t count = 0;
//array para respuesta
char TTN_response[30];


// LoRaWAN NwkSKey, network session key
// This should be in big-endian (aka msb).
//static const PROGMEM u1_t NWKSKEY[16] = {0x55, 0xe5, 0x4c, 0x76, 0x97, 0xcb, 0x14, 0x9e, 0x03, 0xe9, 0xbe, 0x94, 0xce, 0xe9, 0xec, 0xa7};
static const u1_t NWKSKEY[16] = {0x55, 0xe5, 0x4c, 0x76, 0x97, 0xcb, 0x14, 0x9e, 0x03, 0xe9, 0xbe, 0x94, 0xce, 0xe9, 0xec, 0xa7};
//55e54c7697cb149e03e9be94cee9eca7
// LoRaWAN AppSKey, application session key
// This should also be in big-endian (aka msb).
static const u1_t APPSKEY[16] = {0xca, 0xc8, 0x58, 0x39, 0xf9, 0x83, 0x7f, 0x75, 0x33, 0xa7, 0x8c, 0x14, 0x2f, 0xb3, 0xe5, 0xe7};
//ca c8 58 39 f9 837f7533a78c142fb3e5e7

// LoRaWAN end-device address (DevAddr)
// See http://thethingsnetwork.org/wiki/AddressSpace
// The library converts the address to network byte order as needed, so this should be in big-endian (aka msb) too.
static const u4_t DEVADDR = 0x260211DB;
//0x2602142D // <-- Change this address for every node!