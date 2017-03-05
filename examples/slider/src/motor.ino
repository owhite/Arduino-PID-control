#include <EEPROM.h>
#include <PID_v1.h>

#define SERIAL_SPEED  115200
#define S_START       1
#define S_RUNNING     2
#define S_REPORT      3
#define S_PLOT        4
#define S_STOP        6
#define S_RANDOM_START 7
#define S_RANDOM      8

char states[8][15] = {
  "DUMMY", 
  "S_START",
  "S_RUNNING",
  "S_REPORT",
  "S_PLOT",
  "S_STOP",
  "S_RANDOM_START",
  "S_RANDOM"
};

long previousMillis = 0;  
long currentMillis = 0;  
long interval = 1000; 

double kp=0.5;
double ki=0.0;
double kd=0.0;
double position=0, output=0, target=0;
PID myPID(&position, &output, &target, kp, ki, kd, DIRECT);

int LEDPin = 13;
int en_1  = 9; 
int in_1A = 10;
int in_1B = 11;
int sliderPin  = A0;
int inputPin  = A1;

int frequency = 1;
int state = S_STOP;
int prevState;
boolean usePotentiometer = true;

String inputString = "";         // a string to hold incoming data

void setup() {
  Serial.begin(SERIAL_SPEED);
  inputString.reserve(200);
  
  pinMode(en_1, OUTPUT);
  pinMode(in_1A, OUTPUT);
  pinMode(in_1B, OUTPUT);

  pinMode(LEDPin, OUTPUT);

  recoverPIDfromEEPROM();

  //Setup the pid 
  myPID.SetMode(AUTOMATIC);
  myPID.SetSampleTime(1);
  myPID.SetOutputLimits(-255,255);

  Serial.println("lets rock");

  setPWMFrequency(en_1, frequency);
}

void loop() {
  switch (state) {
  case S_START:
    myPID.SetTunings(kp,ki,kd);
    state = S_RUNNING;
    digitalWrite(LEDPin, HIGH);
    break;
  case S_RUNNING:
    if (usePotentiometer) {
      target = analogRead(inputPin);
    }
    else {
      // target comes from somewhere else
    }
    position = analogRead(sliderPin);

    target = constrain(target, 0, 1024);

    while(!myPID.Compute()); // wait to compute PID

    if(output < 0) {
      digitalWrite(in_1A, LOW);
      digitalWrite(in_1B, HIGH); 
      analogWrite(en_1,abs(output));
    }
    else {
      digitalWrite(in_1A, HIGH);
      digitalWrite(in_1B, LOW); 
      analogWrite(en_1,abs(output));
    }
    state = S_RUNNING;
    break;
  case S_RANDOM_START:
    myPID.SetTunings(kp,ki,kd);
    target = random(1024);
    target = constrain(target, 40, 950);
    Serial.println(target);
    digitalWrite(LEDPin, HIGH);
    previousMillis = millis();
    state = S_RANDOM;
    break;
  case S_RANDOM:
    position = analogRead(sliderPin);
    while(!myPID.Compute()); // wait to compute PID

    if(output < 0) {
      digitalWrite(in_1A, LOW);
      digitalWrite(in_1B, HIGH); 
      analogWrite(en_1,abs(output));
    }
    else {
      digitalWrite(in_1A, HIGH);
      digitalWrite(in_1B, LOW); 
      analogWrite(en_1,abs(output));
    }
    state = S_RANDOM;

    currentMillis = millis();
    if(currentMillis - previousMillis > interval) {
      state = S_RANDOM_START;
    }
    break;
  case S_REPORT:
    Serial.println("kP :: kI :: kD");
    Serial.print(kp);
    Serial.print(" :: ");
    Serial.print(ki);
    Serial.print(" :: ");
    Serial.println(kd);

    Serial.println("position :: target");
    Serial.print(analogRead(sliderPin));
    Serial.print(" :: ");
    Serial.println(target);

    Serial.println(states[prevState]);

    state = S_STOP;
    
    break;

  case S_STOP:
    // Turn off motor
    digitalWrite(LEDPin, LOW);
    digitalWrite(in_1A, LOW);
    digitalWrite(in_1B, LOW);  
    analogWrite(en_1, 0);
    state = S_STOP;
    break;
  default:
    Serial.println ("Unknown state") ;
    break;
  }

}

void handle_cmd() {
  inputString.trim(); // removes beginning and ending white spaces
  int idx = inputString.indexOf(' ');   
  String cmd = inputString.substring(0, idx); 
  String value = inputString.substring(idx + 1);

  if ((cmd.length() > 0)) {
    if (cmd.equals("probe_device")) {
      Serial.println("PID_device");
      state = S_STOP;
    }
    if (cmd.equals("p")) {
      Serial.println("pause");
      state = S_STOP;
    }
    if (cmd.equals("report")) {
      Serial.println("reporting");
      prevState = state;
      state = S_REPORT;
    }
    if (cmd.equals("random")) {
      Serial.println("random movement");
      usePotentiometer = false;
      state = S_RANDOM_START;
    }
    if (cmd.equals("W")) {
      Serial.println("W - write EEPROM:");
      state = S_STOP;
      writetoEEPROM(); 
    }
    if (cmd.equals("dump")) {
      Serial.println("dumping  EEPROM:");
      state = S_STOP;
      eedump(); 
    }
    if (cmd.equals("reset")) {
      Serial.println("get PIDs from EEPROM");
      state = S_STOP;
      recoverPIDfromEEPROM() ;
    }
    if (cmd.equals("plot")) {
      state = S_PLOT;
    }
    if (cmd.equals("R")) {
      Serial.println("run potentiometer control");
      usePotentiometer = true;
      state = S_START;
    }
    if (cmd.equals("F")) {
      frequency = value.toInt();
      setPWMFrequency(en_1, frequency);
      Serial.println("frequencey");
      state = S_STOP;
    }
    if (cmd.equals("P")) {
      Serial.println("set P");
      kp = value.toFloat();
      state = S_STOP;
    }
    if (cmd.equals("I")) {
      Serial.println("set I");
      ki = value.toFloat();
      state = S_STOP;
    }
    if (cmd.equals("D")) {
      Serial.println("set D");
      kd = value.toFloat();
      state = S_STOP;
    }
    if (cmd.equals("target")) {
      Serial.print("target: ");
      target = value.toInt();
      Serial.println(target);
      usePotentiometer = false;
      state = S_START;
    }
    if (cmd.equals("B") || cmd.equals("b")) {
      target = 1024 - target;
      Serial.print("bounce: ");
      Serial.println(target);
      usePotentiometer = false;
      state = S_START;
    }
    if (cmd.equals("?")) {
      Serial.println("help:");
      state = S_STOP;
    }
    inputString = "";
  }
}

void serialEvent() {
  while (Serial.available()) {
    char inChar = (char)Serial.read();
    inputString += inChar;
    if (inChar == '\n') {
      handle_cmd();
    }
  }
}

void setPWMFrequency(int pin, int divisor) {
  byte mode;
  if(pin == 5 || pin == 6 || pin == 9 || pin == 10) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 64: mode = 0x03; break;
      case 256: mode = 0x04; break;
      case 1024: mode = 0x05; break;
      default: return;
    }
    if(pin == 5 || pin == 6) {
      TCCR0B = TCCR0B & 0b11111000 | mode;
    } else {
      TCCR1B = TCCR1B & 0b11111000 | mode;
    }
  } else if(pin == 3 || pin == 11) {
    switch(divisor) {
      case 1: mode = 0x01; break;
      case 8: mode = 0x02; break;
      case 32: mode = 0x03; break;
      case 64: mode = 0x04; break;
      case 128: mode = 0x05; break;
      case 256: mode = 0x06; break;
      case 1024: mode = 0x07; break;
      default: return;
    }
    TCCR2B = TCCR2B & 0b11111000 | mode;
  }
}

// keep PID set values in EEPROM so they are kept when arduino goes off
void writetoEEPROM() { 
  eeput(kp,0);
  eeput(ki,4);
  eeput(kd,8);
  double cks=0;
  for(int i=0; i<12; i++) cks+=EEPROM.read(i);
  eeput(cks,12);
  Serial.println("\nPID values stored to EEPROM");
}

void recoverPIDfromEEPROM() {
  double cks=0;
  double cksEE;
  for(int i=0; i<12; i++) cks+=EEPROM.read(i);
  cksEE=eeget(12);
  if(cks==cksEE) {
    kp=eeget(0);
    ki=eeget(4);
    kd=eeget(8);
    myPID.SetTunings(kp,ki,kd); 
  }
  else Serial.println(F("*** Bad checksum"));
}

void eeput(double value, int dir) { 
  char * addr = (char * ) &value;
  for(int i=dir; i<dir+4; i++)  EEPROM.write(i,addr[i-dir]);
}

double eeget(int dir) { 
  double value;
  char * addr = (char * ) &value;
  for(int i=dir; i<dir+4; i++) addr[i-dir]=EEPROM.read(i);
  return value;
}

void eedump() {
  for(int i=0; i<16; i++) {
    Serial.print(EEPROM.read(i),HEX); Serial.print(" ");
  }
  Serial.println(); 
}
