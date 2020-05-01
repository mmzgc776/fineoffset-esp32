#include <usbhub.h>
#include "pgmstrings.h"
// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>
#include "fineoffset.h"

//Recibe un registro y lo envía a través de LORA
void SendRecord()
{
}

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

  Serial.begin(115200);
#if !defined(__MIPSEL__)
  while (!Serial)
    ; // Wait for serial port to connect - used on Leonardo, Teensy and other boards with built-in USB CDC serial connection
#endif
  Serial.println("Start");

  if (Usb.Init() == -1)
    Serial.println("OSC did not start.");

  delay(200);
}

void loop()
{
  
  DeviceReader dr;
  Serial.println("Objeto DeviceReader");
  delay(5000);
  dr.openPort(Usb);
  //Serial.println("Llamada de loop");
}
