// SAMD21, SimpleFOC Mini v1.0, AS5600, 2804 motor

#include "Arduino.h"
#include <Wire.h>
#include <SimpleFOC.h>

// Config
const float SUPPLY_VOLTAGE = 12;
const int POLE_PAIRS = 7;
const float SWEEP_LOW = 0;
const float SWEEP_HIGH = TWO_PI;
const unsigned long SWEEP_INTERVAL_MS_DEFAULT = 3000;
const unsigned long STATUS_INTERVAL_MS = 500;
const unsigned long SERIAL_WAIT_MS = 3000;
const unsigned long MOTOR_RETRY_MS = 2000;
const int ENCODER_ADDRESS = 0x36;
const int I2C_CLOCK = 400000;

// PID
const float PID_VELOCITY_P = 0.6f;
const float PID_VELOCITY_I = 0;
const float PID_VELOCITY_D = 0.001f;
const float PID_OUTPUT_RAMP = 1000;
const float PID_VELOCITY_FILTER = 0.05f;
const float PID_ANGLE_P = 10;
const float MOTOR_VELOCITY_LIMIT = 5;
const float MOTOR_VOLTAGE_LIMIT = 12;

// Controllers
MagneticSensorI2C sensor = MagneticSensorI2C(AS5600_I2C);
BLDCMotor motor = BLDCMotor(POLE_PAIRS);
BLDCDriver3PWM driver = BLDCDriver3PWM(1, 2, 3, 7);
Commander command = Commander(Serial);

// Globals
float target_angle = SWEEP_LOW;
bool sweep_to_high = true;
bool manual_target = false;
unsigned long last_status_time = 0;
unsigned long last_motor_retry = 0;
unsigned long last_sweep_time = 0;
float sweep_interval_ms = SWEEP_INTERVAL_MS_DEFAULT;

// Setup
void setup() {
  // Start serial
  Serial.begin(115200);
  unsigned long wait_start = millis();
  while (!Serial && millis() - wait_start < SERIAL_WAIT_MS) delay(10);

  // Init board, encoder, motor, and sweep
  disableBoardLeds();
  if (!initEncoder()) {
    while (true) delay(1000);
  }
  initMotor();
  initSweep();

  // Start motor, or wait for 12V power
  if (startMotor()) {
    Serial.println("Motor ready, sweeping 0 to 6.28");
  } else {
    Serial.println("Waiting for 12V motor power");
  }
  Serial.println("T<angle> manual, S resume sweep");
  Serial.println("P I D R F A L V E tune, send letter alone to read");
}

// Loop
void loop() {
  // Retry motor start if 12V was applied after boot
  tryStartMotor();
  command.run();
  if (motor.motor_status != FOCMotorStatus::motor_ready) return;

  // Run FOC, move to target, handle commands and sweep
  motor.loopFOC();
  motor.move(target_angle);
  updateSweep();

  // Print stats on interval
  unsigned long now = millis();
  if (now - last_status_time >= STATUS_INTERVAL_MS) {
    last_status_time = now;
    printStats();
  }
}

// Turn off status LEDs
void disableBoardLeds() {
  // Disable built-in LED
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  pinMode(PIN_LED_TXL, OUTPUT);
  digitalWrite(PIN_LED_TXL, HIGH);
  pinMode(PIN_LED_RXL, OUTPUT);
  digitalWrite(PIN_LED_RXL, HIGH);
}

// Init encoder over I2C
bool initEncoder() {
  // Set up pins
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);

  // Start I2C
  Wire.begin();

  // Set clock
  Wire.setClock(I2C_CLOCK);

  // Check encoder
  if (!encoderPresent()) {
    Serial.println("AS5600 not found on D4/D5");
    return false;
  }

  // Init sensor and link to motor
  sensor.init();
  motor.linkSensor(&sensor);
  return true;
}

// Return true if AS5600 answers on I2C
bool encoderPresent() {
  // Check
  Wire.beginTransmission(ENCODER_ADDRESS);
  return Wire.endTransmission() == 0;
}

// Init driver and motor control
void initMotor() {
  // Set up driver
  driver.voltage_power_supply = SUPPLY_VOLTAGE;
  driver.init();
  motor.linkDriver(&driver);
  motor.foc_modulation = FOCModulationType::SpaceVectorPWM;
  motor.controller = MotionControlType::angle;
  motor.PID_velocity.P = PID_VELOCITY_P;
  motor.PID_velocity.I = PID_VELOCITY_I;
  motor.PID_velocity.D = PID_VELOCITY_D;
  motor.PID_velocity.output_ramp = PID_OUTPUT_RAMP;
  motor.LPF_velocity.Tf = PID_VELOCITY_FILTER;
  motor.P_angle.P = PID_ANGLE_P;
  motor.velocity_limit = MOTOR_VELOCITY_LIMIT;
  motor.voltage_limit = MOTOR_VOLTAGE_LIMIT;
  motor.init();
}

// Run FOC alignment and enable motor
bool startMotor() {
  if (motor.motor_status == FOCMotorStatus::motor_ready) return true;
  motor.enable();
  if (!motor.initFOC()) {
    Serial.println("Waiting for 12V motor power");
    return false;
  }
  Serial.println("Motor started, sweeping 0 to 6.28");
  return true;
}

// Retry motor start on interval until 12V is available
void tryStartMotor() {
  if (motor.motor_status == FOCMotorStatus::motor_ready) return;
  unsigned long now = millis();
  if (now - last_motor_retry < MOTOR_RETRY_MS) return;
  last_motor_retry = now;
  startMotor();
}

// Register serial commands and set sweep start
void initSweep() {
  // Set up sweep
  target_angle = SWEEP_LOW;
  sweep_to_high = true;
  last_sweep_time = millis();
  command.add('T', doTarget, "manual target");
  command.add('S', doResumeSweep, "resume sweep");
  command.add('P', doVelocityP, "velocity P");
  command.add('I', doVelocityI, "velocity I");
  command.add('D', doVelocityD, "velocity D");
  command.add('R', doOutputRamp, "output ramp");
  command.add('F', doVelocityFilter, "velocity filter");
  command.add('A', doAngleP, "angle P");
  command.add('L', doVoltageLimit, "voltage limit");
  command.add('V', doVelocityLimit, "velocity limit");
  command.add('E', doSweepInterval, "sweep interval ms");
}

// Set manual target from serial
void doTarget(char* command_text) {
  // Set manual target
  command.scalar(&target_angle, command_text);
  manual_target = true;
  Serial.print("Manual target ");
  Serial.println(target_angle, 3);
}

// Resume automatic sweep
void doResumeSweep(char* command_text) {
  // Resume sweep
  manual_target = false;
  sweep_to_high = true;
  target_angle = SWEEP_LOW;
  last_sweep_time = millis();
  Serial.println("Sweep resumed");
}

// Set velocity loop P gain
void doVelocityP(char* command_text) {
  command.scalar(&motor.PID_velocity.P, command_text);
}

// Set velocity loop I gain
void doVelocityI(char* command_text) {
  command.scalar(&motor.PID_velocity.I, command_text);
}

// Set velocity loop D gain
void doVelocityD(char* command_text) {
  command.scalar(&motor.PID_velocity.D, command_text);
}

// Set velocity loop output ramp
void doOutputRamp(char* command_text) {
  command.scalar(&motor.PID_velocity.output_ramp, command_text);
}

// Set velocity low pass filter time constant
void doVelocityFilter(char* command_text) {
  command.scalar(&motor.LPF_velocity.Tf, command_text);
}

// Set position loop P gain
void doAngleP(char* command_text) {
  command.scalar(&motor.P_angle.P, command_text);
}

// Set motor voltage limit
void doVoltageLimit(char* command_text) {
  command.scalar(&motor.voltage_limit, command_text);
}

// Set motor velocity limit
void doVelocityLimit(char* command_text) {
  command.scalar(&motor.velocity_limit, command_text);
  motor.updateVelocityLimit(motor.velocity_limit);
}

// Set sweep interval in milliseconds
void doSweepInterval(char* command_text) {
  command.scalar(&sweep_interval_ms, command_text);
}

// Flip sweep target on timer
void updateSweep() {
  // Check manual target
  if (manual_target) return;

  // Check sweep interval
  unsigned long now = millis();
  if (now - last_sweep_time < (unsigned long)sweep_interval_ms) return;
  last_sweep_time = now;

  // Flip sweep target
  if (sweep_to_high) {
    target_angle = SWEEP_HIGH;
    sweep_to_high = false;
  } else {
    target_angle = SWEEP_LOW;
    sweep_to_high = true;
  }

  // Print sweep target
  Serial.print("Sweep target ");
  Serial.println(target_angle, 3);
}

// Print motor stats
void printStats() {
  float error = target_angle - motor.shaft_angle;
  Serial.print("Position: ");
  Serial.print(motor.shaft_angle, 3);
  Serial.print(" Target: ");
  Serial.print(target_angle, 3);
  Serial.print(" Error: ");
  Serial.print(error, 3);
  Serial.print(" Velocity: ");
  Serial.print(motor.shaft_velocity, 3);
  Serial.print(" Voltage: ");
  Serial.println(motor.voltage.q, 3);
}
