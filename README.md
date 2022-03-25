# PongGame
### Ping Pong Game played against the computer. Implemented in C and uses an ATMEGA-1284p microcontroller.

- The game is displayed on a LED Matrix and takes in user input from 3 switches. The top switch resets the game, second moves the players paddle  up, and the third makes the paddle move down. 
- The system recognizes when the ball contacts the center of any paddle, slowing down the ball, or the corner of any paddle, speeding up the ball in a diagonal direction. 
- If the player scores on the computer, a "W" is displayed, otherwise an "L" is displayed if the player is scored on. 
- The opponnent's (the computer's) paddle movement is dictated by the position in which the ball is found. It randomly decides whether to follow the ball or go in the opposite direction (1 means follow, 0 means opposite). It only has one speed. 

[Here](https://drive.google.com/file/d/1gS2goVIaLxzDvcLrOcqs1s-EQEKLaYXo/view?usp=sharing) is a link to a video demonstrating how the system works. 
