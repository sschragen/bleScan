#include "EQ3Device.h"

BLEUUID EQ3_service_id             ("3e135142-654f-9090-134a-a6ff5bb77046"); //eq3_service_id
BLEUUID EQ3_char_id                ("3fa4585a-ce4a-3bad-db4b-b8df8179ea09");
BLEUUID EQ3_filter_char_uuid       ("3fa4585a-ce4a-3bad-db4b-b8df8179ea09");
BLEUUID EQ3_resp_char_id           ("d0e8434d-cd29-0996-af41-6c90f4e0eb2a"); //eq3_resp_char_id
BLEUUID EQ3_resp_filter_char_uuid  ("d0e8434d-cd29-0996-af41-6c90f4e0eb2a");

NimBLEScan* pScan = NULL;
NimBLEScanResults ScanResults;

void MyCallback (NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify)
{
    std::string str = (isNotify == true) ? "\nNotification" : "Indication"; 
    str += " from ";
    /** NimBLEAddress and NimBLEUUID have std::string operators */
    str += std::string(pRemoteCharacteristic->getRemoteService()->getClient()->getPeerAddress());
    str += ": \nService = " + std::string(pRemoteCharacteristic->getRemoteService()->getUUID());
    str += ", Characteristic = " + std::string(pRemoteCharacteristic->getUUID());
    str += ", \nValue = ";
    Serial.print(str.c_str());
    for (int i=0;i<length;i++)
        Serial.printf("%02X-",((uint8_t*)pData)[i]);
    //printUuint64_t((Uuint64_t*)pData);
    Serial.println();

    /*
    "auto" - Bit 1 is not set (mask 0x00)
    "manual" - Bit 1 is set (mask 0x01)
    "vacation" - Bit 2 is set (mask 0x02)
    "boost" - Bit 3 is set (mask 0x04)
    "dst" - Bit 4 is set (mask 0x08)
    "open window" - Bit 5 is set (mask 0x10)
    "locked" - Bit 6 is set (mask 0x20)
    "unknown" - Bit 7 is set (mask 0x40)
    "low battery" - Bit 8 is set (mask 0x80)
    */

    cmdGetStream()->println("Byte 2 : \n"+((Uuint64_t*)pData)->toBinString(2));
    //                     "(0b)10101010"
    cmdGetStream()->printf("    |       |     +-Betriebsmodus : %s\n",  (((Uuint64_t*)pData)->b[2] & 0x01)?"Manuell":"Auto");
    cmdGetStream()->printf("    |       |   +--Urlaubsmodus   : %s\n",  (((Uuint64_t*)pData)->b[2] & 0x02)?"An":"Aus");
    cmdGetStream()->printf("    |       | +---Boost           : %s\n",  (((Uuint64_t*)pData)->b[2] & 0x04)?"An":"Aus");
    cmdGetStream()->printf("    |       +----Sommerzeit       : %s\n",  (((Uuint64_t*)pData)->b[2] & 0x08)?"An":"Aus");
    cmdGetStream()->printf("    |     +-----Fenster           : %s\n",  (((Uuint64_t*)pData)->b[2] & 0x10)?"Auf":"Zu");
    cmdGetStream()->printf("    |   +------Kindersicherung    : %s\n",  (((Uuint64_t*)pData)->b[2] & 0x20)?"An":"Aus");
    cmdGetStream()->printf("    | +-------unknown             : %s\n",  (((Uuint64_t*)pData)->b[2] & 0x20)?"1":"0");
    cmdGetStream()->printf("    +--------Batterie             : %s\n",  (((Uuint64_t*)pData)->b[2] & 0x80)?"Gering":"Ok");


    cmdGetStream()->printf("Valve           : %d %%\n",((Uuint64_t*)pData)->b[3]);

    //temp = dec(value of byte 6) / 2.0
    cmdGetStream()->printf("Zieltemperatur  : %2.1f °\n",((Uuint64_t*)pData)->b[5] / 2.0);

}

bool EQ3Device::CreateCommand (EQ3Device::EQ3Befehl_t Befehl)
{
    switch (Befehl)
    {
        case EQ3Device::Bef_Status :
            EQ3_Command_Array.data[0]   = 0x03;
            EQ3_Command_Array.length    = 1;
            //Datum/Uhrzeit anhängen
            setTimeInCommand(EQ3_Command_Array.length);
        break;
        case EQ3Device::Bef_Auto :
        break;
        case EQ3Device::Bef_Manuel :
        break;
    };

    return true;
};

EQ3Device::EQ3Device (BLEAddress Adresse)
{
    Status.m_address = Adresse;
    cmdGetStream()->println("--EQ3Device::EQ3Device-- starte GetDevice"); 
    Device = ScanResults.getDevice(Adresse);
    Status.m_rssi = Device->getRSSI(); 
    pClient = NimBLEDevice::createClient();  
    cmdGetStream()->println("--EQ3Device::Connect-- pClient erzeugt");     
};

EQ3Device::~EQ3Device()
{
    Disconnect();
    free (pClient);
    cmdGetStream()->println("--EQ3Device::~EQ3Device-- gelöscht");
};

bool EQ3Device::Connect ()
{
    //Status.isConnected = false;
    Disconnect();
    
    if (pClient->connect(Device)) 
    {
        cmdGetStream()->println("--EQ3Device::Connect-- pClient connected");
        pService = pClient->getService(EQ3_service_id);
        cmdGetStream()->println("--EQ3Device::Connect-- pService erzeugt");
        if (pService != nullptr) 
        {
            cmdGetStream()->println("--EQ3Device::Connect-- pClient erzeugt");
            pRecCharacteristic = pService->getCharacteristic(EQ3_resp_char_id);
            if (pRecCharacteristic != nullptr) 
            {
                pRecCharacteristic->subscribe(true,MyCallback,true);
                cmdGetStream()->println("--EQ3Device::Connect-- Subscription erzeugt");

                pSendCharacteristic = pService->getCharacteristic(EQ3_char_id);
                if (pSendCharacteristic != nullptr)
                {
                    cmdGetStream()->println("--EQ3Device::Connect-- Sende Queue erzeugt"); 
                    Status.isConnected = true;
                }
                else cmdGetStream()->println("--EQ3Device::Connect-- No pSendChr created");
            }
            else cmdGetStream()->println("--EQ3Device::Connect-- No pRecChr created");
        }
        else cmdGetStream()->println("--EQ3Device::Connect-- No Service");
    }
    else cmdGetStream()->println("--EQ3Device::Connect-- not connected");
    
    return Status.isConnected;
};

bool EQ3Device::Disconnect()
{
    cmdGetStream()->println("--EQ3Device::Disconnect-- To DO !!!!");
    if (Status.isConnected)
    {
        free (pSendCharacteristic);
        free (pRecCharacteristic);
        free (pService);
        Status.isConnected = false;
    }
    return Status.isConnected;
};

EQ3Device::JsonDocument EQ3Device::getInfoJSON ()
{   
    //getRSSI();

    JsonDocument Doc;
    JsonObject Device = Doc.createNestedObject(Status.m_address.toString());
    Device["Name"]=Status.m_name;
    Device["RSSI"]=Status.m_rssi;

    return Doc;
};

int EQ3Device::getRSSI()
{
    return Status.m_rssi;
};

bool EQ3Device::setTemp (int temperatur)
{
    pClient  = BLEDevice::createClient();
    pClient->connect(Status.m_address);
    BLERemoteService* pRemoteService = pClient->getService(EQ3_service_id);
    if (pRemoteService == nullptr) 
    {
      Serial.print("Failed to find our service UUID: ");
      Serial.println(EQ3_service_id.toString().c_str());
      pClient->disconnect();
      return false;
    }
    pRecCharacteristic = pRemoteService->getCharacteristic(EQ3_char_id);
    if (pRecCharacteristic == nullptr) {
      Serial.print("Failed to find our characteristic UUID: ");
      Serial.println(EQ3_char_id.toString().c_str());
      pClient->disconnect();
      return false;
    }
    Status.isConnected = true;

    cmd_val[0]= EQ3_PROP_TEMPERATURE_WRITE;
    cmd_val[1]= uint8_t(20.5 * 2) & 0xff;
    cmd_len=2;
    pSendCharacteristic->writeValue(cmd_val, cmd_len);

    pClient->disconnect();
    return true;
};

void EQ3Device::setAddress (BLEAddress Adresse)
{
    Status.m_address = Adresse;
    Serial.printf ("mit Adresse : %s\n", Status.m_address.toString().c_str());
};

 BLEAddress EQ3Device::getAddress () 
{
        return Status.m_address;
};      

 bool EQ3Device::sendRequestForStatus ()
 {
     CreateCommand(EQ3Befehl_t::Bef_Status);
     //EQ3_Command_Array.printBefehlscode();
     return sendCommand();
 };    

  bool EQ3Device::sendCommand ()
  {
    if (Status.isConnected)
    {
        cmdGetStream()->println(" Command send");
        return pSendCharacteristic->writeValue(EQ3_Command_Array.data,EQ3_Command_Array.length,true);
    }
    else
    {
        cmdGetStream()->println(" Command NOT send");
        return false;
    }
  };   

  bool EQ3Device::setTimeInCommand (uint8_t pos)
        {
            struct tm timeinfo;
            if(!getLocalTime(&timeinfo))
            {
                Serial.println("Failed to obtain time");
                return false;
            }
            else
            {
                EQ3_Command_Array.data[++pos]=timeinfo.tm_year-100;
                EQ3_Command_Array.data[++pos]=timeinfo.tm_mon+1;
                EQ3_Command_Array.data[++pos]=timeinfo.tm_mday;
                EQ3_Command_Array.data[++pos]=timeinfo.tm_hour;
                EQ3_Command_Array.data[++pos]=timeinfo.tm_min;
                EQ3_Command_Array.data[++pos]=timeinfo.tm_sec;     
                EQ3_Command_Array.length = pos;
                //EQ3_Command_Array.printBefehlscode ();
                return true;
            }
        };
