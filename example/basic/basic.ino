#include <USB_CH376s.h>

USB_CH376s usb(Serial3, Serial);

void setup() {
  Serial.begin(9600);
  Serial3.begin(9600);
}

void printResult(boolean res) {
  if (res == true)
    Serial.println("OK");
  else Serial.println("false");
}

void loop() {
  if (Serial.available()) {
    byte computerByte = Serial.read();
    if (computerByte == 49) {           //1
      Serial.println();
      Serial.println("Testing the connection to the module CH376s");
      printResult(usb.checkConnection(0x01));
    }
    if (computerByte == 50) {           //2
      Serial.println();
      Serial.println("Create and write to file: LOG.TXT");
      usb.writeFile("LOG.TXT", "FoxiK");
    }
    if (computerByte == 52) {           //4
      Serial.println();
      Serial.println("Read file: LOG.TXT");
      usb.setFileName("LOG.TXT");
      usb.fileOpen();
      int i = usb.getFileSize();
      usb.fileClose(0x00);
      String data[i];
      usb.readFile("LOG.TXT", data);
      for (int a = 0; a < i; a++)
        Serial.print(data[a]);
      Serial.println();
    }
    if (computerByte == 51) {           //3
      Serial.println();
      Serial.println("Add to file: LOG.TXT");
      usb.appendFile("LOG.TXT", " - ARDUINO");
    }
    if (computerByte == 53) {           //5
      Serial.println();
      Serial.println("Delete file: LOG.TXT");
      usb.fileDelete("LOG.TXT");
    }
  }

  if (Serial3.available()) {
    Serial.println();
    Serial.print("CH376S code:");
    Serial.println(Serial3.read(), HEX);
  }
}
