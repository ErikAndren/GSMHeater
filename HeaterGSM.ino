#include <SoftwareSerial.h>

SoftwareSerial SIM900(7, 8);

char incoming_char = 0;

const String CHECK_NETWORK = "AT+CREG?\r\n";
const String NETWORK_CONNECTED = "+CREG: 0,1\r\n";
const String SHUT_IP_CONNECTION = "AT+CIPSHUT\r\n";
const String CHECK_IP_STATUS = "AT+CIPSTATUS\r\n";
const String IP_STATUS_SHUTDOWN = "SHUT OK\r\n";

const String SINGLE_CONNECTION_MODE = "AT+CIPMUX=0\r\n";
const String READ_PDP_CONTEXT = "AT+CGDCONT?\r\n";
const String WRITE_PDP_CONTEXT = "AT+CGDCONT=1,\"IP\",\"4g.tele2.se\"\r\n";
const String SET_APN_USER_PWD = "AT+CSTT=\"4g.tele2.se\"\r\n";
const String BRING_UP_GPRS = "AT+CIICR\r\n";
const String GET_LOCAL_IP = "AT+CIFRS\r\n";
const String CONNECT_TO_SERVER = "AT+CIPSTART=\"TCP\",\"home.zachrisson.info\",\"6666\"\r\n";
const String SEND_DATA = "AT+CIPSEND\r\n";
const String TURN_OFF_ECHO = "ATE0\r\n";

void setup() {
  //FIXME: Add possibility to start shield from arduino
//  pinMode(9, OUTPUT); 
//  digitalWrite(9,LOW);
//  delay(1000);
//  digitalWrite(9,HIGH);
//  delay(2500);
//  digitalWrite(9,LOW);
//  delay(3500);
  
  SIM900.begin(19200);
  Serial.begin(19200);

  Serial.print(TURN_OFF_ECHO);
  
  waitForNetwork();
}

int readSIM900UART(boolean echo) {
  if (SIM900.available()) {
      //Get the character from the cellular serial port
      incoming_char = SIM900.read(); 
      if (echo) {
        Serial.print(incoming_char);
      }
      return incoming_char;
  }
  return -1;
}

void writeSIM900UART(byte val) {   
}

void loop() {
  readSIM900UART(true);
  
  if (Serial.available()) {
      incoming_char = Serial.read();
      SIM900.write(incoming_char);
    }
}

// FIXME: Must implement a larger state machine
void setupGPRS() {
  String result = "";
  SIM900.write(SHUT_IP_CONNECTION);
  while (true) {
    if (readSIM900UART(false) > 0) {
      result += incoming_char;
      if (incoming_char == '\n') {
        result.compareTo(IP_STATUS_SHUTDOWN) 
      }
      
    }
  }
}

void waitForNetwork() {
  String result = "";
  
  SIM900.print(CHECK_NETWORK);
  Serial.println("");
  while (true) {
    if (readSIM900UART(false) >= 0) {
      result += incoming_char;

      if (incoming_char == '\n') {        
        if (result.compareTo(NETWORK_CONNECTED) == 0) {
          Serial.println("Connected to network");
          return;
        } else if (result.compareTo(CHECK_NETWORK) == 0) { 
          result = "";
        } else if (result.compareTo("\r\n") == 0) {
          result = "";
        } else {
          result = "";
          delay(1000);
          SIM900.print(CHECK_NETWORK);
        }
      }
    }
  }
}

