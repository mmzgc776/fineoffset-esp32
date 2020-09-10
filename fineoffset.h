#include <usbhid.h>
#include <hiduniversal.h>
#include <stdio.h>
#include <string.h>

#define IN_BUFFER_SIZE 32

//Tratare de implementar WH1080_WORK en mi implementación
// define the readings collector
//Definiciones agregadas para poder adaptar las funciones
#define ERROR 1
#define OK 0
#define ERROR_ABORT 2
#define ARCHIVE_VALUE_NULL          -100000

/* WH1080  <vendorid, productid> */
#define WH1080_VENDOR_ID 0x1941
#define WH1080_PRODUCT_ID 0x8021

// Define the rain rate acuumulator period (minutes):
#define WH1080_RAIN_RATE_PERIOD 5

// Weather Station buffer parameters:
#define WH1080_RAIN_MAX 0x10000   // Wrap value for rain counter
#define WH1080_BUFFER_START 0x100 // Size of fixed block
// start of buffer records
#define WH1080_BUFFER_CHUNK 0x20 // Size of chunk received over USB

// Weather Station record memory positions:
#define WH1080_DELAY 0           // Position of delay parameter
#define WH1080_HUMIDITY_IN 1     // Position of inside humidity parameter
#define WH1080_TEMPERATURE_IN 2  // Position of inside temperature parameter
#define WH1080_HUMIDITY_OUT 4    // Position of outside humidity parameter
#define WH1080_TEMPERATURE_OUT 5 // Position of outside temperature parameter
#define WH1080_ABS_PRESSURE 7    // Position of absolute pressure parameter
#define WH1080_WIND_AVE 9        // Position of wind direction parameter
#define WH1080_WIND_GUST 10      // Position of wind direction parameter
#define WH1080_WIND_DIR 12       // Position of wind direction parameter
#define WH1080_RAIN 13           // Position of rain parameter
#define WH1080_STATUS 15         // Position of status parameter

// Control block offsets:
#define WH1080_SAMPLING_INTERVAL 16 // Position of sampling interval
#define WH1080_DATA_COUNT 27        // Position of data_count parameter
#define WH1080_CURRENT_POS 30       // Position of current_pos parameter

int numLectura = 0;


//the wview daemon work area
typedef struct
{
    pid_t           myPid;
    //WVIEW_MEDIUM    medium;
    void*            stationData;               // station-specific data store
    int             runningFlag;
    //RAD_THREAD_ID   threadId;                   // Non-NULL if station uses threads

    //int             stationGeneratesArchives;
    //char            pidFile[WVIEW_MAX_PATH];
    //char            fifoFile[WVIEW_MAX_PATH];
    //char            statusFile[WVIEW_MAX_PATH];
    char            stationType[64];
    char            stationInterface[16];
    //char            stationDevice[WVIEW_MAX_PATH];
    char            stationHost[256];
    int             stationPort;
    int             stationIsWLIP;
    int             stationToggleDTR;
    int             stationRainSeasonStart;
    float           stationRainStormTrigger;
    int             stationRainStormIdleHours;
    float           stationRainYTDPreset;
    float           stationETYTDPreset;
    int             stationRainETPresetYear;
    uint16_t        archiveInterval;
    int16_t         latitude;
    int16_t         longitude;
    int16_t         elevation;
    time_t          archiveDateTime;
    time_t          nextArchiveTime;            // detect system clock changes
    // TIMER_ID        archiveTimer;
    // TIMER_ID        cdataTimer;
    // TIMER_ID        pushTimer;
    // TIMER_ID        syncTimer;
    // TIMER_ID        ifTimer;
    uint32_t        cdataInterval;
    uint32_t        pushInterval;
    //SENSOR_STORE    sensors;
    //LOOP_PKT        loopPkt;
    //LOOP_PKT        lastLoopPkt;
    int             numReadRetries;
    int             archiveRqstPending;

    // Calibration:
    float           calMBarometer;
    float           calCBarometer;
    float           calMPressure;
    float           calCPressure;
    float           calMAltimeter;
    float           calCAltimeter;
    float           calMInTemp;
    float           calCInTemp;
    float           calMOutTemp;
    float           calCOutTemp;
    float           calMInHumidity;
    float           calCInHumidity;
    float           calMOutHumidity;
    float           calCOutHumidity;
    float           calMWindSpeed;
    float           calCWindSpeed;
    float           calMWindDir;
    float           calCWindDir;
    float           calMRain;
    float           calCRain;
    float           calMRainRate;
    float           calCRainRate;

    // Alert Emails:
    int             IsAlertEmailsEnabled;
    //char            alertEmailFromAdrs[WVIEW_STRING1_SIZE];
    //char            alertEmailToAdrs[WVIEW_STRING1_SIZE];
    int             IsTestEmailEnabled;

    int             showStationIF;

    // Debug:
    int             DebugStationInput;
    int             DebugStationOutput;
    int             DebugStationByteCount;

    int             UsbRawBytes;
    int             StreamBytes;
    int             PacketBytes;
    int             ChecksumBytes;
    int             BadLengthBytes;
    int             StatCount;
    int             UnknownPacketType;

    int             exiting;

} WVIEWD_WORK;

// Types for decoding raw weather station data.
//	ub 	unsigned byte
//	sb	signed byte
//	us unsigned short
//	ss	signed short
//	dt	date time  bcd  yymmddhhmm
//	tt	time bcd  hhmm
//	pb	status - bit 6 lost contact - bit 7 rain counter overflow
//	wa	wind average low bits puls lower bits of address +2
//	wg	wind gust low bits plus upper bits of address +1
//	wd  wind direction

enum ws_types
{
  ub,
  sb,
  us,
  ss,
  dt,
  tt,
  pb,
  wa,
  wg,
  wd
};

#define WH1080_NUM_SENSORS 11

// Define the decoder type structure:
typedef struct _decodeType
{
  int pos;
  enum ws_types ws_type;
  float scale;
  float *var;
} WH1080_DECODE_TYPE;

// define the readings collector
typedef struct
{
  float delay;
  float inhumidity;
  float intemp;
  float outhumidity;
  float outtemp;
  float pressure;
  float windAvgSpeed;
  float windGustSpeed;
  float windDir;
  float rain;
  float status;
} WH1080_DATA;

// define the work area
typedef struct
{
  WH1080_DATA sensorData;
  uint8_t controlBlock[WH1080_BUFFER_CHUNK];
  uint8_t recordBlock[WH1080_BUFFER_CHUNK];
  int lastRecord;
} WH1080_WORK;

typedef struct
{
    int                 secondsInAccumulator;
    float               sum;
} WV_ACCUM, *WV_ACCUM_ID;

// define WH1080-specific interface data here
typedef struct
{
  int elevation;
  float latitude;
  float longitude;
  int archiveInterval;
  WH1080_DATA wh1080Readings;
  float totalRain;                 // to track cumulative changes
  //WV_ACCUM_ID rainRateAccumulator; // to compute rain rate
} WH1080_IF_DATA;

static WH1080_WORK wh1080Work;

class DeviceReader
{
public:
  //int PERADDR = 0xe0;
  DeviceReader();
  int ciclos = 0;
  int vendor_id = 1941;
  int product_id = 8021;
  uint8_t endPoint = 0; //81 en el descriptor de usb y script 0
  //int usb_endpoint = 81; // revisar para que sirve, evitar confusión
  const uint8_t usb_addr = 1; //Dirección en USB_desc 1 dirección del dispositivo en el hub???
  //int address = 1; //Direccion en memoria de la estacion
  const uint16_t kBufferSize = 32; //Esto estaba definido como 10, pero luego lo cambié a 16 observando la script de getData.py

  //Abrimos el puerto y lo dejamos disponible, basicamente llamamos a openPort()
  int openPort(USB Usb);
  //Se supone que esta función buscaría el puerto y direccion, pero al ser un solo adaptador se puede obviar (creo), dejando
  //las direcciones predeterminadas, revisa que esté listo el USB
  int find_device(USB Usb);
  //Llamada a uno de los metodos mas basicos de acceso a la estacion
  //Hace un control transfer y un intransfer para obtener datos desde USB,
  //Retorna un arreglo de 16 bytes, que debe ser la lectura
  //uint8_t *read_usb_block(USB Usb, int usb_address);
  //Funcion que debería llamar a read_usb_block
  uint8_t *read_block(USB Usb, int offset, uint8_t* block);
  //Funcion siguiente read_block, lee de 20 en 20 retorna un arreglo
  //cuando está correcto, usa los magic_numbers para validar
  //Ptr se llama block en el ejemplo de C, pero ptr en el ejemplo de python
  uint8_t *read_fixed_block(USB Usb, uint8_t* block);
  //Función adaptada de wview  
  //int readStationData(USB Usb, WVIEWD_WORK* work, int currentPosition);
  int readStationData(USB Usb,  int currentPosition);
  //Se prepara para comenzar las lecturas o envíos, es donde marcamos el indice de currentPos
  int getLastSentRecordfromFile();
  //Obtenemos la posicion actual del buffer circular de la estacion
  int getCurrentPosition(USB Usb);
  //Obtenemos los datos entre dos espacios de memoria
  int getData(int lastPosition, int currentPosition);
  //Obtenemos los datos de toda la memoria
  int getAllData(USB Usb);
};