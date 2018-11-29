#include "USB_CH376s.h"


USB_CH376s::USB_CH376s(Stream * usb, Stream * debug) {
	m_USB = usb;
	m_Debug = debug;
}

boolean USB_CH376s::checkConnection(byte value) {
  m_USB->write(0x57);
  m_USB->write(0xAB);
  m_USB->write(0x06);
  m_USB->write(value);
  if (waitForResponse("checking connection"))
    if (getResponseFromUSB() == (255 - value))
      result = true;
    else result = false;
  return result;
}

void USB_CH376s::set_USB_Mode (byte value) {
  m_USB->write(0x57);
  m_USB->write(0xAB);
  m_USB->write(0x15);
  m_USB->write(value);
  delay(20);
  if (m_USB->available()) {
    byte USB_Byte = m_USB->read();
    if (USB_Byte == 0x51) {
      USB_Byte = m_USB->read();
      if (USB_Byte == 0x15) {
      } else {
    	if (m_Debug) {
			m_Debug->print("USB not connect. Error code:");
			m_Debug->print(USB_Byte, HEX);
			m_Debug->println("H");
    	}
      }

    } else {
    	if (m_Debug) {
		  m_Debug->print("CH3765 error!   Error code:");
		  m_Debug->print(USB_Byte, HEX);
		  m_Debug->println("H");
    	}
    }
  }
  delay(20);
}

void USB_CH376s::resetALL() {
  m_USB->write(0x57);
  m_USB->write(0xAB);
  m_USB->write(0x05);
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
	  if (m_Debug) {
		  m_Debug->println("File not created...");
	  }
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
  m_USB->write(0x57);
  m_USB->write(0xAB);
  m_USB->write(0x2F);
  m_USB->write(0x2F);
  m_USB->print(fileName);
  m_USB->write((byte)0x00);
  delay(20);
}

void USB_CH376s::diskConnectionStatus() {
  m_USB->write(0x57);
  m_USB->write(0xAB);
  m_USB->write(0x30);
  if (waitForResponse("Connecting to USB disk")) {
    if (getResponseFromUSB() == 0x14) {
    } else {
    	if (m_Debug) {
    		m_Debug->print(">Connected to USB failed....");
    	}
    }
  }
}

void USB_CH376s::USBdiskMount() {
  m_USB->write(0x57);
  m_USB->write(0xAB);
  m_USB->write(0x31);
  if (waitForResponse("mounting USB disk")) {
    if (getResponseFromUSB() == 0x14) {
    } else {
    	if (m_Debug) {
    		m_Debug->print(">USB mounting error...");
    	}
    }
  }
}

void USB_CH376s::fileOpen() {
  m_USB->write(0x57);
  m_USB->write(0xAB);
  m_USB->write(0x32);
  if (waitForResponse("file Open")) {
    if (getResponseFromUSB() == 0x14) {
    } else {
    	if (m_Debug) {
    		m_Debug->print(">Open the file failed...");
    	}
    }
  }
}

boolean USB_CH376s::setByteRead(byte numBytes) {
  boolean bytesToRead = false;
  int timeCounter = 0;
  m_USB->write(0x57);
  m_USB->write(0xAB);
  m_USB->write(0x3A);
  m_USB->write((byte)numBytes);
  m_USB->write((byte)0x00);
  if (waitForResponse("setByteRead")) {
    if (getResponseFromUSB() == 0x1D) {
      bytesToRead = true;
    }
  }
  return (bytesToRead);
}

int USB_CH376s::getFileSize() {
  int fileSize = 0;
  m_USB->write(0x57);
  m_USB->write(0xAB);
  m_USB->write(0x0C);
  m_USB->write(0x68);
  delay(100);
  if (m_USB->available()) {
    fileSize = fileSize + m_USB->read();
  }
  if (m_USB->available()) {
    fileSize = fileSize + (m_USB->read() * 255);
  }
  if (m_USB->available()) {
    fileSize = fileSize + (m_USB->read() * 255 * 255);
  }
  if (m_USB->available()) {
    fileSize = fileSize + (m_USB->read() * 255 * 255 * 255);
  }
  delay(10);
  return (fileSize);
}

void USB_CH376s::fileRead(String *data) {
  if (m_Debug) {
	  m_Debug->print("READ: ");
  }
  byte firstByte = 0x00;
  byte numBytes = 0x40;
  int i = 0;
  while (setByteRead(numBytes)) {
    m_USB->write(0x57);
    m_USB->write(0xAB);
    m_USB->write(0x27);
    if (waitForResponse("reading data")) {
      firstByte = m_USB->read();
      while (m_USB->available()) {
        data[i++] = m_USB->readString();
        delay(1);
      }
    }
    if (!continueRead()) {
      break;
    }
  }
}

void USB_CH376s::fileWrite(String data) {
  if (m_Debug) {
	m_Debug->print("WRITE: ");
  }
  byte dataLength = (byte) data.length();
  if (m_Debug) {
	m_Debug->println(data);
  }
  delay(100);
  m_USB->write(0x57);
  m_USB->write(0xAB);
  m_USB->write(0x3C);
  m_USB->write((byte) dataLength);
  m_USB->write((byte) 0x00);
  if (waitForResponse("setting data Length")) {
    if (getResponseFromUSB() == 0x1E) {
      m_USB->write(0x57);
      m_USB->write(0xAB);
      m_USB->write(0x2D);
      m_USB->print(data);
      waitForResponse("writing data to file");
      if (m_Debug) {
		  m_Debug->print("WRITE OK... (");
		  m_Debug->print(m_USB->read(), HEX);
		  m_Debug->print(",");
      }
      m_USB->write(0x57);
      m_USB->write(0xAB);
      m_USB->write(0x3D);
      waitForResponse("updating file size");
      if (m_Debug) {
		  m_Debug->print(m_USB->read(), HEX);
		  m_Debug->println(")");
      }
    }
  }
}

boolean USB_CH376s::continueRead() {
  boolean readAgain = false;
  m_USB->write(0x57);
  m_USB->write(0xAB);
  m_USB->write(0x3B);
  if (waitForResponse("continueRead")) {
    if (getResponseFromUSB() == 0x14) {
      readAgain = true;
    }
  }
  return (readAgain);
}

boolean USB_CH376s::fileCreate() {
  boolean createdFile = false;
  m_USB->write(0x57);
  m_USB->write(0xAB);
  m_USB->write(0x34);
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
  m_USB->write(0x57);
  m_USB->write(0xAB);
  m_USB->write(0x35);
  if (waitForResponse("deleting file")) {
    if (getResponseFromUSB() == 0x14) {
      if (m_Debug) {
    	  m_Debug->println("Deleted...");
      }
    }
  }
}

void USB_CH376s::filePointer(boolean fileBeginning) {
  m_USB->write(0x57);
  m_USB->write(0xAB);
  m_USB->write(0x39);
  if (fileBeginning) {
    m_USB->write((byte)0x00);
    m_USB->write((byte)0x00);
    m_USB->write((byte)0x00);
    m_USB->write((byte)0x00);
  } else {
    m_USB->write((byte)0xFF);
    m_USB->write((byte)0xFF);
    m_USB->write((byte)0xFF);
    m_USB->write((byte)0xFF);
  }
  if (waitForResponse("setting file pointer")) {
    if (getResponseFromUSB() == 0x14) {
    }
  }
}

void USB_CH376s::fileClose(byte closeCmd) {
  m_USB->write(0x57);
  m_USB->write(0xAB);
  m_USB->write(0x36);
  m_USB->write((byte)closeCmd);
  if (waitForResponse("closing file")) {
    byte resp = getResponseFromUSB();
    if (resp == 0x14) {
    } else {
		if (m_Debug) {
			m_Debug->print(">Failed to close file. Error code:");
			m_Debug->println(resp, HEX);
		}
    }
  }
}

boolean USB_CH376s::waitForResponse(String errorMsg) {
  boolean bytesAvailable = true;
  int counter = 0;
  while (!m_USB->available()) {   //wait for CH376S to verify command
    delay(1);
    counter++;
    if (counter > 2000) {
      if (m_Debug) {
		  m_Debug->print("TimeOut waiting for response: Error while: ");
		  m_Debug->println(errorMsg);
      }
      bytesAvailable = false;
      break;
    }
  }
  delay(1);
  return (bytesAvailable);
}

byte USB_CH376s::getResponseFromUSB() {
  byte response = byte(0x00);
  if (m_USB->available()) {
    response = m_USB->read();
  }
  return (response);
}




