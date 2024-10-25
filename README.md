# Real-time kernel for Arduino UNO R3
**A real-time kernel that runs practically on the Arduino UNO R3 and can be used in the Arduino**  
**IDE environment. With a code size of 644 Bytes, it enables real preemptive multitasking.**  

## Contents
- [Overview](https://github.com/pekopoko-heart/Real-Time-Kernel-for-Arduino-Uno-R3/blob/main/README.md#overview)
- [Dispatch Sequence](https://github.com/pekopoko-heart/Real-Time-Kernel-for-Arduino-Uno-R3/blob/main/README.md#dispatch-sequence)
- [How to use](https://github.com/pekopoko-heart/Real-Time-Kernel-for-Arduino-Uno-R3/blob/main/README.md#how-to-use)
- [Let's check out example2.ino](https://github.com/pekopoko-heart/Real-Time-Kernel-for-Arduino-Uno-R3/blob/main/README.md#lets-check-out-example2ino)
- [License](https://github.com/pekopoko-heart/Real-Time-Kernel-for-Arduino-Uno-R3/blob/main/README.md#license)

## Overview
This real-time kernel runs practically on Arduino UNO R3 and can be used in the Arduino IDE  
environment. Depending on the priority set, each task is managed with 4 statuses: stopped (STOP),   
running (RUN), ready to run (READY), and suspended (SUSPEND).  
The real-time kernel grants execution privilege to the task with the highest priority among those  
that have requested to be started.  

This real-time kernel provides a preemptive multitasking environment. And it has minimal priority  
control overhead and consumes minimal ROM/RAM size.  

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

For more information
[#1](https://pekopoko4control.blogspot.com/2024/09/arduino-uno.html)
[#2](https://pekopoko4control.blogspot.com/2024/10/for-arduino-uno-r3.html)

## Dispatch Sequence
The dispatch process cleverly uses the stack to switch tasks and perform priority control.  
It achieves **extremely simple and real preemptive multitasking**.  

![dispatch](https://github.com/pekopoko-heart/Real-Time-Kernel-for-Arduino-Uno-R3/blob/main/dispatch.png)

## How to use
The following two source files are available in the Arduino IDE environment.  

&nbsp;&nbsp;&nbsp;&nbsp;RTK4ArduinoUnoR3.ino  
&nbsp;&nbsp;&nbsp;&nbsp;example2.ino

RTK4ArduinoUnoR3.ino is the real-time kernel code.  
example2.ino is an example application.  

The application launches three tasks from timer interrupts and one background task.  
At the beginning of the code, two functions of the real-time kernel are declared as prototypes.  

&nbsp;&nbsp;&nbsp;&nbsp;**void task_sw(unsigned char no);**  
&nbsp;&nbsp;&nbsp;&nbsp;**void task_create(void(\*task)(void), unsigned char id, unsigned char level);**

Next, define the number of tasks and the task ID. The task ID is a number from 0 to (TASK_MAX-1).  

&nbsp;&nbsp;&nbsp;&nbsp;**#define  TASK_MAX    4**  
 
&nbsp;&nbsp;&nbsp;&nbsp;**#define  taskId_10ms    0**   
&nbsp;&nbsp;&nbsp;&nbsp;**#define  taskId_100ms   1**   
&nbsp;&nbsp;&nbsp;&nbsp;**#define  taskId_1s      2**  
&nbsp;&nbsp;&nbsp;&nbsp;**#define  taskId_bg      3**  

In setup(), tasks are created by task_create( function name, Task ID, priority ).   

&nbsp;&nbsp;&nbsp;&nbsp;**task_create(task_10ms, taskId_10ms, 8);**  
&nbsp;&nbsp;&nbsp;&nbsp;**task_create(task_100ms, taskId_100ms, 6);**  
&nbsp;&nbsp;&nbsp;&nbsp;**task_create(task_1s, taskId_1s, 4);**  
&nbsp;&nbsp;&nbsp;&nbsp;**task_create(task_bg, taskId_bg, 2);**  

Tasks are defined by functions, and the larger the priority value, the higher the priority.  
The priority of this application is, 
&nbsp;&nbsp;&nbsp;&nbsp;task_10ms > task_100ms > task_1s > task_bg  

Then, at the necessary timing, such as interrupt processing, Simply start each task by  

&nbsp;&nbsp;&nbsp;&nbsp;**task_sw(Task ID);**  

**Just this, Multitasking is performed under the priority control of the real-time kernel.**   

## Let's check out example2.ino
Preemptive multitasking is necessary to efficiently use up processor performance.  

In the example2.ino application example, 3 tasks, task_10ms(), task_100ms(), and task_1s(),  
are invoked from timer interrupts with 10ms, 100ms, and 1s cycles.  

The three tasks only increment the counter, but each has the following processing load.  

| processing cycle | task name | processing load | 
|----------|----------|----------|
| 10ms	| task_10ms()   |1ms |
| 100ms	| task_100ms() |50ms |
| 1sec	| task_1s()  |300ms |

The total processing load per second is **900ms (90%)**.  

This is a condition where processing cannot be performed at the specified frequency  
without appropriate priority setting and multiple processing (multitasking) according to  
the priority order.

You can check the number of times each task is executed per second on PC.  
Each task is executed without exiting.  

Multiple processing (multitasking) is being performed due to the priority control of  
the real-time kernel.  

### Without priority control

If the priority order of the 10ms, 100ms, and 1sec tasks is the same, it will behave like a  
polling process. Since priority control is not available, the execution frequency is about  
40% for the 10 ms task and about 70% for the 100 ms task.  
In addition, the effective utilization rate of the processor dropped by about 20% to about 70%.

It is clear that without appropriate priority setting and multiple processing (multitasking)  
with priority control, processing will not be performed at the specified frequency and processor  
performance will not be efficiently utilized.

## License
This software is licensed under the MIT License, see the [LICENSE.txt](https://github.com/pekopoko-heart/RTKernel-for-Arduino-Uno-R3/blob/main/LISENCE.txt) file for details.  
