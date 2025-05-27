# OS-Simulator

 To run this, you can simply copy this code or download it on any Linux device and run it.

Concepts covered in operating systems courses are included in the simulator. The user can interact with it using a menu system on a console-based interface. Additionally, extra points were also earned for developing a simple GUI using a C++ library.

Following features are included in the simulator:
CPU scheduling:
Priority scheduling, round-robin (RR), and shortest remaining job first (SRJF) are three separate scheduling algorithms that are used to implement three different queues. Scheduling for Multi-Level Feedback Queues: First-Come-First-Serve (FCFS), shortest job first (SJF), and RR are three queues that were implemented with various scheduling algorithms. scheduling according to length of job. lowest feedback ratio, then scheduling.
Semaphore:  Implementation of the Dining Philosopher Problem using semaphores. The solution ensures that each philosopher can eat without causing deadlock or starvation. Deadlock:  Implementation of the Banker's Algorithm, which is used to avoid deadlock and allocate resources safely to each process in the computer system. The algorithm checks whether a loan amount should be approved based on the availability of resources.
Memory Management:  Two-Level Page-Table Scheme implementation for efficient memory management. Hashed Page Table with Translation Lookaside Buffer (TLB) implementation for faster page table access. 
Page Replacement:  Implementation of the Second Chance Algorithm using a circular linked list. The algorithm calculates the number of page faults, page fault probability, and page fault percentage.
Throughout the project, I have followed best practices such as creating a separate class for processes, using proper data structures, and utilizing the pthread library for thread creation. The simulator provides a comprehensive understanding of CPU scheduling, semaphores, deadlock avoidance, memory management, and page replacement algorithms.
