#include <ESP32Servo.h>

#define TRIG_PIN 26      // GPIO untuk Trig
#define ECHO_PIN 25      // GPIO untuk Echo
#define SERVO_PIN 13     // GPIO Servo
#define BUTTON_PIN 33    // GPIO Push Button

Servo myServo;

long duration;
int distance;

bool gateOpen = false;  // Status gerbang

void setup() {
  Serial.begin(115200);

  pinMode(TRIG_PIN, OUTPUT);
  pinMode(ECHO_PIN, INPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  myServo.attach(SERVO_PIN);
  myServo.write(0);  // Gerbang awal tertutup
  Serial.println("🚗 Sistem Parkir Siap!");
}

void loop() {
  // --- Baca Tombol ---
  if (digitalRead(BUTTON_PIN) == LOW && !gateOpen) {
    Serial.println("🔘 Tombol ditekan → Gerbang Dibuka");
    myServo.write(90);  // buka gerbang
    gateOpen = true;
    delay(500); // debounce tombol
  }

  // --- Jika gerbang terbuka, cek sensor ---
  if (gateOpen) {
    int jarak = bacaJarak();

    Serial.print("Jarak: ");
    Serial.print(jarak);
    Serial.println(" cm");

    if (jarak > 0 && jarak < 10) {
      Serial.println("🚧 Mobil terdeteksi → Tunggu sebelum menutup");
      
      delay(1000); // ⏳ TUNGGU 3 DETIK (ubah sesuai kebutuhan)
      
      Serial.println("🔒 Menutup Gerbang...");
      myServo.write(0);  // tutup gerbang
      gateOpen = false;
      delay(2000); // beri waktu servo menutup penuh
    }
  }

  delay(100);
}

// --- Fungsi untuk baca jarak ---
int bacaJarak() {
  digitalWrite(TRIG_PIN, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG_PIN, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG_PIN, LOW);

  duration = pulseIn(ECHO_PIN, HIGH, 30000); // timeout 30ms (~5m)
  int jarak = duration * 0.034 / 2;
  return jarak;
}
