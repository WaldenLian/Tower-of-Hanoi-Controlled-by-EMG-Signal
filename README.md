# Tower-of-Hanoi-Controlled-by-EMG-Signal

### Description
Final project for the course of Embedded Systems. We designed and implemented the canonical mathematical game "Tower of Hanoi" based on PIC32 board. The GUI is displayed on the LCD board, which is kind of virtual. However, the user experience is real. We use EMG signal to simulate the actions of picking up/putting down a disk, and use Pressure signal to simulate the actions of moving the disk towards the left/right.

* EMG sensor: Controlling the operation of picking up & put down.
* Pressure sensor: Controlling the operation of moving left & right.
* LCD displayer (JLX256160G-681-PC LCD board): Displaying the Graphical User Interface (GUI) for the Tower of Hanoi.
* LCD displayer (1602A LCD board): Displaying the time and the number of steps a player takes to complete the game. 
* ADC: Converting the analog signals sampled from EMG & Pressure sensors to digital signals so that the player could control the game.
* UART: Communicating between PIC32 and Arduino to enable the larger LCD board.
* PWM: Playing the music effect when the disk is put down.
* Serial Communication (SPI): Communicating between Arduino and the larger LCD board to display the GUI for the game.

### Demo
See the video clip: https://www.bilibili.com/video/av29026204