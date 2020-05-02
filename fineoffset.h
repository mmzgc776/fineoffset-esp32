#ifndef usbhub
#define usbhub
#include <usbhub.h>
#endif

class DeviceReader
{
public:
  DeviceReader();
  int ciclos = 0;
  int addr = 1;
  int vendor_id = 1941;
  int product_id = 8021;
  int usb_endpoint = 81; // revisar para que sirve, evitar confusión
  uint16_t bufferSize = 20;
  //uint16_t readBuffer[20];
  //Se supone que esta función buscaría el puerto y direccion, pero al ser un solo adaptador se puede obviar (creo), dejando las direcciones predeterminadas
  void find_device(USB Usb);
  //Abrimos el puerto y lo dejamos disponible
  void openPort(USB Usb);
  //Llamada a uno de los metodos mas basicos de acceso a la estacion
  uint8_t* read_usb_block(USB Usb, int usb_address);
  //Funcion que debería llamar a read_usb_block
  void read_block(USB Usb, int ptr, bool retry=true);
  
};