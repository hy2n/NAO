/* NAO : SOURCE
   CAM1 
    ID 1: 검정색 라인
    ID 2: 빨간색 라인
    ID 3: 보행자

   CAM2
    ID 1: 표지판 1
    ID 2: 표지판 2
    ID 3: 표지판 3
    ID 4: 차량

*/
#include "Wire.h"
#include "HUSKYLENS.h"
#include "SoftwareSerial.h"

HUSKYLENS huskylens;


SoftwareSerial CAM1(2, 3);  // RX, TX
int DefaultSpeed = 150;

int MotorL_A = A0;
int MotorL_B = A1;
int MotorR_A = A2;
int MotorR_B = A3;

//Line
int Line_select = 0;                                //선택된 라인
int Line_count = 0;                                 //감지된 라인
int Line_X[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };  //X값변수
int Line_Y[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };  //Y값변수
int Line_Height[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int Line_Width[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int X_LR = 0;        //왼쪽오른쪽구분
int X_root = 0;      //양모서리거리값
int X_io = 0;        //서보회전값
int X_MinValue = 0;  //차선허용범위 (양옆값)
int X_notTurn = 1;   //90도 회전 여부

//Block 1
int X_root_D = 1;
int ObjectnotDected = 1;
int RedLine = 1;
int decOBJ = 1;  // Scan()함수로 발견한거 리턴값
int trigger = 1;

void printResult(HUSKYLENSResult result);

void setup() {
  Serial.begin(9600);
  CAM1.begin(9600);
  while (!huskylens.begin(CAM1)) {
    Serial.println(F("Begin failed! - CAM 1"));
    Serial.println(F("1.Please recheck the \"Protocol Type\" in HUSKYLENS (General Settings>>Protocol Type>>Serial 9600)"));
    Serial.println(F("2.Please recheck the connection."));
    delay(100);
  }
  pinMode(MotorL_A, OUTPUT);  // 5번핀을 출력모드로 설정합니다.
  pinMode(MotorL_B, OUTPUT);  // 6번핀을 출력모드로 설정합니다.
  pinMode(MotorR_A, OUTPUT);  // 5번핀을 출력모드로 설정합니다.
  pinMode(MotorR_B, OUTPUT);  // 6번핀을 출력모드로 설정합니다.
  pinMode(9, OUTPUT);

  pinMode(10, INPUT);
}

void loop() {
  trigger = 1;
  long duration, distance;
  while (trigger) {
    digitalWrite(9, LOW);
    delayMicroseconds(2);
    digitalWrite(9, HIGH);
    delayMicroseconds(10);
    digitalWrite(9, LOW);
    duration = pulseIn(10, HIGH);
    distance = duration * 17 / 1000;
    delay(500);
    Serial.println(distance);
    if (distance < 10) {
      delay(500);
      digitalWrite(9, LOW);
      delayMicroseconds(2);
      digitalWrite(9, HIGH);
      delayMicroseconds(10);
      digitalWrite(9, LOW);
      duration = pulseIn(10, HIGH);
      distance = duration * 17 / 1000;
      if (distance < 10) {
        trigger = 0;
        tone(11,2000);
        delay(1000);
        noTone(11);
      }
    }
  }
  RedLine = 1;
  while (RedLine) {
    ScanTurn();
    if (RedLine) LineTrace(150);
  }
  Stop();
  delay(1000);
  TurnLeft(170);
  delay(700);
  Stop();
  delay(500);

  for (int i = 0; i < 40; i++) {  //2초정도 그냥 자율주행
    LineTrace(150);
  }

  RedLine = 1;
  while (RedLine) {
    ScanTurn();
    if (RedLine) LineTrace(200);
  }
  Serial.println("Stop");
  Stop();
  delay(1000);


  delay(6000);
  Forward(150);

  for (int i = 0; i < 70; i++) {  //2초정도 그냥 자율주행
    LineTrace(180);
  }

  RedLine = 1;
  while (RedLine) {
    ScanTurn();
    if (RedLine) LineTrace(180);
  } //감속구간시작

  Stop();
  tone(11,3000);
  delay(5000);
  noTone(11);

  for (int i = 0; i < 20; i++) {  //2초정도 그냥 자율주행
    LineTrace_slow(150);
  }
  RedLine = 1;
  while (RedLine) {
    ScanTurn();
    if (RedLine) LineTrace(150);
  } //감속구간해제

  for (int i = 0; i < 50; i++) {  //2초정도 그냥 자율주행
    LineTrace_slow(150);
  }

  trigger = 1;
  while (trigger) {  //사람 발견 전까지
    ScanCAM1(3);
    if (decOBJ == 1) {  //발견된 물체가 (보행자)라면
      Stop();           //모터정지
      delay(100);
      trigger = 0;        //반복 해제
      tone(11,3000);
      WaitObjectCAM1(3);  //물체가 사라질 때까지 대기
      delay(5000);
      noTone(11);
    } else {
      LineTrace(150);
    }
  }  //주행 중 보행자 확인하는 코드

  for (int i = 0; i < 70; i++) {  //2초정도 그냥 자율주행
    LineTrace(150);
  }


  while (RedLine) {
    ScanTurn();
    if (RedLine) LineTrace(150);
  } //끝나는 지점까지 직진
  Stop();
  delay(1000);
  TurnLeft(170);
  delay(700);
  Stop();
  delay(500);//신호등 앞에서 회전
  for (int i = 0; i < 20; i++) {  //2초정도 그냥 자율주행
    LineTrace_slow(150);
  }

  while (RedLine) {
    ScanTurn();
    if (RedLine) LineTrace(150);
  } //횡단보도 만났을때

  //횡단보도 코드
  RedLine = 1;
  while (RedLine) {
    ScanTurn();
    if (RedLine) LineTrace(150);
  } //감속구간해제
  Stop();
  delay(3000);
  WaitObjectCAM1(3);  //물체가 사라질 때까지 대기
  delay(3000);
   for (int i = 0; i < 22; i++) {  //2초정도 그냥 자율주행
    LineTrace_slow(150);
  }

  while (RedLine) {
    ScanTurn();
    if (RedLine) LineTrace(150);
  } //끝지점
  tone(11, 3000);
  delay(5000);
  noTone(11);
  while(1) {
    Stop();
  }
}

void Forward(int speed) {
  analogWrite(MotorL_A, 0);
  analogWrite(MotorL_B, speed);
  analogWrite(MotorR_A, 0);
  analogWrite(MotorR_B, speed);
}

void Right(int speed) {
  analogWrite(MotorL_A, 0);
  analogWrite(MotorL_B, speed);
  analogWrite(MotorR_A, 0);
  analogWrite(MotorR_B, 0);
}

void Left(int speed) {
  analogWrite(MotorL_A, 0);
  analogWrite(MotorL_B, 0);
  analogWrite(MotorR_A, 0);
  analogWrite(MotorR_B, speed);
}

void SRight(int speed, int x_speed) {
  if (1) {
    analogWrite(MotorL_A, 0);
    analogWrite(MotorL_B, speed);
    analogWrite(MotorR_A, 0);
    analogWrite(MotorR_B, 0);
    Serial.println("R_B");
  } else {
    Serial.println("R_M");
    analogWrite(MotorL_A, 0);
    analogWrite(MotorL_B, speed);
    analogWrite(MotorR_A, 0);
    analogWrite(MotorR_B, 130);
  }
}

void SLeft(int speed, int x_speed) {
  if (1) {
    analogWrite(MotorL_A, 0);
    analogWrite(MotorL_B, 0);
    analogWrite(MotorR_A, 0);
    analogWrite(MotorR_B, speed);
    Serial.println("L_B");
  } else {
    Serial.println("L_M");
    analogWrite(MotorL_A, 0);
    analogWrite(MotorL_B, 130);
    analogWrite(MotorR_A, 0);
    analogWrite(MotorR_B, speed);
  }
}

void TurnLeft(int speed) {
  analogWrite(MotorL_A, speed);
  analogWrite(MotorL_B, 0);
  analogWrite(MotorR_A, 0);
  analogWrite(MotorR_B, speed);
}

void TurnRight(int speed) {
  analogWrite(MotorL_A, 0);
  analogWrite(MotorL_B, speed);
  analogWrite(MotorR_A, speed);
  analogWrite(MotorR_B, 0);
}

void Stop() {
  analogWrite(MotorL_A, 0);
  analogWrite(MotorL_B, 0);
  analogWrite(MotorR_A, 0);
  analogWrite(MotorR_B, 0);
}

void ScanBlock(int id) {
  if (huskylens.request(id)) {
    Line_count = huskylens.count(id);
    int i = 1;
    while (i < (huskylens.count(id) + 1)) {
      HUSKYLENSResult result = huskylens.get(id, (i - 1));
      Line_X[i] = result.xCenter;  //변수에 값 저장
      Line_Y[i] = result.yCenter;
      Line_Width[i] = result.width;
      Line_Height[i] = result.height;
      i++;
    }
  } else {
    Line_count = 0;
  }
}

void ScanBlockCAM2(int id) {
  if (huskylens2.request(id)) {
    Line_count = huskylens2.count(id);
    int i = 1;
    while (i < (huskylens2.count(id) + 1)) {
      HUSKYLENSResult result2 = huskylens2.get(id, (i - 1));
      Line_X[i] = result2.xCenter;  //변수에 값 저장
      Line_Y[i] = result2.yCenter;
      Line_Width[i] = result2.width;
      Line_Height[i] = result2.height;
      i++;
    }
  } else {
    Line_count = 0;
  }
}


int ScanTurn() {
  ScanBlock(2);
  if (Line_count > 0) {
    if (Line_count == 1) Line_select = 1;                        // 1개 이상의 객체가 발견돼면
    else if (Line_count == 2) Line_select = DefineObject(1, 2);  //2개 발견하면 일반 Define해서 선택
    else Line_select = AdvancedDefine();                         //3개이상 발견하면 고급 Define해서 선택
    if ((Line_Y[Line_select] > 160)) {
      RedLine = 0;
    } else {
      RedLine = 1;
    }
  } else {
    RedLine = 1;
  }
}

int ScanCAM2(int objID) {
  ScanBlockCAM2(objID);  //2번 ID 물체 검색
  if (Line_count > 0) {
    if (Line_count == 1) Line_select = 1;                                     // 1개 이상의 객체가 발견돼면
    else if (Line_count == 2) Line_select = DefineObject(1, 2);               //2개 발견하면 일반 Define해서 선택
    else Line_select = AdvancedDefine();                                      //3개이상 발견하면 고급 Define해서 선택
    if ((Line_Width[Line_select] < 40) && (Line_Height[Line_select] < 40)) {  //너무 작으면 패스
      decOBJ = 0;                                                             //미발견 (대상 물체 너무 작음)
    } else {
      decOBJ = Line_select;
    }
  } else {
    decOBJ = 0;
  }
}

int ScanCAM1(int objID) {
  ScanBlock(objID);  //물체 검색
  if (Line_count > 0) {
    if (Line_count == 1) Line_select = 1;                                     // 1개 이상의 객체가 발견돼면
    else if (Line_count == 2) Line_select = DefineObject(1, 2);               //2개 발견하면 일반 Define해서 선택
    else Line_select = AdvancedDefine();                                      //3개이상 발견하면 고급 Define해서 선택
    if ((Line_Width[Line_select] < 50) && (Line_Height[Line_select] < 50)) {  //너무 작으면 패스
      decOBJ = 0;                                                             //미발견 (대상 물체 너무 작음)
    } else {
      decOBJ = Line_select;
    }
  } else {
    decOBJ = 0;
  }
}

void LineTrace(int speed) {
  ScanBlock(1);
  if (Line_count > 1) {                                     // 1개 이상의 객체가 발견돼면
    if (Line_count == 2) Line_select = DefineObject(1, 2);  //2개 발견하면 일반 Define해서 선택
    else Line_select = AdvancedDefine();                    //3개이상 발견하면 고급 Define해서 선택
  } else {
    Line_select = 1;  //1개 발견시 그거로 선택
  }
  X_range_define(Line_select);

  X_io = X_root * 0.7;  //계산값
  X_io = (int)X_io;
  if (Line_count == 0) Forward(speed);
  else if (X_root > 20) {
    if (X_LR == 1) {
      SRight(130, X_io);
    } else {
      SLeft(130, X_io);
    }
  }
}

void LineTrace_slow(int speed) {
  ScanBlock(1);
  if (Line_count > 1) {                                     // 1개 이상의 객체가 발견돼면
    if (Line_count == 2) Line_select = DefineObject(1, 2);  //2개 발견하면 일반 Define해서 선택
    else Line_select = AdvancedDefine();                    //3개이상 발견하면 고급 Define해서 선택
  } else {
    Line_select = 1;  //1개 발견시 그거로 선택
  }
  X_range_define(Line_select);

  X_io = X_root * 0.7;  //계산값
  X_io = (int)X_io;
  if (Line_count == 0) Forward(speed);
  else if (X_root > 10) {
    if (X_LR == 1) {
      SRight(130, X_io);
    } else {
      SLeft(130, X_io);
    }
  }
}

int DefineObject(int object1, int object2) {
  if (((Line_Width[object1] < 100) || (Line_Width[object2] < 100)) && ((Line_Height[object1] < 60) || (Line_Height[object2] < 60))) {
  } else if (Line_Y[object1] > Line_Y[object2]) return object1;
  else if (Line_Y[object1] < Line_Y[object2]) return object2;
}

int X_range_define(int objID) {
  if (Line_X[objID] > 160) {
    X_root = 320 - Line_X[objID];
    X_LR = 2;
  }  // 오른쪽 축 계산
  else {
    X_root = Line_X[objID];
    X_LR = 1;
  }  //왼쪽 축 계산
}

int AdvancedDefine() {  //가장 큰 오브젝트로 선택한다
  int k = 1;
  int max = 1;
  int maxID = 1;
  while (k < (Line_count + 1)) {
    if ((Line_Width[k] < 100) && (Line_Height[k] < 60)) {  //너무 작으면 패스

    } else if (Line_Width[k] > max) {
      max = Line_Width[k];
      maxID = k;
    }
    k++;
  }
  return maxID;
}

void WaitObjectCAM1(int objID) {  // objID이름의 오브젝트 대기
  ObjectnotDected = 1;            //초기화
  while (ObjectnotDected)
    if (huskylens.request(objID))
      if (huskylens.count() < 1) {  //오브젝트가 없다면
        delay(500);                 //감지시 정확성을 위해 5초후 재검증
        if (huskylens.request(objID))
          if (huskylens.count() < 1) ObjectnotDected = 0;
      }
}

void WaitObjectCAM2(int objID) {  // objID이름의 오브젝트 대기
  ObjectnotDected = 1;            //초기화
  while (ObjectnotDected)
    if (huskylens2.request(objID))
      if (huskylens2.count() < 1) {  //오브젝트가 없다면
        delay(1000);                 //감지시 정확성을 위해 5초후 재검증
        if (huskylens2.request(objID))
          if (huskylens2.count() < 1) ObjectnotDected = 0;
      }
}
