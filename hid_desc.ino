#include <usbhid.h>
#include <hiduniversal.h>
#include <hidescriptorparser.h>
#include <usbhub.h>
#include "pgmstrings.h"
#include <EEPROM.h>
#include <Arduino.h> // for type definitions
#include "lmicmodule.h"
#include <SSD1306.h>
#include <CayenneLPP.h>

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
#define OLED_I2C_ADDR 0x3C
#define OLED_RESET 16
#define OLED_SDA 4
#define OLED_SCL 15

SSD1306 display(OLED_I2C_ADDR, OLED_SDA, OLED_SCL);

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
MyLMIC lmic;
CayenneLPP pld(30);

uint16_t ReadCount = 0;
uint16_t StMempos;
int Correccion;
bool Sent = false;

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
    ReadCount = EEPROM.readShort(0);
    StMempos = EEPROM.readShort(4);
    Serial.println(ReadCount);
    Serial.println(StMempos);
    //Eliminar esta linea para poner en "producción"
    ReadCount = 0;
    lmic.loraSetup(ReadCount);
    //Set StMempos for extraction
    //dr.setLastSentRecord(ReadCount, StMempos);
}

void loop()
{
    //Reiniciamos la variable de control de correcciones
    Correccion = -1;    
    uint16_t currentPos, readPosition; //Aqui almacenaremos la posicion del buffer circular
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
            //Iterar para todas las lecturas 4078 + 1 es la cantidad máxima de lecturas 4096 es el total de segmentos de 16 bytes en el buffer
            for (int indice = 1; indice <= 4096; indice++)
            {
                //Leemos el la lectura inmediata siguiente: cada lectura esta separada por 16 bytes
                //65520 es la posicion max-1, esta es invalida
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
                    // Serial.println(wh1080Work.sensorData.outhumidity); //Faltan
                    // Serial.println(wh1080Work.sensorData.outtemp);
                    // Serial.println(wh1080Work.sensorData.windAvgSpeed);
                    // Serial.println(wh1080Work.sensorData.windGustSpeed);
                    // Serial.println(wh1080Work.sensorData.windDir);
                    // Serial.println(wh1080Work.sensorData.rain);
                    // Serial.println(wh1080Work.sensorData.status);
                    //Se arma el payload                    
                    // pld.delay = wh1080Work.recordBlock[0];
                    // pld.inTemp = wh1080Work.recordBlock[2];
                    // pld.inHum = wh1080Work.recordBlock[1];
                    // pld.pressure = wh1080Work.recordBlock[7];
                    // pld.readcount = numLectura;
                    // pld.mempos = readPosition;
                    //Serial.println(ESP.getFreeHeap());
                    pld.reset();
                    pld.addDigitalOutput(1, (int)(wh1080Work.sensorData.delay));//1+2
                    pld.addTemperature(2, wh1080Work.sensorData.intemp);//2+2
                    pld.addRelativeHumidity(3, wh1080Work.sensorData.inhumidity);//1+2
                    pld.addBarometricPressure(4, wh1080Work.sensorData.pressure);//2+2
                    //Este indice se descontrolaba por que está pensado en un signed int de 32000 (medio maxint)
                    //Serial.println((numLectura/100));
                    //addGenericSensor cambiar a esto pa mandar 4 bytes
                    pld.addGenericSensor(5, numLectura);//4+2
                    pld.addAnalogOutput(6, readPosition);//2+2
                    //Esperamos un segundo para que el SPI cambie a libre
                    //delay(1000);
                    //Restablecemos la bandera del envío
                    Sent = false;
                    // Enviar
                    lmic.do_send(&sendjob, pld);
                    //Iterar el evento del lmic para esperar la respuesta
                    while (!Sent)
                    {
                        Sent = lmic.loraLoop();
                    }
                    //Hubo respuesta,  Co
                    Correccion=lmic.Correccion;
                    lmic.Correccion=-1;
                    if (Correccion != -1)
                    {
                        Serial.print("Intentando corregir a: ");
                        Serial.println(Correccion);
                        //la diferencia entre el numero actual y la correccion que pide el  servidor
                        int dif = numLectura - Correccion;
                        //Ajustamos la posicion de lectura 
                        readPosition=readPosition-(dif*16);
                        //Reducimos el indice para evitar un recorrido incompleto
                        indice = indice - dif;
                        //Ajustamos el numero de envío
                        numLectura=Correccion;
                        Correccion = -1;
                    }                    
                    else //Si no hay nada que corregir
                    {
                        // Guardar
                        if (saveLecnPos(numLectura, readPosition) == OK)
                        {
                            Serial.println("Escrito");
                        }
                        numLectura++;
                    }
                }
                else
                {
                    //Lectura invalida
                    Serial.println("Lectura invalida");
                    //continue;
                }
                Serial.println(" ");
                //Para hacer el debug                
                //delay(10000);
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