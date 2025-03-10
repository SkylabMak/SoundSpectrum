#include <iostream>
#include "WasapiAudio.h"
#include <thread>
#include <algorithm>
#include "FFTProcessor.h"
#include "config.h"
#include "LEDMatrixVisualizer.h"
#include "SerialComm.h"
#include "FrequencyAnalyzer.h"
const char *COM_PORT = "\\\\.\\COM5";
#include <chrono>
#include <unistd.h>

int main()
{
    WasapiAudio audio;
    if (!audio.initialize())
    {
        std::cerr << " Failed to initialize WASAPI!" << std::endl;
        return -1;
    }

    if (!audio.startCapture())
    {
        std::cerr << "Failed to start capturing!" << std::endl;
        return -1;
    }

    if (!openSerialPort(COM_PORT))
    {
        std::cerr << "Failed to open serial port." << std::endl;
        return -1;
    }

    std::cout << audio.getSamplingRate() << std::endl;
    FFTProcessor fft(FFT_SAMPLES, audio.getSamplingRate());
    LEDMatrixVisualizer visualizer;
    FrequencyAnalyzer analyzer(audio.getSamplingRate(), 200, FFT_SAMPLES / 2);
    std::vector<float> buffer;
    while (true)
    {
        auto start = std::chrono::high_resolution_clock::now();

        if (audio.getAudioData(buffer))
        {
            auto timeAudio = std::chrono::high_resolution_clock::now();
            // std::cout << "Audio Capture Time: "
            //           << std::chrono::duration<double, std::milli>(timeAudio - start).count()
            //           << " ms" << std::endl;

            // std::cout << " Captured " << buffer.size() << " samples" << std::endl;

            /* ---------------------------------- FFT process ----------------------------------*/
            auto timefft = std::chrono::high_resolution_clock::now();
            std::vector<double> fftResult = fft.processFFT(buffer);
            // std::cout << "FFT time:"
            //           << std::chrono::duration<double, std::milli>(timefft - timeAudio).count()
            //           << " ms" << std::endl;
            // std::cout << fftResult.size() << std::endl;

            /* ---------------------------------- Frequency analysis process ----------------------------------*/
            Pattern detectedPattern = analyzer.detectPattern(fftResult);
            double bpm = analyzer.getCurrentBPM();
            // auto timePattern = std::chrono::high_resolution_clock::now();
            // std::cout << "Detected Pattern: " << static_cast<int>(detectedPattern) << " ";
            // std::cout << "bpm: " << bpm << " ";
            // std::cout << "Pattern Detection Time: "
            //           << std::chrono::duration<double, std::milli>(timePattern - timefft).count()
            //           << " ms" << std::endl;

            /* ---------------------------------- Frequency Customize process ----------------------------------*/
            // --- crop ---
            std::vector<double> fftfilter = fft.filterFrequencies(fftResult);
            auto timeFilter = std::chrono::high_resolution_clock::now();
            // std::cout << "FFT Filtering Time: "
            //           << std::chrono::duration<double, std::milli>(timeFilter - timePattern).count()
            //           << " ms" << std::endl;

            // --- equalizer ---
            std::vector<double> fftEq = fft.applyEqualizer(fftfilter);
            auto timeEq = std::chrono::high_resolution_clock::now();
            // std::cout << "Equalization Time: "
            //           << std::chrono::duration<double, std::milli>(timeEq - timeFilter).count()
            //           << " ms" << std::endl;

            /* ---------------------------------- LED Matrix process ----------------------------------*/
            // --- normaliz Y ---
            // visualizer.normalizeFFTData(fftEq);
            // visualizer.normalizeFFTData(fftfilter);

            // auto timeNormalize = std::chrono::high_resolution_clock::now();
            // std::cout << "Normalization Time: "
            //           << std::chrono::duration<double, std::milli>(timeNormalize - timeEq).count()
            //           << " ms" << std::endl;

            // --- normaliz to led ---
            std::vector<int> ledMatrix = visualizer.transformFFTData(fftEq);
            // std::vector<int> ledMatrix = visualizer.transformFFTData(fftfilter);
            auto timeLedMatrix = std::chrono::high_resolution_clock::now();
            // std::cout << "LED Matrix Transform Time: "
            //           << std::chrono::duration<double, std::milli>(timeLedMatrix - timeNormalize).count()
            //           << " ms" << std::endl;

            /* ---------------------------------- communication process ----------------------------------*/
            // --- prepare ---
            std::string ledData = "";
            for (int j = 0; j < ledMatrix.size(); j++)
            {
                ledData += std::to_string(ledMatrix[j]) + " ";
                // std::cout <<ledMatrix[j] << " ";
            }
            ledData += ":";
            ledData += " ";
            ledData += std::to_string(static_cast<int>(detectedPattern));
            ledData += " ";
            ledData += std::to_string((int)bpm);
            ledData += "\n";
            // std::cout <<ledData ;
            std::cout << " " << std::to_string(static_cast<int>(detectedPattern)) << " ";
            // std::cout << std::to_string((int)bpm) << " ";
            std::cout << std::endl;

            /* send */
            writeToSerial(ledData);
            auto timeSerial = std::chrono::high_resolution_clock::now();
            // std::cout << "Serial Write Time: "
            //           << std::chrono::duration<double, std::milli>(timeSerial - timeLedMatrix).count()
            //           << " ms" << std::endl;

            // std::cout << "Total Processing Time: "
            //           << std::chrono::duration<double, std::milli>(timeSerial - start).count()
            //           << " ms" << std::endl;

            // sleep(1);
        }
    }

    audio.stopCapture();
    return 0;
}
