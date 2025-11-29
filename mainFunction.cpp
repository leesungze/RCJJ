#include <Arduino.h>
#include <Wire.h>
#include <Adafruit_MCP23X17.h>
#include <Adafruit_NeoPixel.h>

// ====== 핀 설정 ======
#define I2C_SDA_PIN  18
#define I2C_SCL_PIN  17

#define MCP_ADDR     0x20
#define MCP_IR_PIN   8   // GPB0

#define LED_PIN      48
#define LED_COUNT    1

Adafruit_MCP23X17 mcp;
Adafruit_NeoPixel strip(LED_COUNT, LED_PIN, NEO_GRB + NEO_KHZ800);

bool lastIrState   = HIGH;
bool ledRedActive  = false;
unsigned long redStartMillis = 0;

unsigned long lastPrintMillis = 0;
const unsigned long PRINT_INTERVAL = 20; // 20ms

void setLedColor(uint8_t r, uint8_t g, uint8_t b) {
  strip.setPixelColor(0, strip.Color(r, g, b));
  strip.show();
}

void setup() {
  Serial.begin(115200);
  delay(500);

  Wire.begin(I2C_SDA_PIN, I2C_SCL_PIN);

  if (!mcp.begin_I2C(MCP_ADDR)) {
    Serial.println("MCP23017 초기화 실패! I2C 배선/주소 확인 필요");
    while (1) delay(1000);
  }

  // B0 핀을 입력으로 사용 (pull-up 없음)
  mcp.pinMode(MCP_IR_PIN, INPUT);

  strip.begin();
  strip.setBrightness(50);
  strip.show();

  setLedColor(0, 150, 0); // 기본 대기: 초록
  Serial.println("시작: 대기 상태 (LED = 초록)");
}

void loop() {
  unsigned long now = millis();

  int irState = mcp.digitalRead(MCP_IR_PIN);

  // HIGH → LOW 하강엣지 감지
  if (lastIrState == HIGH && irState == LOW) {
    Serial.println("IR LOW 감지! LED 빨간색 3초 유지");

    setLedColor(150, 0, 0);
    ledRedActive   = true;
    redStartMillis = now;
  }

  // 3초 뒤 초록 복귀
  if (ledRedActive && (now - redStartMillis >= 3000)) {
    Serial.println("3초 경과 → 초록으로 복귀");
    setLedColor(0, 150, 0);
    ledRedActive = false;
  }

  // === 20ms마다 IR 상태 시리얼 출력 ===
  if (now - lastPrintMillis >= PRINT_INTERVAL) {
    lastPrintMillis = now;

    Serial.print("IR state: ");
    Serial.println(irState == LOW ? "LOW" : "HIGH");
  }

  lastIrState = irState;
}
