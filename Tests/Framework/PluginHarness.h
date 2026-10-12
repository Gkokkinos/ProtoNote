#pragma once

#include <algorithm>
#include <JuceHeader.h>

struct RenderResult {
    juce::AudioBuffer<float> audio;
    juce::MidiBuffer midi;
};

// Hosts a processor without a DAW: feeds it MIDI and renders audio block by block.
template <typename Processor>
class PluginHarness {
public:
    PluginHarness(double sampleRate = 44100.0, int blockSize = 512)
        : sampleRate_(sampleRate), blockSize_(blockSize) {
        processor.setRateAndBufferSizeDetails(sampleRate_, blockSize_);
        processor.prepareToPlay(sampleRate_, blockSize_);
    }

    ~PluginHarness() { processor.releaseResources(); }

    double sampleRate() const { return sampleRate_; }

    Processor processor;

    // Render numSamples, splitting the input MIDI across blocks like a host would.
    RenderResult render(const juce::MidiBuffer& midiIn, int numSamples) {
        const int numChannels = std::max(processor.getTotalNumInputChannels(),
                                         processor.getTotalNumOutputChannels());
        RenderResult result;
        result.audio.setSize(numChannels, numSamples);
        result.audio.clear();

        for (int start = 0; start < numSamples; start += blockSize_) {
            const int n = std::min(blockSize_, numSamples - start);

            juce::AudioBuffer<float> block(numChannels, n);
            block.clear();
            juce::MidiBuffer blockMidi;
            blockMidi.addEvents(midiIn, start, n, -start);

            processor.processBlock(block, blockMidi);

            for (int ch = 0; ch < numChannels; ++ch)
                result.audio.copyFrom(ch, start, block, ch, 0, n);
            result.midi.addEvents(blockMidi, 0, n, start);
        }
        return result;
    }

private:
    double sampleRate_;
    int blockSize_;
};
