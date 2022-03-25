/*	Author: Gabriela Alvarez
 *	Assignment: CS120B - Intro to Embedded Systems, Final Project
 *	Description: Ping Pong Game played against the computer. 
 *
 *	I acknowledge all content contained herein, excluding template or example
 *	code, is my own original work.
 *
 *	DEMO LINK: https://drive.google.com/file/d/1gS2goVIaLxzDvcLrOcqs1s-EQEKLaYXo/view?usp=sharing
 *
 */
#include <avr/io.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifdef _SIMULATE_
#include "simAVRHeader.h"
#include "timer.h"
#include "scheduler.h"
#endif

    /*   column:  player    0     1     2      3    4      5     6     7    ai */
unsigned char column[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};
unsigned char ball = 0xFB, xPos = 0x40; // starting ball in middle row  (11011)
unsigned char b = 6, lastCol = 7, ball_tmp, paddle_tmp, right = 1, left = 0, LOSE, WIN;
unsigned int ball_time = 300;

unsigned char paddle_positions[4] = {0xFC, 0xF9, 0xF3, 0xE7}; // top, middle 1, middle2, bottom
unsigned char paddle = 0xF9, p = 1;

unsigned char easy[4] = {0xFC, 0xF9, 0xF3, 0xE7};	// 2 dot paddle, difficulty 1

unsigned char ai_paddle = 0xFC, ai = 0, goingUp = 0, goingDown = 1, currentMax = 3, aipaddle_tmp, b_tmp;
unsigned int follows;	// starting at easy, going down

enum ai_states {init, waiting, moveUp, moveDown};
int ai_tick(int state) {
	switch (state) {
		case init: state = waiting; break;
		case waiting: 
			follows = (rand() % 2);
			b_tmp = ball ^ 0xFF; 
			aipaddle_tmp = ai_paddle ^ 0xFF; 

			if ((b_tmp > aipaddle_tmp) && follows && (ai < currentMax) && ((b_tmp | aipaddle_tmp) != b_tmp)) {
				++ai;
				state = moveDown;
			} else if ((b_tmp < aipaddle_tmp) && follows && (ai > 0) && ((b_tmp | aipaddle_tmp) != b_tmp)) {
				--ai;
				state = moveUp;
			} else { 
				state = waiting;
			} break;
		case moveUp: 
			state = waiting; break;
		case moveDown: 
			state = waiting; break;
		default: state = waiting; break;
	}	
	switch (state) {
		case init: ai_paddle = easy[ai]; break;
		case waiting: break;
		case moveUp: ai_paddle = easy[ai]; break;
		case moveDown: ai_paddle = easy[ai]; break;
	} 
	return state;
}

enum reset_states {waitR, reset};
int reset_tick(int state) {
	switch (state) {
		case waitR:
			if ((~PINA & 0x01) == 0x01) {		// pressing reset button 
				p = 1;
				paddle = paddle_positions[p];
				ai = 0; goingUp = 0; goingDown = 1;	
				ai_paddle = easy[ai];
				ball = 0xFB; xPos = 0x40; b = 6; right = 1; left = 0;
				ball_time = 300;
				WIN = 0; LOSE = 0;
				state = reset;
			} else {
				state = waitR;
			} break;
		case reset:
			if ((~PINA & 0x01) == 0x01) {		// still pressing 
				state = reset;
			} else {
				state = waitR;			// let go of button 
			} break;	
		default: state = waitR; break;
	}
	return state;
}

enum paddle_states {waitMove, up, down}; 
int paddle_tick(int state) {
	switch (state) {
		case waitMove: 
			if (((~PINA & 0x07) == 0x02) && (p != 0)) { 		    // moving up
				state = up;
			} else if (((~PINA & 0x07) == 0x04) && (p != 4)) {	    // moving down 
				state = down;
			} else {
				state = waitMove;		    		    // waiting 
			} break;	
		case up:
			if (((~PINA & 0x07) == 0x02) && (p != 0)) {
				state = up;
			} else {
				state = waitMove;
			} break;
		case down:
			if (((~PINA & 0x07) == 0x04) && (p != 4)) {
				state = down;
			} else {
				state = waitMove;
			} break;
		default: state = waitMove; break;
	}
	switch (state) {
		case waitMove:
			// keep paddle where it is
			break;
		case up:
			// move paddle up until touches the top 
			--p;
			paddle = paddle_positions[p];
			break;
		case down:
			// move paddle down until it touched bottom 
			++p;
			paddle = paddle_positions[p];
			break;
	}
	return state;
}

enum ball_states{initBall, startPos, checkPlayerPaddle, checkAiPaddle, goStraightRight, goStraightLeft, goDiagonalUp, goDiagonalDown, win, lose};
int ball_tick(int state) {
	switch(state) {
		case initBall: state = startPos; break;
		case startPos: 
			ball_time = 300;
			if (xPos == 0x02) {
				state = checkPlayerPaddle;
			} else {
				state = startPos;
			} break;
		case checkPlayerPaddle:
			ball_tmp = ball ^ 0xFF; 
			paddle_tmp = paddle ^ 0xFF;	
			left = 1; right = 0;

			if ((ball == 0xFE) && ((p == 0) || (p == 1))) {		// TOP CORNER ball: 11110 (paddle: 11100 or 11001)
				state = goDiagonalDown;
			} else if ((ball == 0xEF) && ((p == 2) || (p == 3))) {  // BOTTOM CORNER ball: 01111 
				state = goDiagonalUp;
			} else if ((paddle | ball) == ball) {			// checks if ball is gonna hit paddle
				state = goStraightLeft;
			} else if ((ball_tmp < paddle_tmp) && (((paddle_tmp >> 1) & ball_tmp) == ball_tmp)) {		 
				state = goDiagonalUp;
			} else if ((ball_tmp > paddle_tmp) && (((paddle_tmp << 1) & ball_tmp) == ball_tmp)) {		 
				state = goDiagonalDown;
			} else {
				state = lose;
			} break;
		case goStraightLeft:
			ball_time = 400;
			if (xPos == 0x40) {
				state = checkAiPaddle;
			} else {
				state = goStraightLeft;
			} break;
		case checkAiPaddle:
			ball_tmp = ball ^ 0xFF; 
			paddle_tmp = ai_paddle ^ 0xFF;
			left = 0; right = 1;

			if ((ball == 0xFE) && ((ai == 0) || (ai == 1))) {	 // TOP CORNER ball: 11110 
				state = goDiagonalDown;
			} else if ((ball == 0xEF) && ((ai == 2) || (ai == 3))) { // BOTTOM CORNER ball: 01111 
				state = goDiagonalUp;
			} else if ((ai_paddle | ball) == ball) {		 // checks if ball is gonna hit paddle
				state = goStraightRight;
			} else if ((ball_tmp < paddle_tmp) && (((paddle_tmp >> 1) & ball_tmp) == ball_tmp)) {			 
				state = goDiagonalUp;
			} else if ((ball_tmp > paddle_tmp) && (((paddle_tmp << 1) & ball_tmp) == ball_tmp)) {			 
				state = goDiagonalDown;
			} else {
				state = win;
			} break;
		case goStraightRight: 
			ball_time = 400;
			if (xPos == 0x02) {
				state = checkPlayerPaddle;
			} else {
				state = goStraightRight;
			} break;
		case goDiagonalUp:
			ball_time = 200;
			if (xPos == 0x40) {
				state = checkAiPaddle;
			} else if (xPos == 0x02) {
				state = checkPlayerPaddle;
			} else if (ball == 0xFE) {			// ball hit top row
				state = goDiagonalDown;
			} else {
				state = goDiagonalUp;
			} break;
		case goDiagonalDown:
			ball_time = 200;
			if (xPos == 0x40) {
				state = checkAiPaddle;
			} else if (xPos == 0x02) {
				state = checkPlayerPaddle;
			} else if (ball == 0xEF) {			// ball Bottom top row
				state = goDiagonalUp;
			} else {
				state = goDiagonalDown;
			} break;
		case lose: LOSE = 1; WIN = 0; state = initBall; break;
		case win: LOSE = 0; WIN = 1; state = initBall; break;
		default: state = startPos; break;
	}
	switch(state) {
		case initBall: b = 6; xPos = column[b]; ball = 0xFB; left = 1; right = 0; break;
		case startPos:
			--b;
			xPos = column[b];
			break;
		case checkPlayerPaddle: break;
		case goStraightLeft:
			++b;
			xPos = column[b];
			break;
		case checkAiPaddle: break;
		case goStraightRight: 
			--b;
			xPos = column[b];
			break;
		case goDiagonalUp:
			if (left) {
				ball = (ball >> 1) | 0x80;
				++b;
				xPos = column[b];
			} else if (right) {
				ball = (ball >> 1) | 0x80;
				--b;
				xPos = column[b];
			} break;
		case goDiagonalDown:
			if (left) {
				ball = (ball << 1) | 0x01;
				++b;
				xPos = column[b];
			} else if (right) {
				ball = (ball << 1) | 0x01;
				--b;
				xPos = column[b];
			} break;
	}
	return state;
}

unsigned char j = 0, col_size = 8;
unsigned char game_columns[8] = {0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80};

unsigned char W_rows[8] = {0xFF, 0xF9, 0xF7, 0xFB, 0xF7, 0xF9, 0xFF, 0xFF};
unsigned char L_rows[8] = {0xFF, 0xFF, 0xF7, 0xF7, 0xF1, 0xFF, 0xFF, 0xFF};
unsigned char count = 0, l = 0;

enum display_states {display_game, waitL, rip, winnerWinner, chickenDinner};	// PORTD controls the rows
int display_tick(int state) {					// PORTC the columns
	switch (state) {
		case display_game:
			PORTC = game_columns[j];
			if (j == 0) {
				PORTD = paddle;
			} else if (j == 7) {
				PORTD = ai_paddle;
			} else if (game_columns[j] == xPos) {
				PORTD = ball; 
			} else {
				PORTD = 0xFF;
			}

			++j;
			if (j == col_size) { j = 0; }

			if (LOSE || WIN) {
				PORTC = 0x00; PORTD = 0xFF;
				state = waitL;
			} else { 
				state = display_game; 
			} break;
		case waitL: 
			if (count < 50) {
				l = 0;
				state = rip;
				++count;
			} else { 
				count = 0;
				PORTD = 0xFF;
				PORTC = 0x00;
				LOSE = 0; WIN = 0;
				state = display_game; 
			} break; 
		case rip: 
			++l; 
			if (l == col_size) { l = 0; state = waitL; } 
			else { state = rip; }  break;
		default: state = display_game; break;
	}
	switch(state) {
		case rip:
			PORTC = game_columns[l];
			if (LOSE) {
				PORTD = L_rows[l];
			} else { 
				PORTD = W_rows[l];
			}
			break;
	}
	return state;
}

int main(void) {
	DDRA = 0x00; PORTA = 0xFF;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;

	srand(time(0));
		    
	static task task1, task2, task3, task4, task5;
	task *tasks[] = { &task1, &task2, &task3, &task4, &task5 };
	const unsigned short numTasks = sizeof(tasks)/sizeof(task*);

	const char start = -1;
	
	task1.state = start;
	task1.period = 100;
	task1.elapsedTime = task1.period;
	task1.TickFct = &reset_tick;

	task2.state = start;
	task2.period = 200;
	task2.elapsedTime = task2.period;
	task2.TickFct = &paddle_tick;

	task3.state = start;
	task3.period = 1;
	task3.elapsedTime = task3.period;
	task3.TickFct = &display_tick;

	task4.state = start;
	task4.period = 300;
	task4.elapsedTime = task4.period;
	task4.TickFct = &ai_tick;

	task5.state = start;
	task5.period = ball_time;
	task5.elapsedTime = task5.period;
	task5.TickFct = &ball_tick;

	unsigned long GCD = tasks[0]->period;
	for(unsigned int i = 1; i < numTasks; i++) {
		GCD = findGCD(GCD, tasks[i]->period);	    
	}	

	TimerSet(GCD);
	TimerOn();

	unsigned short i;
	while(1) {
		for (i = 0; i < numTasks; i++) {
			if (tasks[i]->elapsedTime == tasks[i]->period) {
				tasks[i]->state = tasks[i]->TickFct(tasks[i]->state);
				tasks[i]->elapsedTime = 0;
			}
			tasks[i]->elapsedTime += GCD;
			if (tasks[i]->TickFct == &ball_tick) {
        			tasks[i]->period = ball_time;
        		}
		}

		while(!TimerFlag);
		TimerFlag = 0;
	}
	return 1;
}
