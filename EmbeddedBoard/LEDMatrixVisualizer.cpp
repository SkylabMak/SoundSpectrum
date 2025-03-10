#include "LEDMatrixVisualizer.h"

LedControl lc = LedControl(DIN_PIN, CLK_PIN, CS_PIN, MATRIX_COUNT); 
const int NUM_COLUMNS = 16;
const int NUM_ROWS = 8;

void setupLEDMatrix() {
  lc.shutdown(0, false);
  lc.shutdown(1, false);
  lc.setIntensity(0, 8);
  lc.clearDisplay(0);
  lc.shutdown(1, false);
  lc.setIntensity(1, 8);
  lc.clearDisplay(1);
}

void drawColumn(int col, int height) {
  int matrixIndex = (col < 8) ? 1 : 0;  
  int matrixCol = col % 8;
  int flippedHeight = 7 - height;// Flip vertically 

  for (int row = 0; row < 8; row++) {
    bool state = (row >= flippedHeight);
    lc.setLed(matrixIndex, row, matrixCol, state);
  }
}

void displayVisualize(int* ledData) {
  for (int col = 0; col < 16; col++) {
    drawColumn(col, ledData[col]); 
    // Serial.print(ledData[col]); 
  }
    // Serial.println(); 
}


