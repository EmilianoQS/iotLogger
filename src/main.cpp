#include <Arduino.h>

#include "../lib/iotLogger/iotLogger.h"

iotLogger Logger( 10,               /* Buffer size (number of elements) */
                  "test.log",       /* File to store buffer. Can include path (ie: logs/test.txt) */
                  NO_TIMESTAMP);    /* */

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  //
  
  delay(1000);

  Logger.init();
}

void loop() {
  // put your main code here, to run repeatedly:

  float data = 0;
  // float* data_ptr = &data;
  unsigned long time_stamp = 0;
  // unsigned long* time_stamp_ptr = &time_stamp;

  uint8_t i = 0;

  Serial.print("\n Filling buffer \n");
  for(i = 0; i<7 ; i++){

    Logger.add(i,millis());
    delay(10);
  }
  Logger.dumpBuffer();

  Serial.print("\n Filling buffer again \n");
  for(i = 0; i <5; i++){
    Logger.add(i,millis());
    delay(15);
  }
  Logger.dumpBuffer();

  Serial.print("\n Consuming buffer (3)\n");
  for(i=0 ; i<3; i++){
    Logger.getOldest(data, time_stamp);
    Serial.print("\n # Time-stamp: ");
    Serial.print(time_stamp);
    Serial.print("   #Data: ");
    Serial.print(data);
  }
  Logger.dumpBuffer();

  Serial.print("\n Consuming buffer (15)\n");
  for(i = 0; i<= 15; i++){
    if(!Logger.getOldest(data, time_stamp)){
      Serial.print("\n # NO DATA ");
      continue;
    }
    Serial.print("\n # Time-stamp: ");
    Serial.print(time_stamp);
    Serial.print("   #Data: ");
    Serial.print(data);
  }
  Logger.dumpBuffer();

  for(;;){
    asm("nop");
  }

}