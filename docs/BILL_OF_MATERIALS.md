# Bill of Materials (BOM)

| Item | Qty | Notes |
|---|---:|---|
| ESP32 DevKit | 1 | Wi-Fi controller; USB powered for development |
| TB6612FNG breakout | 1 initially | Two channels; verify current before pairing motors |
| Yellow TT-style DC geared motors | 4 | Confirm exact rated voltage and stall current |
| 4WD chassis/wheels | 1 set | Prototype chassis |
| HC-SR04 ultrasonic sensor | 1 | Forward distance sensing |
| Motor battery | 1 | Properly rated rechargeable pack; not rectangular 9 V PP3 |
| 6 V high-current buck regulator (if using 2S pack) | 1 | Must meet combined motor current |
| Compatible 2S BMS and 8.4 V CC/CV charger (if using 2S pack) | 1 each | Match cell chemistry and pack configuration |
| Fuse + holder | 1 | Size to wiring, driver and motor load |
| 1 kΩ resistor | 1 | HC-SR04 ECHO divider upper resistor |
| 2 kΩ resistor | 1 | HC-SR04 ECHO divider lower resistor |
| Jumper wires/connectors | as needed | Use suitable wire gauge for motor current |
| Physical latching motor-power disconnect | 1 | Strongly recommended independent stop |

Prices are intentionally omitted because component ratings and local supplier prices vary. Select the battery, regulator, BMS and fuse only after motor current is known.
