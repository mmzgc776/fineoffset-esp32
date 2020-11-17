/*******************************************************************************
 * Copyright (c) 2015 Thomas Telkamp and Matthijs Kooijman
 * Copyright (c) 2018 Terry Moore, MCCI
 *
 * Permission is hereby granted, free of charge, to anyone
 * obtaining a copy of this document and accompanying files,
 * to do whatever they want with them without any restriction,
 * including, but not limited to, copying, modification and redistribution.
 * NO WARRANTY OF ANY KIND IS PROVIDED.
 *
 * This example sends a valid LoRaWAN packet with payload "Hello,
 * world!", using frequency and encryption settings matching those of
 * the The Things Network.
 *
 * This uses ABP (Activation-by-personalisation), where a DevAddr and
 * Session keys are preconfigured (unlike OTAA, where a DevEUI and
 * application key is configured, while the DevAddr and session keys are
 * assigned/generated in the over-the-air-activation procedure).
 *
 * Note: LoRaWAN per sub-band duty-cycle limitation is enforced (1% in
 * g1, 0.1% in g2), but not the TTN fair usage policy (which is probably
 * violated by this sketch when left running for longer)!
 *
 * To use this sketch, first register your application and device with
 * the things network, to set or generate a DevAddr, NwkSKey and
 * AppSKey. Each device should have their own unique values for these
 * fields.
 *
 * Do not forget to define the radio type correctly in
 * arduino-lmic/project_config/lmic_project_config.h or from your BOARDS.txt.
 *
 *******************************************************************************/

// References:
// [feather] adafruit-feather-m0-radio-with-lora-module.pdf

//Includes de comunicacion
#ifndef lmich
#define lmich
#endif
#include <Arduino.h>
#include <SPI.h>
#include <lmic.h>
#include <hal/hal.h>
#include <stdio.h>
#include <string.h>
#include <CayenneLPP.h>

//Led para envío Azul
#define LEDPIN 2

#define LORA_SS 18

#define CHANNEL 8

// typedef struct {
//     //uint8_t en vez de float
//     float delay;
//     float inTemp;
//     float inHum;
//     float pressure;
//     uint16_t readcount;
//     uint16_t mempos;

// } payload;


class MyLMIC
{
public:
    MyLMIC();

    int Correccion = -1;
    uint16_t ReadCount = 0;
    void loraSetup(int readcount);
    bool loraLoop();    
    void do_send(osjob_t *j, CayenneLPP payload);
    //int loraSend(byte payload[]);
};

//Aquí se guarda el tamaño de la ultima respuesta, cada loop se restablece a 0
static int dataLength = 0;

//array para respuesta
static char TTN_response[30];

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

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 90;
void onEvent(ev_t ev);
static osjob_t sendjob;
//void do_send(osjob_t *j);

void forceTxSingleChannelDr(int channel);

//Multicore para intentar garantizar el tiempo real para el envío
//TaskHandle_t Task1;

// void Task1code( void * pvParameters ){
//    //os_runloop_once();
//   for(;;){
//     Serial.print("Task1 running on core ");
//     Serial.println(xPortGetCoreID());
//     delay(15000);
//   }
// }

