#ifndef usbhub
#include <usbhub.h>
#endif
#include "fineoffset.h"
//Se pretende adaptar los controladores de FineOffset WH1080
//Aquí están las funciones básicas para la implementación de un extractor por USB
//Moisés González Castellanos 04-04-20
DeviceReader::DeviceReader(){}

//Abrimos el puerto y lo dejamos disponible
void DeviceReader::openPort(USB Usb)
{
    Serial.println("Clase DeviceReader/openport");
    find_device(Usb);
}

void DeviceReader::read_usb_block(USB Usb, int usb_address)
{
    Serial.println("Iniciando lectura de usb");
    Serial.println("usb_address " + usb_address);
    int ptr = 0;
    uint8_t addr;
    uint8_t endPoint;
    uint8_t bmReqType;
    uint8_t bRequest;
    uint8_t wValLo;
    uint8_t wValHi;
    uint16_t wInd;
    uint16_t total;
    addr = 1;
    endPoint = 81;
    bmReqType = 0x33; //class+interface en getData.py
    bRequest = 0x09;
    wValLo = 0x00;
    wValHi = 0x02;
    wInd = 0;

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

    uint8_t readBuffer[32]; 

    Serial.println("ptr: " + ptr);
    Serial.println("requestBuffer:");
    Serial.println(rqstBuffer[0]);
    Serial.println(rqstBuffer[1]);
    Serial.println(rqstBuffer[2]);
    Serial.println(rqstBuffer[3]);
    Serial.println(rqstBuffer[4]);
    Serial.println(rqstBuffer[5]);
    Serial.println(rqstBuffer[6]);
    Serial.println(rqstBuffer[7]);
    Serial.println("Termina requestBuffer");
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
    //Serial.println(Usb.inTransfer(addr, endPoint, usb_read_size, 32, &readBuffer));
    Serial.println(Usb.ctrlReq(addr, endPoint, bmReqType, bRequest, wValLo, wValHi, wInd, total, sizeof(rqstBuffer) / sizeof(byte), rqstBuffer, NULL));    
}

//Find the vendor and product ID on the USB.
void DeviceReader::find_device(USB Usb)
{
    //delay(5000);
    Serial.println("Clase DeviceReader/usb.task()");
    Usb.Task();
    Serial.println("Clase DeviceReader/find_device");
    //delay(1000);
    if (Usb.getUsbTaskState() == USB_STATE_RUNNING)
    {
        //Usb.ForEachUsbDevice(&PrintAllDescriptors);
        //Usb.ForEachUsbDevice(&PrintAllAddresses);
        Serial.println("USB_STATE_RUNNING");
        read_usb_block(Usb, addr);
    }
}
void read_block(int ptr, bool retry=True){
    // Read block repeatedly until it's stable. This avoids getting corrupt
    // data when the block is read as the station is updating it.
    //old_block = None
      //  while True:
            //self._wait_for_station()
        //    new_block = self._read_usb_block(ptr)
          //  if new_block:
            //    if (new_block == old_block) or not retry:
              //      break
                //if old_block is not None:
                  //  print('unstable read: blocks differ for ptr 0x%06x' % ptr)
                //old_block = new_block
        //return new_block
}
        

// void PrintAllAddresses(UsbDevice *pdev)
// {
//     UsbDeviceAddress adr;
//     adr.devAddress = pdev->address.devAddress;
//     Serial.print("\r\nAddr:");
//     Serial.print(adr.devAddress, HEX);
//     Serial.print("(");
//     Serial.print(adr.bmHub, HEX);
//     Serial.print(".");
//     Serial.print(adr.bmParent, HEX);
//     Serial.print(".");
//     Serial.print(adr.bmAddress, HEX);
//     Serial.println(")");
// }

// void PrintAddress(uint8_t addr)
// {
//     UsbDeviceAddress adr;
//     adr.devAddress = addr;
//     Serial.print("\r\nADDR:\t");
//     Serial.println(adr.devAddress, HEX);
//     Serial.print("DEV:\t");
//     Serial.println(adr.bmAddress, HEX);
//     Serial.print("PRNT:\t");
//     Serial.println(adr.bmParent, HEX);
//     Serial.print("HUB:\t");
//     Serial.println(adr.bmHub, HEX);
// }

// uint8_t getdevdescr(uint8_t addr, uint8_t &num_conf);

// /* Print a string from Program Memory directly to save RAM */
// void printProgStr(const char *str)
// {
//     char c;
//     if (!str)
//         return;
//     while ((c = pgm_read_byte(str++)))
//         Serial.print(c);
// }

// /* prints hex numbers with leading zeroes */
// // copyright, Peter H Anderson, Baltimore, MD, Nov, '07
// // source: http://www.phanderson.com/arduino/arduino_display.html
// void print_hex(int v, int num_places)
// {
//     int mask = 0, n, num_nibbles, digit;

//     for (n = 1; n <= num_places; n++)
//     {
//         mask = (mask << 1) | 0x0001;
//     }
//     v = v & mask; // truncate v to specified number of places

//     num_nibbles = num_places / 4;
//     if ((num_places % 4) != 0)
//     {
//         ++num_nibbles;
//     }
//     do
//     {
//         digit = ((v >> (num_nibbles - 1) * 4)) & 0x0f;
//         Serial.print(digit, HEX);
//     } while (--num_nibbles);
// }

// /* function to print configuration descriptor */
// void printconfdescr(uint8_t *descr_ptr)
// {
//     USB_CONFIGURATION_DESCRIPTOR *conf_ptr = (USB_CONFIGURATION_DESCRIPTOR *)descr_ptr;
//     printProgStr(Conf_Header_str);
//     printProgStr(Conf_Totlen_str);
//     print_hex(conf_ptr->wTotalLength, 16);
//     printProgStr(Conf_Nint_str);
//     print_hex(conf_ptr->bNumInterfaces, 8);
//     printProgStr(Conf_Value_str);
//     print_hex(conf_ptr->bConfigurationValue, 8);
//     printProgStr(Conf_String_str);
//     print_hex(conf_ptr->iConfiguration, 8);
//     printProgStr(Conf_Attr_str);
//     print_hex(conf_ptr->bmAttributes, 8);
//     printProgStr(Conf_Pwr_str);
//     print_hex(conf_ptr->bMaxPower, 8);
//     return;
// }

// /* function to print interface descriptor */
// void printintfdescr(uint8_t *descr_ptr)
// {
//     USB_INTERFACE_DESCRIPTOR *intf_ptr = (USB_INTERFACE_DESCRIPTOR *)descr_ptr;
//     printProgStr(Int_Header_str);
//     printProgStr(Int_Number_str);
//     print_hex(intf_ptr->bInterfaceNumber, 8);
//     printProgStr(Int_Alt_str);
//     print_hex(intf_ptr->bAlternateSetting, 8);
//     printProgStr(Int_Endpoints_str);
//     print_hex(intf_ptr->bNumEndpoints, 8);
//     printProgStr(Int_Class_str);
//     print_hex(intf_ptr->bInterfaceClass, 8);
//     printProgStr(Int_Subclass_str);
//     print_hex(intf_ptr->bInterfaceSubClass, 8);
//     printProgStr(Int_Protocol_str);
//     print_hex(intf_ptr->bInterfaceProtocol, 8);
//     printProgStr(Int_String_str);
//     print_hex(intf_ptr->iInterface, 8);
//     return;
// }
// /* function to print endpoint descriptor */
// void printepdescr(uint8_t *descr_ptr)
// {
//     USB_ENDPOINT_DESCRIPTOR *ep_ptr = (USB_ENDPOINT_DESCRIPTOR *)descr_ptr;
//     printProgStr(End_Header_str);
//     printProgStr(End_Address_str);
//     print_hex(ep_ptr->bEndpointAddress, 8);
//     printProgStr(End_Attr_str);
//     print_hex(ep_ptr->bmAttributes, 8);
//     printProgStr(End_Pktsize_str);
//     print_hex(ep_ptr->wMaxPacketSize, 16);
//     printProgStr(End_Interval_str);
//     print_hex(ep_ptr->bInterval, 8);

//     return;
// }

// void printhubdescr(uint8_t *descrptr, uint8_t addr)
// {
//     HubDescriptor *pHub = (HubDescriptor *)descrptr;
//     uint8_t len = *((uint8_t *)descrptr);

//     printProgStr(PSTR("\r\n\r\nHub Descriptor:\r\n"));
//     printProgStr(PSTR("bDescLength:\t\t"));
//     Serial.println(pHub->bDescLength, HEX);

//     printProgStr(PSTR("bDescriptorType:\t"));
//     Serial.println(pHub->bDescriptorType, HEX);

//     printProgStr(PSTR("bNbrPorts:\t\t"));
//     Serial.println(pHub->bNbrPorts, HEX);

//     printProgStr(PSTR("LogPwrSwitchMode:\t"));
//     Serial.println(pHub->LogPwrSwitchMode, BIN);

//     printProgStr(PSTR("CompoundDevice:\t\t"));
//     Serial.println(pHub->CompoundDevice, BIN);

//     printProgStr(PSTR("OverCurrentProtectMode:\t"));
//     Serial.println(pHub->OverCurrentProtectMode, BIN);

//     printProgStr(PSTR("TTThinkTime:\t\t"));
//     Serial.println(pHub->TTThinkTime, BIN);

//     printProgStr(PSTR("PortIndicatorsSupported:"));
//     Serial.println(pHub->PortIndicatorsSupported, BIN);

//     printProgStr(PSTR("Reserved:\t\t"));
//     Serial.println(pHub->Reserved, HEX);

//     printProgStr(PSTR("bPwrOn2PwrGood:\t\t"));
//     Serial.println(pHub->bPwrOn2PwrGood, HEX);

//     printProgStr(PSTR("bHubContrCurrent:\t"));
//     Serial.println(pHub->bHubContrCurrent, HEX);

//     for (uint8_t i = 7; i < len; i++)
//         print_hex(descrptr[i], 8);

//     //for (uint8_t i=1; i<=pHub->bNbrPorts; i++)
//     //    PrintHubPortStatus(&Usb, addr, i, 1);
// }

// /*function to print unknown descriptor */
// void printunkdescr(uint8_t *descr_ptr)
// {
//     uint8_t length = *descr_ptr;
//     uint8_t i;
//     printProgStr(Unk_Header_str);
//     printProgStr(Unk_Length_str);
//     print_hex(*descr_ptr, 8);
//     printProgStr(Unk_Type_str);
//     print_hex(*(descr_ptr + 1), 8);
//     printProgStr(Unk_Contents_str);
//     descr_ptr += 2;
//     for (i = 0; i < length; i++)
//     {
//         print_hex(*descr_ptr, 8);
//         descr_ptr++;
//     }
// }

// uint8_t getconfdescr(uint8_t addr, uint8_t conf)
// {
//     uint8_t buf[BUFSIZE];
//     uint8_t *buf_ptr = buf;
//     uint8_t rcode;
//     uint8_t descr_length;
//     uint8_t descr_type;
//     uint16_t total_length;
//     rcode = Usb.getConfDescr(addr, 0, 4, conf, buf); //get total length
//     LOBYTE(total_length) = buf[2];
//     HIBYTE(total_length) = buf[3];
//     if (total_length > 256)
//     { //check if total length is larger than buffer
//         printProgStr(Conf_Trunc_str);
//         total_length = 256;
//     }
//     rcode = Usb.getConfDescr(addr, 0, total_length, conf, buf); //get the whole descriptor
//     while (buf_ptr < buf + total_length)
//     { //parsing descriptors
//         descr_length = *(buf_ptr);
//         descr_type = *(buf_ptr + 1);
//         switch (descr_type)
//         {
//         case (USB_DESCRIPTOR_CONFIGURATION):
//             printconfdescr(buf_ptr);
//             break;
//         case (USB_DESCRIPTOR_INTERFACE):
//             printintfdescr(buf_ptr);
//             break;
//         case (USB_DESCRIPTOR_ENDPOINT):
//             printepdescr(buf_ptr);
//             break;
//         case 0x29:
//             printhubdescr(buf_ptr, addr);
//             break;
//         default:
//             printunkdescr(buf_ptr);
//             break;
//         }                                   //switch( descr_type
//         buf_ptr = (buf_ptr + descr_length); //advance buffer pointer
//     }                                       //while( buf_ptr <=...
//     return (rcode);
// }

// void PrintDescriptors(uint8_t addr)
// {
//     uint8_t rcode = 0;
//     uint8_t num_conf = 0;

//     rcode = getdevdescr((uint8_t)addr, num_conf);
//     if (rcode)
//     {
//         printProgStr(Gen_Error_str);
//         print_hex(rcode, 8);
//     }
//     Serial.print("\r\n");

//     for (int i = 0; i < num_conf; i++)
//     {
//         rcode = getconfdescr(addr, i); // get configuration descriptor
//         if (rcode)
//         {
//             printProgStr(Gen_Error_str);
//             print_hex(rcode, 8);
//         }
//         Serial.println("\r\n");
//     }
// }

// void PrintAllDescriptors(UsbDevice *pdev)
// {
//     Serial.println("\r\n");
//     print_hex(pdev->address.devAddress, 8);
//     Serial.println("\r\n--");
//     PrintDescriptors(pdev->address.devAddress);
// }

// uint8_t getdevdescr(uint8_t addr, uint8_t &num_conf)
// {
//     USB_DEVICE_DESCRIPTOR buf;
//     uint8_t rcode;
//     rcode = Usb.getDevDescr(addr, 0, 0x12, (uint8_t *)&buf);
//     if (rcode)
//     {
//         return (rcode);
//     }
//     printProgStr(Dev_Header_str);
//     printProgStr(Dev_Length_str);
//     print_hex(buf.bLength, 8);
//     printProgStr(Dev_Type_str);
//     print_hex(buf.bDescriptorType, 8);
//     printProgStr(Dev_Version_str);
//     print_hex(buf.bcdUSB, 16);
//     printProgStr(Dev_Class_str);
//     print_hex(buf.bDeviceClass, 8);
//     printProgStr(Dev_Subclass_str);
//     print_hex(buf.bDeviceSubClass, 8);
//     printProgStr(Dev_Protocol_str);
//     print_hex(buf.bDeviceProtocol, 8);
//     printProgStr(Dev_Pktsize_str);
//     print_hex(buf.bMaxPacketSize0, 8);
//     printProgStr(Dev_Vendor_str);
//     print_hex(buf.idVendor, 16);
//     printProgStr(Dev_Product_str);
//     print_hex(buf.idProduct, 16);
//     printProgStr(Dev_Revision_str);
//     print_hex(buf.bcdDevice, 16);
//     printProgStr(Dev_Mfg_str);
//     print_hex(buf.iManufacturer, 8);
//     printProgStr(Dev_Prod_str);
//     print_hex(buf.iProduct, 8);
//     printProgStr(Dev_Serial_str);
//     print_hex(buf.iSerialNumber, 8);
//     printProgStr(Dev_Nconf_str);
//     print_hex(buf.bNumConfigurations, 8);
//     num_conf = buf.bNumConfigurations;
//     return (0);
// }
// 