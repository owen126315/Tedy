#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <SoftwareSerial.h>
#include <Math.h>
PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);
SoftwareSerial mySerial(10, 11); // RX, TX

int file_num = 0;
byte power_on_sound[] = {0x7E,0x07,0xA3,0x70,0x6F,0x77,0x65,0x65,0x7E};
byte record_sound[] = {0x7E,0x07,0xA3,0x72,0x65,0x63,0x6F,0x53,0x7E};
byte complete_sound[] = {0x7E,0x07,0xA3,0x64,0x6F,0x6E,0x65,0x50,0x7E};
void setup()
{
    pinMode(2,INPUT);
    mySerial.begin(9600);
    Serial.begin(9600);
    Serial.println("NDEF Reader");
    //get file num
    
    file_num = get_file_num();
    Serial.print("file num:");
    Serial.println(file_num);
    delay(100);
    nfc.begin();
    mySerial.write(power_on_sound,sizeof(power_on_sound));
}

void loop()
{
  if(digitalRead(2))
    record();
  else if (nfc.tagPresent(500))
  {
    NfcTag tag = nfc.read();
    String text = tag.getText();
    Serial.print("text = ");
    Serial.println(text);
    if(play_sound(text))
      Serial.println("Play Success");
    else
      Serial.println("Play Fail");
    delay(500);
  }
}



int get_file_num()
{ 
  byte rx_buffer[3];
  byte check_file_num[] = {0x7E,0x03,0xC5,0xC8,0x7E};
  
  mySerial.write(check_file_num,sizeof(check_file_num));
  delay(5);
  mySerial.readBytes(rx_buffer,3); 
  for(int i =0; i<3; i++)
  {
    Serial.print(rx_buffer[i],HEX);
    Serial.print(" ");
  }
  Serial.println();
  return rx_buffer[1]*pow(16,2)+rx_buffer[2]-3;
}

bool play_sound(String file_name)
{ 
  byte play_file_command[] = {0x7E,0x07,0xA3,NULL,NULL,NULL,NULL,NULL,0x7E};
   
  //set the file name
  for(int i=0;i<4;i++)
    play_file_command[i+3] = file_name[i];
  
  //reset the check code
  play_file_command[sizeof(play_file_command)-2] = NULL;
  for(int i=1;i<sizeof(play_file_command)-2;i++)
    play_file_command[sizeof(play_file_command)-2]+=play_file_command[i];
    
  //send play command
  byte temp[1];
  mySerial.write(play_file_command,sizeof(play_file_command));
  //delay(5);
  mySerial.readBytes(temp,1);
  
  if(temp[0] == 0x00)
    return 1;
  else
    return 0;
}

void record()
{
  byte record_command[] = {0x7E,0x07,0xD6,NULL,NULL,NULL,NULL,NULL,0x7E};
  byte stop_record_command[] = {0x7E,0x03,0xD9,0xDC,0x7E};
  byte remove_command[] = {0x7E,0x07,0xDB,NULL,NULL,NULL,NULL,NULL,0x7E};
  
  record_command[3] = 0x30 | (file_num/1000);
  record_command[4] = 0x30 | (file_num/100)%10;
  record_command[5] = 0x30 | (file_num/10)%10;
  record_command[6] = 0x30 | (file_num%10);
  //set check code
  for(int i=1;i<sizeof(record_command)-2;i++)
    record_command[sizeof(record_command)-2] += record_command[i];
    
  remove_command[3] = 0x30 | (file_num/1000);
  remove_command[4] = 0x30 | (file_num/100)%10;
  remove_command[5] = 0x30 | (file_num/10)%10;
  remove_command[6] = 0x30 | (file_num%10);
  //set check code
  for(int i=1;i<sizeof(remove_command)-2;i++)
    remove_command[sizeof(remove_command)-2] += remove_command[i];

  NdefMessage message = NdefMessage();
  String text = "0000";
  text[0] = 0x30 | (file_num/1000);
  text[1] = 0x30 | (file_num/100)%10;
  text[2] = 0x30 | (file_num/10)%10;
  text[3] = 0x30 | (file_num%10);
  message.addTextRecord(text);

  if (nfc.tagPresent()) 
  { 
    mySerial.write(record_sound,sizeof(record_sound));
    delay(800);
    mySerial.write(record_command,sizeof(record_command));
    while(digitalRead(2));
    mySerial.write(stop_record_command,sizeof(stop_record_command));
    
    if (nfc.write(message))
    {
      Serial.println("\nTag write complete");
      NfcTag tag = nfc.read();
      if(tag.getText() == text)
      {
        Serial.println("Verified the tag");
        mySerial.write(complete_sound,sizeof(complete_sound));
        delay(1000);
        file_num++;
      }
      else
      {
        Serial.println("\nText on tag do not match the filename");
        mySerial.write(remove_command,sizeof(remove_command));
      }      
    }             
    else
    {
      Serial.println("\nWrite failed.");
      mySerial.write(remove_command,sizeof(remove_command));
    }     
  }
  else
  {
    Serial.println("\nCannot find tag.");
  } 
}
