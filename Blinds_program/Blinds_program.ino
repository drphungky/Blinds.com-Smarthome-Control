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
const int LEDPIN = 13; // GPIO13
const int BITLENGTH = 350; //in microseconds
const int PREAMBLE_1 = 11; //Length of 1st series, of 1s in preamble
const int PREAMBLE_2 = 7; //Length of second series, of 0s in preamble
const int PREAMBLE_3 = 5; //Length of third series, of 1s in preamble

//Setup variables
char blindsChannel = '2';
char command = 'U';
char remoteID[] = "1011011011011010";
char stopCheck = '0';

//Setup functions
void sendPreamble(void);
void byte0(void);
void byte1(void);
void byteFinal(void);

void sendChannel(void);
void sendRemoteID(void);
void channel2_up(void);
void sendMoveOrConfirm(void);
void sendStopCheck(void);
void sendChannelCheck(void);
void sendCommandCheck(void);
void sendMessageEnd(void);
void sendFullCommand(void);
void messageParser(void);

void setup() {
  Serial.begin(115200);
  Serial.println();
  delay(10);

  pinMode(LEDPIN, OUTPUT);


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
  MQTTClient.subscribe("home-assistant/diningRoomBlinds/");
  MQTTClient.publish("home-assistant/diningRoomBlinds/state/", "closed");


  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");

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
  //sendFullCommand();

  Serial.println("Before parser:");
  Serial.print("RemoteID: ");
  Serial.println(remoteID);
  Serial.print("Channel: ");
  Serial.println(blindsChannel);
  Serial.print("Command: ");
  Serial.println(command);

  messageParser(payload);

  Serial.println("After parser:");
  Serial.print("RemoteID: ");
  Serial.println(remoteID);
  Serial.print("Channel: ");
  Serial.println(blindsChannel);
  Serial.print("Command: ");
  Serial.println(command);


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
  for (int i = 0; i < 8; i++) {
    sendPreamble();
    sendRemoteID();
    sendChannel();
    sendCommand();
    sendStopCheck();
    sendMoveOrConfirm();
    sendChannelCheck();
    sendCommandCheck();
    sendMessageEnd();
  }
  Serial.print("RF Command sent");
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

/*Note that the result of setStopCheck isn't used in sendStopCheck even though I could have, because if I ever figure
   out how the checksum is actually calculated, that function will be superfluous. */

//Send the stopcheck portion of a button push (some kind of checkbytes)
void setStopCheck() {
  if (command == 'D' || command == 'U') {
    stopCheck = '0';
  }
  else {
    stopCheck = '1';
  }
}


//Send the stopcheck portion of a button push (some kind of checkbytes)
void sendStopCheck() {
  if (command == 'D' || command == 'U') {
    byte0();
  }
  else {
    byte1();
  }
  byte0();
  byte0();
  byte0();
}




//Send the moveorconfirm portion of a button push (some kind of checkbytes)
void sendMoveOrConfirm() {
  if (command == 'D' || command == 'U' || command == 'C') {
    byte1();
  }
  else {
    byte0();
  }
  byte0();
  byte0();
  byte0();
}

/*Send first 4 of 8 digit checksum that I can't really figure out. Best I can do is this hardcoded list.*/

void sendChannelCheck() {
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


/*Send second 4 of 8 digit checksum that I can't really figure out. Best I can do is this hardcoded list.*/

void sendCommandCheck() {
  switch (blindsChannel) {
    case 'A':
      switch (command) {
        case 'U': //Up = 1010
          byte0();
          byte0();
          byte1();
          byte1();
          break;

        case 'D': //Down = 0000
          byte0();
          byte0();
          byte0();
          byte0();
          break;

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

    default:
      switch (command) {
        case 'U': //Up = 0110
          byte0();
          byte1();
          byte1();
          byte0();
          break;

        case 'D': //Down = 1000
          byte1();
          byte0();
          byte0();
          byte0();
          break;

        case 'S': //Stop = 0111
          byte1();
          byte0();
          byte1();
          byte0();
          break;

        case 'C': //Confirm = 0111
          byte0();
          byte0();
          byte1();
          byte0();
          break;

        case 'L': //Limit = 1000
          byte0();
          byte1();
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

  digitalWrite(13, HIGH);
  delayMicroseconds(a);
  digitalWrite(13, LOW);
  delayMicroseconds(b);
  digitalWrite(13, HIGH);
  delayMicroseconds(c);

}

//The message bytes are three bit bytes. This took forever to decode because of the preamble being different:

void byte0(void) {
  digitalWrite(13, LOW);
  delayMicroseconds(BITLENGTH);
  digitalWrite(13, HIGH);
  delayMicroseconds(BITLENGTH);
  digitalWrite(13, LOW);
  delayMicroseconds(BITLENGTH);
}

void byte1(void) {
  digitalWrite(13, LOW);
  delayMicroseconds(BITLENGTH);
  digitalWrite(13, HIGH);
  delayMicroseconds(BITLENGTH);
  digitalWrite(13, HIGH);
  delayMicroseconds(BITLENGTH);
  digitalWrite(13, LOW);
}

//Special final byte only used at message end:
void byteFinal(void) {
  digitalWrite(13, HIGH);
  delayMicroseconds(BITLENGTH);
  digitalWrite(13, HIGH);
  delayMicroseconds(BITLENGTH);
  digitalWrite(13, HIGH);
  delayMicroseconds(BITLENGTH);
  digitalWrite(13, LOW);
}


void messageParser(byte* payload) {
  Serial.println();
  Serial.println("Parsing start: ");

  char debugJSON[] = " {\"SensorType\": \"Temperature\", \"Value\": 11}"; //Original message
  Serial.println(debugJSON);

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

  std::strcpy(remoteID,remoteID_J);
  //
  blindsChannel=channel_J[0];
  command=command_J[0];

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

  Serial.println();

}
