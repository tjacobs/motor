# motor

Arduino position control for a BLDC gimbal motor using SimpleFOC on a Seeed XIAO SAMD21.

## Hardware

- **MCU:** Seeed XIAO SAMD21
- **Driver:** SimpleFOC Mini v1.0
- **Encoder:** AS5600 over I2C
- **Motor:** QiuLovesYT 2804, 7 pole pairs, 12N14P

## Wiring

| Signal | XIAO | SimpleFOC Mini |
|--------|------|----------------|
| IN1 | D1 | IN1 |
| IN2 | D2 | IN2 |
| IN3 | D3 | IN3 |
| EN | D7 | EN |
| GND | GND | GND |
| AS5600 SDA | D4 | — |
| AS5600 SCL | D5 | — |
| Motor power | — | 12V +/− |
| Motor phases | — | M1, M2, M3 |

Connect GND between the XIAO and the driver. D4 and D5 are reserved for I2C, so PWM uses D1, D2, and D3.

## Setup

Install the Seeeduino SAMD board package and the SimpleFOC library.

```bash
arduino-cli core install Seeeduino:samd
arduino-cli lib install SimpleFOC
```

## Upload

Close Serial Monitor before uploading, it locks the USB port.

```bash
arduino-cli compile --fqbn Seeeduino:samd:seeed_XIAO_m0 position_control_test
arduino-cli upload -p /dev/cu.usbmodem101 --fqbn Seeeduino:samd:seeed_XIAO_m0 position_control_test
```

## Usage

Open Serial Monitor at 115200 baud. On boot the motor sweeps between 0 and 6.28 rad.

| Command | Action |
|---------|--------|
| `T<angle>` | Set manual target in radians, example `T1.57` |
| `S` | Resume automatic sweep |

Stats print every 500 ms: position, target, error, velocity, and voltage.

## Troubleshooting

- **AS5600 not found:** Check SDA on D4, SCL on D5, GND, and 3.3V or 5V on the encoder module.
- **Red LED on SimpleFOC Mini:** DRV8313 fault, often from 12V overcurrent. Disconnect power, wait, and repower.
- **Upload fails:** Close Serial Monitor and retry.

## Project layout

```
motor/
├── AGENTS.md
├── README.md
└── position_control_test/
    └── position_control_test.ino
```
