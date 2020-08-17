// Codes are as follows on original BY-305 remote:
// Learn a remote channel: hold down button on blinds for 3 seconds until Jog Jog, then CONFIRM on channel to learn
// Make a blind forget a channel: hit confirm roughly 10 times in a row, short clicks. All blinds on channel will forget.
// Set limits: hold limit button for 6 seconds until blind double jogs twice. Then tap up to make blind go up, stop to stop when you are close to the upper limit, then tap up or down to move to exact upper limit. Confirm to set upper limit. Repeat for lower limit.
// Set favorite: poistion blind in "favorite" position (probably all the way closed). Hold hte limit button and immediately push the up button (LU in code below). The blind will jog to show recognition. Now the stop button doubles as a favorite button if the blinds have been unmoving for three seconds, or are at the upper or lower limit.

#include <ESP8266WiFi.h>
#include "login.h" //provides ssid and password

//Setup a server
WiFiServer server(80);

//Declare Constants
int LEDPIN = 13; // GPIO13
int BITLENGTH=350; //in microseconds
int PREAMBLE_1=11; //Length of 1st series, of 1s in preamble
int PREAMBLE_2=7; //Length of second series, of 0s in preamble
int PREAMBLE_3=5; //Length of third series, of 1s in preamble


void preamble(void);
void bit0(void);
void bit1(void);
void bitfinal(void);
void channel2_up(void);

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
 
  // Start the server
  server.begin();
  Serial.println("Server started");
 
  // Print the IP address
  Serial.print("Use this URL to connect: ");
  Serial.print("http://");
  Serial.print(WiFi.localIP());
  Serial.println("/");
 
}
 
void loop() {
  // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }
 
  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){
    delay(1);
  }
 
  // Read the first line of the request
  String request = client.readStringUntil('\r');
  Serial.println(request);
  client.flush();
 
  // Match the request
 
  int value = LOW;
  if (request.indexOf("/LED=ON") != -1)  {
    int i;
   
    for (i = 0; i < 8; i++) {
      channel2_up();
    }
    value = HIGH;
    
  }
  if (request.indexOf("/LED=OFF") != -1)  {
    value = LOW;
  }
 
// Set ledPin according to the request
//digitalWrite(ledPin, value);
 
  // Return the response
  client.println("HTTP/1.1 200 OK");
  client.println("Content-Type: text/html");
  client.println(""); //  do not forget this one
  client.println("<!DOCTYPE HTML>");
  client.println("<html>");
 
  client.print("Led pin is now: ");
 
  if(value == HIGH) {
    client.print("On");
  } else {
    client.print("Off");
  }
  client.println("<br><br>");
  client.println("<a href=\"/LED=ON\"\"><button>Turn On </button></a>");
  client.println("<a href=\"/LED=OFF\"\"><button>Turn Off </button></a><br />");  
  client.println("</html>");
 
  delay(1);
  Serial.println("Client disonnected");
  Serial.println("");
 
}



void channel2_up(void) {
  // preamble
  preamble();
 
  // 1011
  bit1();
  bit0();
  bit1();
  bit1();
 
  // 0110
  bit0();
  bit1();
  bit1();
  bit0();
 
  // 1101
  bit1();
  bit1();
  bit0();
  bit1();
 
  // 1010
  bit1();
  bit0();
  bit1();
  bit0();
 
  // 0100
  bit0();
  bit1();
  bit0();
  bit0();
 
  // 0011
  bit0();
  bit0();
  bit1();
  bit1();
 
  // 0000
  bit0();
  bit0();
  bit0();
  bit0();
 
  // 1000
  bit1();
  bit0();
  bit0();
  bit0();
 
  // 1110
  bit1();
  bit1();
  bit1();
  bit0();
 
  // 01101
  bit0();
  bit1();
  bit1();
  bit0();
  bit1();

  bitfinal();
}

void preamble(void) {
  int a=BITLENGTH*PREAMBLE_1;
  int b=BITLENGTH*PREAMBLE_2;
  int c=BITLENGTH*PREAMBLE_3;
 
  // 11 high, 7 low, 5 high
  // 9625 = BITLENGTH * 11
  // 6125 = BITLENGTH * 7
  // 4375 = BITLENGTH * 5

    digitalWrite(13,HIGH);
  delayMicroseconds(a);
  digitalWrite(13,LOW);
  delayMicroseconds(b);
  digitalWrite(13,HIGH);
  delayMicroseconds(c);
 
}

void bit0(void) {
  digitalWrite(13,LOW);
  delayMicroseconds(BITLENGTH);
  digitalWrite(13,HIGH);
  delayMicroseconds(BITLENGTH);
  digitalWrite(13,LOW);
  delayMicroseconds(BITLENGTH);
}

void bit1(void) {
  digitalWrite(13,LOW);
  delayMicroseconds(BITLENGTH);
  digitalWrite(13,HIGH);
  delayMicroseconds(BITLENGTH);
  digitalWrite(13,HIGH);
  delayMicroseconds(BITLENGTH);
  digitalWrite(13,LOW);
}

void bitfinal(void) {
  digitalWrite(13,HIGH);
  delayMicroseconds(BITLENGTH);
  digitalWrite(13,HIGH);
  delayMicroseconds(BITLENGTH);
  digitalWrite(13,HIGH);
  delayMicroseconds(BITLENGTH);
  digitalWrite(13,LOW);
}
 
