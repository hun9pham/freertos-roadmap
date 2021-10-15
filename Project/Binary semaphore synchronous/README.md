<!-- PROJECT LOGO -->
<br />
<p align="center">
  <h1 align="center">Access Shared Resource using MUTEX</h1>
  
  

<!-- TABLE OF CONTENTS -->
<details open="open">
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">About The Project</a>
      <ul>
        <li><a href="#overview">Overview</a></li>
		<li><a href="#expected-behavior">Expected Behavior</a></li>
		<li><a href="#project-structure">Project Structure</a></li>
      </ul>
    </li>
	<li><a href="#comments">Comments</a></li>
    <li><a href="#contact">Contact</a></li>
    <li><a href="#references">References</a></li>
  </ol>
</details>



<!-- ABOUT THE PROJECT -->
## About The Project

This project is built on the EK-TM4C123GXL development board.

### Overview

This project creates two tasks to access the global variable g_SharedResource.
* Task 1 - Task Up <br>
The task up is implemented by the void vTask_ShareResourceUp function and has the priority level below task 2 (specifically 1). This task when accessing the global variable g_SharedResource and make it increase. When the mutex is held by this task, the green led will turn on to signal that this task is occupying the mutex. If the task 2 has priority higher tries to take Mutex, the priority of task 1 will inherit priority from task 2. <br>
When this task does not hold the mutex, it will wait until the mutex is released via the portMAX_DELAY parameter <br>
* Task 2 - Task Down <br>
The task down is implemented by the void vTask_ShareResourceDown() and has the priority level 2. Like task 1, this task will also occupy the mutex when accessing g_SharedResource and make it decrease.<br>
When this task does not hold the mutex and is not in block state, it will immediately request permission to hold the mutex through the second parameter set to 0 of the xSemaphoreTake() function. 

### Expected Behavior
<p>
When the scheduler is started, task 2 takes over first because it has a higher priority. Task 2 will occupy the mutex and turn on the red LED. Put the breakpoint at xSemaphoreGive() when debugging. 
<p align="center">
  <img src="images/RedLED.png" width="350" title="hover text">
  <img src="images/BreakpointTaskDown.png" width="350" title="hover text">
</p>
After task 1 releases the mutex and is blocked by vTaskDelayUntil(), task 2 will start executing and take the mutex and turn on green LED to signal. When task 1 exits the blocking state, it tries to take over the mutex immediately. This leads to task 2 inheriting precedence from task 1.
<p align="center">
  <img src="images/GreenLED.png" width="350" title="hover text">
  <img src="images/BreakpointTaskUp.png" width="350" title="hover text">
</p>

### Project Structure

```
├── README.md              			: Description of project
├── images              			: Folder contains images of project
      ├── BreakdownTaskDown.png
      ├── BreakdownTaskUp.png
      ├── GreenLED.png
      ├── RedLED.png
├── driverlib         				: Folder contains TivaWare™ Peripheral Driver Library
      ├── other peripherals library files
├── inc						: Folder contains TivaWare™ Peripheral Driver Library
      ├── other header files
├── FreeRTOS					: Folder contains FreeRTOS Library
      ├── License
      ├── Source
├── FreeRTOSConfig.h				: Define macro variables for FreeRTOS configuration
├── hardware_config.h				: This file configs three LEDs on board, enable the PLL and initialize the bus frequency to 80MHz
├── hardware_config.c				: Define functions in hardware_config.h
├── main.c					: Main source code
├── startup_rvmdk.S				: File startup code for TM4C123G
│   
```

<!-- GETTING STARTED -->
## Comments
In addition to the FreeRTOS API functions, a peripheral driver library from TI is used in this example.

<!-- CONTACT -->
## Contact

Author - [PHAM NGUYEN QUOC HUNG](https://hun9pham.github.io) - hungpham99er@gmail.com

Project Link: [Souce code](https://github.com/hun9pham/freertos-roadmap/tree/main/Project/Access%20shared%20resource%20using%20Mutex)



<!-- References -->
## References
* [TivaWare™ Peripheral Driver Library](www.ti.com/lit/ug/spmu298e/spmu298e.pdf)
* [Task Creation](https://www.freertos.org/a00019.html)
* [Task Control](https://www.freertos.org/a00112.html)
* [Semaphore / Mutexs](https://www.freertos.org/a00113.html)
