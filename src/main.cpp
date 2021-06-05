#include <Arduino.h>

#include "../lib/iotLogger/iotLogger.h"

iotLoggerFile Logger( 
                10,                 /* Buffer size (number of elements) */
                "/logs/test.log",   /* File to store buffer. Can include path (ie: logs/test.txt) */
                USER_TIMESTAMP);    /* */

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  
  //delay(1000);

  Logger.init();
}

void loop() {

  float data = 0;
  unsigned long time_stamp = 0;
  uint8_t i = 0;


  // Serial.print("\n\n PARSING TEST");
  // if( Logger.parseFloat(data1, data) ){
  //   Serial.print("\nparseFloat() => TRUE");
  // }else{
  //   Serial.print("\nparseFloat() => FALSE");
  // }
  // Serial.print("\nChar array: ");
  // Serial.print(data1);
  // Serial.print("   || Converted float: ");
  // Serial.print(data);

  // if( Logger.parseUL(data2, time_stamp) ){
  //   Serial.print("\nparseUL() => TRUE");
  // }else{
  //   Serial.print("\nparseUL() => FALSE");
  // }
  // Serial.print("\nChar array: ");
  // Serial.print(data2);
  // Serial.print("   || Converted UL: ");
  // Serial.print(time_stamp);

  //FILE TEST
//   Serial.print("\n\n FILE TEST");
//   Serial.print("\n Filling buffer (7) \n");
//   for(i = 0; i<7 ; i++){
//     Logger.add(i+1,millis());
//     delay(10);
//   }
//   Logger.dumpBuffer();

//   if(Logger.memoryToFile()){
//     Serial.print("\n memoryToFile() => TRUE");
//   }else{
//     Serial.print("\n memoryToFile() => FALSE");
//   }

//   Logger.resetBuffer();
//   Logger.dumpBuffer();

//   Serial.print("\n Retrieving data from FILE");
//   if( Logger.fileToMemory()){
//     Serial.print("\n fileToMemory() => TRUE");
//   }else{
//     Serial.print("\n fileToMemory() => FALSE");
//   }
//   Logger.dumpBuffer();

//   Logger.resetBuffer();
//   Logger.fileDelete();

//   Serial.print("\n Retrieving data from FILE (file deleted)");
//   if( Logger.fileToMemory()){
//     Serial.print("\n fileToMemory() => TRUE");
//   }else{
//     Serial.print("\n fileToMemory() => FALSE");
//   }

//   Serial.print("\n Filling buffer (15) \n");
//   for(i = 0; i<15 ; i++){

//     Logger.add(i+1,millis());
//     delay(10);
//   }
//   Logger.dumpBuffer();

//   if(Logger.memoryToFile()){
//     Serial.print("\n memoryToFile() => TRUE");
//   }else{
//     Serial.print("\n memoryToFile() => FALSE");
//   }

//   Serial.print("\n Consuming buffer (7)\n");
//   for(i=0 ; i<7; i++){
//     if(!Logger.getOldest(data, time_stamp)){
//       Serial.print("\n # NO DATA ");
//       continue;
//     }
//     Serial.print("\n # Time-stamp: ");
//     Serial.print(time_stamp);
//     Serial.print("   #Data: ");
//     Serial.print(data);
//   }
//   Logger.dumpBuffer();

//   Serial.print("\n Retrieving data from FILE");
//   if( Logger.fileToMemory()){
//     Serial.print("\n fileToMemory() => TRUE");
//   }else{
//     Serial.print("\n fileToMemory() => FALSE");
//   }
//   Logger.dumpBuffer();

//   Logger.resetBuffer();

//   Serial.print("\n Filling buffer (10) \n");
//   for(i = 0; i<10 ; i++){

//     Logger.add(i+1,millis());
//     delay(10);
//   }
//   Logger.dumpBuffer();

//   Serial.print("\n popWhereData \n");
//   Logger.popWhereData(time_stamp,2,false);
//   Serial.print("\nData Popped (2)- timestamp = ");
//   Serial.print(time_stamp);
//   Logger.popWhereData(time_stamp,5,false);
//   Serial.print("\nData Popped (5)- timestamp = ");
//   Serial.print(time_stamp);
//   Logger.popWhereData(time_stamp,6,false);
//   Serial.print("\nData Popped (6)- timestamp = ");
//   Serial.print(time_stamp);
//   Logger.dumpBuffer();

//  if(Logger.memoryToFile()){
//     Serial.print("\n memoryToFile() => TRUE");
//   }else{
//     Serial.print("\n memoryToFile() => FALSE");
//   }

//   Logger.resetBuffer();

//   Serial.print("\n Retrieving data from FILE");
//   if( Logger.fileToMemory()){
//     Serial.print("\n fileToMemory() => TRUE");
//   }else{
//     Serial.print("\n fileToMemory() => FALSE");
//   }

//   Logger.dumpBuffer();

//   for(;;){  //STOP
//     asm("nop");
//   }


//DEFRAGMENT TEST
  // Serial.print("\n\n DEFRAGMENT TEST with NOT CIRCULAR BUFFER");
  // Serial.print("\n Filling buffer (7) \n");
  // for(i = 0; i<7 ; i++){

  //   Logger.add(i+1,millis());
  //   delay(10);
  // }
  // Logger.dumpBuffer();

  // if( Logger.defragment()){
  //   Serial.print("\n defragment() => TRUE");
  // }else{
  //   Serial.print("\n defragment() => FALSE");
  // }
  

  // Serial.print("\n popWhereData TEST \n");
  // Logger.popWhereData(time_stamp,2,false);
  // Serial.print("\nData Popped (2)- timestamp = ");
  // Serial.print(time_stamp);
  // Logger.popWhereData(time_stamp,5,false);
  // Serial.print("\nData Popped (5)- timestamp = ");
  // Serial.print(time_stamp);
  // Logger.popWhereData(time_stamp,6,false);
  // Serial.print("\nData Popped (6)- timestamp = ");
  // Serial.print(time_stamp);
  // Logger.dumpBuffer();

  // if( Logger.defragment()){
  //   Serial.print("\n defragment() => TRUE");
  // }else{
  //   Serial.print("\n defragment() => FALSE");
  // }
  
  // Logger.dumpBuffer();

  // Serial.print("\n\n DEFRAGMENT TEST with CIRCULAR BUFFER");

  // Serial.print("\n Filling buffer (15) \n");
  // for(i = 0; i <15; i++){
  //   Logger.add(i,millis());
  //   delay(15);
  // }
  // Logger.dumpBuffer();

  // if( Logger.defragment()){
  //   Serial.print("\n defragment() => TRUE");
  // }else{
  //   Serial.print("\n defragment() => FALSE");
  // }
  
  // Logger.dumpBuffer();

  // Logger.add(999,millis());

  // Logger.dumpBuffer();


  // Serial.print("\n Consuming buffer (7)\n");
  // for(i=0 ; i<7; i++){
  //   if(!Logger.getOldest(data, time_stamp)){
  //     Serial.print("\n # NO DATA ");
  //     continue;
  //   }
  //   Serial.print("\n # Time-stamp: ");
  //   Serial.print(time_stamp);
  //   Serial.print("   #Data: ");
  //   Serial.print(data);
  // }
  // Logger.dumpBuffer();

  // if( Logger.defragment()){
  //   Serial.print("\n defragment() => TRUE");
  // }else{
  //   Serial.print("\n defragment() => FALSE");
  // }
  // Logger.dumpBuffer();


  // Serial.print("\n Filling buffer (5) \n");
  // for(i = 0; i <5; i++){
  //   Logger.add(i,millis());
  //   delay(15);
  // }
  // Logger.dumpBuffer();

  // Serial.print("\n Consuming buffer (3)\n");
  // for(i=0 ; i<3; i++){
  //   if(!Logger.getOldest(data, time_stamp)){
  //     Serial.print("\n # NO DATA ");
  //     continue;
  //   }
  //   Serial.print("\n # Time-stamp: ");
  //   Serial.print(time_stamp);
  //   Serial.print("   #Data: ");
  //   Serial.print(data);
  // }
  // Logger.dumpBuffer();

  // if(Logger.popWhereTimestamp(data,Logger.getTimestampByIndex(7),false)){
  //   Serial.print("\npopWhereTimestamp() => TRUE");
  //   Serial.print("\nData Popped (i7)- data = ");
  //   Serial.print(data); 
  // }else{
  //   Serial.print("\npopWhereTimestamp() => FALSE");
  // }

  // if(Logger.popWhereTimestamp(data,Logger.getTimestampByIndex(8),false)){
  //   Serial.print("\npopWhereTimestamp() => TRUE");
  //   Serial.print("\nData Popped (i8)- data = ");
  //   Serial.print(data); 
  // }else{
  //   Serial.print("\npopWhereTimestamp() => FALSE");
  // }
  // Logger.dumpBuffer();

  // if( Logger.defragment()){
  //   Serial.print("\n defragment() => TRUE");
  // }else{
  //   Serial.print("\n defragment() => FALSE");
  // }
  
  // Logger.dumpBuffer();


  // for(;;){
  //   asm("nop");
  // }

  //##################################################################################

  // Serial.print("\n Consuming buffer (10)\n");
  // for(i=0 ; i<10; i++){
  //   if(!Logger.getOldest(data, time_stamp)){
  //     Serial.print("\n # NO DATA ");
  //     break;
  //   }
  //   Serial.print("\n # Time-stamp: ");
  //   Serial.print(time_stamp);
  //   Serial.print("   #Data: ");
  //   Serial.print(data);
  // }
  // Logger.dumpBuffer();
  // Logger.getOldest(data);

  // Serial.print("\n popWhereData TEST \n");
  // Logger.popWhereData(time_stamp,13,false);
  // Serial.print("\nData Popped (13)- timestamp = ");
  // Serial.print(time_stamp);
  // Logger.popWhereData(time_stamp,1,false);
  //   Serial.print("\nData Popped (1)- timestamp = ");
  // Serial.print(time_stamp);
  // Logger.popWhereData(time_stamp,11,false);
  //   Serial.print("\nData Popped (11)- timestamp = ");
  // Serial.print(time_stamp);
  // Logger.dumpBuffer();

  // Serial.print("\n Consuming buffer (2)\n");
  // for(i=0 ; i<2; i++){
  //   if(!Logger.getOldest(data, time_stamp)){
  //     Serial.print("\n # NO DATA ");
  //     break;
  //   }
  //   Serial.print("\n # Time-stamp: ");
  //   Serial.print(time_stamp);
  //   Serial.print("   #Data: ");
  //   Serial.print(data);
  // }
  // Logger.dumpBuffer();

  // Serial.print("\n popWhereTimestamp TEST \n");
  
  // Logger.popWhereTimestamp(data,Logger.getTimestampByIndex(2),false);
  // Serial.print("\nData Popped (i2)- data = ");
  // Serial.print(data);

  // Logger.popWhereTimestamp(data,Logger.getTimestampByIndex(5),true);
  // Serial.print("\nData Popped (i5)- data = ");
  // Serial.print(data);

  // Logger.dumpBuffer();

  // if(Logger.popWhereTimestamp(data,Logger.getTimestampByIndex(5),false)){
  //   Serial.print("\npopWhereTimestamp() => TRUE");
  //   Serial.print("\nData Popped (i5)- data = ");
  //   Serial.print(data); 
  // }else{
  //   Serial.print("\npopWhereTimestamp() => FALSE");
  // }

  // if(Logger.popWhereTimestamp(data,Logger.getTimestampByIndex(8),false)){
  //   Serial.print("\npopWhereTimestamp() => TRUE");
  //   Serial.print("\nData Popped (i8)- data = ");
  //   Serial.print(data); 
  // }else{
  //   Serial.print("\npopWhereTimestamp() => FALSE");
  // }
  // if(Logger.popWhereData(time_stamp,5,false)){
  //   Serial.print("\npopWhereData() => TRUE");
  //   Serial.print("\nData Popped (5)- timestamp = ");
  //   Serial.print(time_stamp); 
  // }else{
  //   Serial.print("\npopWhereData() => FALSE");
  // }

  // Serial.print("\n Filling buffer (7) \n");
  // for(i = 0; i<7 ; i++){

  //   Logger.add(i,millis());
  //   delay(10);
  // }
  // Logger.dumpBuffer();



  // Serial.print("\n Filling buffer (7) \n");
  // for(i = 0; i<7 ; i++){

  //   Logger.add(i,millis());
  //   delay(10);
  // }
  // Logger.dumpBuffer();

  // Serial.print("\n Consuming buffer (10)\n");
  // for(i = 0; i<= 9; i++){
  //   if(!Logger.getOldest(data, time_stamp)){
  //     Serial.print("\n # NO DATA ");
  //     continue;
  //   }
  //   Serial.print("\n # Time-stamp: ");
  //   Serial.print(time_stamp);
  //   Serial.print("   #Data: ");
  //   Serial.print(data);
  // }
  // Logger.dumpBuffer();


  // Serial.print("\n Filling buffer (20) \n");
  // for(i = 0; i<20 ; i++){

  //   Logger.add(i,millis());
  //   delay(10);
  // }
  // Logger.dumpBuffer();

  // Serial.print("\n Consuming buffer (5)\n");
  // for(i = 0; i<= 4; i++){
  //   if(!Logger.getOldest(data, time_stamp)){
  //     Serial.print("\n # NO DATA ");
  //     continue;
  //   }
  //   Serial.print("\n # Time-stamp: ");
  //   Serial.print(time_stamp);
  //   Serial.print("   #Data: ");
  //   Serial.print(data);
  // }
  // Logger.dumpBuffer();

  // Serial.print("\n Consuming buffer (5)\n");
  // for(i = 0; i<= 4; i++){
  //   if(!Logger.getOldest(data, time_stamp)){
  //     Serial.print("\n # NO DATA ");
  //     continue;
  //   }
  //   Serial.print("\n # Time-stamp: ");
  //   Serial.print(time_stamp);
  //   Serial.print("   #Data: ");
  //   Serial.print(data);
  // }
  // Logger.dumpBuffer();

  // if(!Logger.getOldest(data, time_stamp)){
  //   Serial.print("\n # NO DATA ");
  // }
  // Serial.print("\n # Time-stamp: ");
  // Serial.print(time_stamp);
  // Serial.print("   #Data: ");
  // Serial.print(data);
  // Logger.dumpBuffer();

  for(;;){
    asm("nop");
  }

}