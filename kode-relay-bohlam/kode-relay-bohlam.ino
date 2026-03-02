#define SOUND_PIN 2    
#define RELAY_PIN 8    

bool lampState = false;
unsigned long lastTrigger = 0;
const unsigned long debounceTime = 1000; 

void setup() {
  pinMode(SOUND_PIN, INPUT);
  pinMode(RELAY_PIN, OUTPUT);

  digitalWrite(RELAY_PIN, HIGH); 

void loop() {
  int soundValue = digitalRead(SOUND_PIN);

  // Sensor aktif LOW saat ada suara
  if (soundValue == LOW) {
    if (millis() - lastTrigger > debounceTime) {
      lampState = !lampState;
      digitalWrite(RELAY_PIN, lampState ? LOW : HIGH);
      lastTrigger = millis();
    }
  }
}