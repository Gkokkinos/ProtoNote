#pragma once

#include "Framework/BaseFixture.h"
#include "Framework/GoldenCompare.h"
#include "Framework/TestUtils.h"
#include "Framework/PluginHarness.h"
#include "PluginProcessor.h"

class PluginFixture : public BaseFixture {
protected:
    // Built fresh for every test, so each test starts from a newly prepared processor.
    PluginHarness<ProtoNoteAudioProcessor> plugin_;
    float defaultVelocity_ = 0.0f;

    // Remember the processor's own default so tests can change the velocity freely.
    void OnSetUp() override { defaultVelocity_ = plugin_.processor.getNoteOnVelocity(); }

    // Put the default back so a changed velocity can't leak into a later test if plugin_ is ever shared.
    void TearDown() override { plugin_.processor.setNoteOnVelocity(defaultVelocity_); }
};
