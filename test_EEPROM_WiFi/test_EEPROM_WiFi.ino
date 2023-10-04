#include <EEPROM.h>
#include <WiFi.h>

#define EEPROM_INIT_CODE            0xAD
#define EEPROM_LEN_INIT_INFO        1
#define EEPROM_LEN_SSID             16
#define EEPROM_LEN_PASSWORD         16
#define EEPROM_LEN_SERVER_IP        16
#define EEPROM_LEN_SERVER_PORT      3

#define EEPROM_ADR_INIT_INFO        0
#define EEPROM_ADDR_SSID            EEPROM_ADR_INIT_INFO + EEPROM_LEN_INIT_INFO
#define EEPROM_ADDR_PASSWORD        EEPROM_ADDR_SSID + EEPROM_LEN_SSID
#define EEPROM_ADDR_SERVER_IP       EEPROM_ADDR_PASSWORD + EEPROM_LEN_PASSWORD
#define EEPROM_ADDR_SERVER_PORT     EEPROM_ADDR_SERVER_IP + EEPROM_LEN_SERVER_IP
#define EEPROM_ADR_BEGIN            EEPROM_ADDR_SERVER_PORT + EEPROM_LEN_SERVER_PORT

const char *ssid_eep = "SFS OFFICE";
const char *password_eep = "sfs#office!@";
const char *serverIP_eep = "113.161.168.2";
const int port_eep = 82;

char ssid_read[EEPROM_LEN_SSID + 1];
char password_read[EEPROM_LEN_PASSWORD + 1];
char serverIP_read[EEPROM_LEN_SERVER_IP + 1];
int port_read = 0;

char ssid_temp[EEPROM_LEN_SSID];
char password_temp[EEPROM_LEN_PASSWORD];

WiFiServer server(80);

bool isEEPROMInitialized() {
  EEPROM.begin(EEPROM_ADR_BEGIN);
  byte initCode = EEPROM.read(EEPROM_ADR_INIT_INFO);
  return initCode == EEPROM_INIT_CODE;
}

void writeDataToEEPROM() {
  if (!isEEPROMInitialized()) {
    EEPROM.begin(EEPROM_ADR_BEGIN);
    // Ghi thông tin vào EEPROM
    EEPROM.write(EEPROM_ADR_INIT_INFO, EEPROM_INIT_CODE);
    for (int i = 0; i < EEPROM_LEN_SSID; i++) {
      EEPROM.write(EEPROM_ADDR_SSID + i, ssid_eep[i]);
    }
    for (int i = 0; i < EEPROM_LEN_PASSWORD; i++) {
      EEPROM.write(EEPROM_ADDR_PASSWORD + i, password_eep[i]);
    }
    for (int i = 0; i < EEPROM_LEN_SERVER_IP; i++) {
      EEPROM.write(EEPROM_ADDR_SERVER_IP + i, serverIP_eep[i]);
    }
    byte portByte1 = port_eep >> 8;
    EEPROM.write(EEPROM_ADDR_SERVER_PORT, portByte1);
    byte portByte2 = port_eep & 0xFF;
    EEPROM.write(EEPROM_ADDR_SERVER_PORT + 1, portByte2);

    EEPROM.commit();
  }
}

void readDataFromEEPROM() {
  EEPROM.begin(EEPROM_ADR_BEGIN);

  // Đọc và in thông tin từ EEPROM
  byte initCode = EEPROM.read(EEPROM_ADR_INIT_INFO);
  if (initCode == EEPROM_INIT_CODE) {
    for (int i = 0; i < EEPROM_LEN_SSID; i++) {
      ssid_read[i] = EEPROM.read(EEPROM_ADDR_SSID + i);
    }
    ssid_read[EEPROM_LEN_SSID] = '\0';

    for (int i = 0; i < EEPROM_LEN_PASSWORD; i++) {
      password_read[i] = EEPROM.read(EEPROM_ADDR_PASSWORD + i);
    }
    password_read[EEPROM_LEN_PASSWORD] = '\0';

    for (int i = 0; i < EEPROM_LEN_SERVER_IP; i++) {
      serverIP_read[i] = EEPROM.read(EEPROM_ADDR_SERVER_IP + i);
    }
    serverIP_read[EEPROM_LEN_SERVER_IP] = '\0';

    byte portByte1 = EEPROM.read(EEPROM_ADDR_SERVER_PORT);
    byte portByte2 = EEPROM.read(EEPROM_ADDR_SERVER_PORT + 1);
    port_read = (int(portByte1) << 8) | int(portByte2);

    // In thông tin từ EEPROM ra Serial Monitor
    Serial.println("SSID: " + String(ssid_read));
    Serial.println("Password: " + String(password_read));
    Serial.println("Server IP: " + String(serverIP_read));
    Serial.println("Server Port: " + String(port_read));
  } else {
    Serial.println("No valid data found in EEPROM.");
  }
}

void setup() {
  Serial.begin(115200);

  // Kiểm tra xem đã có dữ liệu trong EEPROM hay chưa
  if (!isEEPROMInitialized()) {
    // Nếu chưa có dữ liệu, ghi thông tin vào EEPROM
    writeDataToEEPROM();
    Serial.println("Write new data to EEPROM");
  }
  // Đọc và in thông tin từ EEPROM
  readDataFromEEPROM();

  Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid_read);

    WiFi.begin(ssid_read, password_read);

    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }

    Serial.println("");
    Serial.println("WiFi connected.");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    server.begin();
}

void loop(){
 WiFiClient client = server.available();   // listen for incoming clients

  if (client) {                             // if you get a client,
    Serial.println("New Client.");           // print a message out the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        if (c == '\n') {                    // if the byte is a newline character

          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println();

            // the content of the HTTP response follows the header:
            client.print("Click <a href=\"/H\">here</a> to turn the LED on pin 5 on.<br>");
            client.print("Click <a href=\"/L\">here</a> to turn the LED on pin 5 off.<br>");

            // The HTTP response ends with another blank line:
            client.println();
            // break out of the while loop:
            break;
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        // Check to see if the client request was "GET /H" or "GET /L":
        if (currentLine.endsWith("GET /H")) {
          digitalWrite(5, HIGH);               // GET /H turns the LED on
        }
        if (currentLine.endsWith("GET /L")) {
          digitalWrite(5, LOW);                // GET /L turns the LED off
        }
      }
    }

    if(digitalRead(5) == 1) 
    {
      Serial.println("Pin 5 ON");
    }
    // close the connection:
    client.stop();
    Serial.println("Client Disconnected.");
  }
}