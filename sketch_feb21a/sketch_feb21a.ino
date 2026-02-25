#define RELAY1 5   // IN1 - kontrol arah 1
#define RELAY2 6   // IN2 - kontrol arah 2

// Relay Active LOW (umum pada relay module)
#define RELAY_ON  LOW
#define RELAY_OFF HIGH

// Waktu gerak (milliseconds)
#define WAKTU_KANAN  3000   // 3 detik ke kanan
#define WAKTU_BERHENTI 500  // 0.5 detik berhenti
#define WAKTU_KIRI   3000   // 3 detik ke kiri

void setup() {
  Serial.begin(115200);
  
  pinMode(RELAY1, OUTPUT);
  pinMode(RELAY2, OUTPUT);
  
  // Pastikan motor mati saat awal
  motorBerhenti();
  
  Serial.println("Motor Control Ready!");
  delay(1000);
}

void loop() {
  // Gerak ke KANAN
  motorKanan();
  Serial.println(">> Motor: KANAN");
  delay(WAKTU_KANAN);
  
  // Berhenti sebentar
  motorBerhenti();
  Serial.println(">> Motor: BERHENTI");
  delay(WAKTU_BERHENTI);
  
  // Gerak ke KIRI
  motorKiri();
  Serial.println(">> Motor: KIRI");
  delay(WAKTU_KIRI);
  
  // Berhenti sebentar
  motorBerhenti();
  Serial.println(">> Motor: BERHENTI");
  delay(WAKTU_BERHENTI);
}


// FUNGSI KONTROL MOTOR

void motorKanan() {
  digitalWrite(RELAY1, RELAY_ON);   // Relay 1 ON
  digitalWrite(RELAY2, RELAY_OFF);  // Relay 2 OFF
}

void motorKiri() {
  digitalWrite(RELAY1, RELAY_OFF);  // Relay 1 OFF
  digitalWrite(RELAY2, RELAY_ON);   // Relay 2 ON
}

void motorBerhenti() {
  // Kedua relay OFF = motor mati
  digitalWrite(RELAY1, RELAY_OFF);
  digitalWrite(RELAY2, RELAY_OFF);
}