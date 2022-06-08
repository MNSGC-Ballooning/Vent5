#include <RelayXBee.h> 
#include <SoftwareSerial.h>

//SoftwareSerial port = SoftwareSerial(3,4);//Connect on pins 2-3
#define xbeeSerial Serial4
 
RelayXBee xbee = RelayXBee(&xbeeSerial, "UMN1");

String comm = " ";

void radioSetup(){
  Serial.println("Setting up the the Xbee Radio");
  char xbeeChannel = 'A';
  xbeeSerial.begin(9600);   // Xbee Baud rate is 9600
  xbee.init(xbeeChannel);   // Need to make sure xbees on both ends have the same identifier. This sets "AAAA"
  xbee.enterATmode();       // Allows the setting of xbee parameters                   
  xbee.atCommand("ATDL0");
  xbee.atCommand("ATMY1");
  xbee.exitATmode();
  Serial.println("Finished Setting up the Xbee Radio");
  xbeeSerial.write("Hi, I'm the vent radio and I am paired and working");
}

void updateRadio(){
  comm = xbeeSerial.readString();
  Serial.println(comm);
  xbeeSerial.flush();
  if(comm == "OPENV"){
    xbeeSerial.write("Message received");
    Serial.println("Message received");
    openVent();
  }
    if(comm == "CLOSEV"){
    xbeeSerial.write("Message received");
    Serial.println("Message received");
    closeVent();
  }
  
}
