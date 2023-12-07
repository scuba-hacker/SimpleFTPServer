/*
 * FtpServer esp8266 and esp32 with SD
 *
 * AUTHOR:  Renzo Mischianti
 *
 * https://www.mischianti.org/2020/02/08/ftp-server-on-esp8266-and-esp32
 *
 */

#include <Arduino.h>
#include <mercator_secrets.c>
#include <WiFi.h>
#include "SD.h"

#include <SD-card-API.h>

#include <SimpleFTPServer.h>

const char* ssid = ssid_1;
const char* password = password_1;

FtpServer ftpSrv;   //set #define FTP_DEBUG in ESP8266FtpServer.h to see ftp verbose on serial

void formatFlashCard() ;

void _callback(FtpOperation ftpOperation, unsigned int freeSpace, unsigned int totalSpace)
{
	Serial.print(">>>>>>>>>>>>>>> _callback " );
	Serial.print(ftpOperation);
//	 FTP_CONNECT,
//	  FTP_DISCONNECT,
//	  FTP_FREE_SPACE_CHANGE
 //
	Serial.print(" ");
	Serial.print(freeSpace);
	Serial.print(" ");
	Serial.println(totalSpace);
	// freeSpace : totalSpace = x : 360

	if (ftpOperation == FTP_CONNECT) Serial.println(F("CONNECTED"));
	if (ftpOperation == FTP_DISCONNECT) Serial.println(F("DISCONNECTED"));
};

void _transferCallback(FtpTransferOperation ftpOperation, const char* name, unsigned int transferredSize){
	Serial.print(">>>>>>>>>>>>>>> _transferCallback " );
	Serial.print(ftpOperation);
	/* FTP_UPLOAD_START = 0,
	 * FTP_UPLOAD = 1,
	 *
	 * FTP_DOWNLOAD_START = 2,
	 * FTP_DOWNLOAD = 3,
	 *
	 * FTP_TRANSFER_STOP = 4,
	 * FTP_DOWNLOAD_STOP = 4,
	 * FTP_UPLOAD_STOP = 4,
	 *
	 * FTP_TRANSFER_ERROR = 5,
	 * FTP_DOWNLOAD_ERROR = 5,
	 * FTP_UPLOAD_ERROR = 5
	 */
	Serial.print(" ");
	Serial.print(name);
	Serial.print(" ");
	Serial.println(transferredSize);
};

#define SCK_BAZ 4
#define MISO_BAZ 5
#define MOSI_BAZ 6
#define SS_BAZ 7 

const int beetleLed = 10;

void setup(void)
{
  pinMode(beetleLed,OUTPUT);

  Serial.begin(115200);

  // LED flash - we're alive!
  int warmUp=10;
  
  while (warmUp--)
  {
    digitalWrite(beetleLed,HIGH);
    delay(250);
    digitalWrite(beetleLed,LOW);
    delay(250);
   
    Serial.println("Warming up...");
  }
  Serial.println("\nHere we go...");

  
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  /////FTP Setup, ensure SPIFFS is started before ftp;  /////////

// both disabled, trying to make upload more reliable.
//
// SPI.setFrequency(500000);
//  SPI.setClockDivider(SPI_CLOCK_DIV32);
  
  /////FTP Setup, ensure SPIFFS is started before ftp;  /////////
  SPI.begin(SCK_BAZ, MISO_BAZ, MOSI_BAZ, SS_BAZ); //SCK, MISO, MOSI,SS

  if (SD.begin(SS_BAZ, SPI/*, 500000*/))
  {
      Serial.println("SD opened!");

      mercatorFS::listDir(SD, "/", 1);

      ftpSrv.setCallback(_callback);
      ftpSrv.setTransferCallback(_transferCallback);

      delay(2000);
      ftpSrv.begin("esp32","esp32","Welcome Baz");    //username, password for ftp.   (default 21, 50009 for PASV)

      mercatorFS::listDir(SD, "/", 1);
  }
}

void loop(void)
{
  ftpSrv.handleFTP();        //make sure in loop you call handleFTP()!!
}


