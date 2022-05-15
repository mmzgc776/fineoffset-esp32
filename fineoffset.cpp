
#include <stdio.h>
#include <string.h>
#include <usbhid.h>
#include <hiduniversal.h>
#include <SSD1306.h>
#include "fineoffset.h"


//Contador de envíos
extern uint32_t lecturaNum = 0;

/* Funciones de wview*/
/************************************************************************/
// Local methods:
static uint16_t getUSHORT(char *raw)
{
    unsigned char lo = (unsigned char)raw[0];
    unsigned char hi = (unsigned char)raw[1];
    return lo + (hi * 256);
}
static int16_t getSHORT(char *raw)
{
    unsigned char lo = (unsigned char)raw[0];
    unsigned char hi = (unsigned char)raw[1];
    uint16_t us = lo + (hi * 256);
    if (us >= 0x8000)          // Test for sign bit
        return -(us - 0x8000); // Negative value
    else
        return us; // Positive value
}

static uint8_t bcdDecode(uint8_t byte)
{
    uint8_t lo = byte & 0x0F;
    uint8_t hi = byte / 16;
    return (lo + (hi * 10));
}

static int decodeSensor(char *raw, enum ws_types ws_type, float scale, float *var)
{
    float fresult;
    uint16_t usTemp;

    switch (ws_type)
    {
    case ub:
        if ((unsigned char)raw[0] == 0xFF)
        {
            // Deal with humidity < 10% problem by hard-coding to 9:
            fresult = 9;
        }
        else
        {
            //fresult = (unsigned char)raw[0] * scale;
            fresult = raw[0] * scale;
        }
        break;
    case us:
        usTemp = getUSHORT(raw);
        if (usTemp == 0xFFFF)
        {
            return ERROR;
        }
        fresult = usTemp * scale;
        break;
    case ss:
        if (((unsigned char)raw[0] == 0xFF) && ((unsigned char)raw[1] == 0xFF))
        {
            //Moy
            //Esto es incorrecto
            //return ERROR;
        }
        fresult = getSHORT(raw) * scale;
        break;
    case pb:
        fresult = (unsigned char)raw[0];
        break;
    case wa:
        // wind average - 12 bits split across a byte and a nibble
        if (((unsigned char)raw[0] == 0xFF) && (((unsigned char)raw[2] & 0x0F) == 0x0F))
        {
            //Moy
            //Esto es incorrecto
            //return ERROR;
        }
        fresult = (unsigned char)raw[0] + (((unsigned char)raw[2] & 0x0F) * 256);
        fresult = fresult * scale;
        break;
    case wg:
        // wind gust - 12 bits split across a byte and a nibble
        if (((unsigned char)raw[0] == 0xFF) && (((unsigned char)raw[1] & 0xF0) == 0xF0))
        {
            //Moy
            //Esto es incorrecto
            //return ERROR;
        }
        fresult = (unsigned char)raw[0] + (((unsigned char)raw[1] & 0xF0) * 16);
        fresult = fresult * scale;
        break;
    case wd:
        if (((unsigned char)raw[0] & 0x80) == 0x80)
        {
            //Moy
            //Esto es incorrecto
            // fresult = (unsigned char)raw[0] * scale;
            // fresult *= 22.5;
            //return ERROR; //Valor invalido
        }
        fresult = (unsigned char)raw[0] * scale;
        fresult *= 22.5;
        break;
    default:
        fresult = ARCHIVE_VALUE_NULL;
        break;
    }
    *var = fresult;
    return OK;
}
    /* Funciones de wview*/
/************************************************************************/
 
//Se pretende adaptar los controladores de FineOffset WH1080
//Aquí están las funciones básicas para la implementación de un extractor por USB
//Moisés González Castellanos 04-04-20
DeviceReader::DeviceReader() {}

//Abrimos el puerto y lo dejamos disponible
void DeviceReader::openPort(USB Usb)
{
    Serial.println("Start USB Routine");
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
            
        //delay(3000);
        WVIEWD_WORK work;       
        currentPos=getCurrentPosition(Usb);
        //getLastSentRecordfromFile();
        //saveData();
        //Si no hay lectura valida esperamos un sec
        //while(true){
        //    readStationData(Usb, &work, currentPos);
        //}
        //Luego lo cambiamos
        //return OK;
    }
}

//Escribir una funcion de inicialización
//void DeviceReader::ConfigureDevice(USB Usb){}

uint8_t *DeviceReader::read_fixed_block(USB Usb, uint8_t *block)
{
    Serial.println("Clase DeviceReader/read_fixed_block");
    uint8_t *fixed_block = new uint8_t[IN_BUFFER_SIZE];
    uint8_t new_block[IN_BUFFER_SIZE] = {0};

    //Copiamos lo que devuelve read_block a fixed_block
    //Que es block, el ejemplo de C muestra un puntero pero no sé que es?
    //En el ejemplo en C, el uno es un cero
    //for (int mempos = 0x0000; mempos < 0x0100; mempos = mempos + 0x0020)
    //{
    Serial.println("leyendo las posiciones.");
    //Serial.println(IN_BUFFER_SIZE);
    //memcpy(new_block, read_block(Usb, mempos, true), kBufferSize);
    memcpy(new_block, read_block(Usb, 0, block), IN_BUFFER_SIZE);
    for (int i = 0; i < IN_BUFFER_SIZE; i++)
    {
        Serial.print(new_block[i], HEX);
        Serial.print("-");
    }
    Serial.println(".");
    // Check for valid magic numbers:
    // This is hardly an exhaustive list and I can find no definitive
    // documentation that lists all possible values; further, I suspect it is
    // more of a header than a magic number...
    //Serial.println(new_block[0]);
    if ((new_block[0] == 0x55) || (new_block[0] == 0xFF) || (new_block[0] == 0x01) ||
        ((new_block[0] == 0x00) && (new_block[1] == 0x1E)) ||
        ((new_block[0] == 0x00) && (new_block[1] == 0x01)))
    //if (new_block[0] == 0x55)
    {
        //Retornar el valor del bloque, creo, hay que entender la funcion
        Serial.println("Listo para retornar");
        //return OK;
        return fixed_block;
    }
    else
    {
        Serial.println("WH1080: readFixedBlock bad magic number %2.2X %2.2X");
        //Serial.println((int)new_block[0]);
        //Serial.println((int)new_block[1]);
        Serial.println("WH1080: You may want to clear the memory on the station "
                       "console to remove any invalid records or data...");
        //delay(5000);
        //Valio madres, no se leeyo correctamente, restablecer la estación
        //return ERROR_ABORT;
    }
    //}
}

// Read block repeatedly until it's stable. This avoids getting corrupt
// data when the block is read as the station is updating it.
uint8_t *DeviceReader::read_block(USB Usb, int offset, uint8_t *block)
{
    Serial.println("Iniciando lectura de bloque: read_block");

    bool writeDone = false, readDone = false, firstTime = true;
    uint16_t BUFFER_SIZE = IN_BUFFER_SIZE; //kBufferSize; //32
    uint8_t readBuffer[IN_BUFFER_SIZE], old_block[IN_BUFFER_SIZE], new_block[IN_BUFFER_SIZE];

    // uint8_t readBuffer[BUFFER_SIZE];
    // uint8_t *output = new uint8_t[BUFFER_SIZE];

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

    //Leer y descartar lo anterior, esto venía en el ejemplo en C
    //( *( work->medium.usbhidRead ) )( &work->medium, newBuffer, 32, 500 );
    //Usb.inTransfer(usb_addr, endPoint, &BufferSize, readBuffer);
    Serial.print("Control Request: ");
    Serial.print(bmReqType);
    Serial.print(", ");
    Serial.print(bRequest);
    Serial.print(", ");
    for (int i = 0; i < 8; i++)
    {
        Serial.print(rqstBuffer[i], HEX);
        Serial.print(", ");
    }
    Serial.println(" ");
    while (!readDone)
    {
        delay(7);
        uint8_t result;
        if (!writeDone)
        {
            //Serial.println("Control Request:");
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
            //Serial.print("rcode=");
            //Serial.println(result, HEX);
            writeDone = true;
        }
        //Esperamos 7 ms 15 ms en la traza del analizador, amole poniendo poquito más
        if (result == 0)
        {
            //Volteamos esta variable solo para usarla de nuevo como bandera
            result = 1;
            //Haremos 5 lecturas 4 bastan, pero va una de más
            for (int read = 0; read < 5; read++)
            {
                delay(7);
                //Intentamos leer
                //Leer hasta que tengamos 32 bytes de datos
                //Serial.println("In transfer: ");                
                //result = Usb.inTransfer(usb_addr, 1, &BUFFER_SIZE, readBuffer, 8);
                Serial.print("Intentando leer bytes: ");
                //Incrementé el tiempo de 8 a 1000                
                Serial.println(BUFFER_SIZE);
                result = Usb.inTransfer(usb_addr, 1, &BUFFER_SIZE, readBuffer, 1000);
                //Serial.println(BUFFER_SIZE);
                //Restablecemos buffer_Size
                BUFFER_SIZE=IN_BUFFER_SIZE;
                //Serial.print("rcode=");
                //Serial.println(result, HEX);
                //Si sale bien
                if (result == 0)
                {                    
                    //Asignar los bytes al output
                    // Compare to previous result:
                    Serial.print("ReadBuffer: ");
                    for (int i = 0; i < IN_BUFFER_SIZE; i++)
                    {
                        //Serial.print(i);
                        Serial.print(readBuffer[i], HEX);
                        new_block[i]=readBuffer[i];
                        Serial.print(" ");
                    }
                    Serial.println(" ");
                    break;              
                }
            }
            if (!firstTime)
                    {   
                        //Serial.println("Comparamos");
                        if (memcmp(old_block, new_block, 32) == 0)
                        {
                            // They match and we are done:
                            memcpy(block, new_block, 32);
                            //Fuga
                            //Serial.println("Listo");
                            readDone = true;
                        }
                        else
                        {
                            Serial.println("WH1080: readBlock buffer still changing");
                        }
                    }
                    else
                    {
                        //Serial.println("Volvemos a escribir-leer");
                        writeDone = false;
                        firstTime = false;
                    }
            memcpy(old_block, new_block, 32);
        }
        //Manejo de errores  
        //---------Sería bueno programar el caso de una desconeccion subita, para hacerlo tolerante a esta falla
        if (result == 0x0D || result == 0xD6 || result == 0xDB)
        {
            //hrJERR                                          0x0D
            //USB_ERROR_ADDRESS_NOT_FOUND_IN_POOL             0xD6
            //USB_ERROR_EP_NOT_FOUND_IN_TBL                   0xDB
            delay(8);
            //break;
        }
        //Este no recuerdo que es, pero sé que es un error
        if (result == 0x0E)
        {
            delay(8);
            //break;
        }
    }
    //Serial.println("Lectura completa o error");
    delay(8);
    return block;
}

//*************************************************************************************************************************************
//Funciones tomadas  de wview
static WH1080_WORK wh1080Work;

//Funcion de inicialización mia:
//Aquí debemos ir leyendo el lastrecord para saber donde nos quedamos,
// la primera vez es -1
//Aquí debemos poner la funcion que inicialice y continue despues de dormir
int DeviceReader::setLastSentRecord(int readcount, int stmempos){
    //Si es la primera vez
    if (readcount==0){
        //Comenzar desde el más antiguo
         wh1080Work.lastRecord = -1;
    }
    //Comenzar desde el ultimo enviado, y corregir
    else{
        wh1080Work.lastRecord = stmempos;
    }
}

int DeviceReader::getCurrentPosition(USB Usb){
    //Leemos el bloque fijo
    read_fixed_block(Usb, &wh1080Work.controlBlock[0]);
    // Get the current record position; the WH1080 reports the record it is
    // building, thus if it changes we need the prior just finished record:
    return (int)getUSHORT((char *)&wh1080Work.controlBlock[WH1080_CURRENT_POS]);
}

int DeviceReader::getAllData(USB Usb){
    int currentPosition;
    //Nos preparamos para extraer datos, obtenemos los datos del bloque fijo
    //getLastSentRecordfromFile();
    Serial.print("Last sent record: ");
    Serial.println(wh1080Work.lastRecord);
    currentPosition = getCurrentPosition(Usb);        
    Serial.print("Current pos: ");
    Serial.println(currentPosition);
}


// Define the position, decode type, conversion factor and the storage variable
// for the station data retrieved via USB:
static WH1080_DECODE_TYPE decodeVals[] =
    {
        {0, ub, 1.0, &wh1080Work.sensorData.delay},          // Minutes since last stored reading (1:240)
        {1, ub, 1.0, &wh1080Work.sensorData.inhumidity},     // Indoor relative humidity % (1:99), 0xFF means invalid
        {2, ss, 0.1, &wh1080Work.sensorData.intemp},         // Indoor temp; Multiply by 0.1 to get C (-40:+60), 0xFFFF means invalid
        {4, ub, 1.0, &wh1080Work.sensorData.outhumidity},    // Outdoor relative humidity % (1:99), 0xFF means invalid
        {5, ss, 0.1, &wh1080Work.sensorData.outtemp},        // Outdoor temp; Multiply by 0.1 to get C (-40:+60) , 0xFFFF means invalid
        {7, us, 0.1, &wh1080Work.sensorData.pressure},       // Pressure; Multiply by 0.1 to get hPa (920:1080), 0xFFFF means invalid
        {9, wa, 0.1, &wh1080Work.sensorData.windAvgSpeed},   // Avg Wind; Multiply by 0.1 to get m/s (0:50), 0xFF means invalid
        {10, wg, 0.1, &wh1080Work.sensorData.windGustSpeed}, // Gust Wind; Multiply by 0.1 to get m/s (0:50), 0xFF means invalid
        {12, wd, 1.0, &wh1080Work.sensorData.windDir},       // Wind Dir; Multiply by 22.5 (0-15), 7th bit indicates invalid data
        {13, us, 0.3, &wh1080Work.sensorData.rain},          // Rain; Multiply by 0.33 to get mm
        {15, pb, 1.0, &wh1080Work.sensorData.status}         // Status; 6th bit indicates loss of contact with sensors, 7th bit indicates rainfall overflow
};

// Returns:
// OK - if new record retrieved
// ERROR - if there was an interface error
// ERROR_ABORT - if there is no new record (WH1080 generates new records at
//               best once a minute)
int DeviceReader::readStationData(USB Usb, WVIEWD_WORK *work, int currentPosition)
{
    WH1080_IF_DATA *ifWorkData = (WH1080_IF_DATA *)work->stationData;
    int readPosition, index, retVal;

    // if( ( *( work->medium.usbhidInit ) )( &work->medium ) != OK )
    // {
    //     return ERROR;
    // }

    // Read the WH1080 fixed block:
    //read_fixed_block(Usb, &wh1080Work.controlBlock[0]);
    //retVal = read_fixed_block( Usb, &wh1080Work.controlBlock[0] );
    // if( retVal == ERROR_ABORT )
    // {
    //     // Try again later (bad magic number):
    //     // ( *( work->medium.usbhidExit ) )( &work->medium );
    //     return ERROR_ABORT;
    // }
    // else if( retVal == ERROR )
    // {
    //     // USB interface error:
    //     // ( *( work->medium.usbhidExit ) )( &work->medium );
    //     return ERROR;
    // }

    // Get the current record position; the WH1080 reports the record it is
    // building, thus if it changes we need the prior just finished record:
    Serial.print("Fixed Current pos: ");
    Serial.println(currentPosition);
    // Make sure the index is aligned on 16-byte boundary:
    if ((currentPosition % 16) != 0)
    {
        // bogus, try again later:
        // ( *( work->medium.usbhidExit ) )( &work->medium );
        return ERROR_ABORT;
    }

    // Is this the first time?
    Serial.print("Primera vez? (-1) ");
    //Hay que manejar lastRecord en la rutina de init 
    Serial.println(wh1080Work.lastRecord);
    if (wh1080Work.lastRecord == -1)
    {
        // Yes.
        wh1080Work.lastRecord = currentPosition;
        // ( *( work->medium.usbhidExit ) )( &work->medium );
        return ERROR_ABORT; 
    }

    // Is there a new record?
    //Hay que manejar lastRecord en la rutina de init 
    // if (currentPosition == wh1080Work.lastRecord)
    // {
    //     // No, wait till it is finished.
    //     // ( *( work->medium.usbhidExit ) )( &work->medium );
    //     return ERROR_ABORT;
    // }

    // Read last record that is now complete:
    Serial.print("Tratando de leer el lastRecord: ");
    Serial.println(wh1080Work.lastRecord - (16 * lecturaNum));
    //Esta será la nueva posición, el ultimo registro más el numero de lectura * 16 
    readPosition = wh1080Work.lastRecord - (16 * lecturaNum);
    if (!read_block(Usb, readPosition, &wh1080Work.recordBlock[0]))
    {
        Serial.print("WH1080: read data block at index %d failed!");
        Serial.println(wh1080Work.lastRecord);
        // ( *( work->medium.usbhidExit ) )( &work->medium );
        return ERROR;
    }
    // ( *( work->medium.usbhidExit ) )( &work->medium );
    Serial.print("Pos: ");
    Serial.println(readPosition);
    wh1080Work.lastRecord = currentPosition;

    //radMsgLogData(wh1080Work.recordBlock, 32);
    // Hace comparacioon entre el status (0x40(64) & wh1080Work.recordBlock[WH1080_STATUS](recordBlock[15]==FF)) = 1 segun esto invalido...
    // Is the record valid? Check for unpopulated record or no sensor data
    // received status bit:
    // if ((wh1080Work.recordBlock[WH1080_STATUS] & 0x40) != 0)
    // {
    //     // No!
    //     Serial.println("WH1080: data block at index %d has bad status, ignoring the record");
    //     Serial.println(readPosition);
    //     return ERROR_ABORT;
    // }

    // Parse the data received:
    for (index = 0; index < WH1080_NUM_SENSORS; index++)
    {
        if (decodeSensor((char *)&wh1080Work.recordBlock[decodeVals[index].pos],
                         decodeVals[index].ws_type,
                         decodeVals[index].scale,
                         decodeVals[index].var) != OK)
        {
            // Bad sensor data, abort this cycle:
            Serial.println("WH1080: data block at index %d has bad sensor value, ignoring the record");
            Serial.println(index);
            Serial.println(wh1080Work.recordBlock[decodeVals[index].pos], HEX);
            return ERROR_ABORT;
        }
    }
    Serial.print("Delay:   ");
    Serial.println(wh1080Work.sensorData.delay); //Ok
    Serial.print("InTemp:   ");
    Serial.println(wh1080Work.sensorData.intemp); //Ok
    Serial.print("InHum:   ");
    Serial.println(wh1080Work.sensorData.inhumidity); //Ok    
    Serial.print("Press:   ");
    Serial.println(wh1080Work.sensorData.pressure); //ok
    // Serial.println(wh1080Work.sensorData.outhumidity); //:(
    // Serial.println(wh1080Work.sensorData.outtemp);
    // Serial.println(wh1080Work.sensorData.windAvgSpeed);
    // Serial.println(wh1080Work.sensorData.windGustSpeed);
    // Serial.println(wh1080Work.sensorData.windDir);
    // Serial.println(wh1080Work.sensorData.rain);
    // Serial.println(wh1080Work.sensorData.status);
    
    //Contador de ciclos entre lectura y lectura a partir de wh1080Work.lastRecord
    Serial.print("Numero de lectura: ");
    Serial.println (lecturaNum);
    lecturaNum++;    
    return OK;
}