# Real-time kernel for Arduino UNO R3

A real-time kernel that runs practically on the Arduino UNO R3 and can be used in the Arduino  
IDE environment. With a code size of 644 Bytes, it enables a preemptive multitasking environment.  
(Arduino UNO R3上で実用的に動作し、Arduino IDE環境で使えるリアルタイム・カーネルです。  
コードサイズ 644 Byte でプリエンプティブなマルチタスク環境を実現します。)  

## 目次
- [概要](#概要)
- [利用方法](#利用方法)
- [ライセンス](#ライセンス)

## 概要

This real-time kernel runs practically on Arduino UNO R3 and can be used in the Arduino IDE  
environment. Depending on the priority set, each task is managed with 4 statuses: stopped (STOP),   
running (RUN), ready to run (READY), and suspended (SUSPEND).  
The real-time kernel grants execution privilege to the task with the highest priority among those  
that have requested to be started.  
(Arduino UNO R3上で実用的に動作し、Arduino IDE環境で使えるリアルタイム・カーネルです。  
リアルタイム・カーネルは、各タスクに設定した優先順位により、各タスクを停止 (STOP)、実行 (RUN)、  
実行可能 (READY)、中断 (SUSPEND)の4つのステータスで管理し、イベントにより起動を要求された  
タスクの中で、最も優先順位の高いタスクに実行権を与えタスクを切り替えます。)  

This real-time kernel provides a preemptive multitasking environment. And it has minimal priority  
control overhead and consumes minimal ROM/RAM size.  
(このリアルタイム・カーネルは、優先制御のオーバーヘッドと消費するROM/RAMを最小としながら、  
プリエンプティブなマルチタスク環境を実現します。)  

- Real-time kernel code size: 644 Bytes
- Stack: 6Byte additional at startup of task.  
  Only one stack is required. There is no need to reserve a stack for each task.
- Non-stack: Number of tasks (TASK_MAX)×7+6 Byte  
  7Byte of TCB per task and 6Byte of SCB  
- Task switching time  
  Measure task switching time with micros() in Arduino IDE: 8 to 16 μsec.  

This real-time kernel does not support the WAIT state. WAIT is a state in which the highest-priority  
task temporarily passes execution privilege to a lower-priority task.  
This limitation eliminates the need for a dedicated stack for each task.  
Therefore, this real-time kernel achieves task switching with only minimal register saving.  
(このリアルタイム・カーネルでは、優先順位が最も高いタスクが下位のタスクに一時的に実行権を渡す  
WAIT 状態はサポートしていません。この機能制限により、タスク毎にスタックを準備する必要がなくなり、  
最低限のレジスタの退避のみでタスクの切り替えを実現します。)  

For more information
[#1](https://pekopoko4control.blogspot.com/2024/09/arduino-uno.html)
[#2](https://pekopoko4control.blogspot.com/2024/10/for-arduino-uno-r3.html)

## Dispatch Sequence
![dispatch](https://github.com/pekopoko-heart/Real-Time-Kernel-for-Arduino-Uno-R3/blob/main/dispatch.png)

## 利用方法
The following two source files are available in the Arduino IDE environment.  
(下記の2つのソースファイルは Arduino IDE で利用可能です。)  
&nbsp;&nbsp;RTK4ArduinoUnoR3.ino  
&nbsp;&nbsp;example2.ino

RTK4ArduinoUnoR3.ino is the real-time kernel code.  
example2.ino is an example application.  
(RTK4ArduinoUnoR3.ino はリアルタイム・カーネルのコードです。  
example2.ino はアプリケーションの例です。)  

The application launches three tasks from timer interrupts and one background task.  
(このアプリケーションでは、タイマー割り込みから3つ、バックグラウンドのタスクを1つ起動しています。)  

At the beginning of the code, two functions of the real-time kernel are declared as prototypes.  
(コードの先頭で、リアルタイム・カーネルの2つの関数をプロトタイプ宣言しています。)  

　void task_sw(unsigned char no);	// タスク起動要求  
　void task_create(void(*task)(void), unsigned char id, unsigned char level); // タスク生成

Next, define the number of tasks and the task ID. The task ID is a number from 0 to (TASK_MAX-1).  
(次にタスクの定義数と、タスクIDを定義しています。タスクIDは 0～(TASK_MAX-1)の数値です。)  

 　#define  TASK_MAX  4  
 
 　#define  taskId_10ms	   0   
 　#define  taskId_100ms	  1   
 　#define  taskId_1s	    	2  
 　#define  taskId_bg   	 	3  

In setup(), tasks are created by task_create( function name, Task ID, priority ).   
(setup()で、task_create( 関数名, Task ID, 優先順位 ) によりタスクを生成します。)

　task_create(task_10ms, taskId_10ms, 8);  
　task_create(task_100ms, taskId_100ms, 6);  
　task_create(task_1s, taskId_1s, 4);  
　task_create(task_bg, taskId_bg, 2);  

Tasks are defined by functions, and the higher the value, the higher the priority.  
(タスクは関数で定義し、優先順位の値が大きい方が優先順位が高くなります。)  
The priority of this application is,(このアプリケーションの優先順位は)  
　task_10ms > task_100ms > task_1s > task_bg  

Then, at the necessary timing, such as interrupt processing,   
Simply start each task by  
(次に割り込み処理など、必要なタイミングで各タスクを起動するだけです。)  

  **task_sw(Task ID);**  

Just this, Multitasking is performed under the priority control of the real-time kernel.   
これで、リアルタイム・カーネルの優先制御により優先順位に従った多重処理(マルチタスク)が実施されます。  

## ライセンス
This software is licensed under the MIT License, see the [LICENSE.txt](https://github.com/pekopoko-heart/RTKernel-for-Arduino-Uno-R3/blob/main/LISENCE.txt) file for details.  
このソフトウエアはMITライセンスの下でライセンスされます。詳細は LICENSE.txt をご覧ください。  
