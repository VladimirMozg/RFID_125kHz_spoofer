#include <SoftwareSerial.h>

SoftwareSerial mySerial(3, 11); // RX, TX

uint8_t buffer1[14];
uint8_t* buffer_at;
uint8_t* buffer_end = buffer1 + sizeof(buffer1);

uint8_t bufferCounter = 0;//счётчик итерраций первого цикла вайл
 
String checksum;
boolean tagfound = false;
 
void setup()
{
    Serial.begin(9600);
    Serial.println("Serial Ready");
 
    mySerial.begin(9600);
    Serial.println("RFID Ready");
}
 
void loop()
{
    if (mySerial.available())// если на чипе есть данные
    {
        delay(20);
        buffer_at = buffer1;
 
        while ( buffer_at < buffer_end )
        {
            *buffer_at++ = mySerial.read();//считываем данные с чипа, здесь получилось 14 итерраций
            bufferCounter++;//++++++++++++++++++++++
        }
        
        Serial.println(bufferCounter);
        bufferCounter = 0;
        
        tagfound = true;
       // Serial1.end();
       // Serial1.begin(9600);
    }
 
    if (tagfound)
    {
        buffer_at = buffer1;
        uint32_t result = 0;
 
        // Skip the preamble
        ++buffer_at;
        // Accumulate the checksum, starting with the first value
        uint8_t checksum = rfid_get_next();
        
        // We are looking for 4 more values
        int i = 4;
        while(i--)//------------------------------------------- классный трюк
        {//---------------------------------------------------- здесь считываются данные
            // Grab the next value
            uint8_t value = rfid_get_next();
            // Add it into the result
            result <<= 8;
            result |= value;
            // Xor it into the checksum
            checksum ^= value;
            
            Serial.println(value, DEC);
        }
        // Pull out the checksum from the data
        uint8_t data_checksum = rfid_get_next();
 
        // Print the result
        Serial.print("Tag: ");
        Serial.print(result, DEC);
        if ( checksum == data_checksum )
            Serial.println(" OK");
        else
            Serial.println(" CHECKSUM FAILED");
        // We're done processing, so there is no current value
 
        tagfound = false;
    }
 
}
 
uint8_t rfid_get_next(void)//здесь происходит считывание метки через RDM6300
{
    // sscanf needs a 2-byte space to put the result but we
    // only need one byte.
    uint16_t hexresult;
    // Working space to assemble each byte
    static char byte_chars[3];
    // Pull out one byte from this position in the stream
    snprintf(byte_chars,3,"%c%c",buffer_at[0],buffer_at[1]);
    sscanf(byte_chars,"%x",&hexresult);
    buffer_at += 2;
    return static_cast<uint8_t>(hexresult);
}
