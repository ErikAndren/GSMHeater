#include <SoftwareSerial.h>

SoftwareSerial SIM900(7, 8);

char incoming_char = 0;

const String CHECK_NETWORK = "AT+CREG?\r\n";
const String NETWORK_CONNECTED = "+CREG: 0,1\r\n";

void setup() {
  SIM900.begin(19200);
  Serial.begin(19200);

  delay(1000);
  
  waitForNetwork();
}

int readSIM900UART() {
  if (SIM900.available()) {
      //Get the character from the cellular serial port
      incoming_char = SIM900.read(); 
      //Print the incoming character to the terminal
      //Serial.print(incoming_char);
      return incoming_char;
  }
  return -1;
}

void writeSIM900UART(byte val) { 
}

void loop() {
  readSIM900UART();
  
  if (Serial.available()) {
      incoming_char = Serial.read();
      SIM900.write(incoming_char);
    }
}

void waitForNetwork() {
  String result = "";
  
  SIM900.print(CHECK_NETWORK);
  Serial.println("");
  while (true) {
    if (readSIM900UART() >= 0) {
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
          return;
          result = "";
          delay(1000);
          SIM900.println(CHECK_NETWORK);
        }
      }
    }
  }
}

