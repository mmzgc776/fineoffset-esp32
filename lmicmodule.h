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
#include <Arduino.h>
#include <SPI.h>
#include <lmic.h>
#include <hal/hal.h>
#include <stdio.h>
#include <string.h>


#include <SSD1306.h>
#ifndef config
#include "config.h"
#endif
//Led para envío Azul
#define LEDPIN 2

#define LORA_SS 18

//#include <SSD1306.h>

//#define OLED_I2C_ADDR 0x3C
//#define OLED_RESET 16
//#define OLED_SDA 4
//#define OLED_SCL 15


class MyLMIC
{
public:
    MyLMIC();
};

// Schedule TX every this many seconds (might become longer due to duty
// cycle limitations).
const unsigned TX_INTERVAL = 90;

static osjob_t sendjob;

void loraSetup(int readcount);
bool loraLoop();
void forceTxSingleChannelDr(int channel);
//int loraSend(byte payload[]);
void onEvent(ev_t ev);
void do_send(osjob_t *j);


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