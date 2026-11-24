/*
 * Autonomous Vacuum Robot
 * -----------------------
 * A 4-wheel Arduino cleaning robot with two modes:
 *
 *   1. Bluetooth mode (default): drive it from a phone with the Dabble app (Gamepad).
 *   2. Automatic mode: it drives on its own, avoids obstacles with an ultrasonic
 *      sensor on a servo, uses IR sensors on the sides to steer away from walls,
 *      and runs a cleaning routine when its IR sensor (A4) is triggered.
 *
 * Press START on the Dabble Gamepad to switch between the two modes.
 *
 * Bluetooth controls:
 *   Up / Down     -> forward / backward
 *   Left / Right  -> turn
 *   Triangle      -> vacuum ON
 *   Circle        -> vacuum OFF
 *
 * Author: Ali Imran
 */

// Only load the Gamepad module from the Dabble library
#define CUSTOM_SETTINGS
#define INCLUDE_GAMEPAD_MODULE
#include <AFMotor.h>
#include <Dabble.h>
#include <Servo.h>

// ---------------- Pins ----------------
#define trigPin A1    // ultrasonic sensor trigger
#define echoPin A0    // ultrasonic sensor echo
#define IRsense A4    // IR sensor that starts the cleaning routine
#define IRRight A5    // right-side IR sensor
int IRLeft = 2;       // left-side IR sensor
int vacuum = 10;      // vacuum output (HIGH = on)

Servo myservo;        // servo that rotates the ultrasonic sensor (pin 9)

// ---------------- Mode toggle (START button) ----------------
int buttonState = HIGH;     // HIGH = Bluetooth mode, LOW = Automatic mode
int buttonCurrent;
int buttonPrevious = LOW;

// ---------------- Motors (motor shield ports M1-M4) ----------------
AF_DCMotor motor1(1, MOTOR12_64KHZ);
AF_DCMotor motor2(2, MOTOR12_64KHZ);
AF_DCMotor motor3(3, MOTOR34_64KHZ);
AF_DCMotor motor4(4, MOTOR34_64KHZ);

#define MAX_SPEED 150   // top speed when ramping up in moveBackward()

// ---------------- Obstacle avoidance ----------------
long dis;               // raw echo time from the ultrasonic sensor
boolean goesForward = false;
int distance = 100;     // latest distance reading (cm)
int speedSet = 0;
int distanceR = 0;      // distance when looking right (cm)
int distanceL = 0;      // distance when looking left (cm)
int set = 15;           // obstacle distance threshold (cm)

void setup()
{
  Dabble.begin(9600, 16, 17);   // Bluetooth module: RX pin 16 (A2), TX pin 17 (A3)
  Serial.begin(9600);
  Serial.println("Motor Test!");

  myservo.attach(9);
  myservo.write(90);            // point the ultrasonic sensor straight ahead

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(IRsense, INPUT);
  pinMode(IRLeft, INPUT);
  pinMode(IRRight, INPUT);
  pinMode(vacuum, OUTPUT);

  motor1.setSpeed(255);
  motor2.setSpeed(255);
  motor3.setSpeed(255);
  motor4.setSpeed(255);
}

void loop()
{
  Serial.println(digitalRead(IRsense));   // debug: print the IR sensor reading
  Dabble.processInput();

  // START button toggles between Bluetooth and Automatic mode
  buttonCurrent = GamePad.isStartPressed();

  if (buttonCurrent == HIGH && buttonPrevious == LOW)
  {
    if (buttonState == HIGH)
    {
      buttonState = LOW;
    }
    else
    {
      buttonState = HIGH;
    }
  }

  if (buttonState == HIGH)
  {
    bluetooth();
  }
  else if (buttonState == LOW)
  {
    automatic();
  }

  buttonPrevious = buttonCurrent;
}

// ---------------- Bluetooth mode ----------------
// Drive the robot and control the vacuum from the phone.
void bluetooth()
{
  motor1.setSpeed(200);
  motor2.setSpeed(200);
  motor3.setSpeed(200);
  motor4.setSpeed(200);
  Dabble.processInput();

  if (GamePad.isUpPressed())            // forward
  {
    motor1.run(FORWARD);
    motor2.run(FORWARD);
    motor3.run(FORWARD);
    motor4.run(FORWARD);
  }
  else if (GamePad.isDownPressed())     // backward
  {
    motor1.run(BACKWARD);
    motor2.run(BACKWARD);
    motor3.run(BACKWARD);
    motor4.run(BACKWARD);
  }
  else if (GamePad.isLeftPressed())     // turn left
  {
    motor1.run(FORWARD);
    motor2.run(FORWARD);
    motor3.run(BACKWARD);
    motor4.run(BACKWARD);
  }
  else if (GamePad.isRightPressed())    // turn right
  {
    motor1.run(BACKWARD);
    motor2.run(BACKWARD);
    motor3.run(FORWARD);
    motor4.run(FORWARD);
  }
  else if (GamePad.isTrianglePressed()) // vacuum on
  {
    digitalWrite(vacuum, HIGH);
  }
  else if (GamePad.isCirclePressed())   // vacuum off
  {
    digitalWrite(vacuum, LOW);
  }
  else                                  // no button: stop
  {
    motor1.run(RELEASE);
    motor2.run(RELEASE);
    motor3.run(RELEASE);
    motor4.run(RELEASE);
  }
}

// ---------------- Automatic mode ----------------
void automatic()
{
  motor1.setSpeed(140);
  motor2.setSpeed(140);
  motor3.setSpeed(140);
  motor4.setSpeed(140);

  Serial.println(digitalRead(IRsense));   // debug

  // IR sensor triggered: stop, turn the vacuum on, and clean the spot
  if (digitalRead(IRsense) == 0)
  {
    delay(100);
    digitalWrite(vacuum, HIGH);
    motor1.setSpeed(140);
    motor2.setSpeed(140);
    motor3.setSpeed(140);
    motor4.setSpeed(140);
    motor1.run(RELEASE);
    motor2.run(RELEASE);
    motor3.run(RELEASE);
    motor4.run(RELEASE);
    delay(2000);

    // Wiggle back and forth over the spot: 6 times, 1.15 s each direction
    for (int i = 0; i < 6; i++)
    {
      motor1.run(BACKWARD);   // same direction as the Right button
      motor2.run(BACKWARD);
      motor3.run(FORWARD);
      motor4.run(FORWARD);
      delay(1150);
      motor1.run(FORWARD);    // same direction as the Left button
      motor2.run(FORWARD);
      motor3.run(BACKWARD);
      motor4.run(BACKWARD);
      delay(1150);
    }

    motor1.run(RELEASE);
    motor2.run(RELEASE);
    motor3.run(RELEASE);
    motor4.run(RELEASE);
  }
  else
  {
    // Normal driving: vacuum off, drive forward, avoid obstacles
    motor1.setSpeed(140);
    motor2.setSpeed(140);
    motor3.setSpeed(140);
    motor4.setSpeed(140);
    digitalWrite(vacuum, LOW);
    distance = obstacle();

    if (distance > set)
    {
      // Path is clear: drive forward
      motor1.run(FORWARD);
      motor2.run(FORWARD);
      motor3.run(FORWARD);
      motor4.run(FORWARD);
    }
    else
    {
      // Obstacle ahead: back up, look both ways, and turn
      hcr4();
    }

    // Left IR sensor detects something: turn away (same direction as the Right button)
    if (digitalRead(IRLeft) == 0)
    {
      motor1.setSpeed(140);
      motor2.setSpeed(140);
      motor3.setSpeed(140);
      motor4.setSpeed(140);
      motor1.run(BACKWARD);
      motor2.run(BACKWARD);
      motor3.run(FORWARD);
      motor4.run(FORWARD);
      delay(1000);
    }

    // Right IR sensor detects something: turn away (same direction as the Left button)
    if (digitalRead(IRRight) == 0)
    {
      motor1.setSpeed(140);
      motor2.setSpeed(140);
      motor3.setSpeed(140);
      motor4.setSpeed(140);
      motor1.run(FORWARD);
      motor2.run(FORWARD);
      motor3.run(BACKWARD);
      motor4.run(BACKWARD);
      delay(1000);
    }
  }
}

// ---------------- Sensors ----------------

// Reads the HC-SR04 ultrasonic sensor and returns the distance in cm.
// (Returns 0 if no echo is received.)
int obstacle()
{
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);    // 10 microsecond trigger pulse
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  dis = pulseIn(echoPin, HIGH);   // echo time in microseconds
  return dis / 29 / 2;            // sound takes ~29 us per cm, there and back
}

// Turns the sensor to the right, measures, then turns back.
int lookRight()
{
  myservo.write(0);
  delay(500);
  distance = obstacle();
  delay(200);
  myservo.write(70);
  return distance;
}

// Turns the sensor to the left, measures, then turns back.
int lookLeft()
{
  myservo.write(170);
  delay(500);
  distance = obstacle();
  delay(200);
  myservo.write(100);
  return distance;
}

// ---------------- Movement helpers ----------------

void moveStop()
{
  motor1.run(RELEASE);
  motor2.run(RELEASE);
  motor3.run(RELEASE);
  motor4.run(RELEASE);
}

// Reverses while gradually increasing speed.
void moveBackward()
{
  goesForward = false;
  motor1.run(BACKWARD);
  motor2.run(BACKWARD);
  motor3.run(BACKWARD);
  motor4.run(BACKWARD);
  for (speedSet = 0; speedSet < MAX_SPEED; speedSet += 50) {
    motor1.setSpeed(speedSet);
    motor2.setSpeed(speedSet);
    motor3.setSpeed(speedSet);
    motor4.setSpeed(speedSet);
    delay(5);
  }
}

// Spins for 1.4 s, then drives forward.
// Note: this spins the same direction as the Left button in Bluetooth mode.
void turnRight()
{
  motor1.setSpeed(155);
  motor2.setSpeed(155);
  motor3.setSpeed(155);
  motor4.setSpeed(155);
  motor1.run(FORWARD);
  motor2.run(FORWARD);
  motor3.run(BACKWARD);
  motor4.run(BACKWARD);
  delay(1400);
  motor1.run(FORWARD);
  motor2.run(FORWARD);
  motor3.run(FORWARD);
  motor4.run(FORWARD);
}

// Spins for 1.4 s, then drives forward.
// Note: this spins the same direction as the Right button in Bluetooth mode.
void turnLeft()
{
  motor1.setSpeed(155);
  motor2.setSpeed(155);
  motor3.setSpeed(155);
  motor4.setSpeed(155);
  motor1.run(BACKWARD);
  motor2.run(BACKWARD);
  motor3.run(FORWARD);
  motor4.run(FORWARD);
  delay(1400);
  motor1.run(FORWARD);
  motor2.run(FORWARD);
  motor3.run(FORWARD);
  motor4.run(FORWARD);
}

// Obstacle routine: stop, back up, check left and right,
// then turn away from the closer side.
void hcr4()
{
  moveStop();
  delay(100);
  moveBackward();
  delay(300);
  moveStop();
  delay(200);
  distanceR = lookRight();
  delay(200);
  distanceL = lookLeft();
  delay(200);
  if (distanceR < distanceL)
  {
    turnRight();
    moveStop();
  } else if (distanceR > distanceL)
  {
    turnLeft();
    moveStop();
  }
}
