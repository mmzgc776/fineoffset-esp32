#include <usbhub.h>
#include "pgmstrings.h"
// Satisfy the IDE, which needs to see the include statment in the ino too.
#ifdef dobogusinclude
#include <spi4teensy3.h>
#endif
#include <SPI.h>
//#include "extractor.h"

USB     Usb;

class DeviceReader
{
public:
  int ciclos = 0;
  int addr = 1;
  int vendor_id = 1941;
  int product_id = 8021;
  int usb_endpoint = 81; // revisar para que sirve, evitar confusión
  int usb_read_size = 20;

  void read_usb_block(USB Usb, int usb_address);

}