#include <Servo.h>
#include <Stepper.h>

#define SW 8

const int stepsPerRevolution = 2048;
Stepper myStepper(stepsPerRevolution, 9, 11, 10, 12);

Servo myServo;

// ===== BIẾN =====
String data = "";
bool isBusy = false;
bool atHome = false;

int currentStep = 0;

// ===== VỊ TRÍ =====
int pos_90  = 512;
int pos_180 = 1024;
int pos_270 = 1536;

void setup() {
  Serial.begin(9600);

  pinMode(SW, INPUT_PULLUP);
  myStepper.setSpeed(12);

  myServo.attach(3);
  myServo.write(0);

  Serial.println("READY");
}

void loop() {
  readSerial();
}

//////////////////////////////////////////////////
// ===== ĐỌC SERIAL =====
//////////////////////////////////////////////////
void readSerial() {
  while (Serial.available()) {
    char c = Serial.read();

    if (c == '\n') {
      data.trim();

      Serial.print("Nhan: [");
      Serial.print(data);
      Serial.println("]");

      handleCommand(data);
      data = "";
    } else {
      data += c;
    }
  }
}

//////////////////////////////////////////////////
// ===== XỬ LÝ =====
//////////////////////////////////////////////////
void handleCommand(String cmd) {

  cmd.trim();

  if (isBusy) return;

  // ===== CÓ VẬT =====
  if (cmd.indexOf("Aluminum") >= 0) {
    Serial.println("→ CAN");
    isBusy = true;
    atHome = false;
    runProcess(pos_90);
    isBusy = false;
  }
  else if (cmd.indexOf("Plastic") >= 0) {
    Serial.println("→ PLASTIC");
    isBusy = true;
    atHome = false;
    runProcess(pos_180);
    isBusy = false;
  }
  else if (cmd.indexOf("Cardboard") >= 0) {
    Serial.println("→ CARDBOARD");
    isBusy = true;
    atHome = false;
    runProcess(pos_270);
    isBusy = false;
  }

  // ===== KHÔNG CÓ VẬT =====
  else if (cmd.indexOf("NONE") >= 0) {

    if (!atHome) {
      Serial.println("→ VỀ LIMIT");
      goHome();
      atHome = true;
    }
  }
}

//////////////////////////////////////////////////
// ===== PROCESS =====
//////////////////////////////////////////////////
void runProcess(int pos) {
  moveTo(pos);
  actionServo();
  // ❌ KHÔNG về home
}

//////////////////////////////////////////////////
// ===== QUAY =====
//////////////////////////////////////////////////
void moveTo(int target) {
  Serial.println("MOVE");

  int diff = target - currentStep;
  myStepper.step(diff);

  currentStep = target;
}

//////////////////////////////////////////////////
// ===== VỀ LIMIT =====
//////////////////////////////////////////////////
void goHome() {
  Serial.println("GO HOME");

  while (digitalRead(SW) == HIGH) {
    myStepper.step(-5);
  }

  currentStep = 0;
}

//////////////////////////////////////////////////
// ===== SERVO =====
//////////////////////////////////////////////////
void actionServo() {
  Serial.println("SERVO");

  myServo.write(120);
  delay(2000);
  myServo.write(0);
}