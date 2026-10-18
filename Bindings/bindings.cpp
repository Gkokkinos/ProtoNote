// pybind11 bindings for the ProtoNote processor + test harness.
// Mirrors the C++ test framework (PluginHarness/TestUtils) 

#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <pybind11/stl.h>

#include <JuceHeader.h>
#include "PluginProcessor.h"
#include "PluginHarness.h"

namespace py = pybind11;

namespace {

juce::MidiMessage messageFromBytes(const py::bytes& data) {
    char* buffer = nullptr;
    Py_ssize_t length = 0;
    if (PyBytes_AsStringAndSize(data.ptr(), &buffer, &length) != 0)
        throw py::error_already_set();
    return juce::MidiMessage(buffer, static_cast<int>(length));
}

py::bytes bytesFromMessage(const juce::MidiMessage& message) {
    return py::bytes(static_cast<const char*>(static_cast<const void*>(message.getRawData())),
                     static_cast<size_t>(message.getRawDataSize()));
}

// AudioBuffer -> (channels, samples) float32 numpy array.
py::array_t<float> copyAudioToArray(const juce::AudioBuffer<float>& buffer) {
    py::array_t<float> array({buffer.getNumChannels(), buffer.getNumSamples()});
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        std::memcpy(array.mutable_data(ch, 0), buffer.getReadPointer(ch),
                    static_cast<size_t>(buffer.getNumSamples()) * sizeof(float));
    return array;
}

juce::AudioBuffer<float> copyArrayToAudio(const py::array_t<float>& array) {
    if (array.ndim() != 2)
        throw py::value_error("audio array must be 2-D (channels, samples)");

    juce::AudioBuffer<float> buffer(static_cast<int>(array.shape(0)),
                                    static_cast<int>(array.shape(1)));
    for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
        std::memcpy(buffer.getWritePointer(ch), array.data(ch, 0),
                    static_cast<size_t>(buffer.getNumSamples()) * sizeof(float));
    return buffer;
}

// Same plugin-host loop the GoogleTest suite uses, wrapped for Python.
class Harness {
public:
    Harness(double sampleRate, int blockSize) : harness_(sampleRate, blockSize) {}

    float getNoteOnVelocity() const { return harness_.processor.getNoteOnVelocity(); }
    void setNoteOnVelocity(float velocity) { harness_.processor.setNoteOnVelocity(velocity); }
    double sampleRate() const { return harness_.sampleRate(); }

    // events: iterable of (sample_position, raw_midi_bytes) tuples.
    // Returns (audio_array, [(sample_position, raw_midi_bytes), ...]).
    py::tuple render(const py::sequence& events, int numSamples) {
        juce::MidiBuffer input;
        for (const auto& item : events) {
            const auto event = item.cast<py::tuple>();
            if (event.size() != 2)
                throw py::value_error("each event must be a (sample_position, midi_bytes) tuple");
            input.addEvent(messageFromBytes(event[1].cast<py::bytes>()),
                           event[0].cast<int>());
        }

        RenderResult result;
        {
            py::gil_scoped_release release;
            result = harness_.render(input, numSamples);
        }

        py::list midiOut;
        for (const auto metadata : result.midi)
            midiOut.append(py::make_tuple(metadata.samplePosition,
                                          bytesFromMessage(metadata.getMessage())));

        return py::make_tuple(copyAudioToArray(result.audio), midiOut);
    }

private:
    PluginHarness<ProtoNoteAudioProcessor> harness_;
};

// Mirrors PluginTest::readWav / writeWav so the golden-file test can stay in Python.
py::tuple readWav(const std::string& path) {
    juce::WavAudioFormat wav;
    std::unique_ptr<juce::AudioFormatReader> reader(
        wav.createReaderFor(juce::File(path).createInputStream().release(), true));
    if (reader == nullptr)
        throw std::runtime_error("golden file missing or unreadable: " + path);

    juce::AudioBuffer<float> audio(static_cast<int>(reader->numChannels),
                                   static_cast<int>(reader->lengthInSamples));
    if (!reader->read(&audio, 0, audio.getNumSamples(), 0, true, true))
        throw std::runtime_error("could not read samples from: " + path);

    return py::make_tuple(copyAudioToArray(audio), reader->sampleRate);
}

bool writeWav(const std::string& path, const py::array_t<float>& audio, double sampleRate) {
    const auto buffer = copyArrayToAudio(audio);

    const juce::File file(path);
    file.getParentDirectory().createDirectory();
    file.deleteFile();

    juce::WavAudioFormat wav;
    std::unique_ptr<juce::OutputStream> stream = file.createOutputStream();
    if (stream == nullptr)
        return false;

    auto writer = wav.createWriterFor(stream, juce::AudioFormatWriterOptions{}
                                                      .withSampleRate(sampleRate)
                                                      .withNumChannels(buffer.getNumChannels())
                                                      .withBitsPerSample(32));
    return writer != nullptr && writer->writeFromAudioSampleBuffer(buffer, 0, buffer.getNumSamples());
}

}  // namespace

PYBIND11_MODULE(protonote, m) {
    m.doc() = "ProtoNote processor test bindings";

    py::class_<Harness>(m, "Harness")
        .def(py::init<double, int>(),
             py::arg("sample_rate") = 44100.0, py::arg("block_size") = 512)
        .def("set_note_on_velocity", &Harness::setNoteOnVelocity, py::arg("velocity"))
        .def("get_note_on_velocity", &Harness::getNoteOnVelocity)
        .def("sample_rate", &Harness::sampleRate)
        .def("render", &Harness::render, py::arg("events"), py::arg("num_samples"));

    m.def("note_on", [](int channel, int pitch, int velocity) {
              return bytesFromMessage(
                  juce::MidiMessage::noteOn(channel, pitch, static_cast<juce::uint8>(velocity)));
          },
          py::arg("channel"), py::arg("pitch"), py::arg("velocity"));

    m.def("read_wav", &readWav, py::arg("path"));
    m.def("write_wav", &writeWav, py::arg("path"), py::arg("audio"), py::arg("sample_rate"));
}
