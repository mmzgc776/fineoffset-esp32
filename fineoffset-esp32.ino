#include <usbhid.h>
#include <hiduniversal.h>
#include <hidescriptorparser.h>
#include <usbhub.h>
#include "pgmstrings.h"
#include <Arduino.h> // for type definitions
#include <EEPROM.h>
//custom headers
#include "lmicmodule.h"
//#include "fineoffset.h"
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

//Sleeping
#define uS_TO_S_FACTOR 1000000 /* Conversion factor for micro seconds to seconds */
//Dormirá por una hora, tiempo suficiente para generar una lectura
#define TIME_TO_SLEEP 3600 /* Time ESP32 will go to sleep (in seconds) */

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

unsigned long start = 0; //Timing variable
uint16_t readCount; //Numero de lectura en la que se quedó
uint16_t readStationPos; //Posición guardada del último que envió
int Correccion;
bool Sent = false;
//Para medir el tiempo
unsigned long delta=0;

//static WH1080_WORK wh1080Work;

void setup()
{
    start = micros();
    Serial.begin(115200);
    
#if !defined(__MIPSEL__)
    while (!Serial); // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
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
    //Eliminar esta linea para poner en "producción", esto restablece los dos registros
    reset();
    //Get readCount and readStationPos
    readCount = EEPROM.readShort(0);
    readStationPos = EEPROM.readShort(4);
    Serial.println(readCount);
    //Descomentar esto para la siguiente prueba
    Serial.println(readStationPos);    
    lmic.loraSetup(readCount);
    //Set readStationPos for extraction
    //dr.setLastSentRecord(readCount, readStationPos);
}

void loop()
{
    //Reiniciamos la variable de control de correcciones
    Correccion = -1;
    uint16_t currentPos;   //La posicion actual del buffer circular al encender
    
    //Iniciamos el usb
    Usb.Task();

    //Si el usb está listo para trabajar
    if (dr.openPort(Usb) == OK)
    {
        //Se obtiene la posicion actual del buffer
        currentPos = dr.getCurrentPosition(Usb);
        Serial.print("Current Position: ");
        Serial.println(currentPos);        
        sincronizar(currentPos, readStationPos, readCount);
    }
    //Si no está listo el USB, estoy tentado a eliminar esto para hacerlo más rapido
    delay(10000);
    //Dormir
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


int sincronizar(uint16_t currentPos, //La posicion actual del buffer circular al encender
                uint16_t readPosition, //Posicion del ultimo enviado
                uint16_t readCount //indice del ultimo enviado
                ){
    
    uint16_t lecturasPendientes; //Numero de iteraciones para alcanzar currentPos
    uint16_t referenceAddress; //Será la dirección que se tomará de referencia para recorrer el buffer
    uint16_t steps = 1; //Será el numero de pasos de 16 bytes que habrá de adelantarse el bloque en el ciclo for


    //Determinar el numero de iteraciones
    //Comenzar a intentar sincronizar completo (toda la vuelta al buffer)
    Serial.print("Sincronización ");
    if (readCount == 0)
    {
        //Sincronizar todo
        lecturasPendientes = 4077;
        referenceAddress=currentPos; //Posicion del buffer de la estación
        Serial.print("completa, ");        
    }
    else //11-1-21, al continuar debemos comenzar uno adelante en el indice, pero igual en la direccion
    {                
        //¿Que va a pasar en los valores frontera? Poner mucha atención
        //Si currentPos está adelante:
        Serial.print("parcial, ");
        if(currentPos > readPosition){
            Serial.print("currentPos adelante, ");
            lecturasPendientes = (currentPos - readPosition - 16) / 16;
            referenceAddress = readPosition;
        }
        //Si currentPos está atrás:
        if(currentPos < readPosition){
            //El ultimo valido menos las posicion de lectura 
            //más
            //El ultimo menos los primeros 16 invalidos 
            //18 son invalidos el ultimo, los 16 primeros y el de currentPos
            Serial.print("currentPos atras, ");
                        
            lecturasPendientes =  ((65520 - readPosition)/16 + (currentPos-256)/16);
            referenceAddress = readPosition;
        }if (lecturasPendientes == 0){
            Serial.println("Está actualizado debería dormir");
        }                           
    }
    Serial.print("lecturas pendientes: ");
    Serial.println(lecturasPendientes);    

    //Sincronizar del ultimo enviado al actual
    //Ciclo de sincronización
    //Iterar para todas las lecturas 4078 es la cantidad máxima de lecturas
    //4096 es el total de segmentos de 16 bytes en el buffer ahí da la vuelta a 0
    //De los cuales el ultimo, el current y los 16 primeros son invalidos
    for (int indice = 1; indice <= lecturasPendientes; indice++)
    {
        Serial.print("indice: ");
        Serial.println(indice);
        Serial.print("Lecturas pendientes: ");
        Serial.println(lecturasPendientes-indice);
        //Leemos el la lectura inmediata siguiente: cada lectura esta separada por 16 bytes
        //65520 es la posicion max-1, esta es invalida (max = 0, vuelve a iniciar)?             
        
        //Observar si el primero inmediato pide correción, si lo hace debo hacer +1
        readPosition = referenceAddress + (steps * 16);        
        if (dr.readStationData(Usb, readPosition) == OK)
        {
            Serial.print("ReadPos:  ");
            Serial.print(readPosition);
            Serial.print(", Fixed Pos:  ");
            Serial.print(currentPos);
            Serial.print(", Lectura:  ");
            Serial.print(readCount);
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
            // pld.readcount = readCount;
            // pld.mempos = readPosition;
            //Serial.println(ESP.getFreeHeap());
            pld.reset();
            pld.addDigitalOutput(1, (int)(wh1080Work.sensorData.delay));  //1+2
            pld.addTemperature(2, wh1080Work.sensorData.intemp);          //2+2
            pld.addRelativeHumidity(3, wh1080Work.sensorData.inhumidity); //1+2
            pld.addBarometricPressure(4, wh1080Work.sensorData.pressure); //2+2

            //Este indice se descontrolaba por que está pensado en un signed int de 32000 (medio maxint)
            //addGenericSensor cambiar a esto pa mandar 4 bytes
            //se (ab)usa de esta medida para envíar numeros más grandes como dice aquí:
            // https://www.thethingsnetwork.org/forum/t/cayenne-lpp-format-analog-data-wrong/14676
            pld.addLuminosity(5, (float)readCount); //4+2
            pld.addAnalogOutput(6, readPosition);    //2+2

            //Restablecemos la bandera del envío
            Sent = false;

            // Enviar
            lmic.do_send(&sendjob, pld);

            //Iterar el evento del lmic para esperar la respuesta
            while (!Sent)
            {
                Sent = lmic.loraLoop();
            }

            //Corrección
            //Hubo respuesta?, obtenemos la corrección, un entero con el indice esperado
            Correccion = lmic.Correccion;
            //restablecemos el lmic para poderlo volver a usar
            lmic.Correccion = -1;
            //Corregir
            if (Correccion != -1)
            {
                Serial.print("Intentando corregir a: ");
                Serial.println(Correccion);
                //la diferencia entre el numero actual y la correccion que pide el servidor
                int dif = readCount - Correccion;
                //Ajustamos la posicion de lectura
                //readPosition = readPosition - (dif * 16); //Creí que esto no era necesario
                //Reducimos el indice para evitar un recorrido incompleto
                steps = steps - dif;
                //Dado que tuvimos que corregir, esta iteración debe repetirse, 
                //no estoy seguro de que esto sea necesario
                indice--;
                //Ajustamos el numero de envío
                readCount = Correccion;
                Correccion = -1;
            }
            else //Si no hay nada que corregir
            {
                // Guardar
                if (saveLecnPos(readCount, readPosition) == OK)
                {
                    Serial.println("Escrito");
                }
                //Se envió correctamente, incrementamos   
                readCount++;
            }
        }
        else
        {
            //Lectura invalida
            Serial.println("Lectura invalida");
            //Si no sale bien esa iteración no cuenta
            indice--;            
        }
        Serial.println(" ");
        //Para hacer el debug
        //delay(10000);
        Serial.print("Partial run time: ");
        delta = micros() - start;
        Serial.print(delta/1000000);
        Serial.println(" secs");
        steps++;
    }   
    
    Serial.print("Total run time: ");
    delta = micros() - start;
    Serial.print(delta/1000000);
    Serial.print(" secs");

    //Dormir
    Serial.println("A dormir :)");
    
    Serial.flush();
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
    
    //return OK;
}

//restablece los registros
void reset(){
    saveLecnPos(0,0);
}