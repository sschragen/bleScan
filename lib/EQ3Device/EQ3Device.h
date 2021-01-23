#ifndef EQ3DEVICE_H
#define EQ3DEVICE_H

#include <Arduino.h>
#include "NimBLEDevice.h"
#include <ArduinoJson.h>
#include "Cmd.h"

extern BLEUUID EQ3_service_id; 
extern BLEUUID EQ3_char_id;
extern BLEUUID EQ3_filter_char_uuid;
extern BLEUUID EQ3_resp_char_id; 
extern BLEUUID EQ3_resp_filter_char_uuid;

/* Request ids for TRV */
#define EQ3_PROP_ID_QUERY            0x00
#define EQ3_PROP_ID_RETURN           0x01
#define EQ3_PROP_INFO_RETURN         0x02
#define EQ3_PROP_INFO_QUERY          0x03
#define EQ3_PROP_COMFORT_ECO_CONFIG  0x11
#define EQ3_PROP_OFFSET              0x13
#define EQ3_PROP_WINDOW_OPEN_CONFIG  0x14
#define EQ3_PROP_SCHEDULE_QUERY      0x20
#define EQ3_PROP_SCHEDULE_RETURN     0x21
#define EQ3_PROP_MODE_WRITE          0x40
#define EQ3_PROP_TEMPERATURE_WRITE   0x41
#define EQ3_PROP_COMFORT             0x43
#define EQ3_PROP_ECO                 0x44
#define EQ3_PROP_BOOST               0x45
#define EQ3_PROP_LOCK                0x80

/* Status bits */
#define AUTO                     0x00
#define MANUAL                   0x01
#define AWAY                     0x02
#define BOOST                    0x04
#define DST                      0x08
#define WINDOW                   0x10
#define LOCKED                   0x20
#define UNKNOWN                  0x40
#define LOW_BATTERY              0x80

#define BYTE_TO_BINARY_PATTERN "%c-%c-%c-%c-%c-%c-%c-%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0') 


extern NimBLEScan* pScan;
extern NimBLEScanResults ScanResults;

struct Uuint64_t
{
    union 
    {
        uint64_t llu;
        uint32_t lu[2];
        uint16_t w[4];
        uint8_t  b[8];
        char     s[9];
    };
    String toHexString()
    {
        String s = "0x" + String(lu[0],HEX) + String(lu[1],HEX);
        return s;
    };
    String toBinString(int index)
    {
        char s[20];
        sprintf (s,"(0b)" BYTE_TO_BINARY_PATTERN , BYTE_TO_BINARY(b[index]));
        String str (s);
        return str;
    };
};


extern void MyCallback (NimBLERemoteCharacteristic* pRemoteCharacteristic, uint8_t* pData, size_t length, bool isNotify);

class EQ3Device
{   public:
        enum EQ3Befehl_t 
        {   
            Bef_Status=0x03,
            Bef_Auto,
            Bef_Manuel
        };

        struct EQ3_Command_Array_t
        {
            uint8_t length;
            uint8_t data[20];
            void printBefehlscode ()
            {
                Serial.printf ("Befehl : Länge %d Code : ",length);
                for (int i=0;i<length-1;i++)
                    Serial.printf("%02X-",data[i]);
                Serial.printf("%02X\n",data[length-1]);
            };
        } EQ3_Command_Array;

        enum EQ3_Stat_Batt  { VOLL,LEER };
        enum EQ3_Stat_Wind  { ZU, AUF};
        enum EQ3_Stat_OnOff { AUS, AN};
        enum EQ3_Stat_Mode  { AUTOMATISCH, MANUELL};
        struct Info_t
        {
            BLEAddress  m_address   = BLEAddress((uint8_t*)"\0\0\0\0\0\0") ;
            String      m_name      = "BLE";
            int         m_rssi      = 0;
            double      m_temp      = 0;
            uint32_t    m_owner     = 0;
            EQ3_Stat_OnOff  m_vacation = EQ3_Stat_OnOff::AUS;
            EQ3_Stat_OnOff  m_boost    = EQ3_Stat_OnOff::AUS;
            EQ3_Stat_OnOff  m_dst      = EQ3_Stat_OnOff::AN;
            EQ3_Stat_Wind   m_window   = EQ3_Stat_Wind::ZU;
            EQ3_Stat_OnOff  m_lock     = EQ3_Stat_OnOff::AUS;
            EQ3_Stat_Batt   m_battery  = EQ3_Stat_Batt::VOLL;
            
            bool        isConnected = false;
        } Status;

    private:        
        /* Current TRV command being sent to EQ-3 */ 
        uint16_t cmd_len = 0;
        uint8_t cmd_val[10] = {0};

        bool setTimeInCommand (uint8_t pos);

    public:
        EQ3Device (BLEAddress Adresse);
        ~EQ3Device();

        bool Connect ();
        bool Disconnect();

        
        NimBLEAdvertisedDevice *Device;
        NimBLEClient*  pClient;
        NimBLERemoteService *pService;
        NimBLERemoteCharacteristic *pRecCharacteristic;
        NimBLERemoteCharacteristic *pSendCharacteristic;

        void setAddress (BLEAddress Adresse);
        BLEAddress getAddress ();

        bool sendRequestForStatus ();

        bool CreateCommand (EQ3Device::EQ3Befehl_t Befehl);

        using JsonDocument = StaticJsonDocument<200>;
        JsonDocument getInfoJSON ();

        int getRSSI();
        
        bool sendCommand ();

       

        bool setTemp (int temperatur);

    
};

#endif