#include "EQ3List.h"

int EQ3List::Add (NimBLEAddress adr) // Return Anzahl an ELementen in der Liste
{
    Serial.print("--EQ3List::Add--");
    print ();
    Serial.printf ("now adding : %s\n",adr.toString().c_str());
    if (TheList.Anker==NULL) // Liste ist leer ... also einfügen
    {
        Serial.println("Erster Eintag in Liste.");
        //Liste ist leer dann einfügen  
        TheElement_t *TheNewElement = new TheElement_t(adr);
        cmdGetStream()->println("--EQ3List::Add-- 1 created");
        if (TheNewElement->Device->Connect())
        {
            cmdGetStream()->println("--EQ3List::Add-- 2 Connected");
            TheNewElement->Device->Status.m_owner = (uint32_t)0x00000000;
            TheNewElement->Next = NULL; 
            TheNewElement->Prev = NULL;  
            TheList.Tail    = TheNewElement;
            TheList.Anker   = TheNewElement;   

            TheList.Anzahl++;     
            cmdGetStream()->println("--EQ3List::Add-- Added");
        }
        else cmdGetStream()->println("--EQ3List::Add-- Connect fehlgeschlagen 1");     
    }
    else 
    {  
        Serial.println("nächster Eintag in Liste aufnehmen ... "); 
        if (!isInList(adr)) //prüfe ob schon in Liste
        {                   // nein also einfügen
            Serial.println("ist noch nicht erfasst");
            TheElement_t *TheNewElement = new TheElement_t(adr);
            if (TheNewElement->Device->Connect())
            {
                TheNewElement->Device->Status.m_owner = (uint32_t)0x00000000;
                TheNewElement->Next = TheList.Anker; 
                TheNewElement->Prev = NULL;
                TheList.Anker->Prev = TheNewElement;

                TheList.Anker = TheNewElement;

                TheList.Anzahl++;
                cmdGetStream()->println("--EQ3List::Add-- Added");
            }
            else cmdGetStream()->println("--EQ3List::Add-- Connect fehlgeschlagen 2");
        }
        else Serial.println("ist bereits erfasst");
        
    }
    return TheList.Anzahl;
};

bool EQ3List::isInList (NimBLEAddress adr)
{
    bool found = false;
    if (TheList.Anker!=NULL)
    {
        TheElement_t *Act = TheList.Anker;
        do
        {
            found = (Act->Device->Status.m_address==adr);  
            Act = Act->Next;                  
        } while ((Act!=NULL) & (!found));
    }
    return found;
};

TheElement_t* EQ3List::getElement(NimBLEAddress adr)
{
    TheElement_t *found = NULL;
    if (TheList.Anker!=NULL)
    {
        TheElement_t *Act = TheList.Anker;
        do
        {
            if (Act->Device->getAddress()==adr)
                found = Act;  
            Act = Act->Next;                  
        } while ((Act!=NULL) & (found!=NULL));
    }
    return found;
};

int EQ3List::Delete (NimBLEAddress adr)
{   
    if (isInList(adr))
    {
        TheElement_t *Found = getElement(adr);
        if (TheList.Anzahl==1)
        {
            TheList.Anker = NULL;
            TheList.Tail = NULL;
        }
        else if (Found == TheList.Anker)
        {
            TheList.Anker = Found->Next;
            TheList.Anker->Prev = NULL;
        }
        else if (Found == TheList.Tail)
        {
            TheList.Tail = Found->Prev;
            TheList.Tail->Next = NULL;
        }
        else 
        {
            Found->Prev->Next = Found->Next;
            Found->Next->Prev = Found->Prev;
        }

        free (Found);
        TheList.Anzahl--;
    }
    return TheList.Anzahl;
};

int EQ3List::ClearList()
{
    TheElement_t *Act = TheList.Anker;
    while (TheList.Anker!=NULL)
    {
        TheList.Anker = Act->Next;
        free (Act);
        Act = TheList.Anker;
        TheList.Anzahl--;
    }
    TheList.Tail = NULL;
    return 0;
};

bool EQ3List::RequestStatusFromAll ()
{
    bool ret=true;
    if (TheList.Anzahl>0)
    {
        TheElement_t *Act = TheList.Anker;
        do
        {
            cmdGetStream()->print (Act->Device->getAddress().toString().c_str());
            ret &= Act->Device->sendRequestForStatus();
            Act = Act->Next;
        } while (Act!=NULL);
    }
    return ret;
};

void EQ3List::print ()
{
    Serial.println  ("-- EQ3List Status --");
    Serial.printf   (" Anzahl an Einträgen : %d\n", TheList.Anzahl);    
};

DynamicJsonDocument EQ3List::toJSON()
{
    DynamicJsonDocument Doc(1000);
    JsonObject Devices = Doc.createNestedObject  ("Devices");

    TheElement_t *Act = TheList.Anker;
    Serial.println ("Generiere JSON ...");
    //JsonObject Device;
    
    while (Act!=NULL)
    {   
        String s = Act->Device->Status.m_address.toString().c_str(); 
        JsonObject Device = Devices.createNestedObject(s);
        
        s = NimBLEAddress(ESP.getEfuseMac()).toString().c_str();
        JsonObject Seen   = Device.createNestedObject(s);
        Seen["RSSI"]=Act->Device->Status.m_rssi;
        
        
        Device["Owner"]= "none";

        Serial.printf ("Datensatz : %s RSSI : %d \n",Act->Device->Status.m_address.toString().c_str(),Act->Device->Status.m_rssi);

        Act = Act->Next;
    }
    //serializeJsonPretty(Doc, Serial);
    return Doc;
};