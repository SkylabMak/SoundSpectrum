#ifndef FFT_PROCESSOR_H
#define FFT_PROCESSOR_H

#include <vector>
#include <fftw3.h>

class FFTProcessor
{
public:
    FFTProcessor(int fftSize, int samplingRate);
    ~FFTProcessor();

    std::vector<double> processFFT(const std::vector<float> &audioData);
    std::vector<double> filterFrequencies(const std::vector<double> &magnitude);
    std::vector<double> applyEqualizer(std::vector<double> &magnitude);

private:
    int fftSize;
    int samplingRate;
    double freqResolution;
    double freqResolutionhalf;
    size_t minIndex, maxIndex;
    std::vector<double> bandGains; 
    std::vector<size_t> bandBins;  
    void setupEqualizer();         
    double *fftInput;
    fftw_complex *fftOutput;
    fftw_plan fftPlan;
};

#endif
