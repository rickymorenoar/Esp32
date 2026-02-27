#include <Servo.h>

#define TRIG_PIN 9
#define ECHO_PIN 10
#define SERVO_PIN 6

#define OPEN_ANGLE 90
#define CLOSE_ANGLE 0
#define DISTANCE_LIMIT 15      // cm (ubah sesuai kebutuhan)
#define CLOSE_DELAY 500     // 10 detik (ms)

Servo myServo;

unsigned long lastDetectedTime = 0;
bool objectDetected = false;
bool servoOpen = false;

long readDistanceCM() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  long duration = pulseIn(ECHO_PIN, HIGH, 30000); // timeout 30ms
  long distance = duration * 0.034 / 2;
  return distance;
}

void setup() {
  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);

  myServo.attach(SERVO_PIN);
  myServo.write(CLOSE_ANGLE);

  Serial.begin(9600);
}

void loop() {
  long distance = readDistanceCM();
  Serial.print("Jarak: ");
  Serial.println(distance);

  if (distance > 0 && distance <= DISTANCE_LIMIT) {
    // Ada objek
    objectDetected = true;
    lastDetectedTime = millis();

    if (!servoOpen) {
      myServo.write(OPEN_ANGLE);
      servoOpen = true;
    }
  } else {
    // Tidak ada objek
    objectDetected = false;
  }

  // Jika objek sudah tidak ada dan servo terbuka
  if (!objectDetected && servoOpen) {
    if (millis() - lastDetectedTime >= CLOSE_DELAY) {
      myServo.write(CLOSE_ANGLE);
      servoOpen = false;
    }
  }
}