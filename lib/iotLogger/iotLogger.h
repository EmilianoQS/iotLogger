#include    <Arduino.h>
#include    <SPIFFS.h>

// WRAPPERS for portability
#define iotPRINT(...)       Serial.print(PSTR(__VA_ARGS__))    /* Prints text */
#define iotPRINTv(...)      Serial.print(__VA_ARGS__)          /* Prints variables */
#define iotSERIALavaiable() Serial.available()                 /* Checks if serial is avaiable */
#define iotSERIALread()     Serial.read()                      /* Serial Read wrapper */
#define iotMillis()         millis()                           /* millis() Macro wrapper */
#define iotDelay(...)       delay(__VA_ARGS__)                 /* Delay wrapper */

enum {  
    NO_TIMESTAMP,       /* No time-stamp provided by user (time-log disabled).                      */
    AUTO_TIMESTAMP,     /* Time-stamp generated automatically using millis() function.              */
    USER_TIMESTAMP      /* User provided time-stamp. Recommended use of "epoch" type times-stamp    */ 
};

enum iotLogger_errno {
    BUF_INIT_ERROR,     /* Error allocating buffer in memory (HEAP) */
    STORAGE_INIT_ERROR, /* Error starting SPIFFS file-system */
    BUFFER_CORRUPTED,   /* Found buffer data corruption.*/
    CRITICAL,
    BUFFER_EMPTY,       /* Buffer is empty */
    INDEX_OUT_OF_RANGE, /* Entered index is out of range */
    SERIAL_TIMEDOUT,    /* Serial port timedout */ 
    WARNING,
    VERBOSE,
};

class   iotLogger;
//typedef uint8_t iotLogger_errno;

class iotLogger{

    //Corregir variables que deben ser privadas.

    const char*     FILE_NAME;                /* File-name */
    float*          D_BUFFER;                 /* Data buffer */
    unsigned long*  TS_BUFFER;                /* Time-stamp buffer */
    uint16_t        BUFFER_SIZE;              /* Size of data buffer (number of elements). Also of time-stamp buffer */

    /**** Control Variables ****/
    uint16_t        store_index = 0;           /* Index to store data (oldest data or empty space on buffer) */ 
    uint16_t        consume_index = 0;         /* Index to consume data (oldest data) */
    bool            buffer_isCircular = false; /* TRUE if buffer is in circular mode (store index restarted to 0) */
    bool            buffer_isPopped = false;   /* Marks if items has been popped off the buffer */
    

    public:
    iotLogger(uint16_t BUFFER_SIZE, const char* FILE_NAME, uint8_t TS_MODE);
    bool init();
    void end();
    void add(float data = 0, unsigned long timestamp = 0);
    bool getOldest(float &data_out, unsigned long &timestamp_out, bool peek = false);
    bool getItem(uint16_t index, float* data_out, unsigned long* timestamp_out);   //TODO
    uint8_t isData(unsigned long timestamp); //TODO

    bool dumpBuffer(uint8_t chunk_size = 0, uint16_t start = 0, uint16_t end = 0);
    bool dataAvailable();
    iotLogger_errno errno;

    private:

    void incDataIndex();
    void incConsumeIndex();
    void checkConsumeIndex();
    bool isValidData(uint16_t index);
    void empty();


    void setErrno(iotLogger_errno errno,uint8_t error_level = VERBOSE);

};