#include <usbhub.h>
#include "pgmstrings.h"
// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>
#include "funciones.h"

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

//Se pretende adaptar los controladores de FineOffset WH1080
//Aquí están las funciones básicas para la implementación de un extractor por USB
//Moisés González Castellanos 04-04-20
class DeviceReader : public USB
{
  int addr=1;
  int vendor_id = 1941;
  int product_id = 8021;

public:
  int ciclos = 0;

  //Abrimos el puerto y lo dejamos disponible
  void openPort(USB Usb)
  {
    Serial.println("Clase DeviceReader/openport");
    find_device(Usb);
  }

  //Find the vendor and product ID on the USB.
  void find_device(USB Usb)
  {
    delay(5000);
    Serial.println("Clase DeviceReader/usb.task()");
    
    //delay(1000);
    Serial.println("Clase DeviceReader/find_device");
    Serial.println(vendor_id);
    Serial.println(product_id);
    Usb.Task();
    delay(1000);
    if (Usb.getUsbTaskState() == USB_STATE_RUNNING)
    {
      Usb.ForEachUsbDevice(&PrintAllDescriptors);
      Usb.ForEachUsbDevice(&PrintAllAddresses);
      Serial.println("USB_STATE_RUNNING"); 
    }
  }
};

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
  Serial.println("Objeto DeviceReader");
  DeviceReader dr;
  delay(5000);
  dr.openPort(Usb);  
  Serial.println("Llamada de loop");
  
}
