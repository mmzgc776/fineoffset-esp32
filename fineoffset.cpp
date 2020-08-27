#include "fineoffset.h"
#include <stdio.h>
#include <string.h>
#include <usbhid.h>
#include <hiduniversal.h>
// #include <hidescriptorparser.h>

//Se pretende adaptar los controladores de FineOffset WH1080
//Aquí están las funciones básicas para la implementación de un extractor por USB
//Moisés González Castellanos 04-04-20
DeviceReader::DeviceReader() {}

//Abrimos el puerto y lo dejamos disponible
void DeviceReader::openPort(USB Usb)
{
  Serial.println("Start USB");
  Serial.println("Clase DeviceReader/openport");
  find_device(Usb);
}

//Find the vendor and product ID on the USB.
void DeviceReader::find_device(USB Usb)
{
  // HIDUniversal2 Hid(&Usb);
  // UniversalReportParser Uni;
  //Marcas de control
  Serial.println("Clase DeviceReader/usb.task()");

  //Se inicia el host usb
  //Usb.Task();
  Serial.println("Clase DeviceReader/find_device");
  if (Usb.getUsbTaskState() == USB_STATE_RUNNING)
  {
    Serial.println("USB_STATE_RUNNING > read_block, ptr");

    //Device reset aparentemente solo necesario sobre windows
    //Trataremos de imitar la negociación que realiza el controlador en windows.
    //Terminamos por la linea 165...

    //Device Request Get Descriptor: Configuration 0 (59bytes)
    // Serial.println("Get Descriptor: Configuration 0 (59bytes)");

    // uint8_t conf=1; // Que significa este parametro de configuracion?
    // uint8_t rcode;
    // uint16_t total = 59; //rqstBuffer.size()
    // uint8_t confDescData59[total] ; //bytes que leeremos de los descriptores
    // rcode = Usb.getConfDescr(usb_addr, endPoint, total, conf, confDescData59);
    // Serial.print("rcode=");
    // Serial.println(rcode, HEX);
    // for (int i = 0; i < total; i++)
    // {
    //   //Serial.print(i);
    //   Serial.print(" ");
    //   Serial.print(confDescData59[i], HEX);
    //   Serial.print(" ");
    // }
    // Serial.println(".");

    // delay(70);
    // //Device Request Get Descriptor: Device 0 (18bytes)
    // Serial.println("Get Descriptor: Device 0 (18bytes)");
    // total = 18; //rqstBuffer.size()
    // uint8_t devDescData[total] ; //bytes que leeremos de los descriptores
    // rcode = Usb.getDevDescr(usb_addr, endPoint, total, devDescData);
    // Serial.print("rcode=");
    // Serial.println(rcode, HEX);
    // for (int i = 0; i < total; i++)
    // {
    //   //Serial.print(i);
    //   Serial.print(" ");
    //   Serial.print(devDescData[i], HEX);
    //   Serial.print(" ");
    // }
    // Serial.println(".");

    // //Device Request Get Descriptor: Configuration 0 (9bytes)
    // Serial.println("Get Descriptor: Configuration 0 (9bytes)");
    // conf=1;
    // total = 9; //rqstBuffer.size()
    // uint8_t confDescData[total] ; //bytes que leeremos de los descriptores
    // rcode = Usb.getConfDescr(usb_addr, endPoint, total, conf, confDescData);
    // Serial.print("rcode=");
    // Serial.println(rcode, HEX);
    // for (int i = 0; i < total; i++)
    // {
    //   //Serial.print(i);
    //   Serial.print(" ");
    //   Serial.print(confDescData[i], HEX);
    //   Serial.print(" ");
    // }
    // Serial.println(".");

    // //Device Request Get Descriptor: Configuration 0 (34bytes)
    // Serial.println("Get Descriptor: Configuration 0 (34bytes)");
    // conf=1;
    // total = 34; //rqstBuffer.size()
    // uint8_t confDescData34[total] ; //bytes que leeremos de los descriptores
    // rcode = Usb.getConfDescr(usb_addr, endPoint, total, conf, confDescData34);
    // Serial.print("rcode=");
    // Serial.println(rcode, HEX);
    // for (int i = 0; i < total; i++)
    // {
    //   //Serial.print(i);
    //   Serial.print(" ");
    //   Serial.print(confDescData34[i], HEX);
    //   Serial.print(" ");
    // }
    // Serial.println(".");

    // //Intentamos ajustar la configuración a 1 hid?  y luego continuamos
    // Serial.println("Escribimos la configuracion a 1");
    // uint8_t retval = Usb.setConf(usb_addr, endPoint, 1);

    // Serial.println(retval, HEX);
    // // delay(45);
    // // Serial.println("Delay 90ms");
    // // delay(45);
    // //Controlrequest 8 bytes
    // Serial.println("Control Request 8b");
    // uint8_t bmReqType = 0x21;  //Tomado de la traza del analizador
    // uint8_t bRequest = 0x0A; //usb request, get desc Tomado de la traza del analizador
    // uint8_t wValLo = 0x00;
    // uint8_t wValHi = 0x00;
    // uint16_t wInd = 0;
    // total = 0; //rqstBuffer.size()
    // //uint8_t rqstBuffer[total];
    // rcode = Usb.ctrlReq(usb_addr,                          //Address
    //                      endPoint,                          //usb Endpoint
    //                      bmReqType,                         //bmReqType, //33 (ejemplo ps3 bmREQ_HID_OUT, es similar al de wview )
    //                      bRequest,                          //09 set configuration
    //                      wValLo,                            //bConfigurationValue 0x0000200: 00 02 o 02 00
    //                      wValHi,                            //bConfigurationValue este o el de arriba
    //                      wInd,                              //siempre es 0 inguesu
    //                      total,                             //bytes?
    //                      NULL, //bytes a leer
    //                      NULL,                        //puntero del bufer de request
    //                      NULL);                             //Timeout? Parser!
    // //Esto solo espera un rcode de 0 pero sin respuesta
    // Serial.print("rcode=");
    // Serial.println(rcode, HEX);

    // //Recibir 52 bytes DeviceRequest get descriptor unknown
    // Serial.println("Control Request 52 b");
    // bmReqType = 0x81;  //Tomado de la traza del analizador
    //  bRequest = 0x06; //usb request, get desc Tomado de la traza del analizador
    //  wValLo = 0x00;
    //  wValHi = 0x22;
    //  wInd = 0;
    // total = 52; //rqstBuffer.size()
    // uint8_t rqstBuffer52[total];

    // rcode = Usb.ctrlReq(usb_addr,                          //Address
    //                      endPoint,                          //usb Endpoint
    //                      bmReqType,                         //bmReqType, //33 (ejemplo ps3 bmREQ_HID_OUT, es similar al de wview )
    //                      bRequest,                          //09 set configuration
    //                      wValLo,                            //bConfigurationValue 0x0000200: 00 02 o 02 00
    //                      wValHi,                            //bConfigurationValue este o el de arriba
    //                      wInd,                              //siempre es 0 inguesu
    //                      total,                             //bytes?
    //                      sizeof(rqstBuffer52) / sizeof(byte), //bytes a leer
    //                      rqstBuffer52,                        //puntero del bufer de request
    //                      NULL);                             //Timeout? Parser!
    // Serial.print("rcode=");
    // Serial.println(rcode, HEX);

    // for (int i = 0; i < total; i++)
    // {
    //   //Serial.print(i);
    //   Serial.print(" ");
    //   Serial.print(rqstBuffer52[i], HEX);
    //   Serial.print(" ");
    // }
    // Serial.println(".");
    delay(3000);    
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
  endPoint = 0;
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

  //Leer y descartar lo anterior, esto venía en el ejemplo en C
  //( *( work->medium.usbhidRead ) )( &work->medium, newBuffer, 32, 500 );
  //Usb.inTransfer(usb_addr, endPoint, &BufferSize, readBuffer);
  Serial.println("Control Request:");
  Serial.print(bmReqType);
  Serial.print(", ");
  Serial.print(bRequest);
  Serial.print(", ");
  for (int i = 0; i < 8; i++)
  {
    Serial.print(rqstBuffer[i]);
    Serial.print(", ");
  }
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
  Serial.print("rcode=");
  Serial.println(result, HEX);
  //Esperamos 7 ms 15 ms en la traza del analizador, amole poniendo poquito más
  delay(7);
  //endPoint = 1;
  //Intentamos leer
  int outIndex = 0;
  while (true)
  {
    //Leer hasta que tengamos 32 bytes de datos
    Serial.println("In transfer: ");
    //Haremos 4 lecturas
    for (int read = 0; read < 4; read++)
    {
      //de 8 bytes cada una
      //result = Usb.inTransfer(usb_addr, 1, &BUFFER_SIZE, readBuffer, 8);
      Serial.print("Intentando leer bytes: ");
      result = Usb.inTransfer(usb_addr, 1, &BUFFER_SIZE, readBuffer, 8);
      Serial.print("rcode=");
      Serial.println(result, HEX);
      //Si sale bien
      if (result == 0)
      {
        Serial.println("Correctoooooooooooooooooooooooooooooooooooooooooooooooooo");
        //Asignar los bytes al output
        for (int index = 0; index < 32; index++)
        {          
          Serial.print(readBuffer[index],HEX);
          Serial.print(" ");
          output[outIndex] = readBuffer[index];
          outIndex++;
        }
      }
      //Reset buffer size
      BUFFER_SIZE = IN_BUFFER_SIZE;
      //Manejo de errores
      //hrJERR                                          0x0D
      //USB_ERROR_ADDRESS_NOT_FOUND_IN_POOL             0xD6
      //USB_ERROR_EP_NOT_FOUND_IN_TBL                   0xDB
      if (result == 0x0D || result == 0xD6 || result == 0xDB)
      {
        delay(8);
        //break;
      }
      if (result == 0x0E)
      {
        delay(8);
        //break;
      }
    }
    //Si el indice de lectura llegó a 32 fuga
    if (outIndex >= 32)
    {
      Serial.println("Listo");
      break;
    }
  }
  Serial.println("Lectura completa o error");
  for (int index = 0; index < 32; index++)
        {
          Serial.print(output[index]);
          Serial.print(" ");
        }
  Serial.println(".");       

  
  delay(8);
  return output;
}