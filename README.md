# Real-time kernel for Arduino UNO R3

A real-time kernel that runs practically on the Arduino UNO R3 and can be used in the Arduino  
IDE environment. With a code size of 644 Bytes, it enables a preemptive multitasking environment.  
(Arduino UNO R3上で実用的に動作し、Arduino IDE環境で使えるリアルタイム・カーネルです。  
コードサイズ 644 Byte でプリエンプティブなマルチタスク環境を実現します。)  

## Contents (目次)
- [Overview (概要)](https://github.com/pekopoko-heart/Real-Time-Kernel-for-Arduino-Uno-R3/blob/main/README.md#overview%E6%A6%82%E8%A6%81)
- [Dispatch Sequence](https://github.com/pekopoko-heart/Real-Time-Kernel-for-Arduino-Uno-R3/blob/main/README.md#dispatch-sequence)
- [How to use (利用方法)](https://github.com/pekopoko-heart/Real-Time-Kernel-for-Arduino-Uno-R3/blob/main/README.md#how-to-use-%E5%88%A9%E7%94%A8%E6%96%B9%E6%B3%95)
- [Let's check out example2.ino (example2.ino を確認してみよう)](https://github.com/pekopoko-heart/Real-Time-Kernel-for-Arduino-Uno-R3/blob/main/README.md#lets-check-out-example2ino-example2ino-%E3%82%92%E7%A2%BA%E8%AA%8D%E3%81%97%E3%81%A6%E3%81%BF%E3%82%88%E3%81%86)
- [License (ライセンス)](https://github.com/pekopoko-heart/Real-Time-Kernel-for-Arduino-Uno-R3/blob/main/README.md#license-%E3%83%A9%E3%82%A4%E3%82%BB%E3%83%B3%E3%82%B9)

## Overview　(概要)

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

## How to use (利用方法)
The following two source files are available in the Arduino IDE environment.  
(下記の2つのソースファイルは Arduino IDE で利用可能です。)  

&nbsp;&nbsp;&nbsp;&nbsp;RTK4ArduinoUnoR3.ino  
&nbsp;&nbsp;&nbsp;&nbsp;example2.ino

RTK4ArduinoUnoR3.ino is the real-time kernel code.  
example2.ino is an example application.  
(RTK4ArduinoUnoR3.ino はリアルタイム・カーネルのコードです。  
example2.ino はアプリケーションの例です。)  

The application launches three tasks from timer interrupts and one background task.  
(このアプリケーションでは、タイマー割り込みから3つ、バックグラウンドのタスクを1つ起動しています。)  

At the beginning of the code, two functions of the real-time kernel are declared as prototypes.  
(コードの先頭で、リアルタイム・カーネルの2つの関数をプロトタイプ宣言しています。)  

&nbsp;&nbsp;&nbsp;&nbsp;**void task_sw(unsigned char no);**  
&nbsp;&nbsp;&nbsp;&nbsp;**void task_create(void(\*task)(void), unsigned char id, unsigned char level);**

Next, define the number of tasks and the task ID. The task ID is a number from 0 to (TASK_MAX-1).  
(次にタスクの定義数と、タスクIDを定義しています。タスクIDは 0～(TASK_MAX-1)の数値です。)  

&nbsp;&nbsp;&nbsp;&nbsp;**#define  TASK_MAX    4**  
 
&nbsp;&nbsp;&nbsp;&nbsp;**#define  taskId_10ms    0**   
&nbsp;&nbsp;&nbsp;&nbsp;**#define  taskId_100ms   1**   
&nbsp;&nbsp;&nbsp;&nbsp;**#define  taskId_1s      2**  
&nbsp;&nbsp;&nbsp;&nbsp;**#define  taskId_bg      3**  

In setup(), tasks are created by task_create( function name, Task ID, priority ).   
(setup()で、task_create( 関数名, Task ID, 優先順位 ) によりタスクを生成します。)

&nbsp;&nbsp;&nbsp;&nbsp;**task_create(task_10ms, taskId_10ms, 8);**  
&nbsp;&nbsp;&nbsp;&nbsp;**task_create(task_100ms, taskId_100ms, 6);**  
&nbsp;&nbsp;&nbsp;&nbsp;**task_create(task_1s, taskId_1s, 4);**  
&nbsp;&nbsp;&nbsp;&nbsp;**task_create(task_bg, taskId_bg, 2);**  

Tasks are defined by functions, and the larger the priority value, the higher the priority.  
(タスクは関数で定義し、優先順位の値が大きい方が優先順位が高くなります。)  
The priority of this application is, (このアプリケーションの優先順位は)  
&nbsp;&nbsp;&nbsp;&nbsp;task_10ms > task_100ms > task_1s > task_bg  

Then, at the necessary timing, such as interrupt processing, Simply start each task by  
(次に割り込み処理など、必要なタイミングで各タスクを起動するだけです。)  

&nbsp;&nbsp;&nbsp;&nbsp;**task_sw(Task ID);**  

**Just this, Multitasking is performed under the priority control of the real-time kernel.**   
(これで、リアルタイム・カーネルの優先制御により優先順位に従った多重処理(マルチタスク)が実施されます。)  

## Let's check out example2.ino (example2.ino を確認してみよう)
Preemptive multitasking is necessary to efficiently use up processor performance.  
(プリエンプティブなマルチタスク機能は効率的にプロセッサの性能を使い切る場合に必要になります。)  

In the example2.ino application example, 3 tasks, task_10ms(), task_100ms(), and task_1s(),  
are invoked from timer interrupts with 10ms, 100ms, and 1s cycles.  
(example2.ino のアプリケーションの例では、10ms、100ms、1秒周期のタイマー割り込みからtask_10ms()と  
task_100ms()、task_1s()の３つのタスクを起動しています。)

The three tasks only increment the counter, but each has the following processing load.  
(3つのタスクでは、カウンタをインクリメントしているだけですが、下記の処理負荷になっています。)  

| processing cycle | task name | processing load | 
|----------|----------|----------|
| 10ms	| task_10ms()   |1ms |
| 100ms	| task_100ms() |50ms |
| 1sec	| task_1s()  |300ms |

The total processing load per second is **900ms (90%)**.  
(1秒間のトータルの処理負荷は900ms(90%)です。)  

This is a condition where processing cannot be performed at the specified frequency  
without appropriate priority setting and multiple processing (multitasking) according to  
the priority order.
(適切な優先順位の設定と、優先順位に従った多重処理(マルチタスク)が実施されなければ、  
処理が規定頻度で実施できない条件です。)  

You can check the number of times each task is executed per second on PC.  
(PCで1秒ごとの各タスクの実行回数を確認できます。)  
Each task is executed without exiting.  
(各タスクは抜けることなく実行されます。)  

Multiple processing (multitasking) is being performed due to the priority control of  
the real-time kernel.  
(リアルタイムカーネルの優先制御により、多重処理(マルチタスク)が実施されています。)  

## License (ライセンス)
This software is licensed under the MIT License, see the [LICENSE.txt](https://github.com/pekopoko-heart/RTKernel-for-Arduino-Uno-R3/blob/main/LISENCE.txt) file for details.  
(このソフトウエアはMITライセンスの下でライセンスされます。詳細は LICENSE.txt をご覧ください。)  
