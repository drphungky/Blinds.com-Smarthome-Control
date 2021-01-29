// Codes are as follows on original BY-305 remote:
// Learn a remote channel: hold down button on blinds for 3 seconds until Jog Jog, then CONFIRM on channel to learn
// Make a blind forget a channel: hit confirm roughly 10 times in a row, short clicks. All blinds on channel will forget.
// Set limits: hold limit button for 6 seconds until blind double jogs twice. Then tap up to make blind go up, stop to stop when you are close to the upper limit, then tap up or down to move to exact upper limit. Confirm to set upper limit. Repeat for lower limit.
// Set favorite: poistion blind in "favorite" position (probably all the way closed). Hold the limit button and immediately push the up button (LU in code below). The blind will jog to show recognition. Now the stop button doubles as a favorite button if the blinds have been unmoving for three seconds, or are at the upper or lower limit.

#include <cstring>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "login.h" //provides ssid and password

//Setup a server
WiFiClient espClient;
PubSubClient MQTTClient(espClient);

//Declare Constants
const int GPIOPIN = 13; // GPIO13
const int BITLENGTH = 350; //in microseconds
const int PREAMBLE_1 = 11; //Length of 1st series, of 1s in preamble
const int PREAMBLE_2 = 7; //Length of second series, of 0s in preamble
const int PREAMBLE_3 = 5; //Length of third series, of 1s in preamble
const float HOLD_PAUSE = 390000; //Number of cycles to pause between commands to signal a hold rather than a tap command.
const float FIRST_HOLD_PAUSE = 340000; //First number of cycles to pause between commands to signal a hold rather than a tap command is slightly shorter
const char KITCHEN_BLINDS_CONFIG_TOPIC[] = "homeassistant/cover/kitchenBlinds/config";
const char KITCHEN_BLINDS_SET_TOPIC[] = "homeassistant/cover/kitchenBlinds/+/set";

//Setup variables
char blindsChannel = '2';
char command = 'U';
char remoteID[] = "1011011011011010";
char stopCheck = '0';
char tapOrHold='H';

//Setup functions
void sendPreamble(void);
void byte0(void);
void byte1(void);
void byteFinal(void);

void sendChannel(void);
void sendRemoteID(void);
void channel2_up(void);
void sendFirstMoveOrConfirm(void);
void setStopCheck(void);
void sendStopCheck(void);
void sendChannelCheck(void);
void sendCommandCheck(void);
void sendMessageEnd(void);
void sendFullCommand(void);
void sendBaseCommand(void);
void adminMessageParser(void);

void publishBlindsConfig(const char* blindsName,const char* friendlyName);


void setup() {
  Serial.begin(115200);
  Serial.println();
  delay(10);

  pinMode(GPIOPIN, OUTPUT);


  // Connect to WiFi network
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  //MQTT Server connect

  MQTTClient.setBufferSize(1000);
  MQTTClient.setServer(mqttServer, mqttPort);
  MQTTClient.setCallback(callback);

  while (!MQTTClient.connected()) {
    Serial.println("Connecting to MQTT...");

    if (MQTTClient.connect("ESP8266Client", mqttUser, mqttPassword )) {

      Serial.println("connected");

    } else {

      Serial.print("failed with state ");
      Serial.print(MQTTClient.state());
      delay(2000);

    }
  }

  //MQTT Subscriptions
  MQTTClient.publish("esp/test", "Hello from ESP8266");
  MQTTClient.subscribe("esp/test");


  MQTTClient.subscribe("homeassistant/cover/#");

 

  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

  //Publish the MQTT startup config

  //Remote1
    //Channel 1 
      //Unassigned
    //Channel 2 
      //Unassigned
    //Channel 3 
      publishBlindsConfig("homeassistant/cover/guestRoomBlinds","Guest Room Blinds");
    //Channel 4 
      publishBlindsConfig("homeassistant/cover/kidsRoomBlinds","Kids Room Blinds");
    //Channel 5 
      publishBlindsConfig("homeassistant/cover/masterBedroomBlinds","Master Bedroom Blinds");
  //Remote2
    //Channel 1 
      publishBlindsConfig("homeassistant/cover/livingRoomBlinds","Living Room Blinds");
    //Channel 2 
      publishBlindsConfig("homeassistant/cover/eastKitchenBlinds","East Kitchen Blinds");
    //Channel 3 
      publishBlindsConfig("homeassistant/cover/kitchenSinkBlinds","Kitchen Sink Blinds");
    //Channel 4 
      publishBlindsConfig("homeassistant/cover/diningRoomBlinds","Dining Room Blinds");
    //Channel 5 
      publishBlindsConfig("homeassistant/cover/guestRoomBlinds","Guest Room Blinds");
}


void callback(char* topic, byte* payload, unsigned int length) {

  Serial.print("Message arrived in topic: ");
  Serial.println(topic);

  Serial.print("Message:");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  Serial.println();
  Serial.println("-----------------------");

  if (strcmp(topic,"homeassistant/cover/admin")==0){
    adminMessageParser(payload);
    sendFullCommand();
  }

//Remote1

  else if (strcmp(topic,"homeassistant/cover/guestRoomBlinds/position/set")==0){
    std::strcpy(remoteID,"1011011011011010");
    blindsChannel='3';
    command=payload[0];
    if (command=='S'){
      tapOrHold='T';
      sendFullCommand();        
    }
    if (command=='U' || command=='D'){
      tapOrHold='H';
      sendFullCommand();        
    }
  }
  
    else if (strcmp(topic,"homeassistant/cover/kidsRoomBlinds/position/set")==0){
    std::strcpy(remoteID,"1011011011011010");
    blindsChannel='4';
    command=payload[0];
    if (command=='S'){
      tapOrHold='T';
      sendFullCommand();        
    }
    if (command=='U' || command=='D'){
      tapOrHold='H';
      sendFullCommand();        
    }
  }
  
  else if (strcmp(topic,"homeassistant/cover/masterBedroomBlinds/position/set")==0){
    std::strcpy(remoteID,"1011011011011010");
    blindsChannel='5';
    command=payload[0];
    if (command=='S'){
      tapOrHold='T';
      sendFullCommand();        
    }
    if (command=='U' || command=='D'){
      tapOrHold='H';
      sendFullCommand();        
    }
  }

//Remote2
//This remote only issues hold (i.e. open or close all the way) commands on a tap somehow, so all commands are set to taps below to avoid the hold checksum exception processing.
 
  else if (strcmp(topic,"homeassistant/cover/livingRoomBlinds/position/set")==0){
    std::strcpy(remoteID,"1010110110001010");
    blindsChannel='1';
    command=payload[0];
    if (command=='S'){
      tapOrHold='T';
      sendFullCommand();        
    }
    if (command=='U' || command=='D'){
      tapOrHold='T';
      sendFullCommand();        
    }
  }
  
  else if (strcmp(topic,"homeassistant/cover/eastKitchenBlinds/position/set")==0){
    std::strcpy(remoteID,"1010110110001010");
    blindsChannel='2';
    command=payload[0];
    if (command=='S'){
      tapOrHold='T';
      sendFullCommand();        
    }
    if (command=='U' || command=='D'){
      tapOrHold='T';
      sendFullCommand();        
    }
  }

  else if (strcmp(topic,"homeassistant/cover/kitchenSinkBlinds/position/set")==0){
    std::strcpy(remoteID,"1010110110001010");
    blindsChannel='3';
    command=payload[0];
    if (command=='S'){
      tapOrHold='T';
      sendFullCommand();        
    }
    if (command=='U' || command=='D'){
      tapOrHold='T';
      sendFullCommand();        
    }
  }
  else if (strcmp(topic,"homeassistant/cover/diningRoomBlinds/position/set")==0){
    std::strcpy(remoteID,"1010110110001010");
    blindsChannel='4';
    command=payload[0];
    if (command=='S'){
      tapOrHold='T';
      sendFullCommand();        
    }
    if (command=='U' || command=='D'){
      tapOrHold='T';
      sendFullCommand();        
    }
  }

  else if (strcmp(topic,"homeassistant/cover/guestRoomBlinds/position/set")==0){
    std::strcpy(remoteID,"1010110110001010");
    blindsChannel='5';
    command=payload[0];
    if (command=='S'){
      tapOrHold='T';
      sendFullCommand();        
    }
    if (command=='U' || command=='D'){
      tapOrHold='T';
      sendFullCommand();        
    }
  }
}


void loop() {
  MQTTClient.loop();
}


/*Remote Functions*/

void sendFullCommand() {
  /*Necessary to run setStopCheck first so it can be used in sendChannelCheck. I kept all the sends together in case clock speed
    became an issue. Not sure how finicky Arduino and RF transmitters are.*/
  setStopCheck();

  //Send actual commands:  
  sendBaseCommand();

  //Repeat command after 1000 cycles off if a hold (as opposed to a tap) command.
  //Do this a couple times to make sure the blinds grab it.
  if (tapOrHold=='H') {
    //This "holding down the button" takes long enough that ESP will think it's in a loop, so we
    //Disable the Software Watchdog
    ESP.wdtDisable();
    delayMicroseconds(FIRST_HOLD_PAUSE);
    for (int i = 0; i < 3; i++) {
       sendBaseCommand();
       delayMicroseconds(HOLD_PAUSE);
    }
    //Reenable Software watchdog
    ESP.wdtEnable(1000);
  }
  
  Serial.print("RF Command sent");
}


void sendBaseCommand() {
  for (int i = 0; i < 8; i++) {
    sendPreamble();

//    Serial.println();
//    Serial.print("RemoteID: ");
    sendRemoteID();
    
//    Serial.println();
//    Serial.print("Channel: ");
    sendChannel();

//    Serial.println();
//    Serial.print("Command: ");
    sendCommand();
    
//    Serial.println();
//    Serial.print("StopCheck: ");
    sendStopCheck();
    
//    Serial.println();
//    Serial.print("FirstMoveOrConfirm: ");
    sendFirstMoveOrConfirm();
    
//    Serial.println();
//    Serial.print("ChannelCheck: ");
    sendChannelCheck();
    
//    Serial.println();
//    Serial.print("CommandCheck: ");
    sendCommandCheck();

//    Serial.println();
//    Serial.print("MessageEnd: ");
    sendMessageEnd();
  }
}




//Send the RemoteID portion of a button push
void sendRemoteID() {
  for (int i = 0; i < strlen(remoteID); i++) {
    char c = remoteID[i];
    if (c == '0') {
      byte0();
    }
    else if (c == '1') {
      byte1();
    }
  }
}



//Send the channel portion of a button push
void sendChannel() {
  switch (blindsChannel) {
    case '1': //1 = 1000
      byte1();
      byte0();
      byte0();
      byte0();
      break;

    case '2': //2 = 0100
      byte0();
      byte1();
      byte0();
      byte0();
      break;

    case '3': //3 = 1100
      byte1();
      byte1();
      byte0();
      byte0();
      break;

    case '4': //4 = 0010
      byte0();
      byte0();
      byte1();
      byte0();
      break;

    case '5': //5 = 1010
      byte1();
      byte0();
      byte1();
      byte0();
      break;

    case 'A': //All = 1111
      byte1();
      byte1();
      byte1();
      byte1();
      break;

  }

}


//Send the command portion of a button push
void sendCommand() {
  switch (command) {
    case 'U': //Up = 0011
      byte0();
      byte0();
      byte1();
      byte1();
      break;

    case 'D': //Down = 1000
      byte1();
      byte0();
      byte0();
      byte0();
      break;

    case 'S': //Stop = 1010
      byte1();
      byte0();
      byte1();
      byte0();
      break;

    case 'C': //Confirm = 0010
      byte0();
      byte0();
      byte1();
      byte0();
      break;

    case 'L': //Limit = 0100
      byte0();
      byte1();
      byte0();
      byte0();
      break;

    case 'F': //Set Favorite (i.e. press and hold limit then immediately press up) = 1001
      byte1();
      byte0();
      byte0();
      byte1();
      break;

  }

}


//Send the stopcheck portion of a button push (some kind of checkbytes)
void setStopCheck() {
  if ((command == 'D') || (command == 'U')) {
    stopCheck = '0';
  }
  else {
    stopCheck = '1';
  }
  if ((strcmp(remoteID,"1010110110001010")==0)) {
    stopCheck = '1';
  }  
    Serial.print("stopCheck is: ");
    Serial.println(stopCheck);
}


//Send the stopcheck portion of a button push (some kind of checkbytes)
void sendStopCheck() {
  if (stopCheck == '0') {
    byte0();
  }
  else {
    byte1();
  }
  byte0();
  byte0();
  byte0();
}




//Send the firstmoveorconfirm portion of a button push (some kind of checkbytes)
void sendFirstMoveOrConfirm() {
  if (
      (strcmp(remoteID,"1011011011011010")==0) &&
      (
        ( (command == 'D') && (tapOrHold == 'T') )
        || ( (command == 'U') && (tapOrHold == 'T') )
        || (command == 'C')
        )
      )
  {
    byte1();
  }
  //always 0 for the other remote or holds
  else {
    byte0();
  }
  byte0();
  byte0();
  byte0();
}

/*Send first 4 of 8 digit checksum that I can't really figure out. Best I can do is this hardcoded list.*/

void sendChannelCheck() {
  if ((strcmp(remoteID,"1011011011011010")==0)){
    switch (stopCheck) {
      case '1':
        switch (blindsChannel) {
          case '1': // blindsChannel 1 = 1110 on stopcheck
            byte1();
            byte1();
            byte1();
            byte0();
            break;
          case '2': // blindsChannel 2 = 0110 on stopcheck
            byte0();
            byte1();
            byte1();
            byte0();
            Serial.println("correct stop check");
            break;
          case '3': // blindsChannel 3 = 1010 on stopcheck
            byte1();
            byte0();
            byte1();
            byte0();
            break;
          case '4': // blindsChannel 4 = 0010 on stopcheck
            byte0();
            byte0();
            byte1();
            byte0();
            break;
          case '5': // blindsChannel 5 = 1100 on stopcheck
            byte1();
            byte1();
            byte0();
            byte0();
            break;
          case 'A': // blindsChannel All = 1001 on stopcheck
            byte1();
            byte0();
            byte0();
            byte1();
            break;
  
        }
        break;
      case '0':
        switch (blindsChannel) {
          case '1': // blindsChannel 1 = 0001 on nostopcheck
            byte0();
            byte0();
            byte0();
            byte1();
            break;
          case '2': // blindsChannel 2 = 1110 on nostopcheck
            byte1();
            byte1();
            byte1();
            byte0();
            Serial.println("incorrect stop check");
            break;
          case '3': // blindsChannel 3 = 0110 on nostopcheck
            byte0();
            byte1();
            byte1();
            byte0();
            break;
          case '4': // blindsChannel 4 = 1010 on nostopcheck
            byte1();
            byte0();
            byte1();
            byte0();
            break;
          case '5': // blindsChannel 5 = 0010 on nostopcheck
            byte0();
            byte0();
            byte1();
            byte0();
            break;
          case 'A': // blindsChannel All = 0101 on nostopcheck
            byte0();
            byte1();
            byte0();
            byte1();
            break;
        }
        break;
    }
  }
  else if ((strcmp(remoteID,"1010110110001010")==0)){
    switch (blindsChannel) {
      case '1': // blindsChannel 1 = 1001
        byte1();
        byte0();
        byte0();
        byte1();
        break;
      case '2': // blindsChannel 2 = 0001
        byte0();
        byte0();
        byte0();
        byte1();
        break;
      case '3': // blindsChannel 3 = 1110
        byte1();
        byte1();
        byte1();
        byte0();
        break;
      case '4': // blindsChannel 4 = 0110
        byte0();
        byte1();
        byte1();
        byte0();
        break;
      case '5': // blindsChannel 5 = 1010
        byte1();
        byte0();
        byte1();
        byte0();
        break;
      case 'A': // blindsChannel All = 1101
        byte1();
        byte1();
        byte0();
        byte1();
        break;
    }
  }
}

/*Send second 4 of 8 digit checksum that I can't really figure out. Best I can do is this hardcoded list.*/

void sendCommandCheck() {
  if ((strcmp(remoteID,"1011011011011010")==0)){
    switch (blindsChannel) {
      case 'A': //All channels
        switch (tapOrHold){
          case 'T': //All tap
            switch (command) {
              case 'U': //All Tap Up = 1010
                byte1();
                byte0();
                byte1();
                byte0();
                break;
      
              case 'D': //All Tap Down = 0000
                byte0();
                byte0();
                byte0();
                byte0();
                break;
              }
              break;
          case 'H': //All hold
            switch (command) {
              case 'U': //All Hold Up = 0110
                byte0();
                byte1();
                byte1();
                byte0();
                break;
      
              case 'D': //All Hold Down = 1000
                byte1();
                byte0();
                byte0();
                byte0();
                break;
              }
            break;
        }       
        switch(command) {
          case 'S': //Stop = 1011
            byte1();
            byte0();
            byte1();
            byte1();
            break;
  
          case 'C': //Confirm = 1011
            byte1();
            byte0();
            byte1();
            byte1();
            break;
  
          case 'L': //Limit = 0000
            byte0();
            byte0();
            byte0();
            byte0();
            break;
  
          case 'F': //Set Favorite (i.e. press and hold limit then immediately press up) = 1001
            byte1();
            byte0();
            byte0();
            byte1();
            break;
        }
        break;
  
      default: //Any single channel
        switch (tapOrHold){
          case 'T': //Single channel tap
            switch (command) {
              case 'U': //single channel tap up = 0110
                byte0();
                byte1();
                byte1();
                byte0();
                break;
      
              case 'D': //Single Channel tap Down = 1000
                byte1();
                byte0();
                byte0();
                byte0();
                break;
              }
              break;
          case 'H': //single channel hold
            switch (command) {
              case 'U': //single channel hold up = 1110
                byte1();
                byte1();
                byte1();
                byte0();
                break;
      
              case 'D': //single channel hold down = 0100
                byte0();
                byte1();
                byte0();
                byte0();
                break;
              }
              break;
            }       
        switch(command) {
          case 'S': //Stop = 0111
            byte0();
            byte1();
            byte1();
            byte1();
            break;
  
          case 'C': //Confirm = 0111
            byte0();
            byte1();
            byte1();
            byte1();
            break;
  
          case 'L': //Limit = 1000
            byte1();
            byte0();
            byte0();
            byte0();
            break;
  
          case 'F': //Set Favorite (i.e. press and hold limit then immediately press up) = 0101
            byte0();
            byte1();
            byte0();
            byte1();
            break;
        }
        break;
    }
  }
  else if ((strcmp(remoteID,"1010110110001010")==0)){
    switch (command){
          case 'U': //Up
            switch (blindsChannel) {
              case 'A': //All Up = 0100
                byte0();
                byte1();
                byte0();
                byte0();
                break;
      
              default: //Any single channel up = 1100
                byte1();
                byte1();
                byte0();
                byte0();
                break;
              }
             break;
           case 'D': //Down
            switch (blindsChannel) {
              case 'A': //All Down = 1011
                byte1();
                byte0();
                byte1();
                byte1();
                break;
      
              default: //Any single channel down = 0111
                byte0();
                byte1();
                byte1();
                byte1();
                break;
              }
             break;
          case 'S': //Stop
            switch (blindsChannel) {
              case 'A': //All Stop = 1001
                byte1();
                byte0();
                byte0();
                byte1();
                break;
      
              default: //Any single channel stop = 0101
                byte0();
                byte1();
                byte0();
                byte1();
                break;
              }
            break;
          case 'C': //Confirm 1101
            byte1();
            byte1();
            byte0();
            byte1();
            break;
          case 'L': //Limit 1011
            byte1();
            byte0();
            byte1();
            byte1();
            break;
          case 'F': //LU= 0110
            byte0();
            byte1();
            byte1();
            byte0();
            break;
    }
  }
}

//All messages end with a 1 byte and then a special ending byte (111 instead of the normal tribits of 010 or 011)
void sendMessageEnd() {
  byte1();
  byteFinal();
}



void channel2_up(void) {
  // preamble
  sendPreamble();

  // 1011
  byte1();
  byte0();
  byte1();
  byte1();

  // 0110
  byte0();
  byte1();
  byte1();
  byte0();

  // 1101
  byte1();
  byte1();
  byte0();
  byte1();

  // 1010
  byte1();
  byte0();
  byte1();
  byte0();

  // 0100
  byte0();
  byte1();
  byte0();
  byte0();

  // 0011
  byte0();
  byte0();
  byte1();
  byte1();

  // 0000
  byte0();
  byte0();
  byte0();
  byte0();

  // 1000
  byte1();
  byte0();
  byte0();
  byte0();

  // 1110
  byte1();
  byte1();
  byte1();
  byte0();

  // 01101
  byte0();
  byte1();
  byte1();
  byte0();
  byte1();

  byteFinal();
}

//All messages have a preamble that is NOT made up of tribit bytes.
void sendPreamble() {
  int a = BITLENGTH * PREAMBLE_1;
  int b = BITLENGTH * PREAMBLE_2;
  int c = BITLENGTH * PREAMBLE_3;

  // 11 high, 7 low, 5 high
  // 9625 = BITLENGTH * 11
  // 6125 = BITLENGTH * 7
  // 4375 = BITLENGTH * 5

  digitalWrite(GPIOPIN, HIGH);
  delayMicroseconds(a);
  digitalWrite(GPIOPIN, LOW);
  delayMicroseconds(b);
  digitalWrite(GPIOPIN, HIGH);
  delayMicroseconds(c);
  //Serial.print("P");

}

//The message bytes are three bit bytes. This took forever to decode because of the preamble being different:

void byte0(void) {
  digitalWrite(GPIOPIN, LOW);
  delayMicroseconds(BITLENGTH);
  digitalWrite(GPIOPIN, HIGH);
  delayMicroseconds(BITLENGTH);
  digitalWrite(GPIOPIN, LOW);
  delayMicroseconds(BITLENGTH);
//Serial.print("0");
}

void byte1(void) {
  digitalWrite(GPIOPIN, LOW);
  delayMicroseconds(BITLENGTH);
  digitalWrite(GPIOPIN, HIGH);
  delayMicroseconds(BITLENGTH);
  digitalWrite(GPIOPIN, HIGH);
  delayMicroseconds(BITLENGTH);
  digitalWrite(GPIOPIN, LOW);
//Serial.print("1");
}

//Special final byte only used at message end:
void byteFinal(void) {
  digitalWrite(GPIOPIN, HIGH);
  delayMicroseconds(BITLENGTH);
  digitalWrite(GPIOPIN, HIGH);
  delayMicroseconds(BITLENGTH);
  digitalWrite(GPIOPIN, HIGH);
  delayMicroseconds(BITLENGTH);
  digitalWrite(GPIOPIN, LOW);
  //Serial.print("2");
}

void adminMessageParser(byte* payload) {
  Serial.println();
  Serial.println("Parsing start: ");

  //char debugJSON[] = " {\"SensorType\": \"Temperature\", \"Value\": 11}"; //Original message
  //Serial.println(debugJSON);

  StaticJsonDocument<300> JSONDocument;                         //Memory pool
  DeserializationError err = deserializeJson(JSONDocument, payload); //Parse message

  if (err) {   //Check for errors in parsing

    Serial.println("Parsing failed. ERROR:");
    Serial.println(err.c_str());
    delay(5000);
    return;

  }

  const char* remoteID_J = JSONDocument["remoteID"]; //Get remoteID
  const char* channel_J = JSONDocument["channel"]; //Get blindsChannel
  const char* command_J  = JSONDocument["command"]; //Get command
  const char* tapOrHold_J  = JSONDocument["tapOrHold"]; //Get tapOrHold

  std::strcpy(remoteID,remoteID_J);
  
  //ArduinoJSON returns a single character as a one character array. That only took a month to figure out.
  blindsChannel=channel_J[0];
  command=command_J[0];
  tapOrHold=tapOrHold_J[0];

  Serial.println();
  Serial.println("In parser:");
  Serial.print("RemoteID_J: ");
  Serial.println(remoteID_J);
  Serial.print("RemoteID ");
  Serial.println(remoteID);
  Serial.print("Channel_J: ");
  Serial.println(channel_J);
  Serial.print("Channel: ");
  Serial.println(blindsChannel);
  Serial.print("Command_J: ");
  Serial.println(command_J);
  Serial.print("Command: ");
  Serial.println(command);
  Serial.print("tapOrHold_J: ");
  Serial.println(tapOrHold_J);
  Serial.print("tapOrHold: ");
  Serial.println(tapOrHold);
  Serial.println();

}


//MQTT Sections
void publishBlindsConfig(const char* blindsName, const char* friendlyName) {

  //Set up the command topic root address to save space (not sure if it actually saves space, but means I can avoid string concatenation, so yay.
  char configTopic[100]{};
 
  std::strcat(configTopic,blindsName);
  Serial.println(configTopic);
  std::strcat(configTopic,"/config");
  Serial.println(configTopic);

 
  const int capacity = JSON_OBJECT_SIZE(33);
  StaticJsonDocument<capacity> configJson;

  //Basic config values for a cover in Home Assistant
  configJson["name"] = friendlyName;
  configJson["~"] = blindsName; //HA allows a tilde to define the root topic path so it doesn't need to be rebroadcast for each topic. Nice.
  configJson["command_topic"] = "~/position/set";
  configJson["state_topic"] = "~/position/state";
  JsonObject device = configJson.createNestedObject("device");
  device["name"] = friendlyName;
  device["identifiers"] = friendlyName;
  device["via_device"] = "esp8266";
  device["manufacturer"] = "Blinds.com (resold Comfortex with Bofu Motor)";
  device["model"] = "Room Darkening Sheer Shades";
  
  configJson["device_class"] = "blind";
  //configJson["availability_topic"] = "~/availability";
  configJson["qos"] = 0;
  configJson["retain"] = true;
  configJson["payload_open"] = "U";
  configJson["payload_close"] = "S";
  configJson["payload_stop"] = "D";
  configJson["state_open"] = "open";
  configJson["state_opening"] = "opening";
  configJson["state_closed"] = "closed";
  configJson["state_closing"] = "closing";
  configJson["payload_available"] = "online";
  configJson["payload_not_available"] = "offline";
  configJson["optimistic"] = true;
  configJson["value_template"] = "'{{value_template}}'";
  configJson["tilt_command_topic"] = "~/tilt/set";
  configJson["tilt_status_topic"] = "~/tilt/state";
  configJson["tilt_status_template"] = "'{{value_json.tilt_status_template}}'";
  configJson["tilt_min"] = 0;
  configJson["tilt_max"] = 1;
  configJson["tilt_closed_value"] = 0;
  configJson["tilt_opened_value"] = 1;
  configJson["unique_id"] = friendlyName;
  
  //serializeJsonPretty(configJson, Serial);
  //Serial.println("");
  char message[1200];

  serializeJson(configJson,message,sizeof(message));  
  MQTTClient.publish(configTopic, message, true);
}
