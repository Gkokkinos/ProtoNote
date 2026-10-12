#pragma once

#include <string>
#include <gtest/gtest.h>
#include <JuceHeader.h>
#include "TestUtils.h"

namespace PluginTest {

inline bool updateGolden() {
    return juce::SystemStats::getEnvironmentVariable("UPDATE_GOLDEN", {}) == "1";
}

inline juce::File goldenFile(const std::string& name) {
    return juce::File(PLUGIN_TEST_DATA_DIR).getChildFile("golden").getChildFile(name);
}

// Overwrites Test_Data/golden/<name> with the given audio.
inline testing::AssertionResult recordGolden(const std::string& name,
                                             const juce::AudioBuffer<float>& audio,
                                             double sampleRate) {
    const auto file = goldenFile(name);
    if (!writeWav(file, audio, sampleRate))
        return testing::AssertionFailure() << "could not write golden file " << file.getFullPathName();
    return testing::AssertionSuccess();
}

// Compares against Test_Data/golden/<name>; with UPDATE_GOLDEN=1 it records instead.
inline testing::AssertionResult matchesGolden(const std::string& name,
                                              const juce::AudioBuffer<float>& actual,
                                              double sampleRate,
                                              float tolerance = 1e-6f) {
    if (updateGolden())
        return recordGolden(name, actual, sampleRate);

    const auto file = goldenFile(name);
    juce::AudioBuffer<float> expected;
    double expectedRate = 0.0;
    if (!readWav(file, expected, expectedRate))
        return testing::AssertionFailure() << "golden file missing or unreadable: " << file.getFullPathName()
                                           << " (run with UPDATE_GOLDEN=1 to record it)";
    if (expectedRate != sampleRate)
        return testing::AssertionFailure() << "sample rate differs: actual " << sampleRate
                                           << ", golden " << expectedRate;
    return audioMatches(actual, expected, tolerance);
}

}  // namespace PluginTest
