#include <genieArduino.h>
#include <SoftwareSerial.h>


#include <stdio.h>
#include <stdint.h>
#include <ctype.h>

Genie genie;
#define RESETLINE 4 // 4D Systems Display Reset Pin

#define FORM_BOOT_BLANK 0
#define FORM_BOOT_APPLE 1
#define FORM_BOOT_AL 2
#define FORM_BOOT_MAC 3
#define FORM_DESKTOP 4
#define FORM_SD_INFO 5
#define FORM_ABOUT_MAC 6
#define FORM_BLUETOOTH_PHONE 7
#define FORM_BLUETOOTH_MUSIC 8
#define FORM_BLUETOOTH_INCOMING 9
#define FORM_BLUETOOTH_ACTIVE 10

#define DESKTOP_SD_BUTTON 3
#define DESKTOP_APPLE_BUTTON 2
#define DESKTOP_BLUETOOTH_BUTTON 7

#define BLUETOOTH_PHONE_INPUT_STRING 5
#define BLUETOOTH_PHONE_KEYBOARD 0

#define BLUETOOTH_PHONE_CALL_BUTTON 8
#define BLUETOOTH_PHONE_MUSIC_BUTTON 14
#define BLUETOOTH_PHONE_FIRMWARE_STRING 4
#define BLUETOOTH_MUSIC_FIRMWARE_STRING 6
#define BLUETOOTH_MUSIC_PLAY_PAUSE_BUTTON 11
#define BLUETOOTH_MUSIC_PREV_BUTTON 10
#define BLUETOOTH_MUSIC_NEXT_BUTTON 12
#define BLUETOOTH_MUSIC_PHONE_BUTTON 13
#define BLUETOOTH_MUSIC_VOL_UP_BUTTON 15
#define BLUETOOTH_MUSIC_VOL_DOWN_BUTTON 16

#define BLUETOOTH_INCOMING_ACCEPT_BUTTON 17
#define BLUETOOTH_INCOMING_DENY_BUTTON 18
#define BLUETOOTH_ACTIVE_MUTE_BUTTON 20
#define BLUETOOTH_ACTIVE_DENY_BUTTON 21
#define BLUETOOTH_ACTIVE_VOL_UP_BUTTON 22
#define BLUETOOTH_ACTIVE_VOL_DOWN_BUTTON 23


String phoneNumber;

SoftwareSerial btSerial(6, 7);

int prevGPIO2 = HIGH;


void setup() {
  pinMode(13, OUTPUT);
  digitalWrite(13,LOW);

  pinMode(8, INPUT);

  btSerial.begin(9600);

  Serial.begin(115200); // Serial0 @ 115200 (115K) Baud
  genie.Begin(Serial); // Use Serial0 for talking to the Genie Library, and to the 4D Systems display
  delay(1000);
  genie.AttachEventHandler(myGenieEventHandler);
  boot();

  genie.WriteObject (GENIE_OBJ_FORM, FORM_DESKTOP, 0);
}

void boot() {
  pinMode(RESETLINE, OUTPUT);  // Set D4 on Arduino to Output (4D Arduino Adaptor V2 - Display Reset)
  /*
  digitalWrite(RESETLINE, 1);  // Reset the Display via D4
   delay(100);
   digitalWrite(RESETLINE, 0);  // unReset the Display via D4
   */

  delay (5000); //let the display start up after the reset (This is important)]
  genie.WriteObject (GENIE_OBJ_FORM, FORM_BOOT_BLANK, 0);
  delay (500);
  genie.WriteObject (GENIE_OBJ_FORM, FORM_BOOT_APPLE, 0);
  delay (1000);
  genie.WriteObject (GENIE_OBJ_FORM, FORM_BOOT_AL, 0);
  delay (1000);
  genie.WriteObject (GENIE_OBJ_FORM, FORM_BOOT_MAC, 0);
  delay (1000);
  genie.WriteObject (GENIE_OBJ_FORM, FORM_BOOT_BLANK, 0);
  delay (350);
  btSerial.write("V\r");
  while(btSerial.available() > 0) {
    btSerial.read();
  }
}

String getBluetoothVersion () {
  btSerial.write("V\r");
  String versionStr = "";
  delay(500);
  if (btSerial.available() > 0) {

    while (btSerial.available() > 0){
      int readByte = btSerial.read();
      if (readByte==13) {
        while (btSerial.available() > 0) {
          btSerial.read();

        }
        break;
      }
      versionStr+=char(readByte);
    }
    if (versionStr!="?"&&versionStr!="AOK"){
      char buf[versionStr.length()];
      versionStr.toCharArray(buf, versionStr.length()+1);
      genie.WriteStr(BLUETOOTH_PHONE_FIRMWARE_STRING, buf);
      genie.WriteStr(BLUETOOTH_MUSIC_FIRMWARE_STRING, buf);
    }

  }
  else {
    char buf[] = "fail";
    genie.WriteStr(BLUETOOTH_MUSIC_FIRMWARE_STRING, buf);
  }
  return versionStr;
}

void loop() {
  genie.DoEvents();
}

void myGenieEventHandler() {
  genieFrame Event;
  genie.DequeueEvent(&Event); // Remove this event from the queue

  if (Event.reportObject.cmd != GENIE_REPORT_EVENT) // If this event is NOT a Reported Message
  {
    digitalWrite(13,HIGH);
  }


  if (Event.reportObject.object == GENIE_OBJ_ANIBUTTON) //  If this event is from a Button
  {

    if (Event.reportObject.index == DESKTOP_SD_BUTTON)
    {
      genie.WriteObject (GENIE_OBJ_FORM, FORM_SD_INFO, 0);
    }
    else if (Event.reportObject.index == DESKTOP_APPLE_BUTTON)
    {
      genie.WriteObject (GENIE_OBJ_FORM, FORM_ABOUT_MAC, 0);
    }
    else if (Event.reportObject.index == DESKTOP_BLUETOOTH_BUTTON||Event.reportObject.index == BLUETOOTH_PHONE_MUSIC_BUTTON)
    {
      genie.WriteObject (GENIE_OBJ_FORM, FORM_BLUETOOTH_MUSIC, 0);
      while(true) {
        String s = getBluetoothVersion();
        if (s!="?"&&s!="AOK") break;
      }
    }
    else if (Event.reportObject.index == BLUETOOTH_PHONE_CALL_BUTTON)
    {
      char callCommandChar[phoneNumber.length()+2];
      String callCommandString = "A,"+phoneNumber;
      callCommandString.toCharArray(callCommandChar,callCommandString.length()+1);
      btSerial.print(callCommandChar);
      btSerial.print("\r");
    }
    else if (Event.reportObject.index == BLUETOOTH_MUSIC_PLAY_PAUSE_BUTTON)
    {
      btSerial.print("AP");
      btSerial.print("\r");
    }
    else if (Event.reportObject.index == BLUETOOTH_MUSIC_PREV_BUTTON)
    {
      btSerial.print("AT-");
      btSerial.print("\r");
    }
    else if (Event.reportObject.index == BLUETOOTH_MUSIC_NEXT_BUTTON)
    {
      btSerial.print("AT+");
      btSerial.print("\r");
    }
    else if (Event.reportObject.index == BLUETOOTH_MUSIC_VOL_UP_BUTTON)
    {
      btSerial.print("AV+");
      btSerial.print("\r");
    }
    else if (Event.reportObject.index == BLUETOOTH_MUSIC_VOL_DOWN_BUTTON)
    {
      btSerial.print("AV+");
      btSerial.print("\r");
    }
    else if (Event.reportObject.index == BLUETOOTH_MUSIC_PHONE_BUTTON)
    {
      phoneNumber="";
      genie.WriteObject (GENIE_OBJ_FORM, FORM_BLUETOOTH_PHONE, 0);
      while(true) {
        String s = getBluetoothVersion();
        if (s!="?"&&s!="AOK") break;
      }
    }
    else if (Event.reportObject.index == BLUETOOTH_INCOMING_ACCEPT_BUTTON) {
      btSerial.print("C\r");
      if (serialReturnedError()) {
        genie.WriteObject (GENIE_OBJ_FORM, FORM_DESKTOP, 0);
      }
      else {
        genie.WriteObject (GENIE_OBJ_FORM, FORM_BLUETOOTH_ACTIVE, 0);
      }
    }
    else if (Event.reportObject.index == BLUETOOTH_INCOMING_DENY_BUTTON) {
      genie.WriteObject(GENIE_OBJ_FORM, FORM_DESKTOP, 0);
    }
  }
  else if (Event.reportObject.object == GENIE_OBJ_KEYBOARD) // If this event is from a Keyboard
  {
    if (Event.reportObject.index == 0)
    {
      handlePhoneNumberInput(genie.GetEventData(&Event));

    }
  }
}

bool serialReturnedError() {
  String btString = "";
  if (btSerial.available() > 0) {
    while (btSerial.available() > 0){
      int readByte = btSerial.read();
      if (readByte==13) {
        while (btSerial.available() > 0) {
          btSerial.read();
        }
        break;
      }
      btString+=char(readByte);
    }
    if (btString=="ERR") return true;
  }
  return false;
}

void handlePhoneNumberInput (int key) {
  char buf[11];

  if (isdigit(key)) {
    if (phoneNumber.length()<10){
      phoneNumber+=String(key-char('0'));
    }
    phoneNumber.toCharArray(buf,phoneNumber.length()+1);
  }
  else {
    if (phoneNumber.length()>0){
      phoneNumber=phoneNumber.substring(0,phoneNumber.length()-1);
    }
    phoneNumber.toCharArray(buf,phoneNumber.length()+1);
  }
  genie.WriteStr (BLUETOOTH_PHONE_INPUT_STRING, buf);
}







