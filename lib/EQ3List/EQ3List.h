#ifndef EQ3LIST_H
#define EQ3LIST_H

#include <ArduinoJson.h>
#include <NimBLEAddress.h>
#include <WiFi.h>
#include <inttypes.h>
#include "EQ3Device.h"
#include "Task.h"

struct TheElement_t
{      
    TheElement_t (NimBLEAddress adr)
    {
        Device = new EQ3Device(adr);
        Device->Status.m_name = "Neues Element";

    };      
    EQ3Device       *Device;
    TheElement_t    *Next = NULL;
    TheElement_t    *Prev = NULL;
};
struct TheList_t
{
    int Anzahl = 0;
    TheElement_t *Anker = NULL;
    TheElement_t *Tail  = NULL;
};

class EQ3List : public Task
{
    private:
        Uuint64_t   theadr;
             
    public:
        TheList_t TheList;

        void run(void *data) { };

        EQ3List ()
        { 
            TheList.Anker = NULL; 
            TheList.Tail  = NULL;
            TheList.Anzahl = 0;
        };
        /**
         * @brief Fügt einen EQ3 Thermostat der Liste hinzu
         *
         * @param [in] NimBLEAddress MAC Adresse des Thermostats
         * @param [in] int RSSI der ermittelt wurde
         * @param [out] int Anzahl der Elemente in der Liste
         */
        int Add (NimBLEAddress adr);   /*!< Return Anzahl an ELementen in der Liste */ 
        bool isInList (NimBLEAddress adr);           /*!< Return Bool */
        TheElement_t* getElement(NimBLEAddress adr);   /*!< Return das gefundene Element, ansonsten NULL */
        int Delete (NimBLEAddress adr);         /*!< Return Anzahl an ELementen in der Liste */
        int ClearList();                        /*!< Return Anzahl an ELementen in der Liste */
        DynamicJsonDocument toJSON();
        bool RequestStatusFromAll ();
        void print ();

        bool LoadFromSPIFFS (String filename)
        {
            return true;
        };
        bool SaveToSPIFFS (String filename)
        {
            return true; 
        };

        
};

#endif