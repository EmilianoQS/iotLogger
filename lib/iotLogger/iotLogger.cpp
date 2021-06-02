#include "iotLogger.h"


iotLogger::iotLogger(uint16_t BUFFER_SIZE, ts_mode TS_MODE){

    this->BUFFER_SIZE = BUFFER_SIZE;
    this->TS_MODE = TS_MODE; 
}

/**
 * @brief Destroy the iot Logger::iot Logger object
 * 
 */
iotLogger::~iotLogger(){
    free(D_BUFFER);
    if(TS_MODE != NO_TIMESTAMP) 
        free(TS_BUFFER);
}

/**
 * @brief Initializes class. Creates buffers in memory.
 * 
 * @return TRUE if initialization OK.  
 */
bool iotLogger::init(){

    // D_BUFFER = (float*)malloc(BUFFER_SIZE * sizeof(float));
    D_BUFFER = (float*)calloc(BUFFER_SIZE,sizeof(D_BUFFER));           //Allocates and initialize DATA buffer
    if (D_BUFFER == NULL){
        setErrno(BUF_INIT_ERROR,CRITICAL);
        return false;
	}

    if(TS_MODE != NO_TIMESTAMP){
        TS_BUFFER = (unsigned long*)calloc(BUFFER_SIZE,sizeof(TS_BUFFER));  //Allocates and initialize TIMESTAMP buffer
        if (TS_BUFFER == NULL){

            setErrno(INIT_ERROR_MEMORY, CRITICAL);
            free(D_BUFFER);
            return false;
	    }
    }


    return true;
}

void iotLogger::add(float data, unsigned long timestamp){
    D_BUFFER[store_index] = data;

    switch (TS_MODE)
    {
        case NO_TIMESTAMP:
            return;
        break;
        
        case AUTO_TIMESTAMP:
            TS_BUFFER[store_index] = iotMillis();
        break;

        case USER_TIMESTAMP:
            TS_BUFFER[store_index] = timestamp;
        break;
    }
    incStoreIndex();
    return;
}

/**
 * @brief Returns the oldest data in the buffer.
 * 
 * @param[out] data_out -float- Variable where data is going to be stored
 * @param[out] timestamp_out -unsigned long- Variable where timestamp is going to be stored
 * @param peek If TRUE it doesn't deletes consumed data from buffer.
 * 
 * @return FALSE if no data is available.
 */
bool iotLogger::getOldest(float &data_out,unsigned long &timestamp_out, bool peek){

    if(TS_MODE == NO_TIMESTAMP){
        setErrno(NO_TS_UNAVAILABLE, CRITICAL);
        return false;
    }
    //Checks if data is avaiable, if not, returns false.
    if(!dataAvailable()){
        setErrno(BUFFER_EMPTY,WARNING);
        return false;
    }
    // Serial.print(D_BUFFER[consume_index]);
    // Serial.print(TS_BUFFER[consume_index]);
    data_out = D_BUFFER[consume_index];
    timestamp_out = TS_BUFFER[consume_index];
    consumeData(consume_index);
    //D_BUFFER[consume_index] = consumed_data;    //Stores consumed marker in consumed index.

    if(!peek){
        incConsumeIndex();
    }
    return true;
}

bool iotLogger::getOldest(float &data_out, bool peek){
    //Checks if data is avaiable, if not, returns false.
    if(!dataAvailable()){
        setErrno(BUFFER_EMPTY,WARNING);
        return false;
    }
    // Serial.print(D_BUFFER[consume_index]);
    // Serial.print(TS_BUFFER[consume_index]);
    data_out = D_BUFFER[consume_index];
    consumeData(consume_index);
    // D_BUFFER[consume_index] = consumed_data;    //Stores consumed marker in consumed index.

    if(!peek){
        incConsumeIndex();
    }
    return true;
}
/**
 * @brief Searchs for the provided data in the buffer. If it exists, stores
 *        its timestamp in the passed variables. If data is not found, returns FALSE.
 *        If peek = FALSE, this function will remove the data from the buffer. That
 *        causes and overhead on every other fetch operation over this buffer (getOldest())
 *        until bufferDefrag() is used.
 *        Returns oldest log found.
 * 
 * @param timestamp_out Variable to store timestamp if found.
 * @param data_in Data to search.
 * @param peek TRUE -> Does not deletes the data from buffer (FAST). FALSE -> Deletes data (SLOW).
 * @return true -> Data found on the buffer and loaded on variables
 * @return false -> Data not found.
 */
bool iotLogger::popWhereData(unsigned long &timestamp_out, float data_in, bool peek){

    if(TS_MODE == NO_TIMESTAMP){
        setErrno(NO_TS_UNAVAILABLE, WARNING);
        return false;
    }
    uint16_t data_index = (BUFFER_SIZE+1);
    //Because we want to retrieve the oldest match, we start from consume_index to the future.
    for(uint16_t i = consume_index ; i<BUFFER_SIZE ; i++){
        if(D_BUFFER[i] == data_in){
            data_index = i;
            break;
        }
    }
    if(data_index == (BUFFER_SIZE+1) ){
        for(uint16_t i = 0 ; i < consume_index ; i++){
            if(D_BUFFER[i] == data_in){
                data_index = i;
            break;
            }
        }
    }

    if(data_index == (BUFFER_SIZE+1) ){
        setErrno(NOT_FOUND, VERBOSE);
        return false;
    }
    timestamp_out = TS_BUFFER[data_index];
    if(peek){
        return true;
    }
    consumeData(data_index);

    //Must be carefull if the index of the selected data is equals to consumed_index or store_index
    if(data_index == store_index){
        incStoreIndex();
    }
    //If store_index == consume_index, this if will never be true, because incStoreIndex() will
    //increase consume_index too.
    if(data_index == consume_index){
        incConsumeIndex();
    }

    //D_BUFFER[data_index] = consumed_data;
    
    buffer_isPopped = true;

    return true;
}

/**
 * @brief Searchs for the provided timestamp in the buffer. If it exists, stores
 *        its data in the passed variables. If timestamp is not found, returns FALSE.
 *        If peek = FALSE, this function will remove the data from the buffer. That
 *        causes and overhead on every other fetch operation over this buffer (getOldest())
 *        until bufferDefrag() is used.
 *        Returns oldest log found.
 * 
 * @param data_out Variable to store data if found
 * @param timestamp_in Timestamp to search in the buffer.
 * @param peek TRUE -> Doesn't delete data from buffer
 * @return true 
 * @return false 
 */
bool iotLogger::popWhereTimestamp(float &data_out, unsigned long timestamp_in, bool peek){
    if(TS_MODE == NO_TIMESTAMP){
        setErrno(NO_TS_UNAVAILABLE, WARNING);
        return false;
    }

    uint16_t timestamp_index = (BUFFER_SIZE+1);
    for(uint16_t i = consume_index ; i<BUFFER_SIZE ; i++){
        if(TS_BUFFER[i] == timestamp_in){
            //We must check validData because we mark consumed data only in D_BUFFER.
            if(isValidData(i)){
                timestamp_index = i;
                break;
            }
            continue;
        }
    }
    if(timestamp_index == (BUFFER_SIZE+1) ){
        for(uint16_t i = 0 ; i < consume_index ; i++){
            if(TS_BUFFER[i] == timestamp_in){
                if(isValidData(i)){
                    timestamp_index = i;
                    break;
                }
                continue;
            }
        }
    }

    if(timestamp_index == (BUFFER_SIZE+1) ){
        setErrno(NOT_FOUND, VERBOSE);
        return false;
    }


    data_out = D_BUFFER[timestamp_index];
    if(peek){
        return true;
    }
    consumeData(timestamp_index);

    //Must be carefull if the index of the selected data is equals to consumed_index or store_index
    if(timestamp_index == store_index){
        incStoreIndex();
    }
    if(timestamp_index == consume_index){
        Serial.print("\nEntro a incConsumeIndex();");
        incConsumeIndex();
    }
    
    buffer_isPopped = true;

    return true;
}

/**
 * @brief Prints to console the content of the buffer. It can print just a part of 
 * the buffer (defining "start" and "end"), or the entire buffer. The buffer can
 * be printed in "chunks". That is to avoid filling the serial monitor when printing
 * big buffers. To print more chunks you have to enter any character on the serial monitor.
 * 
 * @param chunk_size Amount of logs printed for every chunk.
 * @param start Start index to print.
 * @param end End index to print.
 * @return false -> Wrong indexes (out of range) 
 */
bool iotLogger::dumpBuffer(uint8_t chunk_size, uint16_t start, uint16_t end){
    //First we must validate inputs.
    if( (end >= BUFFER_SIZE) || (chunk_size >= BUFFER_SIZE) || 
        (start >= BUFFER_SIZE) ){
        setErrno(INDEX_OUT_OF_RANGE, WARNING);
        return false;
    }
    if(end == 0){
        end = BUFFER_SIZE-1;
    }
    
    uint8_t printedlogs = 0;
    unsigned long prevMillis = iotMillis();
    iotPRINT("\n####################### LOG DUMP #########################\n");
    if(DEBUG_LEVEL == VERBOSE){
        iotPRINT("\n # consume_index = ");
        iotPRINTv(consume_index);
        iotPRINT("\n # store_index = ");
        iotPRINTv(store_index);
        iotPRINT("\n # buffer_isCircular = ");
        iotPRINTv(buffer_isCircular);
        iotPRINT("\n # buffer_isPopped = ");
        iotPRINTv(buffer_isPopped);
        iotPRINT("\n # consume_isCircular = ");
        iotPRINTv(consume_isCircular);
    }
    if(TS_MODE == NO_TIMESTAMP){
        iotPRINT("\n # INDEX #            # ########## #             # DATA # ");
    }else{
        iotPRINT("\n # INDEX #            # TIME-STAMP #             # DATA # ");
    }
    for(uint16_t i = start ; i<= end; i++){
        //If chunk_size = 0, we print everything in one chunk.
        if( (chunk_size != 0) && (printedlogs >= chunk_size) ){
            iotPRINT("\n Send any character to load more logs . . .");
            while(!iotSERIALavaiable()){
                // To avoid blocking everything in case of error. We use 8sec timeout.
                if( (iotMillis() - prevMillis) >= 8000 ){
                    setErrno(SERIAL_TIMEDOUT, WARNING);
                    return false;
                }
                iotDelay(1);
            }
            while(iotSERIALavaiable()){
                // When serial becomes available, we flush the serial buffer.
                iotSERIALread();
            }
            printedlogs = 0;
        }

        iotPRINT("\n     ");
        iotPRINTv(i);
        iotPRINT("                     ");
        if(TS_MODE == NO_TIMESTAMP){
            iotPRINT("________");
        }else{
            iotPRINTv(TS_BUFFER[i]);
        }

        iotPRINT("                     ");
        iotPRINTv(D_BUFFER[i]);
        // Serial.print("\n#Data: ");
        // Serial.print(D_BUFFER[i]);
        printedlogs++;
    }
    iotPRINT("\n##########################################################\n");
    return true;
}

/**
 * @brief Checks if there's data available to consume in the buffer.
 * 
 * @return true  -> Data available
 */
bool iotLogger::dataAvailable(){

    /* Possible cases:
    -- NOT CIRCULAR BUFFER --
    store_index if empty. 
    If consume_index = store_index => NO DATA. (###)
    If consume_index < store_index => Data available
       consume_index > store_index => Prohibited
    
    -- CIRCULAR BUFFER --
     -NOT CIRCULAR CONSUME
     consume_index = store_index => Data available (at the past of store_index)
     consume_index > store_index => Data available (same previous)
     consume_index < store_index => Prohibited (not circular consume)

     -CIRCULAR CONSUME
     consume_index = store_index => NO DATA. (###) Buffer should reset in incConsumeIndex();
     consume_index < store_index => Data available 
     consume_index > store_index => Prohibited (buffer should have resetted previously)

     ---POPPED BUFFER --
     -NOT circular buffer: must check just in the past of store_index
     -Circular buffer: must check entire buffer for data.
     
    */

    if(!buffer_isPopped){
        if(!buffer_isCircular){
            //Not circular and not popped buffer.
            if(consume_index == store_index){
                return false;
            }else{
                return true;
            }
        }else{
            //Circular and not popped buffer.
            if(buffer_isCircular && consume_isCircular
                && (consume_index == store_index) ){
                //resetBuffer();
                return false;
            }
            return true;
        }
    }else{
    }
    return true;
}

/**
 * @brief DEBUG FUNCTION.
 * 
 * @param index 
 * @return unsigned long 
 */
unsigned long iotLogger::getTimestampByIndex(uint16_t index){
    return TS_BUFFER[index];
}

/**
 * @brief DEBUG FUNCTION.
 * 
 * @param index 
 * @return unsigned long 
 */
float iotLogger::getDataByIndex(uint16_t index){
    return D_BUFFER[index];
}
//////////////////// PRIVATE ///////////////////////

/**
 * @brief Increments the data index, checks for buffer max size (circular),
 *        and resets indexes as needed.
 * 
 */
void iotLogger::incStoreIndex(){

    /* Possible cases:
    -- NOT CIRCULAR BUFFER --
    consume_index < store_index => store_index++
    consume_index+1 == BUFFER_SIZE => consume_index = 0 ; buffer_isCircular TRUE

    -- CIRCULAR BUFFER --
    consume_index == store_index => consume_index = store_index
    store_index > consume_index  => store_index++ (check buffer end)
    store_index < consume_index  => store_index++ (check buffer end)
    */

   if(!buffer_isCircular){
       //NOT Circular buffer
        if( (store_index+1) >= BUFFER_SIZE ){
           store_index = 0;
           buffer_isCircular = true;
           return;
        }
        //If not in the last position, just increment.
        store_index++;
        return;
    }else{
        //Circular buffer
        uint16_t prev_store_index = store_index;
        if( (store_index+1) >= BUFFER_SIZE ){
            store_index = 0;
        }else{
            store_index++;
        }

        //If both indexes where together, they must keep together.
        if(prev_store_index == consume_index){
            consume_index=store_index;
        }

        if(buffer_isPopped){
            //If buffer is popped, we must check consume_index is correct.
            uint16_t temporal_index = searchValidIndex();
            if(temporal_index == (BUFFER_SIZE+1) ){
                //This shouldn't be an option. We've just added a log!
                setErrno(UNEXPECTED_ERROR, CRITICAL);
                Serial.print("\nDEBUG MEEEE 990");
                resetBuffer();
                return;
            }else{
                consume_index = temporal_index;
            }
        }
       return;
    //     if( (store_index+1) >= BUFFER_SIZE ){
    //         //If both indexes where together, they must keep together.
    //         if(store_index == consume_index){
    //             consume_index = 0;
    //         }
    //         store_index = 0;
    //         return;
    //     }
    //     //If both indexes where together, they must keep together.
    //     if(store_index == consume_index){
    //         consume_index++;
    //     }
    //    store_index++;
    //    return;
    }
    return;
}

/**
 * @brief Increments consume index. And checks if it's necessary to reset
 *        the buffer, update control variables, etc.
 */
void iotLogger::incConsumeIndex(){

    /* Possible cases:
    -- NOT CIRCULAR BUFFER --
    consume_index == store_index => NO DATA (dataAvailable() => FALSE)
    consume_index+1 < store_index => consume_index++
    consume_index+1 = store_index => resetBuffer()

    -- CIRCULAR BUFFER --
    --- consume_circular = FALSE
    consume_index+1 = BUFFER_SIZE => consume_index = 0 ; consume_circular = TRUE
    consume_index++;

    --- consume_circular = TRUE
    consume_index+1 < store_index => searchValidIndex();
    consume_index+1 > store_index => consume_index++ ; Check BUFFER_SIZE
    consume_index+1 == store_index => resetBuffer();
    */

   // Wathever is the case, the popped data will always be "in the future" of consume_index.

   if(!buffer_isCircular){
        //NOT Circular buffer
        //Consume can't be circular!

        //If the buffer is popped then the data hole is in between 0 and (BUFFER_SIZE-1) indexes.
        //Must fint next valid index, and if not found, then resetBuffer.
        if(buffer_isPopped){
            uint16_t temporal_index = searchValidIndex();
            if(temporal_index == (BUFFER_SIZE+1) ){
                resetBuffer();
                return;
            }else{
                consume_index = temporal_index;
                return;
            }
        }
        if( (consume_index+1) == store_index){
            resetBuffer();
            return;
        }
        if( (consume_index+1) < store_index ){
            consume_index++;
            return;
        }
    }else{
        if(buffer_isPopped){
            uint16_t temporalConsumeIndex = searchValidIndex();
            Serial.print("\nsearchValidIndex = ");
            Serial.print(temporalConsumeIndex);
            if(temporalConsumeIndex == (BUFFER_SIZE+1) ){
                resetBuffer();
                return;
            }

            //Checking if consume_index is circular now... (is nessesary?)
            if(temporalConsumeIndex < consume_index){
                consume_isCircular = true;   
            }
            consume_index = temporalConsumeIndex;
            return;
        }

        // Serial.print("\n#incConsumeIndex => Circular buffer");
        //Circular buffer
        if(!consume_isCircular){
            //NOT circular consume
            if( (consume_index+1) == BUFFER_SIZE ){
                consume_index = 0;
                if(consume_index == store_index){
                    resetBuffer();
                    return;
                }
                consume_isCircular = true;
                return;
            }
                consume_index++;
                return;
        }else{
            // Serial.print("\n#incConsumeIndex => YES circular consume");
            //Circular consume
            if( (consume_index+1) == store_index ){
                resetBuffer();
                return;
            }
            if( (consume_index+1) == BUFFER_SIZE ){
                consume_index = 0;
                if(consume_index == store_index){
                    resetBuffer();
                    return;
                }
            }
            if( (consume_index+1) < store_index ){
                consume_index++;
                // uint16_t temporalConsumeIndex = searchValidIndex();
                // if(temporalConsumeIndex == (BUFFER_SIZE+1) ){
                //     //No valid data found in the buffer.
                //         // Serial.print("\nsearchValidIndex(): ");
                //         // Serial.print(temporalConsumeIndex);
                //     resetBuffer();
                //     return;
                // }
                //     // Serial.print("\n#incConsumeIndex => After searchValidIndex error check");
                // consume_index = temporalConsumeIndex;
                //     // if(consume_index == store_index){
                //     //    resetBuffer();
                //     //    return;
                //     // }
                // return;
            }
        }
    }
    return; //To avoid warning.
}

/**
 * @brief Returns the next (oldest data) index with valid data. Starts searching from  
 *        current consume_index.
 * 
 * @return uint16_t Index to valid data or (BUFFER_SIZE+1) if not valid data is found.
 */
uint16_t iotLogger::searchValidIndex(){
    
    //Searching from consume_index to the end of the array
    // Serial.print("\n#searValidIndex => Start consume_index: ");
    // Serial.print(consume_index);
    for(uint16_t i = consume_index ; i < BUFFER_SIZE ; i++){
        if(isValidData(i)){
            // Serial.print("\n#searValidIndex => FOR 1 return consume_index: ");
            // Serial.print(i);
            // Serial.print("\nAnalized Data: ");
            // Serial.print(D_BUFFER[i]);
            return i;
        }
    }

    //If the buffer is not circular, then we don't need the following for loop.
    if(!buffer_isCircular){
        return (BUFFER_SIZE+1);
    }
    //Searching from beggining of the buffer to consume_index
    for(uint16_t i = 0 ; i < consume_index ; i++){
        if(isValidData(i)){
            // Serial.print("\n#searValidIndex => FOR 2 return consume_index: ");
            // Serial.print(i);
            return i;
        }
    }
    return (BUFFER_SIZE+1);
}

/**
 * @brief Resets the buffer (just Data buffer, no time-stamp buffer) and
 *        all control variables.
 * 
 */
void iotLogger::resetBuffer(bool just_indexes){
    Serial.print("\n resetBuffer() => CALLED\n");
    store_index = 0;
    consume_index = 0;
    consume_isCircular = false;
    buffer_isCircular = false;
    buffer_isPopped = false;

    // for(uint16_t i=0; i<BUFFER_SIZE; i++){
    //     D_BUFFER[i] = 0;
    //     if(TS_MODE != NO_TIMESTAMP)
    //         TS_BUFFER[i] = 0;
    // }
    if(just_indexes)
        return;
    
    memset(D_BUFFER, 0, BUFFER_SIZE*sizeof(float));
    if(TS_MODE != NO_TIMESTAMP)
        memset(TS_BUFFER, 0, BUFFER_SIZE*sizeof(unsigned long));
}

/**
 * @brief Erases (consumes) the data from the buffer.
 * 
 * @param index Index to consume the data.
 */
void iotLogger::consumeData(uint16_t index){
    D_BUFFER[index] = consumed_data;
    if(TS_MODE == NO_TIMESTAMP)
        return;
    TS_BUFFER[index] = consumed_timestamp;
}


/**
 * @brief Checks if data stored in index "index" is valid or not.
 * 
 * @param index Index to check for valid data.
 * @return true  -> Data is valid.
 */
bool iotLogger::isValidData(uint16_t index){
    //Cambiar al D_BUFFER para poder no utilizar time-stamp
    if(D_BUFFER[index] != consumed_data && TS_BUFFER[index] != consumed_timestamp){
        return true;
    }
    return false;
}



void iotLogger::setErrno(iotLogger_errno errno, uint8_t error_level){
    this->errno = errno;
    printErrno(errno);
    return;
}

void iotLogger::printErrno(iotLogger_errno errno){

    if(DEBUG_LEVEL == NO_DEBUG){
        return;
    }

    // Serial.print("\nError code:");
    // Serial.print(errno);
    bool must_print = false;
    if(errno < DEBUG_LEVEL){
        must_print = true;
    }

    switch (errno)
    {
    case BUF_INIT_ERROR:
        if(must_print)
            iotPRINT("\n#iotLogger ERROR: Can't intialize buffer. Enought memory?\n");
    break;

    case INIT_ERROR_MEMORY:
        if(must_print)
            iotPRINT("\n#iotLogger ERROR: Not enought memory to initializa TimeStamp Buffer.\n");
    break;

    case STORAGE_INIT_ERROR:
        if(must_print)
            iotPRINT("\n#iotLogger ERROR: Can't initialize file system.\n");    
    break;
    
    case UNEXPECTED_ERROR:
        if(must_print)
            iotPRINT("\n#iotLogger ERROR: Unexpected error.\n");
    break;

    case SPIFFS_INIT_ERROR:
        if(must_print)
            iotPRINT("\n#iotLogger ERROR: Error initializing SPIFFS.\n");
    break;

    case FILE_OPEN_ERROR:
        if(must_print)
            iotPRINT("\n#iotLogger ERROR: Error opening the file.\n");    
    break;

    case FILE_DPRINT_ERROR:
        if(must_print)
            iotPRINT("\n#iotLogger ERROR: Error when storing data to data file.\n");
    break;

    case FILE_TSPRINT_ERROR:
        if(must_print)
            iotPRINT("\n#iotLogger ERROR: Error when storing data to time-stamp file.\n");
    break;

    case CONVERSION_ERROR:
    if(must_print)
            iotPRINT("\n#iotLogger ERROR: Conversion from char array FAILED.\n");
    break;

    case DEFRAG_ERROR:
        if(must_print)
            iotPRINT("\n#iotLogger ERROR: Error when trying to defragment buffer.\n");
    break;

    case BUFFER_EMPTY:
        if(must_print)
            iotPRINT("\n#iotLogger ERROR: The buffer is empty.\n");
    break;

    case INDEX_OUT_OF_RANGE:
        if(must_print)
            iotPRINT("\n#iotLogger ERROR: Entered index is out of range.\n");
    break;

    case SERIAL_TIMEDOUT:
        if(must_print)
            iotPRINT("\n#iotLogger ERROR: Serial port timedout.\n");
    break;

    case NO_TS_UNAVAILABLE:
        if(must_print)
            iotPRINT("\n#iotLogger WARNING: Not available in NO_TIMESTAMP mode.\n");
    break;

    case NOT_FRAGMENTED:
        if(must_print)
            iotPRINT("\n#iotLogger VERBOSE: Buffer not fragmented.\n");
    break;

    case NOT_FOUND:
        if(must_print)
            iotPRINT("\n#iotLogger VERBOSE: Requested data not found.\n");

    break;

    case DEFRAG_OK:
        if(must_print)
            iotPRINT("\n#iotLogger VERBOSE: Defragmentation completed.\n");

    break;

    case FILE_NOT_EXISTS:
        if(must_print)
        iotPRINT("\n#iotLogger VERBOSE: File doesn't exists.\n");
    break;

    case FILE_LOAD_OK:
        if(must_print)
        iotPRINT("\n#iotLogger VERBOSE: File loaded OK.\n");
    break;

    default:
        if(DEBUG_LEVEL == VERBOSE)
            iotPRINT("\n#iotLogger ERROR: Unknown error code.\n");
    break;
    }
return;
}

/**
 * @brief This function defragments the buffer. This means it removes the
 * holes leaved there by popWhere** functions. To do this, it creates a copy
 * of the entire buffer in memory (RAM), so be carefull to have enought space.
 * By "entire buffer" it means it also coppies TIMESTAMP buffer. So to use this 
 * function we must have [...] of free space: 
 *  sizeof(float)*BUFFER_SIZE + sizeof(unsigned long)*BUFFER_SIZE
 * 
 */


bool iotLogger::defragment(){

    //First we'll check if there's data in the buffer.
    if(!dataAvailable()){
        //No data.
        setErrno(BUFFER_EMPTY, WARNING);
        return true;    //Or should be FALSE? ..... TODO
    }

    if(!buffer_isPopped && !buffer_isCircular){ 
        //If the buffer is not popped, then we don't need to defragment!
        //If the buffer is just circular (no popped) we don't need either, but
        //we'll do it anyways to avoid complications storing to file.
        setErrno(NOT_FRAGMENTED, WARNING);
        return true;    //Or should be FALSE? ..... TODO
    }

    // getMemoryStatistics();

    //Here we don't need to initialize elements to cero, so we use malloc.
    float* temp_D_BUFFER = (float*)calloc(BUFFER_SIZE,sizeof(D_BUFFER));   //Allocates and initialize DATA buffer
    unsigned long* temp_TS_BUFFER = nullptr;

    if(TS_MODE != NO_TIMESTAMP){
        temp_TS_BUFFER = (unsigned long*)calloc(BUFFER_SIZE,sizeof(TS_BUFFER));  //Allocates and initialize TIMESTAMP buffer

        if (temp_TS_BUFFER == NULL){
            setErrno(BUF_INIT_ERROR, CRITICAL);
            return false;
	    }
    }

    // getMemoryStatistics();

	if (temp_D_BUFFER == NULL){
        setErrno(BUF_INIT_ERROR, CRITICAL);
        return false;
	}

    //We'll copy every log to the temporal buffers in order, and then re-generate the indexes.
    bool parseEnd = false;     // Flag to mark when we finished parsing the entire buffer.
    uint16_t index = 0;
    uint16_t temp_consume_index;
 
    //In the first loop, temp_consume_index(=consume_index) is always valid data. 
    for(temp_consume_index = consume_index ; temp_consume_index < BUFFER_SIZE ; temp_consume_index++){
        if(isValidData(temp_consume_index)){
            temp_D_BUFFER[index] = D_BUFFER[temp_consume_index];
            if(TS_MODE != NO_TIMESTAMP)
                temp_TS_BUFFER[index] = TS_BUFFER[temp_consume_index];

            index++;    //index will always be +1 ahead of temp_consume_index
            continue;
        }
    }

    //If the buffer IS NOT in circular mode, then here we end.
    if(!buffer_isCircular){
        parseEnd = true;
    }

    //Only if we parsed the buffer from 0 -> END and the buffer IS circular... 
    if(!parseEnd){
    //We parse the buffer from 0 to consume_index (only in circular buffers)
        for(temp_consume_index = 0; temp_consume_index < consume_index ; temp_consume_index++){
            if(isValidData(temp_consume_index)){
                temp_D_BUFFER[index] = D_BUFFER[temp_consume_index];
                if(TS_MODE != NO_TIMESTAMP)
                    temp_TS_BUFFER[index] = TS_BUFFER[temp_consume_index];

                index++;
                continue;
            }
        }
    }

    //Here we have our defragmented buffer in temp_**_BUFFER arrays. We must find the new indexes.
    /* POSIBLE CASES
        - NOT circular buffer: Then our consume_index is the first element (0) and our store_index
                               is just "index" EXCEPT in the case the previous buffer was no fragmented
                               and all their indexes had "valid information" in that case:
                               index = BUFFER_SIZE. This shouldn't be possible.
        - Circular Buffer:     Then our consume_index is the first element (0), and the store_index is,
                               again, just "index"
    */

    resetBuffer(true);  //Just resetting indexes.

    // getMemoryStatistics();

    consume_index = 0;

    if(index >= BUFFER_SIZE){
        //Possible case when buffer is circular, not popped, and full.
        store_index = 0;
        //The new buffer has to start as circular so the consumed_index follows store_index.
        buffer_isCircular = true;
    }else{
        store_index = index;
    }
    memcpy(D_BUFFER, temp_D_BUFFER, sizeof(D_BUFFER)*BUFFER_SIZE);
    if(TS_MODE != NO_TIMESTAMP)
        memcpy(TS_BUFFER, temp_TS_BUFFER, sizeof(TS_BUFFER)*BUFFER_SIZE);

    
    free(temp_D_BUFFER);
    if(TS_MODE != NO_TIMESTAMP)
        free(temp_TS_BUFFER);

    // getMemoryStatistics();
    setErrno(DEFRAG_OK, VERBOSE);
    return true;
}

void iotLogger::getMemoryStats(){

    iotPRINT("\n\n # ESP32 Memory Statistics #");
    iotPRINT("\n # Free Internal heap size: ");     //Internal => Doesn't take in count external DRAM
    iotPRINTv(esp_get_free_internal_heap_size());
    iotPRINT("\n # Free heap size (all): ");        //Every available HEAP in the chip.
    iotPRINTv(esp_get_free_heap_size());
    iotPRINT("\n # Free Minimum heap size: ");      //Minimum allocable block (with ONE malloc)
    iotPRINTv(esp_get_minimum_free_heap_size());
    return;
}


/**************************************************************************
 * 
 *  iotLoggerFile class extension.
 * 
 *  This class extends the capabilities of the iotLogger class, to add the 
 *  feature of File Persistence.
 *  To implement File Persistence, this class uses SPIFFS File System. If 
 *  you want to use a different file system, then you have to do your own
 *  implementation of the following functions.
 * 
 **************************************************************************/

bool iotLoggerFile::init(){
    //Initializes iotLogger class (base class).
    if( !iotLogger::init() )
        return false;

    if(!SPIFFS.begin(true)){    //Si no dejamos el "format on fail" como TRUE, entonces el 1er clean run de codigo en una ESP falla (Failed to mount).
        setErrno(SPIFFS_INIT_ERROR, CRITICAL);
        return false;
    }

    if(fileExists()){
        if(fileToMemory()){
            if(DEBUG_LEVEL == VERBOSE)
                iotPRINT("\niotLogger => Buffer initialized from file.");
        
        return true;
        }
    }

    if(DEBUG_LEVEL == VERBOSE)
        iotPRINT("\niotLogger => No file found. Buffer intialized empty.");

    return true;
}

/**
 * @brief Destroy the iot Logger File::iot Logger File object
 *
 */
iotLoggerFile::~iotLoggerFile(){
}

void iotLoggerFile::setStoreInterval(unsigned long store_interval){
    this->store_interval = store_interval;
    return;
}
/**
 * @brief This function must run in a endless loop. If there's an error storing logs to
 * file, then it returns -1. If it's not the time to store the file, returns 0.
 * If the file has been stored successfully returns the time it took it to store the file
 * (>1)
 * 
 * @return long = 0 => Not time to store || =-1 => Error storing || >1 Store OK (time elapsed)
 */
long iotLoggerFile::storeFileTimed(){
  
    if((iotMillis() - prev_millis) >= store_interval){
        long time_elapsed = iotMillis();
        long total_time = -1;

        if(memoryToFile()){
            total_time = iotMillis() - time_elapsed;
            return total_time == 0 ? 1 : total_time;

        }else{
            return -1;
        }
    }
    
    return 0;
}

/**
 * @brief Stores the logs on the SPIFFS File. It overwrites the old file.
 *        No backup copy is made! Critical Section!
 * 
 * @return true 
 * @return false 
 */
bool iotLoggerFile::memoryToFile(){

    File iotFile;
    iotFile = SPIFFS.open(FILE_NAME, FILE_WRITE);
    if(iotFile < 0){
        setErrno(FILE_OPEN_ERROR, CRITICAL);
        return false;
    }

    if(!defragment()){
        setErrno(DEFRAG_ERROR, WARNING);
        return false;
    }

    // Configuration variables storing.
    iotFile.println(store_index);
    iotFile.println(buffer_isCircular);

    //Because the buffer has been defragmented, we'll have the consume_index = 0
    //and the store_index in the last position unless store_index = 0.

    uint16_t end = 0;
    uint16_t i = 0;
    uint32_t total_size = sizeof(buffer_isCircular) + sizeof(store_index);
    uint8_t data_size = sizeof(D_BUFFER);
    uint8_t timestamp_size = sizeof(TS_BUFFER);
    bool error = false;

    if(store_index == 0){
        end = BUFFER_SIZE;
    }else{
        end = store_index; //Last valid data in (store_index-1)
    }

    for(i = 0; i < end; i++){
        if(iotFile.println(D_BUFFER[i]) == 0){
            setErrno(FILE_DPRINT_ERROR, CRITICAL);
            error = true;
            break;
        }
        total_size += data_size;
        if(TS_MODE != NO_TIMESTAMP){
            if(iotFile.println(TS_BUFFER[i]) == 0){
                setErrno(FILE_DPRINT_ERROR, CRITICAL);
                error = true;
                break;
            } 
        }
        total_size += timestamp_size;
    }

    if(error){
        iotFile.close();
        if(i>1)     //File stored partially. NOT GOOD.            
            fileDelete();

        return false;
    }

    iotFile.close();
    
    if(DEBUG_LEVEL == VERBOSE){
        iotPRINT("\n iotLogger => File path: ");
        iotPRINTv(FILE_NAME);
        iotPRINT("  || File size: ");
        iotPRINTv(total_size);
    }
    
    return true;
}

/**
 * @brief Retrieves file from memory and loads it to RAM memory.
 * 
 * @return true 
 * @return false 
 */
bool iotLoggerFile::fileToMemory(){


    enum conf_variable{
        conf_store_index,           //uint16_t
        conf_buffer_isCircular      //bool (int)
    };

    if(!fileExists()){
        setErrno(FILE_NOT_EXISTS, VERBOSE);
        return true;
    }

    File iotFile;
    iotFile = SPIFFS.open(FILE_NAME, FILE_READ);
    if(iotFile < 0){
        setErrno(FILE_OPEN_ERROR, CRITICAL);
        return false;
    }

    //Setting the indexes to default values. Just to be sure.
    resetBuffer(true);

    char temp_buffer[20];           //Temporal buffer to store retrieved char array
    unsigned long temp_ul = 9999;   //Variable to store temporal time-stamp
    float temp_float = 9999;        //Variable to store temporal float
    uint16_t file_index = 0;        //readBytes returns the number of char readed
    bool config_done = false;       //Flag to mark configuratino variables loaded.
    bool isError = false;           //Flag to mark error
    uint8_t i = 0;                  //Config Switch index
    uint16_t index = 0;             //Memory buffer index (data/timestamp index)

    while(iotFile.available()){

        if(isError)
            break;          //Break the while.

        if(!config_done){

            file_index = iotFile.readBytesUntil('\n', temp_buffer, sizeof(temp_buffer));
            temp_buffer[file_index] = 0;       //null terminator.

            // Serial.println("\nCONFIG readed from file: ");
            // Serial.print(temp_buffer);
            // Serial.print("\0");

            if( !parseUL(temp_buffer, temp_ul) ){
                //Conversion error
                // Serial.println("\nFirst conversion error.");
                isError = true;
                break;      //Break the while.
            }


            // Serial.print("  || Parsed as: ");
            // Serial.print(temp_ul);
            
            //First 3 lines of the file, are the configuration variables.
            switch (i)
            {
            case conf_store_index:
                if(temp_ul < BUFFER_SIZE){
                    store_index = temp_ul;
                    i++;
                }else{
                    //ERROR
                    // Serial.println("\nConf_store_index error");
                    // isError = true;
                }           
            break;

            case conf_buffer_isCircular:
                if(temp_ul == 0){
                    buffer_isCircular = false;
                    config_done = true;
                    break;
                }

                if(temp_ul == 1){
                    buffer_isCircular = true;
                    config_done = true;
                    break;
                }else{
                    // Serial.println("\n Conf_buffer_isCircular error");
                    isError = true;
                }  
            break;
            }

            if(isError)
                break;  //Breaks while.

        }else{      //Config DONE.

            // FETCHING DATA
            file_index = iotFile.readBytesUntil('\n', temp_buffer, sizeof(temp_buffer));
            if(file_index == 0){    //No valid data readed.
                isError = true;
                break;
            }
                
            temp_buffer[file_index] = 0;    //null terminator.
            //Remember order {data , timestamp , data , timestamp , ...}
            if( !parseFloat(temp_buffer, temp_float)){
                isError = true;
                break;  //break the while
            }

            // Serial.print("\n################################################");
            // Serial.println("\n# DATA readed from file: ");
            // Serial.println(temp_buffer);
            // Serial.print("\n# Parsed as: ");
            // Serial.print(temp_float);
            // Serial.print("\n# Stored at index: ");
            // Serial.print(index);
            D_BUFFER[index] = temp_float;

            //FETCHING TIMESTAMP
            if(TS_MODE != NO_TIMESTAMP){
                file_index = iotFile.readBytesUntil('\n', temp_buffer, sizeof(temp_buffer));
                if(file_index == 0){    //No valid data readed.
                    isError = true;
                    break;
                }

                temp_buffer[file_index] = 0;    //null terminator.
                if( !parseUL(temp_buffer, temp_ul)){
                    isError = true;
                    break;
                }
                // Serial.print("\n$TIMESTAMP readed from file: ");
                // Serial.println(temp_buffer);
                // Serial.print("\n# Parsed as: ");
                // Serial.print(temp_ul);
                TS_BUFFER[index] = temp_ul; //timestamp to RAM buffer.
            }
            index++;
        }
        //Seguimos con el resto...
    }

    if(isError){
        setErrno(CONVERSION_ERROR, CRITICAL);
        iotFile.close();
        resetBuffer();      //Some indexes or values could have changed.
        return false;
    }

    setErrno(FILE_LOAD_OK, VERBOSE);
    return true;
}

/**
 * @brief Converts a char buffer containing a (unsigned long) number to unsigned long var type
 *        If function returns false, do not trust in ul_out value!
 * 
 * @param str  Char buffer to convert. Must be null terminated.
 * @param ul_out variable passed by reference to store the unsigned long. 
 * @return TRUE - Conversion OK || FALSE -> Conversion ERROR.
 */
bool iotLoggerFile::parseUL(const char *str, unsigned long &ul_out){
    char *endptr;
    ul_out = strtoul(str, &endptr, 10);
    // Serial.print("\n parseUL => ul_out = ");
    // Serial.print(ul_out);

    if (endptr == str || ul_out == ULONG_MAX)
        return false;

    return true;
}

/**
 * @brief Converts a char buffer containing a (float) number to float var type
 *        If function returns false, do not trust in float_out value!
 * 
 * @param str  Char buffer to convert. Must be null terminated.
 * @param float_out variable passed by reference to store the float. 
 * @return TRUE - Conversion OK || FALSE -> Conversion ERROR.
 */
bool iotLoggerFile::parseFloat(const char *str, float &float_out){
    char *endptr;
    float_out = strtof(str, &endptr);
    // Serial.print("\n parseFloat => float_out = ");
    // Serial.print(float_out);

    if (float_out == LONG_MAX || float_out == LONG_MIN || endptr == str)
        return false;

    return true;
}

/**
 * @brief Prints statistics about SPIFFS memory usage.
 * 
 */
void iotLoggerFile::getFileStats(){

    iotPRINT("iotLogger => SPIFFS File system total bytes:");
    iotPRINTv(SPIFFS.totalBytes());
    iotPRINT("iotLogger => SPIFFS File system total bytes used:");
    iotPRINTv(SPIFFS.usedBytes());

}
/**
 * @brief Checks if file exists.
 * 
 * @return TRUE -> File exists || FALSE -> File doesn't exists.
 */
bool iotLoggerFile::fileExists(){   
    return SPIFFS.exists(FILE_NAME);
}

/**
 * @brief Deletes file
 * 
 * @return TRUE -> Deletion OK || FALSE -> Deleteon FAIL (file exists?) 
 */
bool iotLoggerFile::fileDelete(){
    return SPIFFS.remove(FILE_NAME);
}