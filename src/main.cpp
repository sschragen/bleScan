
#include <Arduino.h>

#include <WiFi.h>
const char* ssid = "Schragen2.4";
const char* password =  "warpdrive";

#include <time.h>
#define NTP_SERVER "de.pool.ntp.org"
#define TZ_INFO "WEST-1DWEST-2,M3.5.0/02:00:00,M10.5.0/03:00:00" // Western European Time
#define GMT_OFFSET 3600
#define GMT_DAYLIGHT_OFFSET 3600

#include "NimBLEDevice.h"
#include "Cmd.h"
//#include "EQ3Device.h"
#include "EQ3List.h"

EQ3List Liste;

#include <ArduinoJson.h>

#define LED 2

struct selected_t
{
    int index;
    NimBLEAddress Adr;
    bool selected = false;
}selected;

void scanEndedCB(NimBLEScanResults results);

static uint32_t scanTime = 20; /** 0 = scan forever */
//NimBLEUUID EQ3_service_id             ("3e135142-654f-9090-134a-a6ff5bb77046"); //eq3_service_id
//NimBLEUUID EQ3_resp_char_id           ("d0e8434d-cd29-0996-af41-6c90f4e0eb2a"); //eq3_resp_char_id
//NimBLEUUID EQ3_char_id                ("3fa4585a-ce4a-3bad-db4b-b8df8179ea09");



//*********************************************
//NimBLEClient *pClient;
//NimBLERemoteCharacteristic *pCharacteristic;
//*********************************************


void printUuint64_t (const Uuint64_t *val)
{
    cmdGetStream()->printf("(0x)"); 
    for (int i=0;i<16;i++)
        cmdGetStream()->printf("%02X-",val->b[i]); 
    cmdGetStream()->println("\b");
}


//Cmd Commandline;
void CMD_Info(int arg_cnt, char **args)
{
  cmdGetStream()->println("Hello world.");
  
}

void CMD_List(int arg_cnt, char **args)
{
    if (pScan->isScanning())
    {
        cmdGetStream()->println ("Es wird zur Zeit gescannt ...");
    }
       
    cmdGetStream()->printf ("Anzahl an bisher gefundenen Geräten : %d \n",ScanResults.getCount());
    for(int i = 0; i < ScanResults.getCount(); i++) 
    {
        NimBLEAdvertisedDevice device = ScanResults.getDevice(i);
        
        if (device.isAdvertisingService(EQ3_service_id)) 
        {
            // create a client and connect
            cmdGetStream()->printf ("Device [%d] : %s RSSI %d\n",i,device.getAddress().toString().c_str(),device.getRSSI());
        }
    }
}

void CMD_Status(int arg_cnt, char **args)
{
  cmdGetStream()->println("Status :");
  CMD_List(1,NULL);

  Liste.print();
  Liste.RequestStatusFromAll ();
    //cmdGetStream()->println(Act->Device->pClient->toString().c_str());
}

void CMD_Scan(int arg_cnt, char **args)
{
    pScan->start(scanTime, scanEndedCB,true);
    cmdGetStream()->println("weiterer Scan gestartet.");
  
}

void CMD_Disconnect(int arg_cnt, char **args)
{
    cmdGetStream()->println("All Connections will be disconnected.");
    while (Liste.TheList.Anzahl>0)
    {
        Liste.ClearList();
    }
    
}

void CMD_Connect(int arg_cnt, char **args)
{
    if (!selected.selected)
    {
        cmdGetStream()->println("usage : first select an Bluetooth Device");
    }
    else
    {
        if (selected.index == 1000)
        {
            for (int i=0;i<ScanResults.getCount();i++)
            {
                NimBLEAdvertisedDevice Device = ScanResults.getDevice(i);
                if (Device.isAdvertisingService(EQ3_service_id)) 
                {
                    Liste.Add (Device.getAddress());
                } 
            }
        }
        else
        {
            NimBLEAdvertisedDevice Device = ScanResults.getDevice(selected.index);
            Liste.Add (Device.getAddress());
        }
    }  
};

void CMD_Unselect(int arg_cnt, char **args)
{
    selected.selected = false;
    selected.index = 0;
    cmdGetStream()->println("All EQ3 unselected");
};

void CMD_Select(int arg_cnt, char **args)
{
    if (arg_cnt == 1)
    {
        selected.selected = true;
        selected.index = 1000;
        cmdGetStream()->println("All EQ3 selected");
    }
    else
    {
        uint32_t Arg = cmdStr2Num(args[1],10);

        cmdGetStream()->println("--CMD_Connect-- starte GetDevice"); 
        NimBLEAdvertisedDevice Device = ScanResults.getDevice(Arg);
        if (Device.isAdvertisingService(EQ3_service_id)) 
        {
            selected.Adr = Device.getAddress();
            selected.selected = true;
            selected.index = Arg;
            cmdGetStream()->printf("EQ3 %d selected\n",Arg); 
        }
        else cmdGetStream()->println("Dies ist kein EQ3"); 
    }  
}

void scanEndedCB(NimBLEScanResults results)
{
    ScanResults = results;
    Serial.println();
    //Serial.println("--scanEndedCB-- Scan Ended");
    Serial.printf ("--scanEndedCB-- Anzahl an gefundenen Geräten : %d \n",results.getCount());
    for(int i = 0; i < results.getCount(); i++) 
    {
        NimBLEAdvertisedDevice device = results.getDevice(i);
        
        if (device.isAdvertisingService(EQ3_service_id)) 
        {
            // create a client and connect
            //Serial.printf ("--scanEndedCB-- Device gefunden : %s \n",device.getAddress().toString().c_str());
        }
    }
    CMD_List (1,NULL);
}

void setup ()
{
    Serial.begin(115200);
    cmdInit(&Serial);
    cmdAdd("info", CMD_Info);
    cmdAdd("list",CMD_List);
    cmdAdd("scan",CMD_Scan);
    cmdAdd("con",CMD_Connect);
    cmdAdd("connect",CMD_Connect);
    cmdAdd("dis",CMD_Disconnect);
    cmdAdd("disconnect",CMD_Disconnect);
    cmdAdd("stat",CMD_Status);
    cmdAdd("status",CMD_Status);
    cmdAdd("sel",CMD_Select);
    cmdAdd("select",CMD_Select);
    cmdAdd("unsel",CMD_Unselect);
    cmdAdd("unselect",CMD_Unselect);

    Serial.print("Connecting to WiFi");
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) 
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println(" done.");
    Serial.printf("Connected with IP: %s\n",WiFi.localIP().toString().c_str());
    configTime(GMT_OFFSET, GMT_DAYLIGHT_OFFSET, NTP_SERVER);
    struct tm timeinfo;
    if(!getLocalTime(&timeinfo)){
        Serial.println("Failed to obtain time");
        return;
    }
    Serial.println(&timeinfo, "%A, %B %d %Y %H:%M:%S");

    Serial.println("Starting NimBLE Client");
    NimBLEDevice::init("");
    NimBLEDevice::setSecurityAuth(/*BLE_SM_PAIR_AUTHREQ_BOND | BLE_SM_PAIR_AUTHREQ_MITM |*/ BLE_SM_PAIR_AUTHREQ_SC);
    NimBLEDevice::setPower(ESP_PWR_LVL_P9); /** +9db */
    pScan = NimBLEDevice::getScan(); 
    
    //pScan->setAdvertisedDeviceCallbacks(new AdvertisedDeviceCallbacks());
    pScan->setInterval(45);
    pScan->setWindow(15);
    pScan->setActiveScan(true);

    Serial.println("erster Scan gestartet");
    pScan->start(scanTime, scanEndedCB);
    cmd_display();
}

void loop ()
{
    cmdPoll();
    //delay (1); 
}
