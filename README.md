<!-- PROJECT LOGO -->
<br />
<p align="center">
  <h1 align="center">FreeRTOS Project Repository</h1>
  
  

<!-- TABLE OF CONTENTS -->
<details open="open">
  <summary>Table of Contents</summary>
  <ol>
    <li>
      <a href="#about-the-project">Introduction</a>
      <ul>
        <li><a href="#overview">Overview</a></li>
		    <li><a href="#project-structure">Projects</a></li>
      </ul>
    </li>
    <li><a href="#contact">Contact</a></li>
  </ol>
</details>



<!-- INTRODUCTION -->
## Introduction

### Overview
* This repository will store the projects I worked on while learning about freeRTOS (on the EK-TM4C123GXL development board). Each project will have README.md files that describe the specific operation of the code and reference links to the API functions used.<br>
* Each project is implemented according to the most conventional code, for each line of code there is a specific comment for easy maintenance and reading.<br>
* Check out my [CheckList.xlsx](https://github.com/hun9pham/freertos-work/blob/main/CheckList.xlsx) to see my roadmap.


### Projects

## [Experiment18-RTC (Real-time Clock)](./Experiment18-RTC)

This experiment uses a DS1307 real-time clock IC to manage a calendar and clock.  The running date and time are displayed on an LCD,  and a five-position switch is used to set them.  The Tiva C board communicates with the chip using a 100kbps I2C connection.  A 3V coin cell battery keeps the clock running when main power is removed.   A driver was written that has functions to initialize and enable the clock, and read and write data from it.   This experiment runs on both the TM4C123G and TM4C1294.

[![](https://i.ytimg.com/vi/A5657PIc_eg/default.jpg)](https://youtu.be/A5657PIc_eg)<br>
[Watch the video](https://youtu.be/A5657PIc_eg)

## [Experiment17-I2C](./Experiment17-I2C)
This experiment connects three Tiva C boards to an I2C bus.  One of the boards is an EK-TM4C1294XL and the other two are EK-TM4C123GXLs.  The purpose of the experiment was to get familiar with I2C and I2C programming on the Tivas, and also to learn about the differences between the TM4C1294 and TM4C123.  

Each board is configured to be both a master and slave on the bus, and is connected to eight switches, one tri-color LED (or three separate color LEDs for the TM4C1294), and four green LEDs to display a number pattern. Three different commands can be sent from any one board to another by setting a target slave ID on the switches and pressing one or more of the onboard buttons.  Pressing SW1 invokes the WRITE command, which sends the switch settings from the master board to a slave board, and the slave will display the settings on its LEDs.  Pressing SW2 invokes the READ command, causing the master board to read the switch settings from the slave board and display them on its own LEDs.  Pressing both SW1 and SW2 invokes the BLINKY command, causing the slave board to enter into a blinking pattern.  The WRITE and BLINKY commands can also be sent to all boards on the bus in one transaction that uses repeated starts.

The Keil project has separate build targets for the TM4C1294 and TM4C123, and there are startup folders for each board that contain a different startup file and main function.  Building the project for one target disables the startup files for the other.  In addition, the HAL modules use conditional compilation when there are implementation difference between the two processors.  Flags in the HAL_Config.h file will turn on/off these conditional features.
 
[![](https://img.youtube.com/vi/yLTBi-FEyhM/2.jpg)](https://youtu.be/yLTBi-FEyhM)<br>
[Watch the video](https://youtu.be/yLTBi-FEyhM)

## [Experiment16-PWM-Music](./Experiment16-PWM-Music)
This experiment uses multiple PWM channels to produce audio signals and play tunes from MIDI files. A MIDI file exporter program was written in Visual Studio using the midifile parsing library (http://midifile.sapp.org/). The exporter generates a .c file that is compiled into the Keil project for the Tiva C. Up to four MIDI tracks are exported. 

Each MIDI track is controlled at run time by a FreeRTOS task that schedules and plays the track's note events. Playing a note involves modulating one of the Tiva's PWM channels to a frequency equal to the note's audio frequency. 

The PWM outputs are combined and amplified by a circuit that consists of tri-state buffers (stage 1) and an NJM386 audio amplifier (stage 2). Each PWM output controls the enable pin of a tri-state buffer, such that when the PWM pulse is HIGH, the buffer's output voltage is added to the input signal of stage 2; and when the PWM pulse is LOW, the buffer enters a high impedance state and is effectively removed from the audio input signal.

[![](https://i.ytimg.com/vi/MY4hRxabXYo/default.jpg)](https://youtu.be/MY4hRxabXYo)<br>
[Watch the video](https://youtu.be/MY4hRxabXYo)


<!-- CONTACT -->
## Contact
Author - [PHAM NGUYEN QUOC HUNG](https://hun9pham.github.io) - hungpham99er@gmail.com - [Linkedin](https://www.linkedin.com/in/pham-hung-a12449213/)
