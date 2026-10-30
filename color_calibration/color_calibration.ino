#include <Arduino.h>

// Our connections: Port 3 A3 -> decoder A, Port 3 A2 -> decoder B,
// Port 4 S2 (A1) <- LDR. (A0 is the IR detector, D12 is the ultrasonic)
const uint8_t SELECT_A_PIN = A3;
const uint8_t SELECT_B_PIN = A2;
const uint8_t LDR_PIN = A1;

// Start with 500 ms. Increase if the LDR has not settled.
const unsigned long SETTLE_MS = 500;
const uint8_t SAMPLE_COUNT = 20;

// Array order throughout this sketch: RED, GREEN, BLUE.
// Your circuit connects red to Y3, green to Y1, blue to Y2.
const uint8_t RGB_OUTPUTS[3] = {3, 1, 2};

float whiteRef[3] = {0, 0, 0};
float blackRef[3] = {0, 0, 0};

bool haveWhite = false;
bool haveBlack = false;

// Select one active-low decoder output.
// Output 0 selects Y0 (IR emitter in our circuit), so no RGB LED is on.
void selectOutput(uint8_t output) {
  digitalWrite(SELECT_A_PIN, LOW);
  digitalWrite(SELECT_B_PIN, LOW);

  digitalWrite(SELECT_B_PIN, (output & 2) ? HIGH : LOW);
  digitalWrite(SELECT_A_PIN, (output & 1) ? HIGH : LOW);
}

// Illuminate the surface with each LED and measure the LDR.
void scanRGB(float values[3]) {
  selectOutput(0);
  delay(SETTLE_MS);

  for (uint8_t channel = 0; channel < 3; channel++) {
    selectOutput(RGB_OUTPUTS[channel]);
    delay(SETTLE_MS);

    analogRead(LDR_PIN);  // Discard first conversion.

    unsigned long total = 0;

    for (uint8_t sample = 0; sample < SAMPLE_COUNT; sample++) {
      total += analogRead(LDR_PIN);
      delay(5);
    }

    values[channel] = total / float(SAMPLE_COUNT);
  }

  selectOutput(0);
}

void printRGB(const float values[3]) {
  Serial.print(F("R="));
  Serial.print(values[0], 1);
  Serial.print(F("  G="));
  Serial.print(values[1], 1);
  Serial.print(F("  B="));
  Serial.println(values[2], 1);
}

bool calibrationValid() {
  if (!haveWhite || !haveBlack) {
    return false;
  }

  for (uint8_t channel = 0; channel < 3; channel++) {
    if (whiteRef[channel] <= blackRef[channel]) {
      return false;
    }
  }

  return true;
}

void printReferences() {
  Serial.print(F("WHITE: "));
  if (haveWhite) {
    printRGB(whiteRef);
  } else {
    Serial.println(F("not captured"));
  }

  Serial.print(F("BLACK: "));
  if (haveBlack) {
    printRGB(blackRef);
  } else {
    Serial.println(F("not captured"));
  }

  if (!haveWhite || !haveBlack) {
    Serial.println(F("Capture both references using w and b."));
    return;
  }

  if (!calibrationValid()) {
    Serial.println(F("INVALID calibration."));
    Serial.println(F("Each white reading must exceed its black reading."));
    Serial.println(F("Check wiring, LED selection and sensor position."));
    return;
  }

  for (uint8_t channel = 0; channel < 3; channel++) {
    if (whiteRef[channel] - blackRef[channel] < 20.0f) {
      Serial.println(F("Small reference difference: check repeatability."));
    }
  }

  Serial.println(F("Calibration ready. Send r to read a surface."));
}

void readSurface() {
  float raw[3];
  scanRGB(raw);

  Serial.print(F("RAW: "));
  printRGB(raw);

  if (!calibrationValid()) {
    Serial.println(F("Capture valid white and black references first."));
    return;
  }

  float calibrated[3];

  for (uint8_t channel = 0; channel < 3; channel++) {
    float score =
      255.0f * (raw[channel] - blackRef[channel]) /
      (whiteRef[channel] - blackRef[channel]);

    calibrated[channel] = constrain(score, 0.0f, 255.0f);
  }

  Serial.print(F("RELATIVE 0-255: "));
  printRGB(calibrated);
}

void setup() {
  // Stop the original mCore's motor drivers during calibration.
  digitalWrite(5, LOW);
  pinMode(5, OUTPUT);
  digitalWrite(6, LOW);
  pinMode(6, OUTPUT);

  digitalWrite(SELECT_A_PIN, LOW);
  pinMode(SELECT_A_PIN, OUTPUT);
  digitalWrite(SELECT_B_PIN, LOW);
  pinMode(SELECT_B_PIN, OUTPUT);

  pinMode(LDR_PIN, INPUT);  // Do not use INPUT_PULLUP.
  analogReference(DEFAULT);
  selectOutput(0);

  Serial.begin(115200);
  Serial.println(F("LDR colour sensor calibration"));
  Serial.println(F("w = capture white"));
  Serial.println(F("b = capture black"));
  Serial.println(F("r = read surface"));
  Serial.println(F("p = print references"));
}

void loop() {
  if (!Serial.available()) {
    return;
  }

  char command = Serial.read();

  switch (command) {
    case 'w':
    case 'W':
      Serial.println(F("Measuring WHITE. Keep the surface still..."));
      scanRGB(whiteRef);
      haveWhite = true;
      printReferences();
      break;

    case 'b':
    case 'B':
      Serial.println(F("Measuring BLACK. Keep the surface still..."));
      scanRGB(blackRef);
      haveBlack = true;
      printReferences();
      break;

    case 'r':
    case 'R':
      readSurface();
      break;

    case 'p':
    case 'P':
      printReferences();
      break;

    default:
      break;  // Ignore spaces and line endings.
  }
}
