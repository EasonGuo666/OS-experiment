# OS-experiment
BITCS Operating System Experiment

1.实验1：编译系统内核（无代码）


2.实验2：进程控制（Windows && linux）

 设计并实现Unix的“time”命令。“mytime”命令通过命令行参数接受要运行的程序，创建一个独立的进程来运行该程序，并记录程序运行的时间。

 在Windows下实现:

- 使用CreateProcess()来创建进程
- 使用WaitForSingleObject()在“mytime”命令和新创建的进程之间同步
- 调用GetSystemTime()来获取时间

 在Linux下实现:

- 使用fork()/vfork /exec()来创建进程运行程序
- 使用wait()等待新创建的进程结束
- 调用gettimeofday()来获取时间

 mytime的用法：

- $ mytime.exe program1
要求输出程序program1运行的时间。Pragram1可以为自己写的程序，也可以是系统里的应用程序。

- $ mytime.exe program2 t

 t为时间参数，为program2的输入参数，控制program2的运行时间。最后输出program2的运行时间，应和t基本接近。
 显示结果： ** 小时 ** 分 ** 秒 ** 毫秒 ** 微秒

3.实验3：生产者消费者问题（Windows && linux）
