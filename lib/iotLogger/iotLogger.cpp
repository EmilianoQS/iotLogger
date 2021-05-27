#include "iotLogger.h"


iotLogger::iotLogger(uint16_t BUFFER_SIZE,const char* FILE_NAME, uint8_t TS_MODE){

    this->BUFFER_SIZE = BUFFER_SIZE;
    this->FILE_NAME = FILE_NAME;

}

bool iotLogger::init(){

    // D_BUFFER = (float*)malloc(BUFFER_SIZE * sizeof(float));
    D_BUFFER = (float*)calloc(BUFFER_SIZE,sizeof(float));           //Allocates and initialize DATA buffer

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
}

void iotLogger::add(float data, unsigned long timestamp){
    D_BUFFER[store_index] = data;
    TS_BUFFER[store_index] = timestamp;
    
    Serial.print("\nAdding #Data: ");
    Serial.print(data);
    Serial.print("    #Timestamp: ");
    Serial.print(timestamp);
    Serial.print("    #store_index: ");
    Serial.print(store_index);
    Serial.print("    #consume_index: ");
    Serial.print(consume_index);
    incDataIndex();
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
    // if(!dataAvailable()){
    //     Serial.print("Data not available");
    //     setErrno(BUFFER_EMPTY,WARNING);
    //     return false;
    // }
    // Serial.print("Al menos se ejecuta esta puta vergA?");
    // Serial.print(D_BUFFER[consume_index]);
    // Serial.print(TS_BUFFER[consume_index]);
    data_out = D_BUFFER[consume_index];
    timestamp_out = TS_BUFFER[consume_index];

    if(!peek){
        incConsumeIndex();
    }
    return true;
}


bool iotLogger::getItem(uint16_t index, float* data_out, unsigned long* timestamp_out){

    if(isValidData(index)){
        //Devolvemos los index
    }

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
    iotPRINT("\n # consume_index = ");
    iotPRINTv(consume_index);
    iotPRINT("\n # store_index = ");
    iotPRINTv(store_index);
    iotPRINT("\n # INDEX #            # TIME-STAMP #             # DATA # ");
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

        iotPRINT("\n   ");
        iotPRINTv(i);
        iotPRINT("                   ");
        iotPRINTv(TS_BUFFER[i]);
        iotPRINT("                   ");
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

    //FIX THIS SHIT. MUST TAKE IN COUNT CIRCULAR BUFFER! OR NOT.
    if(consume_index == store_index){
        return false;
    }
    return true;
}
//////////////////// PRIVATE ///////////////////////

void iotLogger::incDataIndex(){
    if( (store_index+1) >= BUFFER_SIZE ){
        //This means we're in the last buffer position.
        //If we increment in this position, it'll cause stackoverflow.
        store_index = 0;            //We must restart the index to 0 (circular buffer)       
        buffer_isCircular = true;   //Mark the buffer as circular.
    }else{

        //If the buffer IS NOT circular, then consumer_index < store_index
        if(!buffer_isCircular){
            store_index++;
            return;
        }
        //If the buffer IS circular, then consumer_index could be > or < store_index (depending on relative speed)
        //If the consume_index was on the same place that store_index (producer faster than consumer)
        //then, consumer must follow store_index increments.
        if( consume_index == store_index ){
            //But what if we had some "popped out" item in the new store_index place? Then we must
            //find the next valid item on the buffer...
            uint16_t temp_store_index = store_index+1;
            bool allAround = false;
            unsigned long prevMillis = millis();
            while((iotMillis() - prevMillis) < 5000){   //5 seconds timeout.
                if(temp_store_index >= BUFFER_SIZE){
                    if(allAround){
                        //Entering here shouldn't be possible, but just in case...
                        iotPRINT("\n !!Weird as S#!t error. !!");
                        consume_index = store_index;
                        break;
                    }
                    temp_store_index = 0;
                    allAround = true;
                }
                if(isValidData(temp_store_index)){
                    //Found valid buffer index to place consume_index
                    consume_index = temp_store_index; //First round temp_store_index = store_index+1
                    break;
                }
                temp_store_index++;
            }
        }
        store_index++;
    }
}

void iotLogger::checkConsumeIndex(){

}

/**
 * @brief Increments consume index. If it matches store index, that means the buffer
 * has been entirely consumed. So it resets indexes and other variables.
 * 
 */
void iotLogger::incConsumeIndex(){

    //Tenemos que saber si queda informacion que consumir antes de incrementar...                                                
    /* Situaciones a evaluar
        -Buffer circular: nos encontramos al store_index
        -Buffer circular: llegamos al final del buffer con consume_index
        
        -Buffer NO circular: el unico posible caso es encontrarnos al store_index. Todo a cero BUFFER_RESET.
    */
    if( (consume_index+1) >= BUFFER_SIZE ){
        //If consume index reaches buffer end 
        consume_index = 0; 
    }else{
        consume_index++;
    }

    // We evaluate the case in which both indexes matches.
    if(consume_index == store_index){
        //Buffer has been totally consumed (no elements left to consume)
        buffer_isCircular = false;
        store_index = 0;
        consume_index = 0;
    }
}


/**
 * @brief Checks if data stored in index "index" is valid or not.
 * 
 * @param index Index to check for valid data.
 * @return true  -> Data is valid.
 */
bool iotLogger::isValidData(uint16_t index){
    if(TS_BUFFER[index] != __DBL_MAX__){
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