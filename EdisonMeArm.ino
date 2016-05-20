#include <Servo.h>
#include <WiFi.h>
#include <WiFiUdp.h>

int status = WL_IDLE_STATUS;
char ssid[] = "SkyNet";
char pass[] = "alphabetasoup";

unsigned int localPort = 5556;      // local port to listen on

char packetBuffer[255]; //buffer to hold incoming packet
char  ReplyBuffer[] = "acknowledged";       // a string to send back

Servo mearmServos[4];
int PGServo[4] = {0,0,0,0};
char PGButton = 0;
char PGPosture = 0;
int PGMotion[2] = {0,0};

WiFiUDP Udp;

const unsigned long updateInterval = 20;
unsigned long elapsed = 0;

void setup() {
  //Initialize serial and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  // attempt to connect to Wifi network:
  while (status != WL_CONNECTED) {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    
    status = WiFi.begin(ssid, pass);
    Serial.println(status);
    delay(10000);
  }
  Serial.println("Connected to wifi");
  printWifiStatus();

  Serial.println("\nStarting connection to server...");
  // if you get a connection, report back via serial:
  Udp.begin(localPort);

  elapsed = millis();

  mearmServos[0].attach(3);
  mearmServos[1].attach(5);
  mearmServos[2].attach(6);
  mearmServos[3].attach(9);
}

void loop() {

  // if there's data available, read a packet
  int packetSize = Udp.parsePacket();

  unsigned long now = millis();
  if (packetSize && now - elapsed >= updateInterval) {
    elapsed = now;
    
    Serial.print("Received packet of size ");
    Serial.println(packetSize);
    Serial.print("From ");
    IPAddress remoteIp = Udp.remoteIP();
    Serial.print(remoteIp);
    Serial.print(", port ");
    Serial.println(Udp.remotePort());

    // read the packet into packetBufffer
    int len = Udp.read(packetBuffer, 255);
    if (len > 0) {
      packetBuffer[len] = 0;
    }
    Serial.println("Contents:");
    Serial.println(packetBuffer);

    int state = 0;
    char* command = strtok(packetBuffer, ",");
    
    while (command != 0) {
      switch (state) {
        case 0:
          PGButton = command[0];
          break;
        case 1:
          PGPosture = command[0];
          break;
        case 2:
          PGMotion[0] = atoi(command);
          if (PGMotion[0] < -400) {
            PGMotion[0] = -400;
          } else if (PGMotion[0] > 400) {
            PGMotion[0] = 400;
          }
          PGMotion[0] = map(PGMotion[0], -400, 400, 180, 0);
          break;
        case 3:
          PGMotion[1] = atoi(command);
          if (PGMotion[1] < -400) {
            PGMotion[1] = -400;
          } else if (PGMotion[1] > 400) {
            PGMotion[1] = 400;
          }
          PGMotion[1] = map(PGMotion[1], -400, 400, 0, 180);
          break;
        default:
          PGServo[state - 4] = atoi(command);
          break;
      }
      state++;
      command = strtok(0, ",");
    }

    
    mearmServos[0].write(PGMotion[0]);
    for (int i = 1; i < 4; i++) {
      mearmServos[i].write(PGServo[i]);
    }
  }
}


void printWifiStatus() {
  // print the SSID of the network you're attached to:
  Serial.print("SSID: ");
  Serial.println(WiFi.SSID());

  // print your WiFi shield's IP address:
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print the received signal strength:
  long rssi = WiFi.RSSI();
  Serial.print("signal strength (RSSI):");
  Serial.print(rssi);
  Serial.println(" dBm");
}




