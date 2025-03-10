#include "LEDMatrixVisualizer.h"
#include <iostream>
#include <algorithm> // สำหรับ std::max, std::min
#include <cmath>     // สำหรับ log scaling

const int LEDMatrixVisualizer::MATRIX_WIDTH = 16;
const int LEDMatrixVisualizer::MATRIX_HEIGHT = 8;

LEDMatrixVisualizer::LEDMatrixVisualizer() {}

std::vector<int> LEDMatrixVisualizer::transformFFTData(const std::vector<double> &fftData)
{
    std::vector<int> matrix(MATRIX_WIDTH);

    if (fftData.empty())
        return matrix;

    int binSize = fftData.size() / MATRIX_WIDTH; //  แบ่ง FFT bins -> 16 columns

    for (int col = 0; col < MATRIX_WIDTH; col++)
    {
        double avgMagnitude = 0;

        // avg
        for (int i = 0; i < binSize; i++)
        {
            avgMagnitude += fftData[col * binSize + i];
        }
        avgMagnitude /= binSize;

        // std::cout << avgMagnitude << " ";

        int height = std::round(avgMagnitude);
        // std::cout << height << " ";
        height = std::clamp(height, 0, MATRIX_HEIGHT); // height  0-8
        matrix[col] = height;                          //  เรียงจากล่างขึ้นบน
        // for (int row = 0; row < height; row++) {
        //     matrix[MATRIX_HEIGHT - 1 - row][col] = 1;  //  เรียงจากล่างขึ้นบน
        // }
    }
    // std::cout << std::endl;

    return matrix;
}
