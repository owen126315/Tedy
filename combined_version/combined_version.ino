#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <SoftwareSerial.h>
PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);
SoftwareSerial mySerial(10, 11); // RX, TX

//char setVol[] = {0x7e,0x4,0xae,NULL,0,0xef};
char working_status[] = {0x7E,0x03,0xC2,0xC5,0xEF};
void setup()
{
    mySerial.begin(9600);
    Serial.begin(9600);
    Serial.println("NDEF Reader");
    nfc.begin();
}

void loop()
{
    Serial.println("\nScan a NFC tag\n");
    if (nfc.tagPresent(1000))
    {
        NfcTag tag = nfc.read();
        String text = tag.getText();
        Serial.println(text);
        if(text.length() == 3)
        {
          send_command(text);
          delay(300);
          check_working_status();
        }
        else
        Serial.println("This tag is not registed");
    }
    delay(1000);
}

void send_command(String text)
{
  char sound_command[] = {0x7e,0x6,0xA3,NULL,NULL,NULL,0,0xEF};
  for(int i=0; i<3; i++)
    sound_command[3+i] = (char)text[i];
  for(int i = 1;i<sizeof(sound_command)-2;i++)
    sound_command[sizeof(sound_command)-2]+=sound_command[i];
  mySerial.write(sound_command);
}

int check_working_status()
{
  byte tem[3];
  mySerial.write(working_status);
  mySerial.readBytes(tem,3);
  
  Serial.print(tem[0],HEX);
  Serial.print(" ");
  Serial.print(tem[1],HEX);
  Serial.print(" ");
  Serial.println(tem[2],HEX);
  
  if(tem[2] == 0x3 || tem[2] == 0x2)
  Serial.println("No such file");

}
