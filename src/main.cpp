/*
 * FtpServer esp8266 and esp32 with SD
 *
 * AUTHOR:  Renzo Mischianti
 *
 * https://www.mischianti.org/2020/02/08/ftp-server-on-esp8266-and-esp32
 *
 */

/*
###############      
###############      RUN FTP SERVER CODE CONNECTED TO BATTERY AND NOT USB AS USB NOT ENOUGH POWER
###############      
*/
#include <Arduino.h>
#include <mercator_secrets.c>
#include <WiFi.h>
#include "SD.h"
#include <memory.h>

#include <SD-card-API.h>

#include <SimpleFTPServer.h>

const char* ssid = ssid_1;
const char* password = password_1;

FtpServer ftpSrv;   //set #define FTP_DEBUG in ESP8266FtpServer.h to see ftp verbose on serial


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
//	Serial.print(">>>>>>>>>>>>>>> _transferCallback " );
//	Serial.print(ftpOperation);
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
//	Serial.print(" ");
//	Serial.print(name);
//	Serial.print(" ");
//	Serial.println(transferredSize);
};

#define SCK_BAZ 4
#define MISO_BAZ 5
#define MOSI_BAZ 6
#define SS_BAZ 7 

const int beetleLed = 10;

void DumpSDStats();
void dumpHeapUsage(const char* msg);

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
//  SPI.setFrequency(500000);
//  SPI.setClockDivider(SPI_CLOCK_DIV32);
  
  /////FTP Setup, ensure SPIFFS is started before ftp;  /////////
  SPI.begin(SCK_BAZ, MISO_BAZ, MOSI_BAZ, SS_BAZ); //SCK, MISO, MOSI,SS

  dumpHeapUsage("Heap prior to SD.begin");

  if (SD.begin(SS_BAZ, SPI, 1000000, "/sd", 5, true))
  {
      Serial.println("SD opened!");

      mercatorFS::listDir(SD, "/", 1);

      ftpSrv.setCallback(_callback);
      ftpSrv.setTransferCallback(_transferCallback);

      delay(2000);
      ftpSrv.begin("esp32","esp32","Welcome Baz");    //username, password for ftp.   (default 21, 50009 for PASV)

      DumpSDStats();
      dumpHeapUsage("Heap after listDir");
  }
}

void DumpSDStats()
{
  switch (SD.cardType()) 
  {
    case CARD_NONE:
      Serial.println("No SD card attached");
      return;
    case CARD_MMC:
      Serial.println("MMC card");
      break;
    case CARD_SD:
      Serial.println("SD card");
      break;
    case CARD_SDHC:
      Serial.println("SDHC card");
      break;
    case CARD_UNKNOWN:
      Serial.println("Unknown card");
      break;
    default:
      Serial.println("Error (unknown card type)");
      return;    
  }

  Serial.print("Sectors:          ");
  Serial.println(SD.numSectors());
  Serial.print("Sector Size:          ");
  Serial.println(SD.sectorSize());
  Serial.print("Total Bytes:          ");
  Serial.println(SD.totalBytes());  
  Serial.print("Used Bytes:          ");
  Serial.println(SD.usedBytes());  
}

void dumpHeapUsage(const char* msg)
{  
  if (true)
  {
    multi_heap_info_t info; 
    heap_caps_get_info(&info, MALLOC_CAP_INTERNAL | MALLOC_CAP_8BIT); // internal RAM, memory capable to store data or to create new task
    Serial.printf("\n%s : free heap bytes: %i  largest free heap block: %i min free ever: %i\n",  msg, info.total_free_bytes, info.largest_free_block, info.minimum_free_bytes);
  }
}

void loop(void)
{
  ftpSrv.handleFTP();        //make sure in loop you call handleFTP()!!
}


