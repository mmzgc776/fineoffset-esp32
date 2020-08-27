//USBhost
// #ifndef usbhub
// #include <usbhub.h>
// #endif
// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>
#include "pgmstrings.h"
//Pantalla
#include <SSD1306.h>

#define OLED_I2C_ADDR 0x3C
#define OLED_RESET 16
#define OLED_SDA 4
#define OLED_SCL 15

//Slave selects para spi
//Slave select para LORA
#define LORA_SS 18
//La libreria de Host fué modificada para adaptarse a esto
#define USB_SS 23

#include "lmicmodule.h"
#include "fineoffset.h"

USB Usb;

//#include "USB_desc.h"

//Pregunta por el último indice entregado
void AskForLastIndex()
{
}

//Envía el dispositivo a dormir
void GotoSleep()
{
}

//Escribe un registro en el archivo intermediario
void WriteRecordToFile()
{
}

void setup()
{
  while (!Serial)
        ; // wait for Serial to be initialized
  Serial.begin(115200);
#if !defined(__MIPSEL__)
  while (!Serial)
    ; // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
  // pinMode(LORA_SS, OUTPUT);
  // //Desactivar radio
  // //Pines de salida
  pinMode(USB_SS, OUTPUT);
  pinMode(LORA_SS, OUTPUT);
  //Activar usb  
  digitalWrite(USB_SS, LOW);
   //Desactivar lora  
  digitalWrite(LORA_SS, HIGH);
  //usbsetup();
  Serial.println("Start");
  if (Usb.Init() == -1)
    Serial.println("OSC did not start.");
  delay(200);
}

//Recibe un registro y lo envía a través de LORA
void SendRecord()
{
    bool finished = false;
      // Serial.println(digitalRead(LORA_SS));  
      // Serial.println(digitalRead(USB_SS));           
      loraSetup();
      while(!finished){
        finished = loraLoop();                
      }
      // Serial.println(digitalRead(LORA_SS));  
      // Serial.println(digitalRead(USB_SS));  
}

void loop()
{   
  DeviceReader dr;
  Serial.println("loop");
  //usbloop();
  delay(5000);
  //SendRecord();  
  //Trataré de leer la tabla del pool y adresses   
  dr.openPort(Usb);
  delay(5000);
  //Serial.println("Llamada de loop");
}
