#pragma once

#include <algorithm>
#include <cmath>
#include <memory>
#include <utility>
#include <vector>
#include <gtest/gtest.h>
#include <JuceHeader.h>

namespace PluginTest {

inline juce::MidiBuffer makeMidi(const std::vector<std::pair<int, juce::MidiMessage>>& events) {
    juce::MidiBuffer buffer;
    for (const auto& [position, message] : events)
        buffer.addEvent(message, position);
    return buffer;
}

inline testing::AssertionResult midiMatches(const juce::MidiBuffer& actual, const juce::MidiBuffer& expected) {
    auto a = actual.begin();
    auto e = expected.begin();
    for (int i = 0; a != actual.end() && e != expected.end(); ++a, ++e, ++i) {
        const auto actualEvent = *a;
        const auto expectedEvent = *e;
        const auto am = actualEvent.getMessage();
        const auto em = expectedEvent.getMessage();
        const bool same = actualEvent.samplePosition == expectedEvent.samplePosition
                          && am.getRawDataSize() == em.getRawDataSize()
                          && std::equal(am.getRawData(), am.getRawData() + am.getRawDataSize(), em.getRawData());
        if (!same)
            return testing::AssertionFailure()
                   << "MIDI event " << i << " differs: actual [" << actualEvent.samplePosition << "] "
                   << am.getDescription() << ", expected [" << expectedEvent.samplePosition << "] "
                   << em.getDescription();
    }
    if (a != actual.end() || e != expected.end())
        return testing::AssertionFailure() << "MIDI event counts differ";
    return testing::AssertionSuccess();
}

inline testing::AssertionResult audioMatches(const juce::AudioBuffer<float>& actual,
                                             const juce::AudioBuffer<float>& expected,
                                             float tolerance = 1e-6f) {
    if (actual.getNumChannels() != expected.getNumChannels() || actual.getNumSamples() != expected.getNumSamples())
        return testing::AssertionFailure()
               << "shape differs: actual " << actual.getNumChannels() << "ch x " << actual.getNumSamples()
               << ", expected " << expected.getNumChannels() << "ch x " << expected.getNumSamples();

    for (int ch = 0; ch < actual.getNumChannels(); ++ch)
        for (int i = 0; i < actual.getNumSamples(); ++i) {
            const float diff = std::abs(actual.getSample(ch, i) - expected.getSample(ch, i));
            if (diff > tolerance)
                return testing::AssertionFailure()
                       << "channel " << ch << " sample " << i << " differs by " << diff
                       << " (actual " << actual.getSample(ch, i) << ", expected " << expected.getSample(ch, i) << ")";
        }
    return testing::AssertionSuccess();
}

inline bool writeWav(const juce::File& file, const juce::AudioBuffer<float>& audio, double sampleRate) {
    file.getParentDirectory().createDirectory();
    file.deleteFile();

    juce::WavAudioFormat wav;
    std::unique_ptr<juce::OutputStream> stream = file.createOutputStream();
    if (stream == nullptr)
        return false;

    auto writer = wav.createWriterFor(stream, juce::AudioFormatWriterOptions{}
                                                  .withSampleRate(sampleRate)
                                                  .withNumChannels(audio.getNumChannels())
                                                  .withBitsPerSample(32));
    return writer != nullptr && writer->writeFromAudioSampleBuffer(audio, 0, audio.getNumSamples());
}

inline bool readWav(const juce::File& file, juce::AudioBuffer<float>& audio, double& sampleRate) {
    juce::WavAudioFormat wav;
    auto stream = file.createInputStream();
    if (stream == nullptr)
        return false;

    std::unique_ptr<juce::AudioFormatReader> reader(wav.createReaderFor(stream.release(), true));
    if (reader == nullptr)
        return false;

    sampleRate = reader->sampleRate;
    audio.setSize(static_cast<int>(reader->numChannels), static_cast<int>(reader->lengthInSamples));
    return reader->read(&audio, 0, audio.getNumSamples(), 0, true, true);
}

}  // namespace PluginTest
