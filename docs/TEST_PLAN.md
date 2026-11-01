# Test Plan

| Test | Procedure | Pass condition |
|---|---|---|
| Logic power | USB power ESP32, motor supply disconnected | ESP32 boots and AP appears |
| Driver standby | No movement command | STBY low; motors stopped |
| Left side | Wheels raised; low PWM forward | Both left motors rotate consistently |
| Right side | Wheels raised; low PWM forward | Both right motors rotate consistently |
| Direction | Test F/B/L/R at low PWM | Chassis directions match controls |
| Stop | Release direction button | Motor outputs go to stop/standby |
| Obstacle | Safely place obstacle in sensor range | Forward command blocked below threshold |
| Sensor failure | Disconnect/obscure sensor safely | Forward movement is blocked on invalid reading |
| Command timeout | Start movement, stop refreshing commands | Motors stop after timeout |
| Software E-stop | Press E-stop | Motion stops and latch remains active |
| Thermal/current | Run short controlled tests while monitoring | Driver, battery, regulator and wiring stay within ratings |

Run tests with wheels raised first. Do not deliberately stall motors for long periods. A physical power disconnect must be tested independently of the web dashboard.
