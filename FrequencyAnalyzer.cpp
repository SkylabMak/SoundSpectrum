#include "FrequencyAnalyzer.h"
#include <cmath>
#include <algorithm>
#include <numeric>
#include <iostream>
#include <iomanip>
#include <bits/chrono.h>

FrequencyAnalyzer::FrequencyAnalyzer(int SamplingFrequency /*SAMPLING_FREQUENCY*/, int historySize, int bufferSize)
    : SamplingFrequency(SamplingFrequency), historySize(historySize)
{
    this->bufferSize = bufferSize;
    bassHistory.resize(historySize, 0);
    melodyHistory.resize(historySize, 0);
    peakFreqHistory.resize(historySize, 0);
    flatnessHistory.resize(historySize, 0);
    dynamicRangeHistory.resize(historySize, 0);
    transientHistory.resize(historySize, 0);
    vocalHistory.resize(historySize, 0);
    hnrHistory.resize(historySize, 0);

    // **Precompute FFT bin ranges for efficiency**
    minBinBass = static_cast<int>(20 * bufferSize / (SamplingFrequency / 2));
    maxBinBass = static_cast<int>(250 * bufferSize / (SamplingFrequency / 2));

    minBinMelody = static_cast<int>(500 * bufferSize / (SamplingFrequency / 2));
    maxBinMelody = static_cast<int>(4000 * bufferSize / (SamplingFrequency / 2));

    minBinVocal = static_cast<int>(250 * bufferSize / (SamplingFrequency / 2));
    maxBinVocal = static_cast<int>(3000 * bufferSize / (SamplingFrequency / 2));
    // maxBinVocal = static_cast<int>(400 * bufferSize / (SamplingFrequency / 2));

    minBinForFlatness = 0; // Full frequency range
    maxBinForFlatness = bufferSize - 1;

    lastBeatTime = std::chrono::steady_clock::now();
    // fftStorage.reserve(SamplingFrequency / bufferSize); // Store FFT results for 1 sec
}

double FrequencyAnalyzer::getBPM()
{
    // if (fftStorage.size() < SamplingFrequency / bufferSize)
    // {
    //     std::cout << " Not enough data yet" << std::endl;
    //     return lastBPM; // Not enough data yet
    // }

    // Compute energy in bass range (20-250Hz) for each stored FFT
    std::vector<double> bassEnergy;
    for (const auto &fft : fftStorage)
    {
        double energy = 0.0;
        for (int i = minBinBass; i <= maxBinBass; i++)
        {
            energy += fft.first[i]; // ✅ Corrected: Access the vector inside the pair
        }
        bassEnergy.push_back(energy);
    }

    // Find peaks in bass energy (potential beats)
    std::vector<std::pair<int, double>> beatCandidates; // Store (index, energy)

    for (size_t i = 1; i < bassEnergy.size() - 1; i++)
    {
        if (bassEnergy[i] > bassEnergy[i - 1] && bassEnergy[i] > bassEnergy[i + 1])
        {
            beatCandidates.push_back({i, bassEnergy[i]});
            // std::cout << std::left << bassEnergy[i] << " ";
        }
    }
    // std::cout << " : ";
    // Sort peaks by energy (highest first)
    std::sort(beatCandidates.begin(), beatCandidates.end(), [](const auto &a, const auto &b)
              {
                  return a.second > b.second; // Sort by energy value (descending)
              });

    // Keep only the top 2 strongest peaks
    std::vector<int> beatIndices;
    for (size_t i = 0; i < std::min(beatCandidates.size(), (size_t)2); i++)
    {
        beatIndices.push_back(beatCandidates[i].first);
    }

    // std::cout << std::left << "Detected Beats: " << std::setw(20) << beatIndices.size();

    if (beatIndices.size() < 2)
    {
        // std::cout << std::setw(20) << " Not enough beats detected";
        return lastBPM; // Not enough beats detected
    }

    static auto firstBeatTime = fftStorage.front().second;

    std::vector<double> beatTimestamps;
    for (int index : beatIndices)
    {
        double relativeTime = std::chrono::duration<double>(
                                  fftStorage[index].second - firstBeatTime)
                                  .count();
        beatTimestamps.push_back(relativeTime);
    }

    std::vector<double> intervals;
    for (size_t i = 1; i < beatTimestamps.size(); i++)
    {
        double interval = beatTimestamps[i] - beatTimestamps[i - 1]; // Actual time difference
        intervals.push_back(interval);
    }

    // Compute BPM: 60 / average interval
    double avgInterval = std::accumulate(intervals.begin(), intervals.end(), 0.0) / intervals.size();

    // std::cout << std::setw(20) << "avgInterval" << avgInterval;
    // std::cout << std::setw(20) << "test BPM" << 60.0 / avgInterval << std::setw(20);
    // filter
    int bpmABS = abs(60.0 / avgInterval); // in 1 sec
    int bpmABS2 = bpmABS * 2;
    int bpmABS3 = bpmABS * 3;
    if ((bpmABS2 < lastBPM + 5) && (bpmABS2 > lastBPM - 5))
    {
        // std::cout << std::setw(20) << "2 work" << std::setw(20);
        bpmABS = bpmABS2;
    }
    else if ((bpmABS3 < lastBPM + 5) && (bpmABS3 > lastBPM - 5))
    {
        // std::cout << std::setw(20) << "3 work" << std::setw(20);
        bpmABS = bpmABS3;
    }
    else if (bpmABS < 61.0)
    {
        bpmABS = lastBPM;
    }
    else if (bpmABS > 220)
    {
        bpmABS = 0;
    }
    if (avgInterval > 0.2 && avgInterval < 2.0)
    { // Ensure BPM is within 30-300 range
        lastBPM = bpmABS;
    }
    else
    {
        lastBPM = lastBPM; // Keep the last valid BPM
    }
    // std::cout << std::setw(20) << "BPM" << bpmABS << std::setw(20);

    return bpmABS;
}

double FrequencyAnalyzer::getCurrentBPM()
{
    return lastBPM;
}

void FrequencyAnalyzer::updateHistory(std::vector<double> &history, double newValue)
{
    history[historyIndex] = newValue;
}

double FrequencyAnalyzer::computeAverage(const std::vector<double> &history) const
{
    int validSize = std::min(historySize, historyCount);
    if (validSize == 0)
        return 0; // ✅ Avoid division by zero
    return std::accumulate(history.begin(), history.begin() + validSize, 0.0) / validSize;
}

double FrequencyAnalyzer::calculateVocalLevel(const std::vector<double> &fftResult)
{
    double vocalSum = 0.0;
    for (int i = minBinVocal; i <= maxBinVocal; i++)
    {
        vocalSum += fftResult[i];
    }
    return vocalSum;
}

double FrequencyAnalyzer::calculateHNR(const std::vector<double> &fftResult)
{
    double harmonicEnergy = 0.0, noiseEnergy = 0.0;
    for (int i = minBinVocal; i <= maxBinVocal; i++)
    {
        if (i % 2 == 0)
            harmonicEnergy += fftResult[i];
        else
            noiseEnergy += fftResult[i];
    }
    return (noiseEnergy == 0) ? 0 : 10 * log10(harmonicEnergy / noiseEnergy);
}

//  Extract Bass Level (20Hz - 250Hz)
double FrequencyAnalyzer::calculateBassLevel(const std::vector<double> &fftResult)
{
    double bassSum = 0.0;
    for (int i = minBinBass; i <= maxBinBass; i++)
        bassSum += fftResult[i];
    return bassSum;
}

// Extract Melody Level (500Hz - 4000Hz)
double FrequencyAnalyzer::calculateMelodyLevel(const std::vector<double> &fftResult)
{
    double melodySum = 0.0;
    for (int i = minBinMelody; i <= maxBinMelody; i++)
        melodySum += fftResult[i];
    return melodySum;
}

double FrequencyAnalyzer::calculatePeakFrequency(const std::vector<double> &fftResult)
{
    int peakIndex = std::distance(fftResult.begin(), std::max_element(fftResult.begin(), fftResult.end()));
    return (peakIndex * SamplingFrequency) / fftResult.size();
}

double FrequencyAnalyzer::calculateFlatness(const std::vector<double> &fftResult)
{
    double geometricMean = 1.0, arithmeticMean = 0.0;
    int n = maxBinForFlatness - minBinForFlatness + 1;

    for (int i = minBinForFlatness; i <= maxBinForFlatness; i++)
    {
        geometricMean *= (fftResult[i] > 0 ? fftResult[i] : 1);
        arithmeticMean += fftResult[i];
    }

    geometricMean = std::pow(geometricMean, 1.0 / n);
    arithmeticMean /= n;
    return geometricMean / arithmeticMean;
}

double FrequencyAnalyzer::calculateDynamicRange(const std::vector<double> &fftResult)
{
    double maxVal = *std::max_element(fftResult.begin(), fftResult.end());
    double minVal = *std::min_element(fftResult.begin(), fftResult.end());
    return maxVal - minVal;
}

double FrequencyAnalyzer::calculateTransientDensity(const std::vector<double> &fftResult)
{
    int count = 0;
    for (size_t i = 1; i < fftResult.size(); i++)
    {
        // std::cout << std::abs(fftResult[i] - fftResult[i - 1]) << " " ;
        if (std::abs(fftResult[i] - fftResult[i - 1]) > 10)
            count++;
    }
    // std::cout<<"TransientDensity" << count << " " ;
    return count;
}

// Pattern Detection with History Smoothing
Pattern FrequencyAnalyzer::detectPattern(const std::vector<double> &fftResult)
{
    // 1️ Extract parameters
    double bassLevel = calculateBassLevel(fftResult);
    double melodyLevel = calculateMelodyLevel(fftResult);
    double peakFreq = calculatePeakFrequency(fftResult);
    double flatness = calculateFlatness(fftResult);
    double dynamicRange = calculateDynamicRange(fftResult);
    double transients = calculateTransientDensity(fftResult);
    double vocalLevel = calculateVocalLevel(fftResult);
    double hnr = calculateHNR(fftResult);

    // 2️ Store in history
    updateHistory(bassHistory, bassLevel);
    updateHistory(melodyHistory, melodyLevel);
    updateHistory(peakFreqHistory, peakFreq);
    updateHistory(flatnessHistory, flatness);
    updateHistory(dynamicRangeHistory, dynamicRange);
    updateHistory(transientHistory, transients);
    updateHistory(vocalHistory, vocalLevel);
    updateHistory(hnrHistory, hnr);

    // 3️ Increment history index (circular buffer)
    historyIndex = (historyIndex + 1) % historySize;
    if (historyCount < historySize)
        historyCount++;

    // 4️ If history is not full, return a default pattern
    if (historyCount < historySize)
        return Pattern::WAVEFORM_SWEEP; // Default

    // 4.2 Store FFT result with timestamp
    auto now = std::chrono::steady_clock::now();
    fftStorage.push_back({fftResult, now});
    // std::cout << fftStorage.size() << std::endl;
    bool hasRemoved = false; // Track if we removed any outdated FFTs

    while (!fftStorage.empty())
    {
        double elapsedTime = std::chrono::duration<double>(now - fftStorage.front().second).count();

        if (elapsedTime < period)
        {
            break; // Stop when we reach valid FFTs
        }
        else
        {
            fftStorage.pop_front(); // Remove outdated FFT
            hasRemoved = true;
        }
    }

    if (hasRemoved && !fftStorage.empty())
    {
        double bpm = getBPM();
        lastBeatTime = now;
        // std::cout << std::left << "Estimated new BPM: " << bpm <<" :: "<< std::setw(500);
    }
    // else std::cout << std::left << "Estimated old BPM: " << lastBPM <<" :: "<< std::setw(500);

    // 5️ Compute avg smoothed values
    double avgBass = computeAverage(bassHistory) * volume;
    double avgMelody = computeAverage(melodyHistory) * volume;
    double avgPeakFreq = computeAverage(peakFreqHistory) * volume;
    double avgFlatness = computeAverage(flatnessHistory) * volume;
    double avgDynamicRange = computeAverage(dynamicRangeHistory) * volume;
    double avgTransients = computeAverage(transientHistory) * volume;
    double avgVocal = computeAverage(vocalHistory) * volume;
    double avgHNR = computeAverage(hnrHistory) * volume;

    std::cout
    << "PF: " << std::setw(5) << avgPeakFreq << "   "
    << "F: " << std::setw(10) << avgFlatness << "   "
    << "D: " << std::setw(8) << avgDynamicRange << " dB   "
    << "T: " << std::setw(10) << avgTransients << "   "
    << "B: " << std::setw(8) << avgBass << "   "
    << "M:" << std::setw(8) << avgMelody << "   "
    << "V: " << std::setw(10) << avgVocal << "   "
    << "H: " << std::setw(10) << avgHNR << " dB   "
              << " : ";

    // 6️ Choose pattern based on **averaged values**
    // if (avgVocal > avgBass && avgVocal > avgMelody && avgHNR > 10)
    bool vocalDetect = (avgVocal > avgBass && avgVocal > avgMelody && (avgFlatness > 0.2 && avgFlatness < 0.4));
    bool onlyVocalDetect = (avgVocal > avgBass && avgVocal > avgMelody && (avgFlatness > 0.2 && avgFlatness < 0.4) && avgHNR > 1);
    bool melodyPeakDetect = (avgMelody > avgBass);
    bool bassPeakDetect = (avgMelody < avgBass);
    // std::cout << std::setw(10) << onlyVocalDetect << (avgTransients < 0.1 )<<(avgDynamicRange < 9);
    if (onlyVocalDetect && avgTransients < 0.1 && avgDynamicRange < 9) //"VOCAL"
    {
        std::cout << std::setw(10) << "ONLYVOCAL";
        return Pattern::FIGURE_8_MOTION; // 5
    }

    if (avgBass < 10 && avgMelody < 10 && avgVocal < 10) // soft
    {
        std::cout << std::setw(10) << "SOFT";
        return Pattern::RANDOM_SWIRL; // 6
    }
    if (melodyPeakDetect && avgFlatness < 0.2) // MELODY
    {
        std::cout << std::setw(10) << "MELODY";
        return Pattern::SPIRAL_EXPANSION; // 2
    }
    if (avgBass > 25 && avgTransients > 0.5) // Drum and bass
    {
        std::cout << std::setw(10) << "D & B";
        return Pattern::BASS_SPIKES; // 1
    }
    if (avgFlatness > 0.4 && (avgTransients > 0.1 && avgTransients < 0.7) && avgDynamicRange > 9) // EDM
    {
        std::cout << std::setw(10) << "EDM";
        return Pattern::STARBURST; // 4
    }
    if (lastBPM > 120 && avgDynamicRange > 20) // 3
    {

        std::cout << std::setw(10) << "SPEED";
        return Pattern::BPM_SCANNING;
    }
    if (avgDynamicRange < 9 && avgTransients > 0.01 && avgTransients < 0.5 && vocalDetect) //"VOCAL"
    {
        std::cout << std::setw(10) << "VOCALMIX";
        return Pattern::RANDOM_SWIRL; // 6
    }
    // if ((avgTransients > 0.1 && avgTransients < 0.7) && avgTransients < 0.5 ) //
    if (avgTransients > 0.1 && avgTransients < 0.5) //
    {
        std::cout << std::setw(10) << "ChillPop";
        return Pattern::RANDOM_SWIRL; // 6
    }
    if (avgTransients > 0.1 && avgFlatness > 0.05) //
    {
        std::cout << std::setw(10) << "Chill1";
        return Pattern::RANDOM_SWIRL; // 6
    }
    if (avgTransients > 0.05 && avgPeakFreq > 300) //
    {

        std::cout << std::setw(10) << "Chill2";
        return Pattern::RANDOM_SWIRL; // 6
    }
    if (avgDynamicRange < 7) //
    {
        std::cout << std::setw(10) << "Chill3";
        return Pattern::FIGURE_8_MOTION;
    }
    std::cout << std::setw(10) << "Default";
    return Pattern::WAVEFORM_SWEEP; // Default 0
}
