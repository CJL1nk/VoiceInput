#include <iostream>
#include <fstream>
#include <algorithm>
#include <portaudio.h>
#include <Windows.h>

// Constants
#define FORMAT       paInt16
#define CHANNELS     1
#define SAMPLE_RATE  44100
#define CHUNK_SIZE   1024

// Global variable to track whether the microphone is currently active
bool isListening = false;

// Global variable to store the threshold
int threshold = 0;

void sendMouseInput(bool down, bool button) {
    POINT pos{}; GetCursorPos(&pos);

    down ? (button ? SendMessage(GetForegroundWindow(), WM_LBUTTONDOWN, MK_LBUTTON, MAKELPARAM(pos.x, pos.y)) :
        SendMessage(GetForegroundWindow(), WM_RBUTTONDOWN, MK_RBUTTON, MAKELPARAM(pos.x, pos.y))) :
        (button ? SendMessage(GetForegroundWindow(), WM_LBUTTONUP, MK_LBUTTON, MAKELPARAM(pos.x, pos.y)) :
            SendMessage(GetForegroundWindow(), WM_RBUTTONUP, MK_RBUTTON, MAKELPARAM(pos.x, pos.y)));
}

// Function to be called when voice input is detected
void onVoiceStart() {
    std::cout << "Sending Input" << std::endl;
    sendMouseInput(true, true);
}

// Function to be called when voice input stops
void onVoiceStop() {
    std::cout << "Stopping Input" << std::endl;
    sendMouseInput(false, true);
}

// Callback function for audio stream
int audioCallback(const void* inputBuffer, void* outputBuffer,
    unsigned long framesPerBuffer,
    const PaStreamCallbackTimeInfo* timeInfo,
    PaStreamCallbackFlags statusFlags,
    void* userData) {
    const short* audioData = static_cast<const short*>(inputBuffer);

    // Check if voice input is above the threshold
    if (*std::max_element(audioData, audioData + framesPerBuffer) > threshold) {
        if (!isListening) {
            isListening = true;
            onVoiceStart();
        }
    }
    else {
        if (isListening) {
            isListening = false;
            onVoiceStop();
        }
    }

    return paContinue;
}
int main() {
    PaError err;

    err = Pa_Initialize();
    if (err != paNoError) {
        std::cerr << "PortAudio initialization failed: " << Pa_GetErrorText(err) << std::endl;
        return 1;
    }

    // Read the threshold from the file only once at the beginning
    std::ifstream thresholdFile("threshold.john");
    if (thresholdFile >> threshold) {
        std::cout << "Threshold read from file: " << threshold << std::endl;
    }
    else {
        std::cerr << "Error reading threshold from file." << std::endl;
        Pa_Terminate();
        return 1;
    }

    PaStream* stream;
    err = Pa_OpenDefaultStream(&stream, CHANNELS, 0, FORMAT, SAMPLE_RATE,
        CHUNK_SIZE, audioCallback, nullptr);

    if (err != paNoError) {
        std::cerr << "PortAudio stream opening failed: " << Pa_GetErrorText(err) << std::endl;
        Pa_Terminate();
        return 1;
    }

    err = Pa_StartStream(stream);
    if (err != paNoError) {
        std::cerr << "PortAudio stream starting failed: " << Pa_GetErrorText(err) << std::endl;
        Pa_CloseStream(stream);
        Pa_Terminate();
        return 1;
    }

    std::cout << "Listening for voice input..." << std::endl;
    std::cout << "Press Ctrl+C to stop." << std::endl;

    while (true) {
        Pa_Sleep(100);  // Sleep for a short duration to reduce CPU usage
    }

    err = Pa_StopStream(stream);
    if (err != paNoError) {
        std::cerr << "PortAudio stream stopping failed: " << Pa_GetErrorText(err) << std::endl;
    }

    err = Pa_CloseStream(stream);
    if (err != paNoError) {
        std::cerr << "PortAudio stream closing failed: " << Pa_GetErrorText(err) << std::endl;
    }

    Pa_Terminate();

    return 0;
}
