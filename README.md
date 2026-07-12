# Motor

Arduino position control for a BLDC gimbal motor using SimpleFOC on a Seeed XIAO SAMD21.

## Hardware

- **MCU:** Seeed XIAO SAMD21
- **Driver:** SimpleFOC Mini v1.0
- **Encoder:** AS5600 over I2C
- **Motor:** [2804 kit](https://www.amazon.com/dp/B0FXKN9YMJ), 7 pole pairs, 12N14P

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

Open Serial Monitor at 115200 baud. On boot the motor sweeps between 0 and 6.28 rad on a timer, default 3 seconds per direction.

| Command | Action |
|---------|--------|
| `T<angle>` | Set manual target in radians, example `T1.57` |
| `S` | Resume automatic sweep |
| `P<value>` | Velocity loop P gain |
| `I<value>` | Velocity loop I gain |
| `D<value>` | Velocity loop D gain |
| `R<value>` | Velocity loop output ramp, volts per second |
| `F<value>` | Velocity low pass filter time constant |
| `A<value>` | Position loop P gain, angle P |
| `L<value>` | Motor voltage limit |
| `V<value>` | Motor velocity limit |
| `E<value>` | Sweep interval in milliseconds |

Send a letter alone to read the current value, for example `P` or `E`.

### What is angle P?

Position control uses two cascaded loops. The outer loop compares target angle to measured angle and outputs a desired velocity. **Angle P** is the proportional gain on that outer loop, `motor.P_angle.P`. Higher values track position faster and feel stiffer. Too high causes overshoot and vibration. The inner velocity loop, tuned with P, I, and D, turns that velocity command into motor voltage.

Stats print every 500 ms: position, target, error, velocity, and voltage.

## Troubleshooting

- **AS5600 not found:** Check SDA on D4, SCL on D5, GND, and 3.3V or 5V on the encoder module.
- **Red LED on SimpleFOC Mini:** DRV8313 fault, often from 12V overcurrent. Disconnect power, wait, and repower.
- **Motor idle after boot:** 12V can be applied after the MCU boots. The sketch retries FOC init every 2 seconds once power is on.
- **Upload fails:** Close Serial Monitor and retry.

## Project layout

```
motor/
├── AGENTS.md
├── README.md
└── position_control_test/
    └── position_control_test.ino
```
