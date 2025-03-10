#ifndef LED_MATRIX_VISUALIZER_H
#define LED_MATRIX_VISUALIZER_H

#include <LedControl.h>

#define DIN_PIN 11
#define CLK_PIN 13
#define CS_PIN 10
#define MATRIX_COUNT 2

extern LedControl lc; 

void setupLEDMatrix();
void drawColumn(int col, int height);
void displayVisualize(int* ledData);

#endif
