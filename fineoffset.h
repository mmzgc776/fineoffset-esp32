#ifndef usbhub
#define usbhub
#include <usbhub.h>
#endif
#include <stdio.h>
#include <string.h>

//Tratare de implementar WH1080_WORK en mi implementación
  // define the readings collector

#define WH1080_BUFFER_CHUNK         0x20        // Size of chunk received over USB
#define IN_BUFFER_SIZE       32 
class DeviceReader
{
public:
  //int PERADDR = 0xe0;
  DeviceReader();
  int ciclos = 0;  
  int vendor_id = 1941;
  int product_id = 8021;
  const uint8_t endPoint = 0; //81 en el descriptor de usb y script 0 
  //int usb_endpoint = 81; // revisar para que sirve, evitar confusión
  const uint8_t usb_addr = 1; //Dirección en USB_desc 1 dirección del dispositivo en el hub???
  //int address = 1; //Direccion en memoria de la estacion
  const uint16_t kBufferSize = 32; //Esto estaba definido como 10, pero luego lo cambié a 16 observando la script de getData.py
  
  //Abrimos el puerto y lo dejamos disponible, basicamente llamamos a openPort()
  void openPort(USB Usb);
  //Se supone que esta función buscaría el puerto y direccion, pero al ser un solo adaptador se puede obviar (creo), dejando 
  //las direcciones predeterminadas, revisa que esté listo el USB
  void find_device(USB Usb); 
  //Llamada a uno de los metodos mas basicos de acceso a la estacion
  //Hace un control transfer y un intransfer para obtener datos desde USB,
  //Retorna un arreglo de 16 bytes, que debe ser la lectura
  uint8_t* read_usb_block(USB Usb, int usb_address);
  //Funcion que debería llamar a read_usb_block
  uint8_t*  read_block(USB Usb, int ptr, bool retry=true);
  //Funcion siguiente read_block, lee de 20 en 20 retorna un arreglo 
  //cuando está correcto, usa los magic_numbers para validar
  //Ptr se llama block en el ejemplo de C, pero ptr en el ejemplo de python
  uint8_t* read_fixed_block(USB Usb, int hi = 0x0100);  

};