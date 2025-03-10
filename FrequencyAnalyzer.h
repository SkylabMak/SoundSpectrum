#ifndef FREQUENCY_ANALYZER_H
#define FREQUENCY_ANALYZER_H

#include <vector>
#include "Pattern.h"  // Enum for patterns
#include <bits/chrono.h>
#include <deque>


class FrequencyAnalyzer {
    public:
    FrequencyAnalyzer(int SamplingFrequency, int historySize,int bufferSize);
    Pattern detectPattern(const std::vector<double>& fftResult);
    double getCurrentBPM();
    double volume = 1.0; // windows 26 ,spotify 70
    
private:
    int SamplingFrequency;
    int historySize;
    int historyIndex = 0;
    int historyCount = 0;
    int bufferSize;
    double lastBPM = 0.0;
    double period = 2.0;

    std::vector<double> bassHistory, melodyHistory, peakFreqHistory;
    std::vector<double> flatnessHistory, dynamicRangeHistory, transientHistory,bpmHistory;
    std::vector<double> vocalHistory,hnrHistory;
    // std::vector<std::vector<double>> fftStorage;
    std::deque<std::pair<std::vector<double>, std::chrono::steady_clock::time_point>> fftStorage;
    std::chrono::steady_clock::time_point lastBeatTime; 

    // **Precomputed min/max bins for performance**
    int minBinBass, maxBinBass;
    int minBinMelody, maxBinMelody;
    int minBinVocal, maxBinVocal;

    // **Stored values for flatness, dynamic range, transients**
    int minBinForFlatness, maxBinForFlatness;

    // **History functions**
    void updateHistory(std::vector<double>& history, double newValue);
    double computeAverage(const std::vector<double>& history) const;

    // **Feature extraction functions**
    double calculateBassLevel(const std::vector<double>& fftResult);
    double calculateVocalLevel(const std::vector<double>& fftResult);
    double calculateHNR(const std::vector<double>& fftResult);
    double calculateBPM(const std::vector<double>& fftResult, double lastBPM);
    double calculateMelodyLevel(const std::vector<double>& fftResult);
    double calculatePeakFrequency(const std::vector<double>& fftResult);
    double calculateFlatness(const std::vector<double>& fftResult);
    double calculateDynamicRange(const std::vector<double>& fftResult);
    double calculateTransientDensity(const std::vector<double>& fftResult);
    double getBPM();
};

#endif // FREQUENCY_ANALYZER_H
