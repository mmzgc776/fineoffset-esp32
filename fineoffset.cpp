#ifndef usbhub
#include <usbhub.h>
#endif
#include <usbhid.h>
#include <hiduniversal.h>
#include "fineoffset.h"
#include <stdio.h>
#include <string.h>

//Se pretende adaptar los controladores de FineOffset WH1080
//Aquí están las funciones básicas para la implementación de un extractor por USB
//Moisés González Castellanos 04-04-20
DeviceReader::DeviceReader() {}

//Abrimos el puerto y lo dejamos disponible
void DeviceReader::openPort(USB Usb)
{
  Serial.println("Clase DeviceReader/openport");
  find_device(Usb);
}

//Find the vendor and product ID on the USB.
void DeviceReader::find_device(USB Usb)
{
  //Marcas de control
  Serial.println("Clase DeviceReader/usb.task()");
  //Se inicia el host usb
  Usb.Task();
  Serial.println("Clase DeviceReader/find_device");
  if (Usb.getUsbTaskState() == USB_STATE_RUNNING)
  {
    Serial.println("USB_STATE_RUNNING > read_block, ptr");
    //Device reset aparentemente solo necesario sobre windows
    //delay 12ms
    delay(12);
    //Device Request Get Descriptor: Configuration 0 (9bytes)

    //Device Request Get Descriptor: Configuration 0 (34bytes)
    //Intentamos ajustar la configuración a 1 hid?  y luego continuamos
    Serial.println("Escribimos la configuracion a 1");
    uint8_t retval = Usb.setConf(usb_addr, endPoint, 1);
    Serial.println(retval, HEX);
    delay(45);
    Serial.println("Delay 90ms");
    delay(45);
    //Controlrequest 8 bytes

    //Recibir 52 bytes DeviceRequest get descriptor unknown
    Serial.println("Control Request 52 b");
    uint8_t result;
    uint16_t total = 0x74; //rqstBuffer.size()
    result = Usb.getDevDescr(usb_addr, endPoint, total, NULL);
    // uint8_t bmReqType = 0x81;  //Tomado de la traza del analizador
    // uint8_t bRequest = 0x06; //usb request, get desc Tomado de la traza del analizador
    // uint8_t wValLo = 0x00;
    // uint8_t wValHi = 0x22;
    // uint16_t wInd = 0;
    // uint16_t total = 0x74; //rqstBuffer.size()
    // uint8_t rqstBuffer[56];    
    // result = Usb.ctrlReq(usb_addr,                          //Address
    //                      endPoint,                          //usb Endpoint
    //                      bmReqType,                         //bmReqType, //33 (ejemplo ps3 bmREQ_HID_OUT, es similar al de wview )
    //                      bRequest,                          //09 set configuration
    //                      wValLo,                            //bConfigurationValue 0x0000200: 00 02 o 02 00
    //                      wValHi,                            //bConfigurationValue este o el de arriba
    //                      wInd,                              //siempre es 0 inguesu
    //                      total,                             //bytes?
    //                      sizeof(rqstBuffer) / sizeof(byte), //bytes a leer
    //                      rqstBuffer,                        //puntero del bufer de request
    //                      NULL);                             //Timeout? Parser!
    Serial.println(result, HEX);
    delay(10000);
    read_fixed_block(Usb);
  }
}

//Escribir una funcion de inicialización
//void DeviceReader::ConfigureDevice(USB Usb){}

uint8_t *DeviceReader::read_fixed_block(USB Usb, int hi)
{
  Serial.println("Clase DeviceReader/read_fixed_block");
  uint8_t *fixed_block = new uint8_t[kBufferSize];
  uint8_t new_block[kBufferSize];

  //Copiamos lo que devuelve read_block a fixed_block
  //Que es block, el ejemplo de C muestra un puntero pero no sé que es?
  //En el ejemplo en C, el uno es un cero
  for (int mempos = 0x0000; mempos < 0x0100; mempos = mempos + 0x0020)
  {
    Serial.println("leyendo las posiciones.");
    memcpy(new_block, read_block(Usb, mempos, true), kBufferSize);
    for (int i = 0; i < kBufferSize; i++)
    {
      //Serial.print(i);
      Serial.print(" ");
      Serial.print(new_block[i], HEX);
      Serial.print(" ");
    }
    Serial.println(".");
  }
  // Check for valid magic numbers:
  // This is hardly an exhaustive list and I can find no definitive
  // documentation that lists all possible values; further, I suspect it is
  // more of a header than a magic number...
  if ((new_block[0] == 0x55) ||
      (new_block[0] == 0xFF) ||
      (new_block[0] == 0x01) ||
      ((new_block[0] == 0x00) && (new_block[1] == 0x1E)) ||
      ((new_block[0] == 0x00) && (new_block[1] == 0x01)))
  {
    //Retornar el valor del bloque, creo, hay que entender la funcion
    Serial.println("Listo para retornar");
    //return OK;
    return fixed_block;
  }
  else
  {
    Serial.println("WH1080: readFixedBlock bad magic number %2.2X %2.2X");
    Serial.println((int)new_block[0]);
    Serial.println((int)new_block[1]);
    Serial.println("WH1080: You may want to clear the memory on the station "
                   "console to remove any invalid records or data...");
    delay(5000);
    //Valio madres, no se leeyo correctamente, restablecer la estación
    //return ERROR_ABORT;
  }
}

// Read block repeatedly until it's stable. This avoids getting corrupt
// data when the block is read as the station is updating it.
uint8_t *DeviceReader::read_block(USB Usb, int ptr, bool retry)
{
  Serial.println("Iniciando lectura de bloque: read_block");
  bool readDone = false;
  uint8_t old_block[kBufferSize];
  uint8_t *new_block;
  //Llenamos el arreglo
  for (int i = 0; i < kBufferSize; i++)
  {
    old_block[i] = 0;
  }
  while (true)
  //for (int t = 0; t < 100; t++)
  {
    //self._wait_for_station()
    //delay(1000);
    new_block = read_usb_block(Usb, ptr);
    Serial.println("Block assigment");
    if (new_block != NULL)
    {
      //Prueba de salida de una lectura unica
      Serial.print("New Buffer: ");
      for (int i = 0; i < kBufferSize; i++)
      {
        //Serial.print(i);
        Serial.print(" ");
        Serial.print(new_block[i], HEX);
        Serial.print(" ");
      }
      Serial.println(".");

      //Prueba de salida de una lectura anterior
      Serial.print("Old Buffer: ");
      for (int i = 0; i < kBufferSize; i++)
      {
        //Serial.print(i);
        Serial.print(" ");
        Serial.print(old_block[i], HEX);
        Serial.print(" ");
      }
      Serial.println(".");

      Serial.println("Array compare");
      //Resvisar si old_block es nulo
      if (old_block != NULL)
      {
        Serial.println("Entra a la comparacion");
        //Metodo con memcomp
        if (memcmp(old_block, new_block, 32) == 0)
        {
          Serial.println("Lecturas iguales, array igual");
          readDone = true;
          break;
        }
        else
        {
          Serial.println("WH1080: readBlock buffer still changing");
          //Serial.println("Elemento diferente");
          memcpy(old_block, new_block, 32);
          readDone = false;
        }
      }
    }
  }
  return new_block;
}

uint8_t *DeviceReader::read_usb_block(USB Usb, int address)
{
  Serial.println("Iniciando lectura de usb: read_usb_block");

  int offset = address;
  uint8_t bmReqType = 33;  //usb.TYPE_CLASS + usb.RECIP_INTERFACE en getData.py
  uint8_t bRequest = 0x09; //usb request, set config
  uint8_t wValLo = 0x00;
  uint8_t wValHi = 0x02;
  uint16_t wInd = 0;
  uint16_t total = 8; //rqstBuffer.size()

  //Buffer a envíar a la estación
  uint8_t rqstBuffer[8];
  rqstBuffer[0] = 0xA1;                  // READ COMMAND
  rqstBuffer[1] = (char)(offset / 256);  // READ ADDRESS HIGH
  rqstBuffer[2] = (char)(offset & 0xFF); // READ ADDRESS LOW
  rqstBuffer[3] = 0x20;                  // END MARK
  rqstBuffer[4] = 0xA1;                  // READ COMMAND
  rqstBuffer[5] = (char)(offset / 256);  // READ ADDRESS HIGH
  rqstBuffer[6] = (char)(offset & 0xFF); // READ ADDRESS LOW
  rqstBuffer[7] = 0x20;                  // END MARK

  uint16_t BUFFER_SIZE = IN_BUFFER_SIZE; //kBufferSize; //32
  uint8_t readBuffer[BUFFER_SIZE];
  uint8_t *output = new uint8_t[BUFFER_SIZE];

  Serial.print(bmReqType);
  Serial.print(", ");
  Serial.print(bRequest);
  Serial.print(", ");
  for (int i = 0; i < 8; i++)
  {
    Serial.print(rqstBuffer[i]);
    Serial.print(", ");
  }
  //Leer y descartar lo anterior, esto venía en el ejemplo en C
  //( *( work->medium.usbhidRead ) )( &work->medium, newBuffer, 32, 500 );
  //Usb.inTransfer(usb_addr, endPoint, &BufferSize, readBuffer);
  Serial.println("Control Request");
  uint8_t result;
  result = Usb.ctrlReq(usb_addr,                          //Address
                       endPoint,                          //usb Endpoint
                       bmReqType,                         //bmReqType, //33 (ejemplo ps3 bmREQ_HID_OUT, es similar al de wview )
                       bRequest,                          //09 set configuration
                       wValLo,                            //bConfigurationValue 0x0000200: 00 02 o 02 00
                       wValHi,                            //bConfigurationValue este o el de arriba
                       wInd,                              //siempre es 0 inguesu
                       total,                             //bytes?
                       sizeof(rqstBuffer) / sizeof(byte), //bytes a leer
                       rqstBuffer,                        //puntero del bufer de request
                       NULL);                             //Timeout?
  Serial.println(result, HEX);
  //Esperamos 7 ms
  delay(8);
  //endPoint = 1;
  //Intentamos leer
  while (true)
  {
    BUFFER_SIZE = IN_BUFFER_SIZE;
    Serial.println("In transfer");
    result = Usb.inTransfer(usb_addr, endPoint, &BUFFER_SIZE, readBuffer, 500);
    Serial.print(usb_addr);
    Serial.print(", ");
    Serial.print(endPoint);
    Serial.print(", ");
    Serial.print(BUFFER_SIZE);
    Serial.print(", ");
    Serial.println(result, HEX);
    if (result == 0)
    {
      Serial.println("Correctoooooooooooooooooooooooooooooooooooooooooooooooooo");
      break;
    }
    if (result == 0x0D || result == 0xD6 || result == 0xDB)
    {
      //break;
    }
    if (result == 0x0E)
    {
      //break;
    }
  }
  //Serial.println(result, HEX);
  delay(8);
  //Prueba de salida de una lectura unica
  Serial.print("ReadBuffer: ");
  for (int i = 0; i < BUFFER_SIZE; i++)
  {
    // Serial.print(" ");
    // Serial.print(readBuffer[i]);
    // Serial.print(" ");
    output[i] = readBuffer[i];
  }
  //Serial.println(".");
  return output;
}