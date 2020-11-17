//Includes de comunicacion
// #include <Arduino.h>
// #include <SPI.h>
#ifndef lmich
#include "lmicmodule.h"
#endif
#include <lmic.h>
#include <hal/hal.h>


MyLMIC::MyLMIC() {}

bool finished = false;

//LORA_SS viene del ino
const lmic_pinmap lmic_pins = {
    .nss = LORA_SS,
    .rxtx = LMIC_UNUSED_PIN,
    .rst = 14,
    .dio = {26, 33, 32}};

// These callbacks are only used in over-the-air activation, so they are
// left empty here (we cannot leave them out completely unless
// DISABLE_JOIN is set in arduino-lmic/project_config/lmic_project_config.h,
// otherwise the linker will complain)
void os_getArtEui(u1_t *buf) {}
void os_getDevEui(u1_t *buf) {}
void os_getDevKey(u1_t *buf) {}

//Force Single Channel
//Segmento copiado del foro de ttn, solo para pruebas, ya que viola el estandar
// Define the single channel and data rate (SF) to use

// Disables all channels, except for the one defined above, and sets the
// data rate (SF). This only affects uplinks; for downlinks the default
// channels or the configuration from the OTAA Join Accept are used.
//
// Not LoRaWAN compliant; FOR TESTING ONLY!

void forceTxSingleChannelDr(int channel)
{
    for (int i = 0; i < 71; i++)
    { // For EU; for US use i<71
        if (i != channel)
        {
            LMIC_disableChannel(i);
        }
    }
}

void onEvent(ev_t ev)
{   
    Serial.print(os_getTime());
    Serial.print(": ");
    switch (ev)
    {
    case EV_SCAN_TIMEOUT:
        Serial.println(F("EV_SCAN_TIMEOUT"));
        break;
    case EV_BEACON_FOUND:
        Serial.println(F("EV_BEACON_FOUND"));
        break;
    case EV_BEACON_MISSED:
        Serial.println(F("EV_BEACON_MISSED"));
        break;
    case EV_BEACON_TRACKED:
        Serial.println(F("EV_BEACON_TRACKED"));
        break;
    case EV_JOINING:
        Serial.println(F("EV_JOINING"));
        break;
    case EV_JOINED:
        Serial.println(F("EV_JOINED"));
        break;
    /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_RFU1:
        ||     Serial.println(F("EV_RFU1"));
        ||     break;
        */
    case EV_JOIN_FAILED:
        Serial.println(F("EV_JOIN_FAILED"));
        break;
    case EV_REJOIN_FAILED:
        Serial.println(F("EV_REJOIN_FAILED"));
        break;
    case EV_TXCOMPLETE:
        //display.clear();
        //display.drawString(0, 20, "EV_TXCOMPLETE event!");
        Serial.println(F("EV_TXCOMPLETE (includes waiting for RX windows)"));
        if (LMIC.txrxFlags & TXRX_ACK)
        {
            Serial.println(F("Received ack"));
            //display.drawString(0, 30, "Received ACK.");
        }
        if (LMIC.dataLen)
        {
            int i = 0;
            Serial.print(F("Received "));
            Serial.print(LMIC.dataLen);
            Serial.println(F(" bytes of payload"));
            //display.drawString(0, 30, "Received DATA.");
            for (i = 0; i < LMIC.dataLen; i++)
                TTN_response[i] = LMIC.frame[LMIC.dataBeg + i];
            dataLength = LMIC.dataLen;
            TTN_response[i] = 0;
            //display.drawString(0, 40, String(TTN_response));
            Serial.println(String(TTN_response));
        }
        // Schedule next transmission
        //Serial.println(US_PER_OSTICK);
        //Serial.println(OSTICKS_PER_SEC);
        //Serial.println(LMIC.radio.rxlate_count);
        //Creo que esta llamada ya no será necesaria, la conservo porque creo que es ilustrativa
        //os_setTimedCallback(&sendjob, os_getTime() + sec2osticks(TX_INTERVAL), do_send);
        finished=true;
        digitalWrite(LEDPIN, LOW);
        //definida en el ino
        //display.drawString(0, 50, String(ReadCount));
        //display.display();
        break;
    case EV_LOST_TSYNC:
        Serial.println(F("EV_LOST_TSYNC"));
        break;
    case EV_RESET:
        Serial.println(F("EV_RESET"));
        break;
    case EV_RXCOMPLETE:
        // data received in ping slot
        Serial.println(F("EV_RXCOMPLETE"));
        break;
    case EV_LINK_DEAD:
        Serial.println(F("EV_LINK_DEAD"));
        break;
    case EV_LINK_ALIVE:
        Serial.println(F("EV_LINK_ALIVE"));
        break;
    /*
        || This event is defined but not used in the code. No
        || point in wasting codespace on it.
        ||
        || case EV_SCAN_FOUND:
        ||    Serial.println(F("EV_SCAN_FOUND"));
        ||    break;
        */
    case EV_TXSTART:
        Serial.println(F("EV_TXSTART"));
        break;
    case EV_TXCANCELED:
        Serial.println(F("EV_TXCANCELED"));
        break;
    case EV_RXSTART:
        /* do not print anything -- it wrecks timing */
        break;
    case EV_JOIN_TXCOMPLETE:
        Serial.println(F("EV_JOIN_TXCOMPLETE: no JoinAccept"));
        break;
    default:
        Serial.print(F("Unknown event: "));
        Serial.println((unsigned)ev);
        break;
    }
}

void MyLMIC::do_send(osjob_t *jn, CayenneLPP payload)
{
    // Check if there is not a current TX/RX job running
    if (LMIC.opmode & OP_TXRXPEND)
    {
        Serial.println(F("OP_TXRXPEND, not sending"));
    }
    else
    {
        Serial.println("Start Payload Building"); //Moy
        // Prepare upstream data transmission at the next possible time.
        // Leer dato de la flash
        // Convertirlo todo a un solo arreglo de bytes
        // byte payload[8];
        // payload[0] = pld.delay;
        // //Serial.println(payload[0]);
        // payload[1] = pld.inTemp;
        // //Serial.println(payload[1]);
        // payload[2] = pld.inHum;
        // //Serial.println(payload[2]);      
        // payload[3] = pld.pressure;
        // //Serial.println(payload[4]);
        // payload[4] = highByte(pld.readcount);        
        // //Serial.println(payload[5]);
        // payload[5] = lowByte(pld.readcount);
        // payload[6] = highByte(pld.mempos);        
        // //Serial.println(payload[5]);
        // payload[7] = lowByte(pld.mempos);
        // //Ver el cpu que se está usando
        // //Serial.print("send() running on core ");
        // //Serial.println(xPortGetCoreID());

        //Check frequency
        //Serial.println(LMIC.freq);
        //readcount++;
        Serial.print("Tamaño de payload: ");
        //Serial.println(ReadCount);
        Serial.println(payload.getSize());
        //Regresar a 0 el ultimo parametro, es la peticion de confirmación
        LMIC_setTxData2(1, payload.getBuffer(), payload.getSize(), 0);
        Serial.println(F("Packet queued"));
        //digitalWrite(LEDPIN, HIGH);
        // display.clear();
        // display.drawString(0, 30, "Sending uplink packet...");
        // display.drawString(0, 50, String(readcount));
        // display.display();
        ReadCount++;
    }    
    // Next TX is scheduled after TX_COMPLETE event.
}


void MyLMIC::loraSetup(int readcount)
{
    ReadCount=readcount;
    //while (!Serial); // wait for Serial to be initialized
    //Serial.begin(115200);
    //delay(1500); // per sample code on RF_95 test
    Serial.println("Starting lmic routine");

    //Ver el cpu que se está usando
    //Serial.print("setup() running on core ");
    //Serial.println(xPortGetCoreID());

    //Desactivar el SS de USB Host, spi.h debería manejarlo por si mismo?
    //digitalWrite(USB_SS, HIGH);
    
    //Todas las referencias a oled y display las eliminé para poder abstraer esos mensajes al loop principal
    // pinMode(LEDPIN, OUTPUT);
    // pinMode(OLED_RESET, OUTPUT);
    // digitalWrite(OLED_RESET, LOW);
    // delay(50);
    // digitalWrite(OLED_RESET, HIGH);

    // display.init();
    // display.flipScreenVertically();
    // display.setFont(ArialMT_Plain_10);

    // display.setTextAlignment(TEXT_ALIGN_LEFT);

    // display.drawString(0, 0, "Init!");
    // display.display();

    // LMIC init
    os_init();

    // display.drawString(0, 8, "LMIC was initiated!");
    // display.display();

    // Reset the MAC state. Session and pending data transfers will be discarded.
    LMIC_reset();

    // display.drawString(0, 16, "LMIC reset -> OK!");
    // display.display();

// Set static session parameters. Instead of dynamically establishing a session
// by joining the network, precomputed session parameters are be provided.
#ifdef PROGMEM
    // On AVR, these values are stored in flash and only copied to RAM
    // once. Copy them to a temporary buffer here, LMIC_setSession will
    // copy them into a buffer of its own again.
    uint8_t appskey[sizeof(APPSKEY)];
    uint8_t nwkskey[sizeof(NWKSKEY)];
    memcpy_P(appskey, APPSKEY, sizeof(APPSKEY));
    memcpy_P(nwkskey, NWKSKEY, sizeof(NWKSKEY));

    LMIC_setSession(0x1, DEVADDR, nwkskey, appskey);

#else
    // If not running an AVR with PROGMEM, just use the arrays directly
    LMIC_setSession(0x1, DEVADDR, NWKSKEY, APPSKEY);
#endif

#if defined(CFG_eu868)
    // Set up the channels used by the Things Network, which corresponds
    // to the defaults of most gateways. Without this, only three base
    // channels from the LoRaWAN specification are used, which certainly
    // works, so it is good for debugging, but can overload those
    // frequencies, so be sure to configure the full frequency range of
    // your network here (unless your network autoconfigures them).
    // Setting up channels should happen after LMIC_setSession, as that
    // configures the minimal channel set. The LMIC doesn't let you change
    // the three basic settings, but we show them here.
    LMIC_setupChannel(0, 868100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
    LMIC_setupChannel(1, 868300000, DR_RANGE_MAP(DR_SF12, DR_SF7B), BAND_CENTI); // g-band
    LMIC_setupChannel(2, 868500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
    LMIC_setupChannel(3, 867100000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
    LMIC_setupChannel(4, 867300000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
    LMIC_setupChannel(5, 867500000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
    LMIC_setupChannel(6, 867700000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
    LMIC_setupChannel(7, 867900000, DR_RANGE_MAP(DR_SF12, DR_SF7), BAND_CENTI);  // g-band
    LMIC_setupChannel(8, 868800000, DR_RANGE_MAP(DR_FSK, DR_FSK), BAND_MILLI);   // g2-band
// TTN defines an additional channel at 869.525Mhz using SF9 for class B
// devices' ping slots. LMIC does not have an easy way to define set this
// frequency and support for class B is spotty and untested, so this
// frequency is not configured here.
#elif defined(CFG_us915) || defined(CFG_au915)
    // NA-US and AU channels 0-71 are configured automatically
    // but only one group of 8 should (a subband) should be active
    // TTN recommends the second sub band, 1 in a zero based count.
    // https://github.com/TheThingsNetwork/gateway-conf/blob/master/US-global_conf.json
    LMIC_selectSubBand(1);
#elif defined(CFG_as923)
// Set up the channels used in your country. Only two are defined by default,
// and they cannot be changed.  Use BAND_CENTI to indicate 1% duty cycle.
// LMIC_setupChannel(0, 923200000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);
// LMIC_setupChannel(1, 923400000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_CENTI);

// ... extra definitions for channels 2..n here
#elif defined(CFG_kr920)
// Set up the channels used in your country. Three are defined by default,
// and they cannot be changed. Duty cycle doesn't matter, but is conventionally
// BAND_MILLI.
// LMIC_setupChannel(0, 922100000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_MILLI);
// LMIC_setupChannel(1, 922300000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_MILLI);
// LMIC_setupChannel(2, 922500000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_MILLI);

// ... extra definitions for channels 3..n here.
#elif defined(CFG_in866)
// Set up the channels used in your country. Three are defined by default,
// and they cannot be changed. Duty cycle doesn't matter, but is conventionally
// BAND_MILLI.
// LMIC_setupChannel(0, 865062500, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_MILLI);
// LMIC_setupChannel(1, 865402500, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_MILLI);
// LMIC_setupChannel(2, 865985000, DR_RANGE_MAP(DR_SF12, DR_SF7),  BAND_MILLI);

// ... extra definitions for channels 3..n here.
#else
#error Region not supported
#endif

    //Desactivar los otros canales y usar solo 8
    forceTxSingleChannelDr(CHANNEL);

    // Disable link check validation
    LMIC_setLinkCheckMode(0);

    // TTN uses SF9 for its RX2 window.
    // En US DR_SF12CR El sketch tenía DR_SF9
    LMIC.dn2Dr = DR_SF10;

    // Set data rate (SF) and transmit power for uplink
    LMIC_setDrTxpow(DR_SF7, 14);
    //ampliamos la ventanda de rx
    //LMIC_setClockError(MAX_CLOCK_ERROR * 10 / 100);
    //Aquí no es necesario envíar aun
    //do_send(&sendjob, ReadCount);
}

bool MyLMIC::loraLoop()
{
    finished = false;
    //Ver el cpu que se está usando
    //Serial.print("loop() running on core ");
    //Serial.println(xPortGetCoreID());
    os_runloop_once();    
    //Si hubo una respuesta, corregir
    if (dataLength>0){        
        Serial.print("Está definida la respuesta como: ");
        Serial.println(String(TTN_response));
        //Al vuelo, aquí somos bien machos antes ReadCount
        Correccion=atoi(TTN_response);
        dataLength=0;       
    }
    return finished;
}
