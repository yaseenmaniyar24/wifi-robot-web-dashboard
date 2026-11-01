# Wi-Fi Robot with Web Dashboard, Telemetry & Safe-Stop

A 4WD differential-drive robot controlled from a phone or laptop browser over the ESP32's Wi-Fi access point. The ESP32 serves the dashboard and handles motor commands, ultrasonic distance telemetry, command timeout and a software emergency-stop latch.

## Project status

This repository contains a starter firmware/dashboard implementation and documentation based on the supplied prototype photo. Confirm the exact board/module labels and wiring before applying power. The photo does not prove the motor-driver current rating, motor voltage, battery type or exact wiring.

## Features

- ESP32-hosted Wi-Fi access point and browser dashboard
- Forward, reverse, left, right and stop controls
- PWM speed setting
- HC-SR04 distance telemetry
- Forward obstacle blocking (including invalid/no-echo readings)
- 700 ms command timeout intended to stop motion if fresh commands stop arriving
- Software emergency-stop latch and reset
- Responsive dashboard for laptop and mobile

## Hardware

- ESP32 DevKit (ESP32 Arduino core 3.x is expected by the LEDC calls)
- TB6612FNG dual H-bridge motor driver
- Four DC geared motors (shown as yellow TT-style motors in the prototype photo)
- HC-SR04 ultrasonic sensor
- Separate motor power supply appropriate for the motors and driver
- USB cable for ESP32 programming/power
- Common ground wiring, fuse, suitable connectors
- Physical latching motor-power emergency disconnect strongly recommended

## Critical power and current warning

**Do not use a rectangular 9 V PP3 battery to power four yellow 3–6 V TT motors.** It is a poor high-current source, and 9 V exceeds the stated 3–6 V motor range.

For motors intended to run at 6 V, use a properly specified 6 V motor supply. A protected 2S (7.4 V nominal, 8.4 V fully charged) lithium-ion pack requires an appropriately rated high-current buck regulator set and verified at 6.0 V before connecting the motors. A suitable 5-cell NiMH pack is another possible 6 V nominal option; verify its fully charged voltage and motor limits.

The TB6612FNG has two motor channels, not four independent channels. This project pairs the two left motors on channel A and the two right motors on channel B. **Do not assume this is safe until you measure or obtain each motor's stall current and confirm the combined current on each channel is within the driver's continuous and peak limits, including thermal conditions.** If not, use two drivers or a higher-current driver.

Laptop USB powers the ESP32. The motor supply powers TB6612FNG VM. Connect ESP32 GND, TB6612FNG GND and motor-supply negative together. Connect TB6612FNG VCC to ESP32 3V3. Never connect motor battery voltage to ESP32 3V3 or USB.

## Pin mapping

| TB6612FNG | ESP32 |
|---|---|
| VCC | 3V3 |
| GND | GND (common with motor supply negative) |
| VM | Regulated motor supply positive |
| STBY | GPIO 23 |
| PWMA | GPIO 25 |
| AIN1 | GPIO 27 |
| AIN2 | GPIO 14 |
| PWMB | GPIO 26 |
| BIN1 | GPIO 32 |
| BIN2 | GPIO 33 |
| AO1/AO2 | Both left-side motors in parallel (only if current-rated) |
| BO1/BO2 | Both right-side motors in parallel (only if current-rated) |

### HC-SR04

| HC-SR04 | ESP32 |
|---|---|
| VCC | 5V |
| GND | GND |
| TRIG | GPIO 5 |
| ECHO | GPIO 18 through a resistor divider or level shifter |

For a 5 V ECHO output, use a divider such as 1 kΩ from ECHO to GPIO18 and 2 kΩ from GPIO18 to GND. Verify the resulting voltage before connecting.

**Check your actual TB6612FNG breakout pin labels. Board layouts vary.**

## Arduino IDE setup

1. Install Arduino IDE.
2. Install Espressif ESP32 board support using Boards Manager.
3. Select the correct ESP32 board (often `ESP32 Dev Module`) and serial port.
4. Open `firmware/wifi_robot.ino`.
5. Confirm you have ESP32 Arduino core 3.x (the sketch uses `ledcAttach`).
6. Upload while the robot is safely supported with its wheels off the ground and motor power disconnected.
7. Connect motor power only after checking all wiring and supply voltage.

## Connect to dashboard

1. Power ESP32 from USB.
2. Open Serial Monitor at 115200 baud.
3. Connect phone/laptop Wi-Fi to `YASEEN_ROBOT`; password is `robot123`.
4. Browse to `http://192.168.4.1`.
5. Hold a direction button to move; release to send stop.
6. Use E-stop to latch a software stop. Reset only clears the software latch; the robot remains stopped until a new movement command.

Change the Wi-Fi password before demonstrating in a shared environment.

## Safe test procedure

1. Keep motor supply disconnected while checking logic wiring.
2. Verify VCC=3.3 V, motor VM is the intended regulated voltage, and all grounds are common.
3. Lift the chassis so wheels cannot touch the floor.
4. Test one motor/channel at low PWM first; confirm both motors on each side rotate in the intended direction.
5. If a wheel runs backward, power off and reverse that motor's two output wires.
6. Monitor driver temperature and supply voltage. Stop if the driver, wires, battery or regulator heats unexpectedly.
7. Test obstacle stop and Wi-Fi command timeout at low speed.
8. Fit a physical latching switch/fuse in the motor supply. A browser E-stop cannot work if Wi-Fi or the ESP32 fails.

## Known limitations / next improvements

- No wheel encoders: speed shown is PWM command, not measured wheel speed.
- No battery-voltage telemetry yet.
- Ultrasonic sensing is a single forward-facing measurement; it is not a certified collision-avoidance system.
- HTTP polling is used for simplicity; WebSocket can be added for lower-latency telemetry.
- The 700 ms timeout is a software safeguard, not a safety-certified stop system.
- Add physical E-stop hardware and validate stopping distance under realistic loads.

## Repository structure

```text
wifi-robot-web-dashboard/
├── README.md
├── LICENSE
├── .gitignore
├── firmware/
│   └── wifi_robot.ino
├── docs/
│   ├── WIRING_AND_POWER.md
│   ├── BILL_OF_MATERIALS.md
│   └── TEST_PLAN.md
└── images/
    ├── robot_prototype.jpg
    └── robot_prototype_edited.jpg
```
