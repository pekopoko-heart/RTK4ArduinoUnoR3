///////////////////////////////////////////////
// アプリケーション
///////////////////////////////////////////////
#include <avr/interrupt.h>
#include <avr/io.h>

void task_sw(unsigned char no);			  // タスク起動要求
void task_create(void(*task)(void), unsigned char id, unsigned char level); // タスク生成

volatile unsigned long c10ms,c100ms,c1s;
unsigned long n10ms=0,n100ms=0,n1s=0;

// Taskの最大定義数
#define TASK_MAX  4

// Task IDの定義   0～1(TASK_MAX-1)の数値
#define	taskId_10ms	  0
#define	taskId_100ms	1
#define	taskId_1s	    2
#define	taskId_bg   	3

// Application Task
void  task_10ms(void);       // task 優先順位:8
void  task_100ms(void);      // task 優先順位:6
void  task_1s(void);         // task 優先順位:4
void  task_bg(void);         // task 優先順位:2

///////////////////////////////////////////////
//  Arduino IDE Setup + Loop
///////////////////////////////////////////////

void setup() {
  // put your setup code here, to run once:

  ///////////////////////////////////////////////
  //  Taskを生成
  //
  //  Taskの関数名、ID（定義No.）、優先順位を登録しTaskを生成する。
  //  Taskの優先順位は数値が大きい方が優先順位が高い。
  //
  //  task_create( 関数名, Task ID, 優先順位)
  //  優先順位 task_10ms > task_100ms > task_1s > task_bg
  ///////////////////////////////////////////////
  task_create(task_10ms, taskId_10ms, 8);
  task_create(task_100ms, taskId_100ms, 6);
  task_create(task_1s, taskId_1s, 4);
  task_create(task_bg, taskId_bg, 2);

  // シリアル通信の設定
  Serial.begin(9600);
  while (!Serial);

  ////////////////////////////////////////
  // Set Up Timer1(16bit)
  ////////////////////////////////////////

  // タイマーコンペア初期値を設定 
  OCR1A = 625;	    // Compare match 10ms
  OCR1B = 6250;	    // Compare match 100ms
  // フリーランモードに設定
  TCCR1A = 0;       // Free Run mode
  // 分周比256に設定(16us) タイマースタート
  TCCR1B = 0x04;    // CS12 1/256(16us)
  // コンペアマッチA/Bとオーバーフロー割り込みを許可
  TIMSK1 = 0x07;    // OCIE1A, OCIE1B, TOIE1

  Serial.println("ready to go");
  task_sw(taskId_bg);   // BG TASK起動

}

void loop() {
  // put your main code here, to run repeatedly:
}

///////////////////////////////////////////////
//  ISR
///////////////////////////////////////////////

// TIMER1 COMPA 割り込み処理
ISR(TIMER1_COMPA_vect) {
  // コンペア値を更新 
  OCR1A = OCR1A+625;    // 10msごとに割り込み
  // 10ms処理を起動する
  task_sw(taskId_10ms);
}

// TIMER1 COMPB 割り込み処理
ISR(TIMER1_COMPB_vect) {
  // コンペア値を更新 
  OCR1B = OCR1B+6250;	  // 100msごとに割り込み
  // 100ms処理を起動する
  task_sw(taskId_100ms);
}

// TIMER1 OVF 割り込み処理
ISR(TIMER1_OVF_vect) {    // オーバーフロー1048msごとに割り込み
  // 1s処理を起動する
  task_sw(taskId_1s);
}

///////////////////////////////////////////////
//  Task
///////////////////////////////////////////////

// 10ms処理
void task_10ms(void) {
  unsigned long i;
  for (i=0;i<588;i++) c10ms++;      // 1ms負荷
  n10ms++;
}

// 100ms処理
void task_100ms(void) {
  unsigned long i;
  for (i=0;i<29452;i++) c100ms++;   // 50ms負荷
  n100ms++;
}

// 1s処理
void task_1s(void) {
  unsigned long i;
  for (i=0;i<183520;i++) c1s++;     // 300ms負荷
  n1s++;
}

// BG処理
void task_bg(void) {
  while(1){
    delay(1000);  // 1000ms待機
    Serial.print("Execution Count ");
    Serial.print(n10ms); 
    Serial.print(" ");
    Serial.print(n100ms); 
    Serial.print(" ");
    Serial.println(n1s); 
  }
}

