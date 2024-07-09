#define L298N_IN1   4
#define L298N_IN2   0
#define L298N_IN3   2
#define L298N_IN4   15

#define L298N_ENA   14
#define L298N_ENB   12

bool    bLED_state = false;
int     sensorValue = 0;
float   voltage = 0;
float   weight = 0;

void setup() {
    pinMode(LED_BUILTIN, OUTPUT);       // Initialize the LED_BUILTIN pin as an output

    pinMode(L298N_IN1, OUTPUT);         // Initialize the L298N Input pins as an output
    pinMode(L298N_IN2, OUTPUT);     
    pinMode(L298N_IN3, OUTPUT);     
    pinMode(L298N_IN4, OUTPUT);     
    pinMode(L298N_ENA, OUTPUT);    
    pinMode(L298N_ENB, OUTPUT);     

    digitalWrite(L298N_IN1, LOW);       // Set the L298N Input pins to LOW
    digitalWrite(L298N_IN2, LOW);      
    digitalWrite(L298N_IN3, LOW);      
    digitalWrite(L298N_IN4, LOW);      
 //   digitalWrite(L298N_ENA, LOW);      
 //   digitalWrite(L298N_ENB, LOW);       
   
    Serial.begin(115200);
      while (!Serial){
        ; // wait for serial port to connect
      }
    Serial.println("L298N Module Demo Code - 1");
}

// the loop function runs over and over again forever
void loop() {

    // Setup Wheel speed by PWM
    sensorValue = analogRead(A0); // read the analog pin A0 which is defined at pins_arduino.h
  // Convert the analog reading value (which goes from 0 - 1023) to voltage (0 - 5V)
  voltage = sensorValue * (5.0 / 1023.0);
  // Convert the analog reading value (which goes from 0 - 1023) to PWM weight (0 - 255)
    weight = sensorValue * (255.0 / 1023.0);
    // print out the value you read:
  Serial.print("A0 Volt: ");
  Serial.println(voltage);
    Serial.print("PWM weight: ");
  Serial.println(weight);
  
    analogWrite(L298N_ENA, (int)weight);
    analogWrite(L298N_ENB, (int)weight);

    if (Serial.available() > 0) {
    char incomingByte = 0;

      // read the incoming byte:
      incomingByte = Serial.read();

      // Print what you got:
    Serial.print("I received: ");
      Serial.println(incomingByte);

      if ( incomingByte == 'F' || incomingByte == 'f' )
            {
                // Go forward
                digitalWrite(L298N_IN1, HIGH);       
                digitalWrite(L298N_IN2, LOW);      
                digitalWrite(L298N_IN3, HIGH);      
                digitalWrite(L298N_IN4, LOW);            
            }
      else if ( incomingByte == 'R' || incomingByte == 'r' )
            {
                // Reverse, Go Back
                digitalWrite(L298N_IN1, LOW);       
                digitalWrite(L298N_IN2, HIGH);      
                digitalWrite(L298N_IN3, LOW);      
                digitalWrite(L298N_IN4, HIGH);            
            }
      else if ( incomingByte == 'H' || incomingByte == 'h' )
            {
                // Stop
                digitalWrite(L298N_IN1, LOW);       
                digitalWrite(L298N_IN2, LOW);      
                digitalWrite(L298N_IN3, LOW);      
                digitalWrite(L298N_IN4, LOW);            
            }
  } 
  
    // Use LED as alive indication
    if ( !bLED_state )
    {
        bLED_state = true;
        digitalWrite(LED_BUILTIN, HIGH);    // Turn the LED on 
    }
    else
    {
         bLED_state = false;
        digitalWrite(LED_BUILTIN, LOW);     // Turn the LED off 
    }
    delay(1000);                            // Wait for two seconds
}
