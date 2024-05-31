#include <iostream>
#include <cmath>
#include <fstream>
using namespace std;

const int sampleRate = 44100;
const int bitDepth = 16;

class SineOscillator {
    float m_Frequency;
    float m_Amplitude;
    float m_angle = 0.0f;
    float m_offset = 0.0f;
public:
    SineOscillator(float frequency, float amplitude) : m_Frequency(frequency), m_Amplitude(amplitude) {
        m_offset = 2 * M_PI * m_Frequency / sampleRate;
    }

    // y = A * sin(2 * pi * f * t + phi)
    float process() {
        // ASin(2pif/sr)      sr: sample rate
        auto sample = m_Amplitude * sin(m_angle);
        m_angle += m_offset;
        return sample;
    }
};

void writeToFile(ofstream& file, int value, int byteSize) {
    file.write(reinterpret_cast<const char*>(&value), byteSize);
}

int main() {
    int duration = 2;
    ofstream audioFile;
    audioFile.open("waveform.wav", ios::binary);

    SineOscillator sineOscillator(440, 0.5);

    // Header Chunk
    audioFile << "RIFF";
    audioFile << "----"; // file size를 넣을 공간
    audioFile << "WAVE";

    // Format Chunk
    audioFile << "fmt ";
    writeToFile(audioFile, 16, 4); // fmt chunk size , 추가 option은 제외함
    writeToFile(audioFile, 1, 2); // Compression code 압축 여부?
    writeToFile(audioFile, 1, 2); // Number of channels 채널 수
    writeToFile(audioFile, sampleRate, 4); // Sample rate 샘플링 레이트
    writeToFile(audioFile, sampleRate * bitDepth / 8, 4); // Average bytes per second  왜 8로 나누지..?
    writeToFile(audioFile, bitDepth / 8, 2); // Block align
    writeToFile(audioFile, bitDepth, 2); // Significant bits per sample  (Bit Depth)

    // Data Chunk
    audioFile << "data";
    audioFile << "----"; // data chunk size 비워둠

    int preAudioPosition = audioFile.tellp(); // 현재 파일 포인터 위치를 반환.  마킹해둠

    auto maxAmplitude = pow(2, bitDepth - 1) - 1;
    for (int i = 0; i < sampleRate * duration; ++i) {
        auto sample = sineOscillator.process();
        int intSample = static_cast<int>(sample * maxAmplitude);
        writeToFile(audioFile, intSample, 2);
    }
    int postAudioPosition = audioFile.tellp();

    audioFile.seekp(preAudioPosition - 4);
    writeToFile(audioFile, postAudioPosition - preAudioPosition, 4);

    audioFile.seekp(4, ios::beg);
    writeToFile(audioFile, postAudioPosition - 8, 4);
    audioFile.close();
    return 0;
}
