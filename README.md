Tetris
======

![screenshot](https://i.imgur.com/ZdGrJmB.png)

A Tetris clone written in C with some inline assembly for an 8-bit Atmel ATmega8515L AVR microcontroller. The project is based on the Tampere University of Technology's "Introduction to IT" course's LED blinker assignment, so credit goes partly to the person who wrote the original program to control the lights. My part of the work was adding the physical buttons and writing the game specific logic.

Click [here](https://vimeo.com/242865040) to see a video of the game in action.

All intellectual rights to the Tetris name and gameplay mechanics belong to Alexey Pajitnov.

Design
------
The game is displayed on a 8 x 16 LED matrix, so that each square represents exactly one light. The area is slightly less than the original game's 10 x 20, but still perfectly playable. Input consists of three physical buttons: move left, move right and rotate clockwise. Rotation in the counterclockwise direction was ommited to simplify design and the physical structure. 

Before starting the actual work, a prototype tetris.c running on desktop was written using the allegro library (www.liballeg.org) for graphics and input. This was done in order to speed up porting of the game to the chip.

Soldering the components
--------------------
The 128 LEDs were soldered onto a pre-made grid that was on the printed circuit board starting from the top left corner. This took a few days to complete, so now I can safely say that I have had more than enough practice soldering tiny components onto a circuit board! After each row they were was tested to make sure they light up and necessary resolderings were done if not. The buttons were connected to any available ports, which in practice meant the last eight vertical columns of connectors to the LED grid that were not in use. This means that five more buttons could be easily added, but more than that requires some planning ahead.

Programming the chip
--------------------
The code itself is located in the three files: led.h, led.c and ledivilkku.c. The first two deal with timing lighting the LEDs correctly and turning on sleep mode, while the last contains the actual game instructions. AVR Studio 4 was used as the IDE and for porting the program via a data cable into the microcontroller's flash memory.

The LEDs are accessed through a 256 length single integer array named l[_index_]. Setting each element to a value between 0-15 controls the brightness of the light. Because dealing with a position in the grid is easier using two values, one for row and second for column, the following numerical transformation can be performed:
```c
  // turn row i, column j's light
  l[i + 16*j] = 15;
```
The state of the buttons is checked using an available variable which points to a port and masking the irrelevant bits to zero. For example D port's first pin can be read with the command
```c
  uint8_t button_right = PIND & 0x01;
  if(button_right == 0x00) {
    // perform wanted action
  }
```
The main function's loop iterates infinitely until the player pushes the power button or the batteries run out of electricity. The state of the buttons is checked at every cycle, but the tetrominos move down roughly once a second. To achieve this a variable called `timer`  is incremented every loop and if it is divisible by 2048 then we update the game logic.

One of the limiting factors when programming was the memory size of the chip which was only 512 bytes. In order to fit the entire game into the allocated space all unnecessary lines from the original blinker project were removed. In addition to this, to represent each tetromino (which fits inside of a 4 x 4 square) a two byte uint8_int variable was used, where each of the 16 bits represent exactly one square. For example a T shaped block is encoded into the value 114, which in binary is 
```c
0b0000000001110010
```
The four least significant bits represent the top row, the four next ones the second row etc. and bitwise operations are used to access each part. As as a result the single uint8_int variable above is broken up into lines that looks like this:
```c
0010
0111
0000
0000
```
When rotating a tetromino rather than calculating each bit individually, a series of if-else statements are used to compare the entire variable against hard-coded values in order to make the program code more straightforward.

Conclusion
----------

This was an interesting project to work on and the result is already in a quite playable condition. The game's development could be continued by adding a high-score counter which is displayed after each game or by housing the controller in a more user friendly case. I'll probabaly make a nice 3D-printed case when I have more free time. It should also be noted that now that the system specific issues have been tackled it would be a fairly straightforward operation to port other simple games, such as snake or pong onto the platform :)



