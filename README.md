# Real-time kernel for Arduino UNO R3
Arduino UNO R3上で実用的に動作し、Arduino IDE環境で使えるリアルタイム・カーネルです。  
コードサイズ 644 Byte でプリエンプティブなマルチタスク環境を実現します。  
A real-time kernel that runs practically on the Arduino UNO R3 and can be used in the Arduino  
IDE environment. With a code size of 644 Bytes, it enables a preemptive multitasking environment.  

## 目次
- [概要](#概要)
- [利用方法](#利用方法)
- [ライセンス](#ライセンス)

## 概要
Arduino UNO R3上で実用的に動作し、Arduino IDE環境で使えるリアルタイム・カーネルです。  
リアルタイム・カーネルは、各タスクに設定した優先順位により、各タスクを停止 (STOP)、実行 (RUN)、  
実行可能 (READY)、中断 (SUSPEND)の4つのステータスで管理し、イベントにより起動を要求された  
タスクの中で、最も優先順位の高いタスクに実行権を与えタスクを切り替えます。

このリアルタイム・カーネルは、優先制御のオーバーヘッドと消費するROM/RAMを最小としながら、  
プリエンプティブなマルチタスク環境を実現します。

- リアルタイムカーネルのコードサイズ：644 Byte
- スタック：タスクの起動時に6Byteを追加で使用。  
  スタックは１本です。タスク毎にスタックを確保する必要はありません。
- スタック以外：タスク数(TASK_MAX)×7+6 Byte  
  タスク毎にTCBの7ByteとSCBの6Byte
- タスク切り替え時間  
  Arduino IDEの micros() でタスクの切り替え時間を計測：8～16μsec。

このリアルタイム・カーネルでは、優先順位が最も高いタスクが下位のタスクに一時的に実行権を渡す  
WAIT 状態はサポートしていません。この機能制限により、タスク毎にスタックを準備する必要がなくなり、  
最低限のレジスタの退避のみでタスクの切り替えを実現します。

For more information
[#1](https://pekopoko4control.blogspot.com/2024/09/arduino-uno.html)
[#2](https://pekopoko4control.blogspot.com/2024/10/for-arduino-uno-r3.html)

![dispatch]()

## 利用方法
下記の2つのソースファイルを Arduino IDE のフォルダに置けば Arduino IDE で利用可能です。  
　RTK4ArduinoUnoR3.ino  
　example2.ino

RTK4ArduinoUnoR3.ino はリアルタイム・カーネルのコードです。  
example2.ino はアプリケーションの例です。  
このアプリケーションでは、タイマー割り込みから3つ、バックグラウンドのタスクを1つ起動しています。

コードの先頭で、リアルタイム・カーネルの2つの関数をプロトタイプ宣言しています。  
　void task_sw(unsigned char no);	// タスク起動要求  
　void task_create(void(*task)(void), unsigned char id, unsigned char level); // タスク生成

次にタスクの定義数と、タスクIDを定義しています。タスクIDは 0～(TASK_MAX-1)の数値です。  
 　#define  TASK_MAX  4  
 
 　#define  taskId_10ms	   0   
 　#define  taskId_100ms	  1   
 　#define  taskId_1s	    	2  
 　#define  taskId_bg   	 	3  

setup()で、task_create( 関数名, Task ID, 優先順位 ) によりタスクを生成します。  
タスクは関数で定義し、優先順位の値が大きい方が優先順位が高くなります。このアプリケーションの例では、  
優先順位を  
　task_10ms > task_100ms > task_1s > task_bg  
としています。  

　task_create(task_10ms, taskId_10ms, 8);  
　task_create(task_100ms, taskId_100ms, 6);  
　task_create(task_1s, taskId_1s, 4);  
　task_create(task_bg, taskId_bg, 2);  

次に必要なタイミングで task_sw(Task ID) で各タスクを起動します。  
リアルタイム・カーネルの優先制御により、優先順位に従った多重処理(マルチタスク)が実施されます。  

## ライセンス
このソフトウエアはMITライセンスの下でライセンスされます。詳細は[LICENSE](https://github.com/pekopoko-heart/RTKernel-for-Arduino-Uno-R3/blob/main/LISENCE.txt)ファイルをご覧ください。

This software is licensed under the MIT License, see the LICENSE.txt file for details
