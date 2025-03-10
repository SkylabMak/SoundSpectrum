#ifndef LED_MATRIX_VISUALIZER_H
#define LED_MATRIX_VISUALIZER_H

#include <vector>

class LEDMatrixVisualizer {
private:
    // static const int MATRIX_WIDTH = 16;
    // static const int MATRIX_HEIGHT = 8;
    int maxFFTValue = 1;  // ค่า max ของ FFT สำหรับ Normalize

public:

    static const int MATRIX_WIDTH;
    static const int MATRIX_HEIGHT; 
    LEDMatrixVisualizer();

    std::vector<int> transformFFTData(const std::vector<double>& fftData);
};

#endif // LED_MATRIX_VISUALIZER_H
