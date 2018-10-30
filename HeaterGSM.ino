#include <SoftwareSerial.h>

SoftwareSerial SIM900(7, 8);

char incoming_char = 0;

void setup() {
  SIM900.begin(19200);
  Serial.begin(19200);
  Serial.println("Arduino booted, waiting for hello from SIM900");

  delay(1000);

  SIM900.println("AT");
  
  //SIM900.println("ATDT0700838506");
}

void loop() {
    if (SIM900.available()) {
      //Get the character from the cellular serial port
      incoming_char = SIM900.read(); 
      //Print the incoming character to the terminal
      Serial.print(incoming_char); 
    }

    if (Serial.available()) {
      incoming_char = Serial.read();
      SIM900.write(incoming_char);
    }
}
