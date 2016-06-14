#include "USB_CH376s.h"


USB_CH376s::USB_CH376s() {
}

void USB_CH376s::printCommandHeader(String header) {
  Serial.println("//////////////////////");
  Serial.println(header);
  Serial.println("----------------------");
}

boolean USB_CH376s::checkConnection(byte value) {
  Serial3.write(0x57);
  Serial3.write(0xAB);
  Serial3.write(0x06);
  Serial3.write(value);
  if (waitForResponse("checking connection"))
    if (getResponseFromUSB() == (255 - value))
      result = true;
    else result = false;
  return result;
}

void USB_CH376s::set_USB_Mode (byte value) {
  Serial3.write(0x57);
  Serial3.write(0xAB);
  Serial3.write(0x15);
  Serial3.write(value);
  delay(20);
  if (Serial3.available()) {
    byte USB_Byte = Serial3.read();
    if (USB_Byte == 0x51) {
      USB_Byte = Serial3.read();
      if (USB_Byte == 0x15) {
      } else {
        Serial.print("USB not connect. Error code:");
        Serial.print(USB_Byte, HEX);
        Serial.println("H");
      }

    } else {
      Serial.print("CH3765 error!   Error code:");
      Serial.print(USB_Byte, HEX);
      Serial.println("H");
    }
  }
  delay(20);
}

void USB_CH376s::resetALL() {
  Serial3.write(0x57);
  Serial3.write(0xAB);
  Serial3.write(0x05);
  delay(200);
}

void USB_CH376s::readFile(String fileName, String *data) {
  resetALL();
  set_USB_Mode(0x06);
  diskConnectionStatus();
  USBdiskMount();
  setFileName(fileName);
  fileOpen();
  int fs = getFileSize();
  fileRead(data);
  fileClose(0x00);
}

void USB_CH376s::writeFile(String fileName, String data) {
  resetALL();
  set_USB_Mode(0x06);
  diskConnectionStatus();
  USBdiskMount();
  setFileName(fileName);
  if (fileCreate()) {
    fileWrite(data);
  } else {
    Serial.println("File not create...");
  }
  fileClose(0x01);
}

void USB_CH376s::appendFile(String fileName, String data) {
  resetALL();
  set_USB_Mode(0x06);
  diskConnectionStatus();
  USBdiskMount();
  setFileName(fileName);
  fileOpen();
  filePointer(false);
  fileWrite(data);
  fileClose(0x01);
}

void USB_CH376s::setFileName(String fileName) {
  Serial3.write(0x57);
  Serial3.write(0xAB);
  Serial3.write(0x2F);
  Serial3.write(0x2F);
  Serial3.print(fileName);
  Serial3.write((byte)0x00);
  delay(20);
}

void USB_CH376s::diskConnectionStatus() {
  Serial3.write(0x57);
  Serial3.write(0xAB);
  Serial3.write(0x30);
  if (waitForResponse("Connecting to USB disk")) {
    if (getResponseFromUSB() == 0x14) {
    } else {
      Serial.print(">Connected to USB failed....");
    }
  }
}

void USB_CH376s::USBdiskMount() {
  Serial3.write(0x57);
  Serial3.write(0xAB);
  Serial3.write(0x31);
  if (waitForResponse("mounting USB disk")) {
    if (getResponseFromUSB() == 0x14) {
    } else {
      Serial.print(">USB mounting error...");
    }
  }
}

void USB_CH376s::fileOpen() {
  Serial3.write(0x57);
  Serial3.write(0xAB);
  Serial3.write(0x32);
  if (waitForResponse("file Open")) {
    if (getResponseFromUSB() == 0x14) {
    } else {
      Serial.print(">Open the file failed...");
    }
  }
}

boolean USB_CH376s::setByteRead(byte numBytes) {
  boolean bytesToRead = false;
  int timeCounter = 0;
  Serial3.write(0x57);
  Serial3.write(0xAB);
  Serial3.write(0x3A);
  Serial3.write((byte)numBytes);
  Serial3.write((byte)0x00);
  if (waitForResponse("setByteRead")) {
    if (getResponseFromUSB() == 0x1D) {
      bytesToRead = true;
    }
  }
  return (bytesToRead);
}

int USB_CH376s::getFileSize() {
  int fileSize = 0;
  Serial3.write(0x57);
  Serial3.write(0xAB);
  Serial3.write(0x0C);
  Serial3.write(0x68);
  delay(100);
  if (Serial3.available()) {
    fileSize = fileSize + Serial3.read();
  }
  if (Serial3.available()) {
    fileSize = fileSize + (Serial3.read() * 255);
  }
  if (Serial3.available()) {
    fileSize = fileSize + (Serial3.read() * 255 * 255);
  }
  if (Serial3.available()) {
    fileSize = fileSize + (Serial3.read() * 255 * 255 * 255);
  }
  delay(10);
  return (fileSize);
}

void USB_CH376s::fileRead(String *data) {
  Serial.print("READ: ");
  byte firstByte = 0x00;
  byte numBytes = 0x40;
  int i = 0;
  while (setByteRead(numBytes)) {
    Serial3.write(0x57);
    Serial3.write(0xAB);
    Serial3.write(0x27);
    if (waitForResponse("reading data")) {
      firstByte = Serial3.read();
      while (Serial3.available()) {
        data[i++] = Serial3.readString();
        delay(1);
      }
    }
    if (!continueRead()) {
      break;
    }
  }
}

void USB_CH376s::fileWrite(String data) {
  Serial.print("WRITE: ");
  byte dataLength = (byte) data.length();
  Serial.println(data);
  delay(100);
  Serial3.write(0x57);
  Serial3.write(0xAB);
  Serial3.write(0x3C);
  Serial3.write((byte) dataLength);
  Serial3.write((byte) 0x00);
  if (waitForResponse("setting data Length")) {
    if (getResponseFromUSB() == 0x1E) {
      Serial3.write(0x57);
      Serial3.write(0xAB);
      Serial3.write(0x2D);
      Serial3.print(data);
      if (waitForResponse("writing data to file")) {
      }
      Serial.print("WRITE OK... (");
      Serial.print(Serial3.read(), HEX);
      Serial.print(",");
      Serial3.write(0x57);
      Serial3.write(0xAB);
      Serial3.write(0x3D);
      if (waitForResponse("updating file size")) {
      }
      Serial.print(Serial3.read(), HEX);
      Serial.println(")");
    }
  }
}

boolean USB_CH376s::continueRead() {
  boolean readAgain = false;
  Serial3.write(0x57);
  Serial3.write(0xAB);
  Serial3.write(0x3B);
  if (waitForResponse("continueRead")) {
    if (getResponseFromUSB() == 0x14) {
      readAgain = true;
    }
  }
  return (readAgain);
}

boolean USB_CH376s::fileCreate() {
  boolean createdFile = false;
  Serial3.write(0x57);
  Serial3.write(0xAB);
  Serial3.write(0x34);
  if (waitForResponse("creating file")) {
    if (getResponseFromUSB() == 0x14) {
      createdFile = true;
    }
  }
  return (createdFile);
}

void USB_CH376s::fileDelete(String fileName) {
  setFileName(fileName);
  delay(20);
  Serial3.write(0x57);
  Serial3.write(0xAB);
  Serial3.write(0x35);
  if (waitForResponse("deleting file")) {
    if (getResponseFromUSB() == 0x14) {
      Serial.println("Delete...");
    }
  }
}

void USB_CH376s::filePointer(boolean fileBeginning) {
  Serial3.write(0x57);
  Serial3.write(0xAB);
  Serial3.write(0x39);
  if (fileBeginning) {
    Serial3.write((byte)0x00);
    Serial3.write((byte)0x00);
    Serial3.write((byte)0x00);
    Serial3.write((byte)0x00);
  } else {
    Serial3.write((byte)0xFF);
    Serial3.write((byte)0xFF);
    Serial3.write((byte)0xFF);
    Serial3.write((byte)0xFF);
  }
  if (waitForResponse("setting file pointer")) {
    if (getResponseFromUSB() == 0x14) {
    }
  }
}

void USB_CH376s::fileClose(byte closeCmd) {
  Serial3.write(0x57);
  Serial3.write(0xAB);
  Serial3.write(0x36);
  Serial3.write((byte)closeCmd);
  if (waitForResponse("closing file")) {
    byte resp = getResponseFromUSB();
    if (resp == 0x14) {
    } else {
      Serial.print(">Failed to close file. Error code:");
      Serial.println(resp, HEX);
    }
  }
}

boolean USB_CH376s::waitForResponse(String errorMsg) {
  boolean bytesAvailable = true;
  int counter = 0;
  while (!Serial3.available()) {   //wait for CH376S to verify command
    delay(1);
    counter++;
    if (counter > 2000) {
      Serial.print("TimeOut waiting for response: Error while: ");
      Serial.println(errorMsg);
      bytesAvailable = false;
      break;
    }
  }
  delay(1);
  return (bytesAvailable);
}

byte USB_CH376s::getResponseFromUSB() {
  byte response = byte(0x00);
  if (Serial3.available()) {
    response = Serial3.read();
  }
  return (response);
}

USB_CH376s usb = USB_CH376s();


