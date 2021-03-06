#define TINY_GSM_MODEM_SIM900

#define RELAY_PIN 2
#define GPRS_SHIELD_POWER_PIN 9
#define LED_PIN 13

// Heartbeat every minute
#define HEARTBEAT_MS 60000L

// 120 minutes
#define MAX_RELAY_TIME_ON_MS 7200000L
#define MAX_RELAY_TIME_HEARTBEAT_TICKS (MAX_RELAY_TIME_ON_MS / HEARTBEAT_MS)

#include <TinyGsmClient.h>

// Set serial for debug console (to the Serial Monitor, default speed 115200)
#define SerialMon Serial

// or Software Serial on Uno, Nano
#include <SoftwareSerial.h>
SoftwareSerial SerialAT(7, 8); // RX, TX

// Your GPRS credentials
const char apn[] = "4g.tele2.se";
const char user[] = "";
const char pass[] = "";

unsigned long timeout;
unsigned int max_relay_time_on_ticks;
unsigned int heartbeat_cnt = 0;

const char server[] = "home.zachrisson.info";
const int port = 6666;

TinyGsm modem(SerialAT);
TinyGsmClient client(modem);

//declare reset function @ address 0
void(* resetFunc) (void) = 0;

void gprsPowerUp(void) {
  SerialMon.println("Powering up modem");
  digitalWrite(GPRS_SHIELD_POWER_PIN, HIGH);
  delay(1000);
  digitalWrite(GPRS_SHIELD_POWER_PIN, LOW);
  delay(5000);
}

void gprsPowerDown(void) {
  SerialMon.println("Powering down modem");
  modem.poweroff();
  delay(5000);
}

void setup() {
  //pinMode(LED_PIN, OUTPUT);
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
  SerialMon.print("Initializing modem...");
  
  //modem.restart();
  modem.init();
  
  String modemInfo = modem.getModemInfo();
  if (modemInfo == "") {
    SerialMon.println(" fail");
    delay(1000);
    resetFunc();
  } else {
    SerialMon.println(" OK");
  }
  SerialMon.println("Modem: " + modemInfo);
//
//  delay(1000);
//  resetFunc();
  
  SerialMon.print("Bringing up network...");
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
    if (client.connect(server, port) == false) {
      SerialMon.println("Failed to connect to server, rebooting");
      delay(1000);
      resetFunc();
    } else {
      SerialMon.print("Connected to ");
      SerialMon.print(server);
      SerialMon.print(":");
      SerialMon.println(port);
    }
  }

  // Send heartbeat
  if (client.connected()) {
    SerialMon.print("Sending heartbeat ");
    SerialMon.print(heartbeat_cnt);
    SerialMon.print(": ");
    SerialMon.println(digitalRead(RELAY_PIN));
    heartbeat_cnt++;

    client.println(digitalRead(RELAY_PIN));

    if (max_relay_time_on_ticks > 0) {
      max_relay_time_on_ticks--;
      if (max_relay_time_on_ticks == 0) {
        SerialMon.println("Max timeout, turning off relay");
        digitalWrite(RELAY_PIN, LOW);
      }
    }
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
        max_relay_time_on_ticks = MAX_RELAY_TIME_HEARTBEAT_TICKS;
      } else if (c == '0') {
        SerialMon.println("Turning off relay");
        digitalWrite(RELAY_PIN, LOW);
      }
      timeout = millis();
    }
  }
}

