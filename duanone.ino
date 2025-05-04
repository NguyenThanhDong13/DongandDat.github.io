#include <DHT.h>

// --- Khai báo chân ---
#define GAS_PIN A0        // Cảm biến gas MQ hoặc cảm biến đất
#define BUZZER_PIN D6
#define RELAY_PIN D5
#define LED_PIN D2
#define BUTTON_PIN D4
#define DHT_PIN D1        // DHT22 dùng chân D1 (GPIO5)

// --- DHT22 ---
#define DHTTYPE DHT22
DHT dht(DHT_PIN, DHTTYPE);

// --- Trạng thái ---
unsigned long gasLowStartTime = 0;
bool alarmActive = false;
bool buzzerSilenced = false;

// LED và buzzer active LOW
#define LED_ON  LOW
#define LED_OFF HIGH
#define BUZZER_ON  LOW
#define BUZZER_OFF HIGH

// Xử lý nút
unsigned long buttonPressStart = 0;
bool buttonWasPressed = false;
bool testModeActivated = false;

void setup() {
  Serial.begin(115200);

  pinMode(BUZZER_PIN, OUTPUT);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(LED_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  digitalWrite(BUZZER_PIN, BUZZER_OFF);
  digitalWrite(RELAY_PIN, LOW);
  digitalWrite(LED_PIN, LED_OFF);

  dht.begin();
}

void loop() {
  // Đọc nút nhấn
  bool buttonPressed = digitalRead(BUTTON_PIN) == LOW;

  // Đọc cảm biến DHT
  float h = dht.readHumidity();
  float t = dht.readTemperature();

  if (!isnan(h) && !isnan(t)) {
    Serial.print("🌡 Nhiệt độ: ");
    Serial.print(t);
    Serial.print("°C, 💧 Độ ẩm không khí: ");
    Serial.print(h);
    Serial.println("%");
  } else {
    Serial.println("⚠️ Lỗi đọc DHT22");
  }

  // Đọc cảm biến gas hoặc đất (tuỳ mục đích)
  int analogValue = analogRead(GAS_PIN);
  Serial.print("📟 Giá trị analog (Gas/Đất): ");
  Serial.println(analogValue);

  // --- Xử lý nút nhấn ---
  if (buttonPressed && !buttonWasPressed) {
    buttonPressStart = millis();
    buttonWasPressed = true;
    testModeActivated = false;
  } else if (!buttonPressed && buttonWasPressed) {
    unsigned long pressDuration = millis() - buttonPressStart;
    buttonWasPressed = false;

    if (pressDuration < 2000) {
      digitalWrite(BUZZER_PIN, BUZZER_OFF);
      digitalWrite(RELAY_PIN, LOW);
      digitalWrite(LED_PIN, LED_OFF);
      alarmActive = false;
      buzzerSilenced = false;
      gasLowStartTime = 0;

      Serial.println("🔴 Nhấn ngắn -> Tắt tất cả!");
    }
  }

  // Chế độ TEST
  if (buttonPressed && buttonWasPressed && !testModeActivated) {
    if (millis() - buttonPressStart >= 2000) {
      Serial.println("🧪 Chế độ TEST hệ thống!");
      digitalWrite(RELAY_PIN, HIGH);
      digitalWrite(LED_PIN, LED_ON);
      digitalWrite(BUZZER_PIN, BUZZER_ON);
      delay(3000);
      digitalWrite(RELAY_PIN, LOW);
      digitalWrite(LED_PIN, LED_OFF);
      digitalWrite(BUZZER_PIN, BUZZER_OFF);
      testModeActivated = true;
    }
  }

  // --- Điều kiện cảnh báo khí gas ---
  if (analogValue > 300) {
    Serial.println("🔥 Phát hiện khí gas hoặc đất quá khô!");
    digitalWrite(RELAY_PIN, HIGH);
    digitalWrite(LED_PIN, LED_ON);
if (!buzzerSilenced) digitalWrite(BUZZER_PIN, BUZZER_ON);
    alarmActive = true;
    gasLowStartTime = 0;
  } else {
    if (alarmActive) {
      if (gasLowStartTime == 0) gasLowStartTime = millis();

      if (millis() - gasLowStartTime >= 4000) {
        Serial.println("✅ Đã ổn định. Tắt cảnh báo.");
        digitalWrite(BUZZER_PIN, BUZZER_OFF);
        digitalWrite(RELAY_PIN, LOW);
        digitalWrite(LED_PIN, LED_OFF);
        alarmActive = false;
        buzzerSilenced = false;
      }
    } else {
      digitalWrite(LED_PIN, LED_OFF);
    }x1
  }

  delay(1000); // Chờ để đọc DHT & analog ổn định
}