#ifndef usb_h
#define usb_h
#include "Wire.h"
#include "Arduino.h"

class USB_CH376s
{
  public:
    USB_CH376s();
    void SETUP();
    void loop();
    void printCommandHeader(String);
    boolean checkConnection(byte);
    void set_USB_Mode (byte);
    void resetALL();
    void readFile(String, String*);
    void writeFile(String, String);
    void appendFile(String, String);
    void setFileName(String);
    void diskConnectionStatus();
    void USBdiskMount();
    void fileOpen();
    boolean setByteRead(byte);
    int getFileSize();
    void fileRead(String*);
    void fileWrite(String);
    boolean continueRead();
    boolean fileCreate();
    void fileDelete(String);
    void filePointer(boolean);
    void fileClose(byte);
    boolean waitForResponse(String);
    boolean result = false;
    byte getResponseFromUSB();

};

extern USB_CH376s usb;

#endif
