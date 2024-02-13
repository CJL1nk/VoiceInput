#pragma once

#include <iostream>
#include <vector>
#include <Windows.h>
#include <portaudio.h>

double kSilenceThreshold = 0.0001;  // Adjust this threshold for sensitivity
constexpr unsigned int kBufferSize = 512;
bool isOn = false;