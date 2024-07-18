#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

Adafruit_MPU6050 mpu;

// 定义ESP32上的SDA和SCL引脚
#define SDA_PIN 21
#define SCL_PIN 22

void setup() {
  // 初始化I2C通信，使用指定的SDA和SCL引脚
  Wire.begin(SDA_PIN, SCL_PIN);
  
  Serial.begin(115200);
  Serial.println("Initialize MPU6050");

  if (!mpu.begin()) {
    Serial.println("Could not find a valid MPU6050 sensor, check wiring!");
    while (1) {
      delay(20);
    }
  }

  // 配置传感器
  mpu.setAccelerometerRange(MPU6050_RANGE_2_G);
  mpu.setGyroRange(MPU6050_RANGE_250_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);

  // 列出各项设定
  checkSettings();
}

void checkSettings() {
  Serial.println();
  Serial.print(" * Accelerometer Range:   ");
  switch(mpu.getAccelerometerRange()) {
    case MPU6050_RANGE_16_G: Serial.println("+/- 16 g"); break;
    case MPU6050_RANGE_8_G: Serial.println("+/- 8 g"); break;
    case MPU6050_RANGE_4_G: Serial.println("+/- 4 g"); break;
    case MPU6050_RANGE_2_G: Serial.println("+/- 2 g"); break;
  }  
  Serial.print(" * Gyro Range:            ");
  switch(mpu.getGyroRange()) {
    case MPU6050_RANGE_250_DEG: Serial.println("+/- 250 deg/s"); break;
    case MPU6050_RANGE_500_DEG: Serial.println("+/- 500 deg/s"); break;
    case MPU6050_RANGE_1000_DEG: Serial.println("+/- 1000 deg/s"); break;
    case MPU6050_RANGE_2000_DEG: Serial.println("+/- 2000 deg/s"); break;
  }
  Serial.print(" * Filter Bandwidth:      ");
  switch(mpu.getFilterBandwidth()) {
    case MPU6050_BAND_260_HZ: Serial.println("260 Hz"); break;
    case MPU6050_BAND_184_HZ: Serial.println("184 Hz"); break;
    case MPU6050_BAND_94_HZ: Serial.println("94 Hz"); break;
    case MPU6050_BAND_44_HZ: Serial.println("44 Hz"); break;
    case MPU6050_BAND_21_HZ: Serial.println("21 Hz"); break;
    case MPU6050_BAND_10_HZ: Serial.println("10 Hz"); break;
    case MPU6050_BAND_5_HZ: Serial.println("5 Hz"); break;
  }
  
  Serial.println();
  Serial.println("       Accelerometer                      Gyrometer           ");
  Serial.println("--------------------------------------------------------------");
  Serial.println(" Xraw      Yraw       Zraw        Xnorm     Ynorm     Znorm   ");
}

void loop() {
  // 读取加速度计和陀螺仪的值
  sensors_event_t accel, gyro, temp;
  mpu.getEvent(&accel, &gyro, &temp);

  Serial.print(accel.acceleration.x);
  Serial.print("   ");
  Serial.print(accel.acceleration.y);
  Serial.print("   ");
  Serial.print(accel.acceleration.z);
  Serial.print("     ");

  Serial.print(gyro.gyro.x);
  Serial.print("     ");  
  Serial.print(gyro.gyro.y);
  Serial.print("      ");  
  Serial.println(gyro.gyro.z);
  
  delay(20);
}
