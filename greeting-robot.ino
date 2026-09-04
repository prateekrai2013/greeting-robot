#include <SoftwareSerial.h>
#include <DFRobotDFPlayerMini.h>
#include <Servo.h>

// ======================================================
//                 GREETING ROBOT
// ======================================================

// ---------------- PIN CONNECTIONS ----------------

// Ultrasonic Sensor HC-SR04
const int TRIG_PIN = 7;
const int ECHO_PIN = 8;

// Servo Motors
const int LEFT_SERVO_PIN = 5;
const int RIGHT_SERVO_PIN = 6;

// Optional Buzzer
const int BUZZER_PIN = 4;

// DFPlayer Mini
const int DFPLAYER_RX_PIN = 10; // Arduino receives from DFPlayer TX
const int DFPLAYER_TX_PIN = 11; // Arduino sends to DFPlayer RX


// ---------------- DETECTION SETTINGS ----------------

// Maximum distance for detection
const int DETECTION_DISTANCE = 150; // centimeters

// Person must be detected for this long
const unsigned long DETECT_CONFIRM_TIME = 700;

// Person must be absent for this long
const unsigned long LEAVE_CONFIRM_TIME = 2000;


// ---------------- SERVO POSITIONS ----------------

// Hands down / resting position
const int LEFT_HAND_DOWN = 20;
const int RIGHT_HAND_DOWN = 160;

// Hands joined / Namaste position
const int LEFT_HAND_JOINED = 110;
const int RIGHT_HAND_JOINED = 70;


// ---------------- OBJECTS ----------------

Servo leftServo;
Servo rightServo;

SoftwareSerial dfSerial(DFPLAYER_RX_PIN, DFPLAYER_TX_PIN);
DFRobotDFPlayerMini dfPlayer;


// ---------------- ROBOT STATE ----------------

bool personPresent = false;

unsigned long detectedStartTime = 0;
unsigned long absentStartTime = 0;

bool detectionTimerRunning = false;
bool absenceTimerRunning = false;


// ======================================================
//                      SETUP
// ======================================================

void setup() {

  // Serial Monitor
  Serial.begin(9600);

  // Ultrasonic Sensor
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  // Buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);

  // Servos
  leftServo.attach(LEFT_SERVO_PIN);
  rightServo.attach(RIGHT_SERVO_PIN);

  // Start with hands down
  moveHandsDown();

  // DFPlayer
  dfSerial.begin(9600);

  Serial.println("Starting Greeting Robot...");

  if (!dfPlayer.begin(dfSerial)) {

    Serial.println("DFPlayer not detected!");

  } else {

    Serial.println("DFPlayer ready.");

    // Volume: 0 to 30
    dfPlayer.volume(25);
  }

  delay(1000);
}


// ======================================================
//                       LOOP
// ======================================================

void loop() {

  int distance = getDistance();

  // If valid distance and person is within range
  bool objectDetected =
      (distance > 0 && distance <= DETECTION_DISTANCE);


  // ==================================================
  //          PERSON DETECTED
  // ==================================================

  if (objectDetected) {

    // Reset absence timer
    absenceTimerRunning = false;


    // Person was previously absent
    if (!personPresent) {

      // Start detection confirmation timer
      if (!detectionTimerRunning) {

        detectedStartTime = millis();
        detectionTimerRunning = true;
      }


      // Confirm person is actually present
      if (millis() - detectedStartTime >= DETECT_CONFIRM_TIME) {

        personPresent = true;
        detectionTimerRunning = false;

        welcomePerson();
      }
    }
  }


  // ==================================================
  //          PERSON NOT DETECTED
  // ==================================================

  else {

    // Reset detection timer
    detectionTimerRunning = false;


    // Person was previously present
    if (personPresent) {

      // Start absence confirmation timer
      if (!absenceTimerRunning) {

        absentStartTime = millis();
        absenceTimerRunning = true;
      }


      // Confirm that person has actually left
      if (millis() - absentStartTime >= LEAVE_CONFIRM_TIME) {

        personPresent = false;
        absenceTimerRunning = false;

        goodbyePerson();
      }
    }
  }

  delay(50);
}


// ======================================================
//              ULTRASONIC DISTANCE
// ======================================================

int getDistance() {

  // Clear trigger
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);

  // Send ultrasonic pulse
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);

  digitalWrite(TRIG_PIN, LOW);


  // Read echo
  long duration = pulseIn(ECHO_PIN, HIGH, 30000);


  // If no echo received
  if (duration == 0) {
    return -1;
  }


  // Calculate distance in cm
  int distance = duration * 0.034 / 2;

  return distance;
}


// ======================================================
//                 WELCOME FUNCTION
// ======================================================

void welcomePerson() {

  Serial.println("Person detected - Welcome!");

  // Play 0001.mp3
  dfPlayer.playMp3Folder(1);

  // Move hands into Namaste position
  moveHandsJoined();

  // Buzzer greeting
  beepGreeting();
}


// ======================================================
//                 GOODBYE FUNCTION
// ======================================================

void goodbyePerson() {

  Serial.println("Person left - Goodbye!");

  // Play 0002.mp3
  dfPlayer.playMp3Folder(2);

  // Move hands down
  moveHandsDown();

  // Make sure buzzer is OFF
  digitalWrite(BUZZER_PIN, LOW);
}


// ======================================================
//             MOVE HANDS TOGETHER
// ======================================================

void moveHandsJoined() {

  // Smooth movement
  for (int i = 0; i <= 90; i++) {

    int leftPosition =
        map(i, 0, 90,
            LEFT_HAND_DOWN,
            LEFT_HAND_JOINED);

    int rightPosition =
        map(i, 0, 90,
            RIGHT_HAND_DOWN,
            RIGHT_HAND_JOINED);

    leftServo.write(leftPosition);
    rightServo.write(rightPosition);

    delay(15);
  }
}


// ======================================================
//              MOVE HANDS DOWN
// ======================================================

void moveHandsDown() {

  // Smooth movement
  for (int i = 0; i <= 90; i++) {

    int leftPosition =
        map(i, 0, 90,
            LEFT_HAND_JOINED,
            LEFT_HAND_DOWN);

    int rightPosition =
        map(i, 0, 90,
            RIGHT_HAND_JOINED,
            RIGHT_HAND_DOWN);

    leftServo.write(leftPosition);
    rightServo.write(rightPosition);

    delay(15);
  }
}


// ======================================================
//                 BUZZER GREETING
// ======================================================

void beepGreeting() {

  // 3 short beeps
  for (int i = 0; i < 3; i++) {

    digitalWrite(BUZZER_PIN, HIGH);
    delay(120);

    digitalWrite(BUZZER_PIN, LOW);
    delay(120);
  }
}
