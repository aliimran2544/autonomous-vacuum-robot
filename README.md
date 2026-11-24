# Autonomous Vacuum Robot

An Arduino cleaning robot that can drive itself around a room or be controlled from a phone. It won **2nd place in both 2024 and 2025 at MASE**, Gulf Medical University's annual Medical and Science Exhibition and one of the largest inter-school science competitions in the UAE.

<!-- Add a photo or video of the robot here -->
<!-- ![Robot](images/robot.jpg) -->

## Features

- **Two modes**, switched with the START button on the phone:
  - **Bluetooth mode:** drive manually and turn the vacuum on/off from the [Dabble](https://thestempedia.com/product/dabble/) app (Gamepad)
  - **Automatic mode:** drives on its own and avoids obstacles
- **Obstacle avoidance:** an ultrasonic sensor on a servo detects objects within 15 cm. The robot backs up, looks left and right, and turns toward the more open side
- **Side IR sensors** steer it away from walls and objects beside it
- **Spot cleaning:** when the front IR sensor is triggered, the robot stops, turns the vacuum on, and moves back and forth over the spot

## Controls (Dabble Gamepad)

| Button | Action |
|---|---|
| START | Switch between Bluetooth and Automatic mode |
| Up / Down | Forward / backward |
| Left / Right | Turn |
| Triangle | Vacuum on |
| Circle | Vacuum off |

## Hardware

| Part | Pin |
|---|---|
| Arduino Uno | |
| L293D Motor Driver Shield (Adafruit Motor Shield v1 compatible) | 4 DC motors on M1–M4 |
| HC-SR04 ultrasonic sensor | Trig → A1, Echo → A0 |
| Servo motor (rotates the ultrasonic sensor) | Pin 9 |
| IR sensor (spot cleaning trigger) | A4 |
| Left IR sensor | Pin 2 |
| Right IR sensor | A5 |
| Vacuum (via relay/driver) | Pin 10 |
| HC-05 Bluetooth module | Pins 16 (A2) and 17 (A3) |

## Libraries

Install from the Arduino IDE Library Manager:

- `Dabble`
- `Adafruit Motor Shield library` (AFMotor, v1)
- `Servo` (included with the Arduino IDE)

## Getting started

1. Open `vacuum_robot/vacuum_robot.ino` in the Arduino IDE.
2. Install the libraries above.
3. Select **Arduino Uno** and upload.
4. Connect to the robot in the Dabble app and open **Gamepad**.

## What I'd improve next

- Add a timeout to the ultrasonic reading so open spaces aren't treated as obstacles
- Replace the long `delay()` calls with non-blocking timing so the robot responds to the phone instantly
- Map the room so it cleans in an organized pattern instead of moving randomly

## Author

**Ali Imran**, CS & Math student at the University of Toronto Scarborough
