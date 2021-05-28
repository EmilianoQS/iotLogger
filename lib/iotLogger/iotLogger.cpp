#include "iotLogger.h"


iotLogger::iotLogger(uint16_t BUFFER_SIZE,const char* FILE_NAME, ts_mode TS_MODE){

    this->BUFFER_SIZE = BUFFER_SIZE;
    this->FILE_NAME = FILE_NAME;
    this->TS_MODE = TS_MODE; 
}

bool iotLogger::init(){

    // D_BUFFER = (float*)malloc(BUFFER_SIZE * sizeof(float));
    D_BUFFER = (float*)calloc(BUFFER_SIZE,sizeof(float));           //Allocates and initialize DATA buffer

    if(TS_MODE != NO_TIMESTAMP)
        TS_BUFFER = (unsigned long*)calloc(BUFFER_SIZE,sizeof(float));  //Allocates and initialize TIMESTAMP buffer

	if (D_BUFFER == NULL)
	{
        setErrno(BUF_INIT_ERROR,CRITICAL);
        return false;
	}

    return true;
}

void iotLogger::end(){
    free(D_BUFFER);
    if(TS_MODE != NO_TIMESTAMP)
        free(TS_BUFFER);
}

void iotLogger::add(float data, unsigned long timestamp){
    D_BUFFER[store_index] = data;
    incDataIndex();

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

    //Checks if data is avaiable, if not, returns false.
    if(!dataAvailable()){
        setErrno(BUFFER_EMPTY,WARNING);
        return false;
    }
    // Serial.print(D_BUFFER[consume_index]);
    // Serial.print(TS_BUFFER[consume_index]);
    data_out = D_BUFFER[consume_index];
    timestamp_out = TS_BUFFER[consume_index];
    D_BUFFER[consume_index] = consumed_data;    //Stores consumed marker in consumed index.

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
    D_BUFFER[consume_index] = consumed_data;    //Stores consumed marker in consumed index.

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

    D_BUFFER[data_index] = consumed_data;
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

    D_BUFFER[timestamp_index] = consumed_data;
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
    iotPRINT("\n # consume_index = ");
    iotPRINTv(consume_index);
    iotPRINT("\n # store_index = ");
    iotPRINTv(store_index);
    iotPRINT("\n # buffer_isCircular = ");
    iotPRINTv(buffer_isCircular);
    iotPRINT("\n # consume_isCircular = ");
    iotPRINTv(consume_isCircular);
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
//////////////////// PRIVATE ///////////////////////

/**
 * @brief Increments the data index, checks for buffer max size (circular),
 *        and resets indexes as needed.
 * 
 */
void iotLogger::incDataIndex(){

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
        if( (store_index+1) >= BUFFER_SIZE ){
            //If both indexes where together, they must keep together.
            if(store_index == consume_index){
                consume_index = 0;
            }
            store_index = 0;
            return;
        }
        //If both indexes where together, they must keep together.
        if(store_index == consume_index){
            consume_index++;
        }
       store_index++;
       return;
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

   if(!buffer_isCircular){
        //NOT Circular buffer
        //Consume can't be circular!
        if( (consume_index+1) == store_index){
            resetBuffer();
            return;
        }
        if( (consume_index+1) < store_index ){
            consume_index++;
            return;
        }
    }else{
        // Serial.print("\n#incConsumeIndex => Circular buffer");
        //Circular buffer
        if(!consume_isCircular){
            //NOT consume circular
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
                uint16_t temporalConsumeIndex = searchValidIndex();
                if(temporalConsumeIndex == (BUFFER_SIZE+1) ){
                    //No valid data found in the buffer.
                        // Serial.print("\nsearchValidIndex(): ");
                        // Serial.print(temporalConsumeIndex);
                    resetBuffer();
                    return;
                }
                    // Serial.print("\n#incConsumeIndex => After searchValidIndex error check");
                consume_index = temporalConsumeIndex;
                    // if(consume_index == store_index){
                    //    resetBuffer();
                    //    return;
                    // }
                return;
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
            return i;
        }
    }

    //Searching from beggining of the array to consume_index
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
void iotLogger::resetBuffer(void){
    Serial.print("\n resetBuffer() => CALLED\n");
    store_index = 0;
    consume_index = 0;
    consume_isCircular = false;
    buffer_isCircular = false;
    buffer_isPopped = false;

    for(uint16_t i=0; i<BUFFER_SIZE; i++){
        D_BUFFER[i] = 0;
        if(TS_MODE != NO_TIMESTAMP)
            TS_BUFFER[i] = 0;
    }
}

/**
 * @brief Checks if data stored in index "index" is valid or not.
 * 
 * @param index Index to check for valid data.
 * @return true  -> Data is valid.
 */
bool iotLogger::isValidData(uint16_t index){
    //Cambiar al D_BUFFER para poder no utilizar time-stamp
    if(D_BUFFER[index] != consumed_data){
        return true;
    }
    return false;
}



void iotLogger::setErrno(iotLogger_errno errno, uint8_t error_level){
    errno = errno;
}

void iotLogger::empty(){
    asm("nop");
}