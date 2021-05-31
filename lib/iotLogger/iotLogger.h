#include    <Arduino.h>
#include    <SPIFFS.h>

// WRAPPERS for portability
#define iotPRINT(...)       Serial.print(PSTR(__VA_ARGS__))    /* Prints text */
#define iotPRINTv(...)      Serial.print(__VA_ARGS__)          /* Prints variables */
#define iotSERIALavaiable() Serial.available()                 /* Checks if serial is avaiable */
#define iotSERIALread()     Serial.read()                      /* Serial Read wrapper */
#define iotMillis()         millis()                           /* millis() Macro wrapper */
#define iotDelay(...)       delay(__VA_ARGS__)                 /* Delay wrapper */

enum ts_mode {  
    NO_TIMESTAMP,       /* No time-stamp provided by user (time-log disabled).                     */
    AUTO_TIMESTAMP,     /* Time-stamp generated automatically (using millis() function).           */
    USER_TIMESTAMP,     /* User provided time-stamp. Recommended use of "epoch" type time-stamp    */ 
    // NO_PERSISTENCE,     /* No file persistence for the logger (just stored in ram) */
    // FILE_PERSISTENCE    /* Uses SPIFFS for file persistence*/
};

enum iotLogger_errno {
    NO_DEBUG,
    BUF_INIT_ERROR,     /* Error allocating buffer in memory (HEAP).        */
    STORAGE_INIT_ERROR, /* Error starting SPIFFS file-system.               */
    UNEXPECTED_ERROR,   /* Found buffer data corruption.                    */
    FILE_OPEN_ERROR,    /* Error when opening SPIFFS file                   */
    FILE_DPRINT_ERROR,  /* Error when printing DATA to SPIFFS file          */
    FILE_TSPRINT_ERROR, /* Error when printing TIMESTAMP to SPIFFS file     */
    CRITICAL,           /*       ## CRITICAL ERROR SEPARATOR ##             */
    BUFFER_EMPTY,       /* Buffer is empty.                                 */
    INDEX_OUT_OF_RANGE, /* Entered index is out of range.                   */
    SERIAL_TIMEDOUT,    /* Serial port timedout.                            */ 
    NO_TS_UNAVAILABLE,  /* Not available in NO_TIMESTAMP mode.              */
    NOT_FRAGMENTED,    /* Buffer not fragmented. Defrag not needed.        */ 
    WARNING,            /*       ## WARNING ERROR SEPARATOR  ##             */
    NOT_FOUND,          /* Value not found in buffer.                       */
    VERBOSE,            /*       ## VERBOSE ERROR SEPARATOR  ##             */
};

class   iotLogger;
//typedef uint8_t iotLogger_errno;

class iotLogger{

    private :
    const char*     FILE_NAME;                /* File-name */
    float*          D_BUFFER;                 /* Data buffer */
    unsigned long*  TS_BUFFER;                /* Time-stamp buffer */
    uint16_t        BUFFER_SIZE;              /* Size of data buffer (number of elements). Also of time-stamp buffer */
    ts_mode         TS_MODE;                  /* Time-Stamp mode */
    iotLogger_errno DEBUG_LEVEL;              /* Debug level.*/


    /**** Control Variables ****/
    float           consumed_data = __FLT_MAX__;
    unsigned long   consumed_timestamp = 0;
    uint16_t        store_index = 0;           /* Index to store data (oldest data or empty space on buffer) */ 
    uint16_t        consume_index = 0;         /* Index to consume data (oldest data) */
    bool            buffer_isCircular = false; /* TRUE if buffer is in circular mode (store index restarted to 0) */
    bool            consume_isCircular = false;/* TRUE if consume_index has reached end of buffer and restarted from 0 */
    bool            buffer_isPopped = false;   /* Marks if items has been popped off the buffer */

    public:
    iotLogger(  uint16_t BUFFER_SIZE,               /* Number of elements to store (used memory x2.5 this value) */
                const char* FILE_NAME,              /* Name of the file to store logs */
                ts_mode TS_MODE = AUTO_TIMESTAMP    /* Time-stamp mode to be used */
            );

    bool init();
    void end();
    void add(float data = 0, unsigned long timestamp = 0);
    bool getOldest(float &data_out, unsigned long &timestamp_out, bool peek = false);
    bool getOldest(float &data_out, bool peek = false);     //NO_TIMESTAMP overload
    bool popWhereData (unsigned long &timestamp_out, float data_in, bool peek = true);
    bool popWhereTimestamp(float &data_out, unsigned long timestamp_in, bool peek = true);
    unsigned long getTimestampByIndex(uint16_t index);
    float getDataByIndex(uint16_t index);
    bool dumpBuffer(uint8_t chunk_size = 0, uint16_t start = 0, uint16_t end = 0);
    bool dataAvailable();
    void printErrno(iotLogger_errno errno);
    iotLogger_errno errno;
    bool defragment();

    private:    
    void incStoreIndex();
    void incConsumeIndex();
    void consumeData(uint16_t index);
    uint16_t searchValidIndex();
    void resetBuffer(bool just_indexes = false);
    bool isValidData(uint16_t index);
    void empty();
    void setErrno(iotLogger_errno errno,uint8_t error_level = VERBOSE);


    /**
     * SPIFFS File System related functions
     */

    private: 
    bool fileToMemory();
    bool memoryToFile();
    bool fileStats();
    bool fileExists();
    bool fileDelete();

};

// class iotLoggertoSPIFFS : public iotLogger{

//    public:
//    void empty(); 
// };