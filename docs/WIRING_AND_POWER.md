# Wiring and Power Notes

## Driver signal wiring

| TB6612FNG | ESP32 GPIO / rail |
|---|---|
| VCC | 3V3 |
| GND | GND |
| VM | Regulated motor supply positive |
| STBY | 23 |
| PWMA | 25 |
| AIN1 | 27 |
| AIN2 | 14 |
| PWMB | 26 |
| BIN1 | 32 |
| BIN2 | 33 |

AO1/AO2 drive the left-side motor pair; BO1/BO2 drive the right-side motor pair. Pair motors in parallel only after checking their combined stall current against the driver's ratings.

## Power arrangement

- Laptop USB -> ESP32 USB connector.
- Motor battery -> fuse -> motor regulator (if needed) -> TB6612FNG VM.
- ESP32 3V3 -> TB6612FNG VCC.
- Join ESP32 GND, TB6612FNG GND and motor-supply negative.
- Never feed the motor battery into ESP32 3V3 or USB.
- Do not connect the rectangular 9 V PP3 battery to 3–6 V TT motors.

For a 6 V motor target, a protected 2S lithium-ion pack plus a high-current buck regulator adjusted to 6.0 V is one option. Size the regulator, battery, BMS, wiring and fuse from measured combined motor startup/stall current. A 5-cell NiMH pack may also be suitable if its voltage and current capability meet motor requirements.

## Ultrasonic divider

HC-SR04 ECHO (5 V) -> 1 kΩ -> GPIO18 node; GPIO18 node -> 2 kΩ -> GND. This produces about 3.3 V from a 5 V input. HC-SR04 GND must join common GND.

## Before power-up

1. Check polarity with a multimeter.
2. Verify regulator output with no load before connecting electronics.
3. Confirm VM never exceeds motor/driver limits.
4. Check the exact breakout pin labels; board layouts differ.
5. Test with wheels raised.
