#include <SoftwareSerial.h>
SoftwareSerial bt(11, 10);


// A4988
int stepsPin = 12;
int dirPin = 13;
int loopTimes = 1;
int Stop = false;
char ACTION = 0;



// SLIDER CONFIGURATION DEFAULT VARS
int limitFrequency = 200;
int currentFrequency = 200;
float currentDistance = 0;
int isFrontwardActived = 0;
int isBackwardActived = 0;
int isLimitedFrequency = false;
int motorTimeInterval = 1;
int totalStepsCount = 0;
int limitSteps = 20;
int isDisplayScrolling = 0;
int stopMotor = 0;
int isMotorActive = 0;
String ReasonStoped = "Ciclo";
int motorDirection = 0; // 0=none, 1=back, 2=front


void setup() {

  Serial.begin(9600);



  /** BT setup **/
  bt.begin(38400); // Comunicación serie entre Arduino y el modulo a 38400 bps

  Serial.print("Iniciando...");
  /** End setup **/

  /** Motor setup **/
  pinMode(stepsPin, OUTPUT);
  pinMode(dirPin, OUTPUT);
  /** End Motor **/

}


void loop() {
//  Serial.println("Hola");


  if (totalStepsCount >= limitSteps - 2)
    stopMotor = 1;

  if (isBackwardActived && !isFrontwardActived && totalStepsCount < limitSteps) {
    isDisplayScrolling = 0;
    totalStepsCount = totalStepsCount + 1;

    digitalWrite(dirPin, LOW);
    rotate();
  }



  if (isFrontwardActived && !isBackwardActived && totalStepsCount < limitSteps) {

    digitalWrite(dirPin, HIGH);
    rotate();
  }


  // Bluetooth Available Clients
  if (bt.available()) {
    Serial.print("Receive Data");
    ACTION = bt.read();

    switch (ACTION) {
      case '1': // EXEC BACKWARD
        motorDirection = 1;
        stopMotor = 0;
        digitalWrite(dirPin, LOW);
        rotate();
        setDisplayText("BACK STEP", "Freq", String(currentFrequency) + " / " + String(motorTimeInterval));
        break;
      case '2': // EXEC FRONTWARD
        motorDirection = 2;
        stopMotor = 0;
        digitalWrite(dirPin, HIGH);
        setDisplayText("FRONT STEP", "Freq", String(currentFrequency) + " / " + String(motorTimeInterval));
        rotate();
        break;
      case '3': // LOOP BACKWARD
        motorDirection = 1;
        stopMotor = 0;
        totalStepsCount = 0;
        isFrontwardActived = 0;
        isBackwardActived = 1;
        isMotorActive = 1;
        break;
      case '4': // LOOP FRONTWARD
        motorDirection = 2;
        stopMotor = 0;
        totalStepsCount = 0;
        isFrontwardActived = 1;
        isBackwardActived = 0;
        isMotorActive = 1;
        break;
      case '5': // STOP
        motorDirection = 0;
        totalStepsCount = 0;
        stopMotor = 1;
        isFrontwardActived = 0;
        isBackwardActived = 0;
        setDisplayText("LOOP STOP", ".", ".");
        break;
      case '6': // ADD frequency +10
        addFrequency(10);
        setDisplayText("ADD FREQ", "Done! V", String(currentFrequency));


        btReport();
        break;
      case '7':  // substract frequency -10
        substractFrequency(10);
        setDisplayText("MIN FREQ", "Done! V", String(currentFrequency));

        btReport();
        break;
      case '8':  // Add Interval +1
        addMotorTimeInterval(1);
        setDisplayText("ADD TIME", "Done! V", String(motorTimeInterval));

        btReport();
        break;
      case '9':  // substract interval -1
        substractMotorTimeInterval(1);
        setDisplayText("MIN TIME", "Done! V", String(motorTimeInterval));


        btReport();
        break;
      case 'A':

        break;
      case 'C': // Show all conditions
        btReport();
        break;
      case 'D':
        reset();
        break;
    }
  }




  /**
    STOP MOTOR
  */
  if (stopMotor && isMotorActive) {
    isFrontwardActived = 0;
    isBackwardActived = 0;
    stopMotor = 0;
    isMotorActive = 0;
  }

}


void setDisplayText(String title, String subtitle, String value) {
  Serial.print(title + " - " + subtitle + " - " + value);
}

bool addMotorTimeInterval(int value) {
  bool response;

  if (motorTimeInterval >= 1 && motorTimeInterval < 1000) {
    setMotorTimeInterval(motorTimeInterval + value);
    response =  true;
  } else {
    response = false;
  }

  return response;
}

bool substractMotorTimeInterval(int value) {
  bool response;
  if (motorTimeInterval > 1 && motorTimeInterval < 1000) {
    setMotorTimeInterval(motorTimeInterval - value);
    response =  true;
  } else {
    response = false;
  }

  return response;
}

// Setting global variable
void setMotorTimeInterval(int value) {
  isMotorActive = 0;
  motorTimeInterval = value;
}



/** Set Frequency **/
bool addFrequency(int interval) {
  bool response;

  if (currentFrequency < limitFrequency && currentFrequency >= 10) {
    setFrequency(currentFrequency + interval);
    response =  true;
  } else if (currentFrequency >= 0 && currentFrequency <= 10) {
    setFrequency(currentFrequency + 1);
    response =  true;
  } else {
    isLimitedFrequency = true;
    response = false;
  }

  return response;
}

bool substractFrequency(int interval) {
  bool response;

  if (currentFrequency <= limitFrequency && currentFrequency > 10) {
    setFrequency(currentFrequency - interval);
    response =  true;
  } else if (currentFrequency > 0 && currentFrequency <= 10) {
    setFrequency(currentFrequency - 1);
    response =  true;
  } else {
    isLimitedFrequency = true;
    response = false;
  }

  return response;
}

// Setting global variable
void setFrequency(int freq) {
  stopMotor = 0;
  currentFrequency = freq;
}
/** End Set Frequency**/

/** Rotate motor**/
void rotate() {
  totalStepsCount = totalStepsCount + 1;
  for (int i = 0; i < currentFrequency; i++)     //Equivale al numero de vueltas (200 es 360º grados) o micropasos
  {
    Serial.print("..\n");
    digitalWrite(stepsPin, HIGH);  // This LOW to HIGH change is what creates the
    delay(motorTimeInterval);
    digitalWrite(stepsPin, LOW); // al A4988 de avanzar una vez por cada pulso de energia.
    delay(motorTimeInterval);
  }
}
/** End Rotate motor**/

void btReport() {
  bt.print("Freq: " + String(currentFrequency) + "\n");
  bt.print("Inverval: " + String(motorTimeInterval) + "\n");
  bt.print("Steps: " + String(totalStepsCount) + "\n");
}

void reset() {

  currentFrequency = 200;
  motorTimeInterval = 1;
  totalStepsCount = 0;
  btReport();
}






void backMotor() {

}
