#ifndef usbhub
#include <usbhub.h>
#endif
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

uint8_t *DeviceReader::read_usb_block(USB Usb, int address)
{
  Serial.println("Iniciando lectura de usb: read_usb_block");
  Serial.print("address: ");
  Serial.println(address);
  int ptr = address;
  uint8_t usb_addr = 1; //Dirección en el usb host
  uint8_t endPoint = 81; //81 en el descriptor de usb y script 0
  uint8_t bmReqType = 33; //usb.TYPE_CLASS + usb.RECIP_INTERFACE en getData.py
  uint8_t bRequest = 0x09;
  uint8_t wValLo = 0x00;;
  uint8_t wValHi = 0x02;
  uint16_t wInd  = 0;
  uint16_t total;

  //Buffer a envíar a la estación
  uint8_t rqstBuffer[8];
  rqstBuffer[0] = 0xA1;               // READ COMMAND
  rqstBuffer[1] = (char)(ptr / 256);  // READ ADDRESS HIGH
  rqstBuffer[2] = (char)(ptr & 0xFF); // READ ADDRESS LOW
  rqstBuffer[3] = 0x20;               // END MARK
  rqstBuffer[4] = 0xA1;               // READ COMMAND
  rqstBuffer[5] = (char)(ptr / 256);  // READ ADDRESS HIGH
  rqstBuffer[6] = (char)(ptr & 0xFF); // READ ADDRESS LOW
  rqstBuffer[7] = 0x20;               // END MARK
  total = 8;                          //rqstBuffer.size()

  uint16_t BufferSize = kBufferSize;
  uint8_t readBuffer[BufferSize];
  uint8_t *output = new uint8_t[BufferSize];
  //Serial.println("requestBuffer:");
  //Serial.println("Termina requestBuffer");
  /*Usb.ctrlReq(usb_address,  //address
                endPoint, //endPoint
                321, //requestType
                0x0000009, //request             
                0, //wValLo
                512, //wValHi
                0, //wInd?
                0x20, //total
                sizeof(rqstBuffer)/sizeof(byte), //size?                
                rqstBuffer, //data
                NULL); //timeout o parser?*/
  //uint16_t* parser = 0;
  Serial.print(bmReqType);
  Serial.print(bRequest);
  for (int i =0; i<8;i++){    
    Serial.print(rqstBuffer[i]);
    Serial.print(" ");
  }



  Serial.println("Control Request");
  Usb.ctrlReq(usb_addr, endPoint, bmReqType, bRequest, wValLo, wValHi, wInd, total, sizeof(rqstBuffer) / sizeof(byte), rqstBuffer, NULL);
  Serial.println("In transfer");
  Usb.inTransfer(usb_addr, endPoint, &BufferSize, readBuffer);
  //Prueba de salida de una lectura unica
  //Serial.print("ReadBuffer: ");
  for (int i = 0; i < BufferSize; i++)
  {
    // Serial.print(" ");
    // Serial.print(readBuffer[i]);
    // Serial.print(" ");
    output[i] = readBuffer[i];
  }
  //Serial.println(".");
  return output;
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
    Serial.println("USB_STATE_RUNNING > read_block, ptr:" + 1);
    read_fixed_block(Usb);
  }
}
// Read block repeatedly until it's stable. This avoids getting corrupt
// data when the block is read as the station is updating it.
uint8_t *DeviceReader::read_block(USB Usb, int ptr, bool retry)
{
  bool flag = false;
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
        Serial.print(new_block[i]);
        Serial.print(" ");
      }
      Serial.println(".");

      //Prueba de salida de una lectura anterior
      Serial.print("Old Buffer: ");
      for (int i = 0; i < kBufferSize; i++)
      {
        //Serial.print(i);
        Serial.print(" ");
        Serial.print(old_block[i]);
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
          // They match and we are done:
          //
          //readDone = TRUE;
          Serial.println("Lecturas iguales, array igual");
          flag = true;
          break;
        }
        else
        {
          Serial.println("WH1080: readBlock buffer still changing");
          //Serial.println("Elemento diferente");
          memcpy(old_block, new_block, 32);
          flag = false;
          //break;
        }
      }
    }
  }
  return new_block;
}
uint8_t *DeviceReader::read_fixed_block(USB Usb)
{
  uint8_t *fixed_block = new uint8_t[kBufferSize];
  uint8_t new_block[kBufferSize];

  //Copiamos lo que devuelve read_block a fixed_block
   //Que es block, el ejemplo de C muestra un puntero pero no sé que es?
   //En el ejemplo en C, el uno es un cero  
  memcpy(new_block, read_block(Usb, 0, true), kBufferSize); 
      
  //Llenamos el arreglo
  // for (int i = 0; i < kBufferSize; i++)
  // {
  //   fixed_block[i] = 0;
  // }
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
    //Valio madres, no se leeyo correctamente, restablecer la estación
    //return ERROR_ABORT;
  }  
}
