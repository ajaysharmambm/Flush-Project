int coinpin = 2; // Pin receiving total number coins through interrupt
int flushpin = 3; // Pin used to receive interrupts from water flow sensor
int flushPinEnable = 4;
int gatepin = 9;  // Pin used for Gate Relay
int relaypin = 13;  // Pin used for LED Relay
int trigpin = 11; // Pin used for Ultrasonic Sensor as Trigger
int echopin = 12; // Pin used for Ultrasonic Sensor as Echo
int ON = HIGH;
int OFF = LOW;

static int coins = 0; // Counting total Number of coins
static int counter = 0; // number of pulses given by water flow sensor

// These all four are used to provide delay to turn OFF the Gate Relay
static unsigned long currentTime = 0;
static int previousTime = 0;
static boolean toWait = false;
int gateOffInterval = 10; //  waiting time to switch Gate Relay Off after Switching OFF the LED Relay


void setup() {
  pinMode(coinpin, INPUT_PULLUP); //Internal pullup resistor used to implement pushbutton
  pinMode(flushpin, INPUT); // Receiving counts form water flow sensor
  pinMode(flushPinEnable, OUTPUT); // Enable or disable to recieving the interrupt on flushpin
  pinMode(gatepin, OUTPUT); // controlling gate relay
  pinMode(relaypin, OUTPUT);  // Controlling LED relay
  pinMode(trigpin, OUTPUT);
  pinMode(echopin, INPUT);
  attachInterrupt(digitalPinToInterrupt(coinpin), switchOnGateRelay, FALLING);
  attachInterrupt(digitalPinToInterrupt(flushpin), flushAll, HIGH);
  Serial.begin(9600);
}

void loop() {

  currentTime = abs(millis()) / 1000;
  static int prevDistance = 0;

  if (digitalRead(relaypin) == HIGH) {
    int distance = ultrasonic(trigpin, echopin);  //Distance measured by ultrasonic sensor for user
    if (distance != 0) {
      Serial.println("Difference in current and previous Positions : " + String(distance - prevDistance));
      if ((distance - prevDistance) > 50 && (prevDistance != 0)) { // Switching ON the flush when user moves backwards with minimum distance of 50 cm
        switchFlush(ON);
      }
      prevDistance = distance;
    }
  }

  // Switch OFF the Gate Relay after 30 Seconds, here :
  // currentTime : is measured in loop function
  //  previosTime : is assigned value in switchRelayPin function
  //  toWait : is a boolean variable used to define wheter we have to wait to switch the gate OFF or not
  if ((currentTime - previousTime) == gateOffInterval && toWait == true) {
    switchOffGateRelay();
  }
  delay(500);
}

/**
   Switch ON the Gate relay when number of coins is 3
*/
void switchOnGateRelay() {
  coins++;
  Serial.println("Coins : " + String(coins));
  if (coins == 3) {
    if (digitalRead(gatepin) == LOW) {
      digitalWrite(gatepin, HIGH);
      Serial.println("Gate Relay : ON");
      switchRelayPin(ON);
    }
  }
}

/**
   Switch OFF the Gate Relay
*/
void switchOffGateRelay() {
  if (digitalRead(gatepin) == HIGH) {
    digitalWrite(gatepin, LOW);
    Serial.println("Gate Relay : OFF");

    //  reset coins and counter after switching OFF the gate relay
    counter = 0;
    coins = 0;
  }
}

/**
   Switch the LED Relay ON or OFF
   @param value = ON or OFF
*/
void switchRelayPin(int value) {
  if (value == ON) {
    if (digitalRead(relaypin) == LOW) {
      digitalWrite(relaypin, HIGH);
      Serial.println("Relay Pin : ON");
    }
  } else {
    if (digitalRead(relaypin) == HIGH) {
      digitalWrite(relaypin, LOW);
      Serial.println("Relay Pin : OFF");

      //    registering the timing when we demanded to turn OFF the Gate Relay. It will be used in loop() function to switch off the Gate Relay
      previousTime = currentTime;
      toWait = true;
      Serial.println(previousTime);
      Serial.println(currentTime);
    }
  }
}

/**
   Switch the Flush pin ON or OFF
   @param value = ON or OFF
*/
void switchFlush(int value) {
  if (value == ON) {
    if (digitalRead(flushPinEnable) == LOW) {
      digitalWrite(flushPinEnable, HIGH);
      Serial.println("Flush Pin Enable: ON");
    }
  } else {
    if (digitalRead(flushPinEnable) == HIGH) {
      digitalWrite(flushPinEnable, LOW);
      Serial.println("Flush Pin Enable: OFF");
      switchRelayPin(OFF);
    }
  }
}

/**
   Start the flow of Water
*/
void flushAll() {
  if (digitalRead(flushPinEnable) == HIGH) {
    counter++;
    if (counter <= 50) {
      if (counter == 1) {
        Serial.println("Flush Started");
      }
      Serial.println("No of pulses : " + String(counter));
      if (counter == 50) {
        Serial.println("Flush Complete");
        switchFlush(OFF);
      }
    }
  }
}

/** returns distance from obstacle measured by Ultrasonic Sensor
   @param trig : Trigger Pin
   @param echo : Echo Pin
   returns -
   0: if any ambiguity of data is there(make an condition not to accept zero distance)
   distance : if distance is accurate
*/
int ultrasonic(int trig, int echo) {
  int duration;
  int distance;
  static int lastdistance = 0;

  digitalWrite(trig, LOW);
  delayMicroseconds(2);
  digitalWrite(trig, HIGH);
  delayMicroseconds(10);  // trigger pulse duration = 10 microseconds

  duration = pulseIn(echo, HIGH); // echo received in microseconds
  distance = duration * 0.034 / 2; // Distance returned in cm, velocity of sound  = 340 m/s

  /**Controlling output distance
     1. removing negative values of distance are returned by sensor( comes when distance with obstacle is <2cm or >4m)
     2. Removing large fluctuations by delaying the measured output
  */
  if (abs(lastdistance - distance) > 50 || distance < 0) { // Removing large fluctuations by delaying the measured output
    delay(200);
    if (distance > lastdistance) {
      lastdistance = lastdistance + 50;
    } else {
      lastdistance = lastdistance - 50;
    }
    distance = 0;
  } else {
    lastdistance = distance;
  }
  return distance;
}

