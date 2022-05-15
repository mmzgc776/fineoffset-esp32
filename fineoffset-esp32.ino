#include <usbhid.h>
#include <hiduniversal.h>
#include <hidescriptorparser.h>
#include <usbhub.h>
#include "pgmstrings.h"

// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>

//Pantalla
#include <SSD1306.h>

// #ifndef config
// #include "config.h"
// #endif

#include "lmicmodule.h"
#include "fineoffset.h"
#include "EEPROMAnything.h"

// #define OLED_I2C_ADDR 0x3C
// #define OLED_RESET 16
// #define OLED_SDA 4
// #define OLED_SCL 15

//Slave selects para spi
//Slave select para LORA
#define LORA_SS 18
//La libreria de Host fué modificada para adaptarse a esto
#define USB_SS 23


//Definiciones de clase hiduniversal para poder acceder al endpoint1
class HIDUniversal2 : public HIDUniversal
{
public:
  HIDUniversal2(USB *usb) : HIDUniversal(usb){};

protected:
  uint8_t OnInitSuccessful();
};

uint8_t HIDUniversal2::OnInitSuccessful()
{
  uint8_t rcode;

  HexDumper<USBReadParser, uint16_t, uint16_t> Hex;
  ReportDescParser Rpt;

  if ((rcode = GetReportDescr(0, &Hex)))
    goto FailGetReportDescr1;

  if ((rcode = GetReportDescr(0, &Rpt)))
    goto FailGetReportDescr2;

  return 0;

FailGetReportDescr1:
  USBTRACE("GetReportDescr1:");
  goto Fail;

FailGetReportDescr2:
  USBTRACE("GetReportDescr2:");
  goto Fail;

Fail:
  Serial.println(rcode, HEX);
  Release();
  return rcode;
}


USB Usb;
HIDUniversal2 Hid(&Usb);
UniversalReportParser Uni;
DeviceReader dr;

int ReadCount=0;
int StMempos;

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
  if (!Hid.SetReportParser(0, &Uni))
    ErrorMessage<uint8_t>(PSTR("SetReportParser"), 1);
  
  //Openfile
  //Get ReadCount and StMempos
  EEPROM_readAnything(0, ReadCount);
  EEPROM_readAnything(2, StMempos);
  Serial.println(ReadCount);
  Serial.println(StMempos);
  //Set StMempos for extraction
  //dr.setLastSentRecord(ReadCount, StMempos);
}

//Recibe un registro y lo envía a través de LORA
void SendRecord()
{
  bool finished = false;
  // Serial.println(digitalRead(LORA_SS));
  // Serial.println(digitalRead(USB_SS));
  loraSetup(ReadCount);
  while (!finished)
  {
    finished = loraLoop();
  }
  // Serial.println(digitalRead(LORA_SS));
  // Serial.println(digitalRead(USB_SS));
}

void loop()
{
  Usb.Task();
  Serial.println("loop");
  //usbloop();
  //SendRecord();
  Serial.println(digitalRead(LORA_SS));
  Serial.println(digitalRead(USB_SS));
  dr.openPort(Usb);
  delay(5000);
  //Serial.println("Llamada de loop");
}
