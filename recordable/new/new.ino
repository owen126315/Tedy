#include <Wire.h>
#include <PN532_I2C.h>
#include <PN532.h>
#include <NfcAdapter.h>
#include <Math.h>
PN532_I2C pn532_i2c(Wire);
NfcAdapter nfc = NfcAdapter(pn532_i2c);


int file_num = 0;


void setup()
{
    pinMode(2,INPUT);
    Serial.begin(9600);
    Serial.println("NDEF Reader");
    //get file num
    
    file_num = get_file_num();
    Serial.print("file num:");
    Serial.println(file_num);
    delay(200);
    nfc.begin();
    
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
    delay(200);
  }
}



int get_file_num()
{ 
  byte rx_buffer[3];
  byte check_file_num[] = {0x7E,0x03,0xC5,0xC8,0x7E};
  
  Serial.write(check_file_num,sizeof(check_file_num));
  delay(5);
  Serial.readBytes(rx_buffer,3); 
  return rx_buffer[1]*pow(16,2)+rx_buffer[2];
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
  Serial.write(play_file_command,sizeof(play_file_command));
  Serial.readBytes(temp,1);
  if(temp[0] == 0x00)
    return 1;
  else
    return 0;
}

void record()
{
  byte record_command[] = {0x7E,0x07,0xD6,NULL,NULL,NULL,NULL,NULL,0x7E};
  byte stop_record_command[] = {0x7E,0x03,0xD9,0xDC,0x7E};

  //set file name
  file_num++;
  record_command[3] = 0x30 | (file_num/1000);
  record_command[4] = 0x30 | (file_num/100)%10;
  record_command[5] = 0x30 | (file_num/10)%10;
  record_command[6] = 0x30 | (file_num%10);
  //set check code
  for(int i=1;i<sizeof(record_command)-2;i++)
    record_command[sizeof(record_command)-2] += record_command[i];

  Serial.write(record_command,sizeof(record_command));
  while(digitalRead(2));
  Serial.write(stop_record_command,sizeof(stop_record_command));
  regist_new_tag();
}

void regist_new_tag()
{
  NdefMessage message = NdefMessage();
  
  if (nfc.tagPresent()) 
  { 
  
    if(nfc.format())
      Serial.println("\nSuccess, tag formatted as NDEF.");
    else
      Serial.println("\nFormat failed.");
    delay(300);
    
    String text = "0000";
    text[0] = 0x30 | (file_num/1000);
    text[1] = 0x30 | (file_num/100)%10;
    text[2] = 0x30 | (file_num/10)%10;
    text[3] = 0x30 | (file_num%10);
    message.addTextRecord(text);
    
    if (nfc.write(message)) 
      Serial.println("\nSuccess. Try reading this tag with your phone.");        
    else 
      Serial.println("\nWrite failed.");
  }
}
