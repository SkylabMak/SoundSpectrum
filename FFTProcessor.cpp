#include "FFTProcessor.h"
#include <fftw3.h> // ใช้ FFTW Library
#include <cmath>
#include <iostream>

FFTProcessor::FFTProcessor(int fftSize, int samplingRate) {
    this->fftSize = fftSize;
    this->samplingRate = samplingRate;
    this->freqResolution = static_cast<double>(samplingRate) / fftSize;
    this->freqResolutionhalf = static_cast<double>(samplingRate) / (fftSize/2);

    // std::cout << freqResolution << std::endl;
    double minFreq = 50.0;
    double maxFreq = 9000.0;
    minIndex = static_cast<size_t>(minFreq / freqResolutionhalf);
    maxIndex = static_cast<size_t>(maxFreq / freqResolutionhalf);

    // std::cout << minIndex << std::endl;
    // std::cout << maxIndex << std::endl;

    //  จำกัด index ให้อยู่ในขอบเขตของ magnitude
    minIndex = std::max<size_t>(minIndex, 0);
    maxIndex = std::min<size_t>(maxIndex, fftSize / 2 - 1);

    std::cout << minIndex << std::endl;
    std::cout << maxIndex << std::endl;

    fftInput = (double*) fftw_malloc(sizeof(double) * fftSize);
    fftOutput = (fftw_complex*) fftw_malloc(sizeof(fftw_complex) * fftSize);
    fftPlan = fftw_plan_dft_r2c_1d(fftSize, fftInput, fftOutput, FFTW_ESTIMATE);

    // Define gain values (adjust these as needed)
    // bandGains = {0.4, 0.1, 1.5, 2.0, 3.0, 2.5}; // Gain factors
    // bandGains = {1.0, 1.0, 1.0, 1.0, 1.0, 1.0}; // Gain factors
    // bandGains = {0.4, 1.0, 1.0, 1.0, 2.0,3.0}; // Gain factors
    bandGains = {0.4, 2.0, 3.0, 4.0, 5.0,6.0}; // Gain factors
    setupEqualizer();
}

FFTProcessor::~FFTProcessor()
{
    fftw_destroy_plan(fftPlan);
    fftw_free(fftInput);
    fftw_free(fftOutput);
}

void FFTProcessor::setupEqualizer() {
    size_t numBands = bandGains.size();
    size_t bandSize = (maxIndex - minIndex + 1) / numBands;

    bandBins.clear(); // Ensure it's empty
    for (size_t i = 0; i < numBands; i++) {
        bandBins.push_back(minIndex + i * bandSize);
    }

    std::cout << "EQ Bands Mapped to Bins:\n";
    for (size_t i = 0; i < bandBins.size(); i++) {
        std::cout << "Band " << i << ": Bin " << bandBins[i] << " -> Gain " << bandGains[i] << "\n";
    }
}

std::vector<double> FFTProcessor::applyEqualizer(std::vector<double>& magnitude) {
    std::vector<double> matrix(magnitude.size());
    size_t numBands = bandGains.size();
    
    for (size_t j = 0; j < numBands; j++) {
        size_t startBin = bandBins[j];
        size_t endBin = (j >= numBands - 1) ? magnitude.size()-1 : bandBins[j + 1];
        
        for (size_t i = startBin; i < endBin; i++) {
            double t = (double)(i - startBin) / (endBin - startBin);
            double gain = bandGains[j] * (1 - t) + bandGains[j + 1] * t; // Linear Interpolation
            // std::cout << magnitude[i]<<"--"<<bandGains[j]<<"with"<<gain<<"("<<i<<","<<j<<")"<<">>";
            matrix[i] = magnitude[i] * gain;  
            // std::cout << matrix[i] <<" ";
        }
    }

    return matrix;
}


std::vector<double> FFTProcessor::filterFrequencies(const std::vector<double>& magnitude) {
    if (magnitude.empty()) return {};

    return std::vector<double>(magnitude.begin() + minIndex, magnitude.begin() + maxIndex + 1);
}

std::vector<double> FFTProcessor::processFFT(const std::vector<float> &audioData)
{
    if (audioData.size() < fftSize)
    {
        std::cerr << " Error: Audio data size is too small for FFT!" << std::endl;
        return {};
    }

    //  1. copy
    for (int i = 0; i < fftSize; i++)
    {
        fftInput[i] = static_cast<double>(audioData[i]); //  float -> double
    }

    //  2.  FFT Processing
    fftw_execute(fftPlan);

    //  3. cal Magnitude 
    std::vector<double> magnitude(fftSize / 2);
    for (int i = 0; i < fftSize / 2; i++)
    {
        magnitude[i] = sqrt(fftOutput[i][0] * fftOutput[i][0] + fftOutput[i][1] * fftOutput[i][1]);
        // std::cout << magnitude[i] ;
    }
    
    // std::cout << std::endl;
    return magnitude;
}
