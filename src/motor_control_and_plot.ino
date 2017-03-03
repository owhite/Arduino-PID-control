#include <EEPROM.h>
#include <Encoder.h>
#include <PID_v1.h>
#include <RunningAverage.h>

// STATES
#define S_MOTOR_INIT      1
#define S_MOTOR_RUN       2
#define S_MOTOR_OFF       3
#define S_OPTOTEST        4
#define S_REPORT          5
#define S_PLOT            6
#define S_FORWARD         7

char states[8][15] = {
  "DUMMY", 
  "S_MOTOR_INIT", 
  "S_MOTOR_RUN", 
  "S_MOTOR_OFF", 
  "S_OPTOTEST", 
  "S_REPORT", 
  "S_PLOT", 
  "S_FORWARD"
};

#define SERIAL_SPEED 115200

// PINS
#define ENC_A      14
#define ENC_B      15

#define IN_A       11
#define IN_B       9
#define PWM_PIN    10

String inputString = "";         // a string to hold incoming data

#define maxCount 500
int start = 0;
int positionCount = 0;
int positionArray[maxCount];
int otherArray[maxCount];

double kp=0.5;
double ki=0.0;
double kd=0.0;
double position=0, output=0, target=0;
PID myPID(&position, &output, &target, kp, ki, kd, DIRECT);

boolean counting=false;

int state = S_MOTOR_OFF;
int prevState = state;

int reporttoggle = 0;

bool dir = false;
byte skip=0;

Encoder encoder(ENC_A, ENC_B);

// number of samples in rolling buffer
int RAsamples = 100;     
RunningAverage RA1(RAsamples);

void setup() { 
  inputString.reserve(200);

  pinMode(IN_A, OUTPUT);
  pinMode(IN_B, OUTPUT);
  pinMode(PWM_PIN, OUTPUT);

  Serial.begin (SERIAL_SPEED);

  recoverPIDfromEEPROM();

  //Setup the pid 
  myPID.SetMode(AUTOMATIC);
  myPID.SetSampleTime(1);
  myPID.SetOutputLimits(-255,255);
} 

void loop(){
  switch (state) {
  case S_REPORT:
    position = encoder.read();
    printPos();
    state = S_MOTOR_OFF;
    break;
  case S_PLOT:
    plot_data();
    state = S_MOTOR_OFF;
    break;
  case S_FORWARD:
      digitalWrite(IN_A, LOW);
      digitalWrite(IN_B, HIGH);
      analogWrite(PWM_PIN,200);
    break;
  case S_MOTOR_INIT:
    // clear array
    for(int i=0; i<maxCount; i++) positionArray[i]=0; 
    for(int i=0; i<maxCount; i++) otherArray[i]=0; 
    myPID.SetTunings(kp,ki,kd);
    start = encoder.read();
    positionCount = 0;
    state = S_MOTOR_RUN;
    break;
  case S_MOTOR_RUN:
    position = encoder.read();

    RA1.addValue(position);

    // wait till PID is actually computed
    while(!myPID.Compute()); 

    if(output < 0) {
      digitalWrite(IN_A, HIGH);
      digitalWrite(IN_B, LOW);
      analogWrite(PWM_PIN,abs(output));
    }
    else {
      digitalWrite(IN_A, LOW);
      digitalWrite(IN_B, HIGH);
      analogWrite(PWM_PIN,abs(output));
    }

    // loading this array to display for graphing
    if(positionCount < maxCount) {
      positionArray[positionCount]=position;
      otherArray[positionCount]=positionCount;
    }
    else {
      state = S_MOTOR_OFF;
      break;
    }
    positionCount++;

    state = S_MOTOR_RUN;
    break;
  case S_MOTOR_OFF:
    digitalWrite(IN_A, LOW);
    digitalWrite(IN_B, LOW);
    digitalWrite(PWM_PIN, LOW);
    state = S_MOTOR_OFF;
    break;
  default:
    Serial.println("unknown state");
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
      state = S_MOTOR_OFF;
    }
    if (cmd.equals("p")) {
      Serial.println("pause");
      state = S_MOTOR_OFF;
    }
    if (cmd.equals("zero")) {
      Serial.println("zero encoder");
      encoder.write(0);
      state = S_MOTOR_OFF;
    }
    if (cmd.equals("report")) {
      Serial.println("reporting");
      prevState = state;
      state = S_REPORT;
    }
    if (cmd.equals("F")) {
      Serial.println("forward");
      state = S_FORWARD;
    }
    if (cmd.equals("W")) {
      Serial.println("W - write EEPROM:");
      state = S_MOTOR_OFF;
      writetoEEPROM(); 
    }
    if (cmd.equals("dump")) {
      Serial.println("dumping  EEPROM:");
      state = S_MOTOR_OFF;
      eedump(); 
    }
    if (cmd.equals("reset")) {
      Serial.println("get PIDs from EEPROM");
      state = S_MOTOR_OFF;
      recoverPIDfromEEPROM() ;
    }
    if (cmd.equals("plot")) {
      state = S_PLOT;
    }
    if (cmd.equals("P")) {
      Serial.println("set P");
      kp = value.toFloat();
      state = S_MOTOR_OFF;
    }
    if (cmd.equals("I")) {
      Serial.println("set I");
      ki = value.toFloat();
      state = S_MOTOR_OFF;
    }
    if (cmd.equals("D")) {
      Serial.println("set D");
      kd = value.toFloat();
      state = S_MOTOR_OFF;
    }
    if (cmd.equals("target")) {
      Serial.print("target: ");
      target = value.toInt();
      Serial.println(target);
      state = S_MOTOR_INIT;
    }
    if (cmd.equals("zt")) {
      Serial.print("zero then target: ");
      target = value.toInt();
      Serial.println(target);
      encoder.write(0);
      state = S_MOTOR_INIT;
    }
    if (cmd.equals("?")) {
      Serial.println("help:");
      state = S_MOTOR_OFF;
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

void plot_data() {
  Serial.println("plot");
  Serial.print("positions ");
  for(int i=0; i < maxCount - 1; i++) {
    Serial.print(positionArray[i]);
    Serial.print(" ");
  }
  Serial.println(positionArray[maxCount - 1]);

  Serial.print("times ");
  for(int i=0; i < maxCount - 1; i++) {
    Serial.print(otherArray[i]);
    Serial.print(" ");
  }
  Serial.println(otherArray[maxCount - 1]);

  position = encoder.read();
  Serial.print("position ");
  Serial.println(position);
  
  Serial.print("target ");
  Serial.println(target);

  Serial.print("start ");
  Serial.println(start);

  Serial.print("kP ");
  Serial.println(kp);

  Serial.print("kI ");
  Serial.println(ki);

  Serial.print("kD ");
  Serial.println(kd);
}

void printPos() {
  Serial.print(F("Position="));
  Serial.print(position);
  Serial.print(F(" PID_output="));
  Serial.print(output);
  Serial.print(F(" Target="));
  Serial.println(target);
  Serial.print(F("P="));
  Serial.print(kp);
  Serial.print(F(" I="));
  Serial.print(ki);
  Serial.print(F(" D="));
  Serial.println(kd);
  Serial.print(F(" state="));
  Serial.println(states[prevState]);
}

void help() {
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
  //Serial.println(cks);
}

void recoverPIDfromEEPROM() {
  double cks=0;
  double cksEE;
  for(int i=0; i<12; i++) cks+=EEPROM.read(i);
  cksEE=eeget(12);
  //Serial.println(cks);
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

