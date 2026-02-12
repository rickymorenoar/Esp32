#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>

#include <Wire.h>
#include <U8g2lib.h>

#define SDA_PIN 8
#define SCL_PIN 9
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0);

#define SERVICE_UUID        "6E400001-B5A3-F393-E0A9-E50E24DCCA9E"
#define CHARACTERISTIC_UUID "6E400003-B5A3-F393-E0A9-E50E24DCCA9E"

String hudText = "Say something...";
bool deviceConnected = false;

/* ===== BLE CALLBACK ===== */
class MyCallbacks : public BLECharacteristicCallbacks {
  void onWrite(BLECharacteristic *pChar) {
    std::string value = pChar->getValue();
    if (value.length() > 0) {
      hudText = "";
      for (char c : value) hudText += c;
    }
  }
};

class MyServerCallbacks : public BLEServerCallbacks {
  void onConnect(BLEServer* pServer) {
    deviceConnected = true;
  }
  void onDisconnect(BLEServer* pServer) {
    deviceConnected = false;
    BLEDevice::startAdvertising();
  }
};

void setup() {
  Serial.begin(115200);

  Wire.begin(SDA_PIN, SCL_PIN);
  u8g2.begin();

  BLEDevice::init("HUD-GLASSES");
  BLEServer *server = BLEDevice::createServer();
  server->setCallbacks(new MyServerCallbacks());

  BLEService *service = server->createService(SERVICE_UUID);

  BLECharacteristic *characteristic = service->createCharacteristic(
    CHARACTERISTIC_UUID,
    BLECharacteristic::PROPERTY_WRITE
  );

  characteristic->addDescriptor(new BLE2902());
  characteristic->setCallbacks(new MyCallbacks());

  service->start();
  BLEDevice::startAdvertising();
}

void loop() {
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_6x10_tf);

  if (deviceConnected) {
    u8g2.drawStr(0, 12, "BLE CONNECTED");
  } else {
    u8g2.drawStr(0, 12, "WAITING BLE...");
  }

  u8g2.drawStr(0, 32, hudText.c_str());
  u8g2.sendBuffer();

  delay(100);
}
