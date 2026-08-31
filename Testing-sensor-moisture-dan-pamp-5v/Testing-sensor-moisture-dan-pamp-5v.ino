const int pinSensorTanah = 32;
const int pinPompa = 27;

const int batasKering = 2500; 

void setup() {
  Serial.begin(115200); // Standar baud rate ESP32
  
  pinMode(pinSensorTanah, INPUT);
  pinMode(pinPompa, OUTPUT);
  digitalWrite(pinPompa, LOW); 
  
  Serial.println("Sistem Fitcom 4.0 Siap Dites!");
  delay(1000);
}

void loop() {
  int nilaiKelembapan = analogRead(pinSensorTanah);
  
  Serial.print("Nilai Sensor: ");
  Serial.println(nilaiKelembapan);

  if (nilaiKelembapan > batasKering) {
    Serial.println("Status: TANAH KERING! Menyiram air...");
    digitalWrite(pinPompa, HIGH); 
  } else {
    Serial.println("Status: TANAH BASAH. Aman.");
    digitalWrite(pinPompa, LOW); 
  }

  Serial.println("-------------------------");
  delay(2000);
}