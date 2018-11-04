

#define TINY_GSM_MODEM_SIM900

#include <TinyGsmClient.h>

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// Use Hardware Serial on Mega, Leonardo, Micro
//#define SerialAT Serial1

// or Software Serial on Uno, Nano
#include <SoftwareSerial.h>
SoftwareSerial SerialAT(7, 8); // RX, TX


// Your GPRS credentials
// Leave empty, if missing user or pass
const char apn[]  = "4g.tele2.se";
const char user[] = "";
const char pass[] = "";

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

#define LED_PIN 13
int ledStatus = LOW;

long lastReconnectAttempt = 0;

const char server[] = "home.zachrisson.info";
int port = 6666;

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void setup() {
  pinMode(LED_PIN, OUTPUT);

  digitalWrite(9, HIGH);
  delay(1000);
  digitalWrite(9, LOW);
  delay(5000);

  // Set console baud rate
  SerialMon.begin(19200);
  
  // Set GSM module baud rate
  SerialAT.begin(19200);
  delay(3000);

  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  SerialMon.println("Initializing modem...");
  modem.restart();

  String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem: ");
  SerialMon.println(modemInfo);

  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    resetFunc();
  }
  SerialMon.println(" OK");

  SerialMon.print("Connecting to ");
  SerialMon.print(apn);
  if (!modem.gprsConnect(apn, user, pass)) {
    SerialMon.println(" fail");
    resetFunc();
  }
  SerialMon.println(" OK");
}

unsigned long timeout;
unsigned char relay_state = '0';

#define TIMEOUT_MS 10000L
void loop() {
  if (client.connected() == false) {
    if (!client.connect(server, port)) {
      SerialMon.println("Failed to connect to server, rebooting");
      resetFunc();
    } else {
      SerialMon.println("Connected to server");
    }
  }

  // Wait for remote command
  timeout = millis();
  while (client.connected() && millis() - timeout < TIMEOUT_MS) {
    // Print available data
    while (client.available()) {
      char c = client.read();
      if (c == '1') {
        SerialMon.println("Turning on relay");
        relay_state = '1';
      } else if (c == '0') {
        SerialMon.println("Turning off relay");
        relay_state = '0';
      }
      SerialMon.print(c);
      
      timeout = millis();
    }
  }

  // Send heartbeat
  if (client.connected()) {
    SerialMon.println("Sending heartbeat");
    client.println(relay_state);
  }
}


