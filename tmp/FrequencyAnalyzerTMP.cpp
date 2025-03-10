#include "FrequencyAnalyzer.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <iomanip>

FrequencyAnalyzer::FrequencyAnalyzer(int SamplingFrequency /*SAMPLING_FREQUENCY*/, int historySize,int bufferSize)
    : SamplingFrequency(SamplingFrequency), historySize(historySize) {
    bassHistory.resize(historySize, 0);
    melodyHistory.resize(historySize, 0);
    peakFreqHistory.resize(historySize, 0);
    flatnessHistory.resize(historySize, 0);
    dynamicRangeHistory.resize(historySize, 0);
    transientHistory.resize(historySize, 0);
    bpmHistory.resize(historySize, 0);

    // **Precompute FFT bin ranges for efficiency**
    minBinBass = static_cast<int>(20 * bufferSize / (SamplingFrequency / 2));
    maxBinBass = static_cast<int>(250 * bufferSize / (SamplingFrequency / 2));

    minBinMelody = static_cast<int>(500 * bufferSize / (SamplingFrequency / 2));
    maxBinMelody = static_cast<int>(4000 * bufferSize / (SamplingFrequency / 2));

    minBinForFlatness = 0;  // Full frequency range
    maxBinForFlatness = bufferSize - 1;
}

// ✅ Update history with new value (circular buffer)
void FrequencyAnalyzer::updateHistory(std::vector<double>& history, double newValue) {
    history[historyIndex] = newValue;
}

// ✅ Compute the average of history (ignoring unused slots if not full)
double FrequencyAnalyzer::computeAverage(const std::vector<double>& history) const {
    int validSize = std::min(historySize, historyCount);
    if (validSize == 0) return 0;  // ✅ Avoid division by zero
    return std::accumulate(history.begin(), history.begin() + validSize, 0.0) / validSize;
}

// ✅ Extract Bass Level (20Hz - 250Hz)
double FrequencyAnalyzer::calculateBassLevel(const std::vector<double>& fftResult) {
    double bassSum = 0.0;
    for (int i = minBinBass; i <= maxBinBass; i++) bassSum += fftResult[i];
    return bassSum;
}

// ✅ Extract Melody Level (500Hz - 4000Hz)
double FrequencyAnalyzer::calculateMelodyLevel(const std::vector<double>& fftResult) {
    double melodySum = 0.0;
    for (int i = minBinMelody; i <= maxBinMelody; i++) melodySum += fftResult[i];
    return melodySum;
}

// ✅ Find Peak Frequency (Most dominant frequency)
double FrequencyAnalyzer::calculatePeakFrequency(const std::vector<double>& fftResult) {
    int peakIndex = std::distance(fftResult.begin(), std::max_element(fftResult.begin(), fftResult.end()));
    return (peakIndex * SamplingFrequency) / fftResult.size();
}

// ✅ Calculate Spectral Flatness
double FrequencyAnalyzer::calculateFlatness(const std::vector<double>& fftResult) {
    double geometricMean = 1.0, arithmeticMean = 0.0;
    int n = maxBinForFlatness - minBinForFlatness + 1;

    for (int i = minBinForFlatness; i <= maxBinForFlatness; i++) {
        geometricMean *= (fftResult[i] > 0 ? fftResult[i] : 1);
        arithmeticMean += fftResult[i];
    }

    geometricMean = std::pow(geometricMean, 1.0 / n);
    arithmeticMean /= n;
    return geometricMean / arithmeticMean;
}

// ✅ Calculate Dynamic Range
double FrequencyAnalyzer::calculateDynamicRange(const std::vector<double>& fftResult) {
    double maxVal = *std::max_element(fftResult.begin(), fftResult.end());
    double minVal = *std::min_element(fftResult.begin(), fftResult.end());
    return maxVal - minVal;
}

// ✅ Calculate Transient Density
double FrequencyAnalyzer::calculateTransientDensity(const std::vector<double>& fftResult) {
    int count = 0;
    for (size_t i = 1; i < fftResult.size(); i++) {
        if (std::abs(fftResult[i] - fftResult[i - 1]) > 50) count++;
    }
    return count;
}

double FrequencyAnalyzer::calculateBPM(const std::vector<double>& fftResult, double lastBPM) {
    static std::vector<double> bpmHistoryBuffer;
    static double lastPeakTime = 0;
    double currentTime = static_cast<double>(clock()) / CLOCKS_PER_SEC;

    double bassLevel = calculateBassLevel(fftResult);
    double threshold = 0.6 * bassLevel;  // ✅ Dynamic threshold based on bass

    if (lastPeakTime == 0 || (currentTime - lastPeakTime) <= 0) {
        lastPeakTime = currentTime;
        return (lastBPM > 0 && lastBPM < 300) ? lastBPM : 0;  // ✅ No BPM until valid detection
    }

    if (bassLevel > threshold && (currentTime - lastPeakTime) > 0.3) {
        double interval = currentTime - lastPeakTime;
        double bpm = 60.0 / interval;
        lastPeakTime = currentTime;

        if (bpm < 40 || bpm > 250) return lastBPM;  // ✅ Ignore unrealistic BPM

        // ✅ Store BPM history (limit buffer size to 5)
        bpmHistoryBuffer.push_back(bpm);
        if (bpmHistoryBuffer.size() > 5) bpmHistoryBuffer.erase(bpmHistoryBuffer.begin());

        // ✅ Compute average BPM
        double sumBPM = std::accumulate(bpmHistoryBuffer.begin(), bpmHistoryBuffer.end(), 0.0);
        return sumBPM / bpmHistoryBuffer.size();
    }

    return lastBPM;
}




// ✅ Pattern Detection with History Smoothing
Pattern FrequencyAnalyzer::detectPattern(const std::vector<double>& fftResult) {
    // 1️⃣ Extract parameters
    double bassLevel = calculateBassLevel(fftResult);
    double melodyLevel = calculateMelodyLevel(fftResult);
    double peakFreq = calculatePeakFrequency(fftResult);
    double flatness = calculateFlatness(fftResult);
    double dynamicRange = calculateDynamicRange(fftResult);
    double transients = calculateTransientDensity(fftResult);
    std::cout << std::left  // Output for debugging
    << std::setw(10) << computeAverage(bpmHistory);
    double bpm = calculateBPM(fftResult, computeAverage(bpmHistory));  // คำนวณ BPM

    // 2️⃣ Store in history
    updateHistory(bassHistory, bassLevel);
    updateHistory(melodyHistory, melodyLevel);
    updateHistory(peakFreqHistory, peakFreq);
    updateHistory(flatnessHistory, flatness);
    updateHistory(dynamicRangeHistory, dynamicRange);
    updateHistory(transientHistory, transients);
    updateHistory(bpmHistory, bpm);

    // 3️⃣ Increment history index (circular buffer)
    historyIndex = (historyIndex + 1) % historySize;
    if (historyCount < historySize) historyCount++;

    // 4️⃣ If history is not full, return a default pattern
    if (historyCount < historySize) return Pattern::WAVEFORM_SWEEP; // Default

    
    // 5️⃣ Compute smoothed values
    double avgBPM = computeAverage(bpmHistory);
    double avgBass = computeAverage(bassHistory);
    double avgMelody = computeAverage(melodyHistory);
    double avgPeakFreq = computeAverage(peakFreqHistory);
    double avgFlatness = computeAverage(flatnessHistory);
    double avgDynamicRange = computeAverage(dynamicRangeHistory);
    double avgTransients = computeAverage(transientHistory);

    std::cout << std::left  // Output for debugging
        << std::setw(10) << avgBass
        << std::setw(10) << avgMelody
        << std::setw(10) << avgPeakFreq
        << std::setw(10) << avgFlatness //Noise หรือเป็นเสียงที่มี Pitch (โน้ต) ชัดเจน (ความ clear ของ note)
        << std::setw(10) << avgDynamicRange //ช่วงความแตกต่างระหว่างเสียงที่เบาที่สุดและเสียงที่ดังที่สุดในเพลง
        << std::setw(10) << avgTransients  //จำนวนจุดที่มีการเปลี่ยนแปลงพลังของเสียงอย่างรวดเร็ว
        << std::setw(10) <<  avgBPM 
        << ":";

    // 6️⃣ Choose pattern based on **averaged values**
    if (avgBPM > 130) return Pattern::BPM_SCANNING;
    if (avgMelody > 50 && avgFlatness < 0.5) return Pattern::WAVEFORM_SWEEP;
    if (avgBass > 60 && avgTransients > 0.03) return Pattern::BASS_SPIKES;
    if (avgPeakFreq > 5000 && avgDynamicRange > 20) return Pattern::BPM_SCANNING;
    if (avgTransients > 0.1) return Pattern::STARBURST;
    if (avgFlatness > 0.6 && avgDynamicRange < 10) return Pattern::FIGURE_8_MOTION;
    if (avgDynamicRange > 25) return Pattern::SPIRAL_EXPANSION;
    if (avgTransients > 0.05 && avgPeakFreq > 300) return Pattern::BEAT_SYNCED_BLINKING;

    return Pattern::WAVEFORM_SWEEP; // Default
}
