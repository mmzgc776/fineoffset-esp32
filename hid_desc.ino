#include <usbhid.h>
#include <hiduniversal.h>
#include <hidescriptorparser.h>
#include <usbhub.h>
#include "pgmstrings.h"
#include <EEPROM.h>
#include <Arduino.h> // for type definitions
#include <SSD1306.h>

#define OLED_I2C_ADDR 0x3C
#define OLED_RESET 16
#define OLED_SDA 4
#define OLED_SCL 15

SSD1306 display(OLED_I2C_ADDR, OLED_SDA, OLED_SCL);
//#include "fineoffset.h"

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

//Slave selects para spi
//Slave select para LORA
#define LORA_SS 18
//La libreria de Host fué modificada para adaptarse a esto
#define USB_SS 23
#define OK 0

template <class T>
int EEPROM_writeAnything(int ee, const T &value)
{
    const byte *p = (const byte *)(const void *)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
        EEPROM.write(ee++, *p++);
    return i;
}

template <class T>
int EEPROM_readAnything(int ee, T &value)
{
    byte *p = (byte *)(void *)&value;
    unsigned int i;
    for (i = 0; i < sizeof(value); i++)
        *p++ = EEPROM.read(ee++);
    return i;
}

class HIDUniversal2 : public HIDUniversal
{
public:
    HIDUniversal2(USB *usb) : HIDUniversal(usb){};

protected:
    uint8_t OnInitSuccessful();
};

uint8_t HIDUniversal2::OnInitSuccessful()
{
    uint8_t rcode;

    HexDumper<USBReadParser, uint16_t, uint16_t> Hex;
    ReportDescParser Rpt;

    if ((rcode = GetReportDescr(0, &Hex)))
        goto FailGetReportDescr1;

    if ((rcode = GetReportDescr(0, &Rpt)))
        goto FailGetReportDescr2;

    return 0;

FailGetReportDescr1:
    USBTRACE("GetReportDescr1:");
    goto Fail;

FailGetReportDescr2:
    USBTRACE("GetReportDescr2:");
    goto Fail;

Fail:
    Serial.println(rcode, HEX);
    Release();
    return rcode;
}

USB Usb;
//USBHub Hub(&Usb);
HIDUniversal2 Hid(&Usb);
UniversalReportParser Uni;
DeviceReader dr;

uint16_t ReadCount = 0;
uint16_t StMempos;

//static WH1080_WORK wh1080Work;

void setup()
{
    Serial.begin(115200);
#if !defined(__MIPSEL__)
    while (!Serial)
        ; // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
    pinMode(USB_SS, OUTPUT);
    pinMode(LORA_SS, OUTPUT);
    //Activar usb
    digitalWrite(USB_SS, LOW);
    //Desactivar lora
    digitalWrite(LORA_SS, HIGH);

    Serial.println("Start");

    if (Usb.Init() == -1)
        Serial.println("OSC did not start.");

    delay(200);
    if (!Hid.SetReportParser(0, &Uni))
        ErrorMessage<uint8_t>(PSTR("SetReportParser"), 1);

    if (!EEPROM.begin(8))
    {
        Serial.println("failed to initialise EEPROM");
        delay(100000);
    }

    //Get ReadCount and StMempos
    ReadCount=EEPROM.readShort(0);
    StMempos=EEPROM.readShort(4);
    Serial.println(ReadCount);
    Serial.println(StMempos);
    //Eliminar esta linea para poner en "producción"
    ReadCount=0;
    //Set StMempos for extraction
    //dr.setLastSentRecord(ReadCount, StMempos);
}

void loop()
{
    uint16_t currentPos, readPosition; //Aqui almacenaremos la posicion del buffer circular
    //int lectura=0;
    //Iniciamos el usb
    Usb.Task();
    //Si el usb está listo para trabajar
    if (dr.openPort(Usb) == OK)
    {
        //Comenzar a intentar sincronizar completo
        if (ReadCount == 0)
        {
            //Se obtiene la posicion del buffer
            currentPos = dr.getCurrentPosition(Usb);
            //Iterar para todas las lecturas 4078 es la cantidad máxima de lecturas 4096 es el total de segmentos de 16 bytes en el buffer
            for (int indice = 1; indice <= 4096; indice++)
            {
                //Leemos el la lectura inmediata siguiente: cada lectura esta separada por 16 bytes
                readPosition = currentPos + (indice * 16);
                if (dr.readStationData(Usb, readPosition) == OK)
                {
                    Serial.print("ReadPos:  ");
                    Serial.print(readPosition);
                    Serial.print(", Fixed Pos:  ");
                    Serial.print(currentPos);
                    Serial.print(", Lectura:  ");
                    Serial.print(numLectura);
                    Serial.print("  Delay:   ");
                    Serial.print(wh1080Work.sensorData.delay); //Ok
                    Serial.print("  InTemp:   ");
                    Serial.print(wh1080Work.sensorData.intemp); //Ok
                    Serial.print("  InHum:   ");
                    Serial.print(wh1080Work.sensorData.inhumidity); //Ok
                    Serial.print("  Press:   ");
                    Serial.println(wh1080Work.sensorData.pressure); //ok
                    // Serial.println(wh1080Work.sensorData.outhumidity); //:(
                    // Serial.println(wh1080Work.sensorData.outtemp);
                    // Serial.println(wh1080Work.sensorData.windAvgSpeed);
                    // Serial.println(wh1080Work.sensorData.windGustSpeed);
                    // Serial.println(wh1080Work.sensorData.windDir);
                    // Serial.println(wh1080Work.sensorData.rain);
                    // Serial.println(wh1080Work.sensorData.status);
                    // Enviar
                    
                    // Hubo respuesta?
                    // Corregir
                    // Guardar
                    if (saveLecnPos(numLectura, readPosition) == OK)
                    {
                        Serial.println("Escrito?");
                    }
                    numLectura++;
                }
                else
                {
                    //Lectura invalida
                    Serial.println("Lectura invalida");
                    //continue;
                }
                Serial.println(" ");
            }
            //getLastSentRecordfromFile();
            //saveData();
            //Si no hay lectura valida esperamos un sec
            //while(true){
            //    readStationData(Usb, &work, currentPos);
            //}
            //Luego lo cambiamos
        }
        else
        {
            //Sincronizar del ultimo enviado al actual
            //Se obtiene la posicion del buffer
            currentPos = dr.getCurrentPosition(Usb);
        }
    }
    //Existe el archivo?
    //Enviar lo que esté dentro del archivo
    //Dormir
    //Nos preparamos para leer los datos de la estación
    //Crear el archivo
    delay(10000);
}
int saveLecnPos(uint16_t lectura, uint16_t posicion)
{
    //Guardamos readcount o lectura
    EEPROM.writeShort(0, lectura);
    //Guardamos lastpos o ultima posicion
    EEPROM.writeShort(4, posicion);
    EEPROM.commit();
    return OK;
}