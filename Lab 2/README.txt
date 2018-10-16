NAME: Wai Yan (Will) Htun
ID: 104941153
EMAIL: willhtun42@gmail.com

lab2_add.c
A driver program that tests the preformance and correctness of a multithreaded counting program.

lab2_add.csv
A file that contains all the output data of the test cases.

lab2_list.c
A driver program that tests the performance and correctness of the linked list implementation SortedList.c.

lab2_list.csv
A file that contains all the output data of the test cases.

SortedList.c
Implementation of a linked list that will work with multithreading.

lab2_add-i.png
Graphs of the result of lab2_add.c.

lab2_list-i.png
Graphs of the result of lab2_list.c.

Makefile
A make file to build a tarball of all required files. Also contains roughly 200 test cases that produces data that is used to plot the graphs.

README
A file containing the description of each file.


ANSWERS TO QUESTIONS

2.1.1
It takes man iterations for error to be seen because with low iterations, there are less chances of threads carrying out the same code at the same time. When this happens, race condition occurs and the result produced by the threads is incorrect. When using a significantly smaller number of iterations, it is less likely to fail because there are less chances of race condition occuring.

2.1.2
The time taken when using the yield operation is due to context switching. It is not possible to get valid per-operation timings when using the yield option because of the unpredictable time taken to switch contexts between threads/processes.

2.1.3
The average cost of operation drops as the number of iterations increases, because it does not cost to run an iteration, only to create a thread. This means that we can increase the number of iterations to run more operations without affecting the cost (because we are not creating new threads). To get the "correct" cost, we need to run variable numbers of iterations, plot the time taken by each run, and find the average. 

2.1.4
Low number of threads almost perform the same because most of the time, only a few of the threads have to wait to acquire the lock, if any. But as the number of threads increases, more threads will have to fight for the lock or wait, increasing the cost. Without proper locking mechanisms, the wait time increases exponentially as number of threads rises.

2.2.1
The time to run add and list with mutex lock option increases with number of threads and iterations. This increase is fairly linear in both add and list. With increase number of threads, the rate of increase is more variable in list because its critical sections are longer and more frequent than add's. As a result, threads have to do context switches a lot more, resulting in varied increased rate depending on the number of threads running.

2.2.2
With low number of threads, the time to run either add or list with mutex or spin locks are about the same as there are less chances of threads not acquiring the lock and blocking. As the number of thread increases, the time to run mutex lock scales a lot better than spin lock. This is because when a thread cannot acquire a spin lock, it will horde the CPU and wait until the lock is free, instead of blocking. It is unnecessarily running the CPU when it can yield it to other threads in need. Mutex lock on the other hand will block and give up the CPU to run other threads, thus increasing productivity and reducing running time.