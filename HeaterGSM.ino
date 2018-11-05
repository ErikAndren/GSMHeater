

#define TINY_GSM_MODEM_SIM900

#define RELAY_PIN 2
#define GPRS_SHIELD_POWER_PIN 9
#define LED_PIN 13
#define HEARTBEAT_MS 10000L

#include <TinyGsmClient.h>

unsigned long timeout;

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

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

int ledStatus = LOW;
long lastReconnectAttempt = 0;

const char server[] = "home.zachrisson.info";
const int port = 6666;

void(* resetFunc) (void) = 0; //declare reset function @ address 0

void gprsPowerUp(void) {
  digitalWrite(GPRS_SHIELD_POWER_PIN, HIGH);
  delay(1000);
  digitalWrite(GPRS_SHIELD_POWER_PIN, LOW);
  delay(5000);
}

void gprsPowerDown(void) {
    modem.poweroff();
    delay(5000);
}

void setup() {
  pinMode(LED_PIN, OUTPUT);
  pinMode(GPRS_SHIELD_POWER_PIN, OUTPUT); 
  pinMode(RELAY_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, LOW);
  
  // Set console baud rate
  SerialMon.begin(19200);

  // Set GSM module baud rate
  SerialAT.begin(19200);

  SerialMon.println("\nInitializing GPRS Shield...");
  gprsPowerDown();
  /* Must have shorted R13 on board to make this work */
  gprsPowerUp();    

  // Restart takes quite some time
  // FIXME: To skip it, call init() instead of restart()
  SerialMon.println("Initializing modem...");
  modem.restart();

  String modemInfo = modem.getModemInfo();
  SerialMon.print("Modem: ");
  SerialMon.println(modemInfo);

  SerialMon.print("Waiting for network...");
  if (!modem.waitForNetwork()) {
    SerialMon.println(" fail");
    delay(1000);

    resetFunc();
  }
  SerialMon.println(" OK");

  SerialMon.print("Connecting to ");
  SerialMon.print(apn);
  if (!modem.gprsConnect(apn, user, pass)) {
    SerialMon.println(" fail");
    delay(1000);

    resetFunc();
  }
  SerialMon.println(" OK");
}

void loop() {
  if (client.connected() == false) {
    if (!client.connect(server, port)) {
      SerialMon.println("Failed to connect to server, rebooting");
      delay(1000);
      resetFunc();
    } else {
      SerialMon.println("Connected to server");
    }
  }

    // Send heartbeat
  if (client.connected()) {
    SerialMon.println("Sending heartbeat");
    client.println(digitalRead(RELAY_PIN));
  }

  // Wait for remote command
  timeout = millis();
  while (client.connected() && millis() - timeout < HEARTBEAT_MS) {
    // Print available data
    while (client.available()) {
      char c = client.read();
      if (c == '1') {
        SerialMon.println("Turning on relay");
        digitalWrite(RELAY_PIN, HIGH);
      } else if (c == '0') {
        SerialMon.println("Turning off relay");
        digitalWrite(RELAY_PIN, LOW);
      }
      SerialMon.print(c);
      
      timeout = millis();
    }
  }
}


