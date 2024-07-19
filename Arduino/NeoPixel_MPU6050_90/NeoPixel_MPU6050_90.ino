#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_NeoPixel.h>

// Constants
#define NEOPIXEL_PIN 32
#define NUM_PIXELS 1
#define LED_COLOR_DEFAULT 0x000000 // Off (Black)
#define LED_COLOR_90DEG 0xFF0000   // Red

// MPU6050
Adafruit_MPU6050 mpu;

// NeoPixel
Adafruit_NeoPixel strip(NUM_PIXELS, NEOPIXEL_PIN, NEO_GRB + NEO_KHZ800);

// Variables to store the cumulative rotation angle
float angleZ = 0.0;

// Timestamp to track the time between readings
unsigned long lastUpdateTime = 0;

void setup() {
  // Initialize serial communication
  Serial.begin(115200);

  // Initialize MPU6050
  if (!mpu.begin()) {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    while (1) {
      delay(500);
    }
  }

  // Set up MPU6050
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // Initialize NeoPixel
  strip.begin();
  strip.show(); // Initialize all pixels to 'off'

  // Set initial timestamp
  lastUpdateTime = millis();

  // Initialize angle
  angleZ = 0.0;

  Serial.println("Initialization complete. Starting angle measurement...");
}

void loop() {
  // Get current timestamp
  unsigned long currentTime = millis();
  // Calculate time difference in seconds
  float deltaTime = (currentTime - lastUpdateTime) / 1000.0;
  lastUpdateTime = currentTime;

  // Read data from MPU6050
  sensors_event_t accel, gyro, temp;
  mpu.getEvent(&accel, &gyro, &temp);

  // Calculate rotation in Z-axis (Yaw) using complementary filter
  float accelAngleZ = atan2(accel.acceleration.y, accel.acceleration.x) * 180 / PI;
  angleZ = 0.98 * (angleZ + gyro.gyro.z * deltaTime) + 0.02 * accelAngleZ;

  // Output current rotation angle
  Serial.print("Current Z-angle: ");
  Serial.println(angleZ);

  // Check if the cumulative rotation angle in Z-axis is approximately 90 degrees
  if (abs(angleZ) >= 90) {
    // Reset cumulative angle
    angleZ = 0.0;
    // Change NeoPixel color to Red
    strip.setPixelColor(0, LED_COLOR_90DEG);
    strip.show();
    delay(1000); // Keep the LED on for 1 second
    // Turn off NeoPixel
    strip.setPixelColor(0, LED_COLOR_DEFAULT);
    strip.show();
  }
  // Small delay to stabilize loop
  delay(10);
}
