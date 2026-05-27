#include "ScrubAudioCHOP.h"

#include <algorithm>
#include <cassert>
#include <cmath>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <limits>

#ifdef _WIN32
#include <mfapi.h>
#include <mferror.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <windows.h>
#include <wrl/client.h>
#endif

namespace {
constexpr int kOutputChannels = 2;

const char* kInterpNames[] = {
    "Linear",
    "Hermite",
    nullptr
};

const char* kInterpLabels[] = {
    "Linear",
    "Hermite",
    nullptr
};

int findChannel(const OP_CHOPInput* input, const char* name, int fallback) {
    if (!input) {
        return -1;
    }
    for (int i = 0; i < input->numChannels; ++i) {
        const char* channelName = input->getChannelName(i);
        if (channelName && std::strcmp(channelName, name) == 0) {
            return i;
        }
    }
    return fallback < input->numChannels ? fallback : -1;
}

double clampDouble(double value, double low, double high) {
    return std::max(low, std::min(high, value));
}

float hermite4(float frac, float xm1, float x0, float x1, float x2) {
    const float c = (x1 - xm1) * 0.5f;
    const float v = x0 - x1;
    const float w = c + v;
    const float a = w + v + (x2 - x0) * 0.5f;
    const float bNeg = w + a;
    return ((((a * frac) - bNeg) * frac + c) * frac + x0);
}

#ifdef _WIN32
std::wstring utf8ToWide(const char* text) {
    if (!text || !text[0]) {
        return {};
    }
    const int count = MultiByteToWideChar(CP_UTF8, 0, text, -1, nullptr, 0);
    if (count <= 0) {
        return {};
    }
    std::wstring wide(static_cast<size_t>(count), L'\0');
    MultiByteToWideChar(CP_UTF8, 0, text, -1, wide.data(), count);
    if (!wide.empty() && wide.back() == L'\0') {
        wide.pop_back();
    }
    return wide;
}
#endif
} // namespace

extern "C" {
DLLEXPORT void FillCHOPPluginInfo(TD::CHOP_PluginInfo* info) {
    (void)info->setAPIVersion(TD::CHOPCPlusPlusAPIVersion);
    info->customOPInfo.opType->setString("Scrubaudio");
    info->customOPInfo.opLabel->setString("Scrub Audio");
    info->customOPInfo.opIcon->setString("AUD");
    info->customOPInfo.minInputs = 1;
    info->customOPInfo.maxInputs = 1;
    info->customOPInfo.authorName->setString("Nobuyasu Fukazawa");
    info->customOPInfo.authorEmail->setString("yasu@skeletoncrew.co.jp");
    info->customOPInfo.cookOnStart = true;
}

DLLEXPORT TD::CHOP_CPlusPlusBase* CreateCHOPInstance(const TD::OP_NodeInfo* info) {
    return new ScrubAudioCHOP(info);
}

DLLEXPORT void DestroyCHOPInstance(TD::CHOP_CPlusPlusBase* instance) {
    delete static_cast<ScrubAudioCHOP*>(instance);
}
}

ScrubAudioCHOP::ScrubAudioCHOP(const OP_NodeInfo* info)
        : myNodeInfo(info),
          myChannels(kOutputChannels),
          mySourceSampleRate(44100.0),
          myPlayhead(0.0),
          myLastPitch(0.0),
          myDurationSeconds(0.0),
          myLoaded(false),
          myLoadFailed(false),
          myMfStarted(false),
          myComInitialized(false),
          myResetPending(false),
          myCueJumpPending(false),
          myCueSetPending(false),
          myLoadCount(0),
          myLoopCount(0),
          myClipCount(0),
          myPeak(0.0f),
          myLastEffectivePitch(0.0f),
          myLastOutputSampleRate(44100.0),
          myLastPreWrapPlayhead(0.0),
          myLastPostWrapPlayhead(0.0),
          myLastRawPlayheadFraction(0.0),
          myCuePosition(0.0),
          myLastCueParam(0.0),
          myLastWrapped(0) {
#ifdef _WIN32
    const HRESULT coResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
    myComInitialized = SUCCEEDED(coResult);
    const HRESULT mfResult = MFStartup(MF_VERSION);
    myMfStarted = SUCCEEDED(mfResult);
#endif
}

ScrubAudioCHOP::~ScrubAudioCHOP() {
    unloadAudio();
#ifdef _WIN32
    if (myMfStarted) {
        MFShutdown();
    }
    if (myComInitialized) {
        CoUninitialize();
    }
#endif
}

void ScrubAudioCHOP::getGeneralInfo(CHOP_GeneralInfo* ginfo, const OP_Inputs*, void*) {
    ginfo->cookEveryFrame = true;
    ginfo->cookEveryFrameIfAsked = true;
    ginfo->timeslice = true;
    ginfo->inputMatchIndex = 0;
}

bool ScrubAudioCHOP::getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs, void*) {
    info->numChannels = kOutputChannels;
    const bool useInputRate = inputs->getParInt("Useinputrate") != 0;
    if (useInputRate && inputs->getNumInputs() > 0) {
        const OP_CHOPInput* input = inputs->getInputCHOP(0);
        info->sampleRate = static_cast<float>(input->sampleRate);
    } else {
        info->sampleRate = static_cast<float>(inputs->getParDouble("Outputrate"));
    }
    return true;
}

void ScrubAudioCHOP::getChannelName(int32_t index, OP_String* name, const OP_Inputs* inputs, void*) {
    const char* leftName = inputs ? inputs->getParString("Leftname") : nullptr;
    const char* rightName = inputs ? inputs->getParString("Rightname") : nullptr;
    switch (index) {
        case 0:
            name->setString(leftName && leftName[0] ? leftName : "left");
            break;
        case 1:
            name->setString(rightName && rightName[0] ? rightName : "right");
            break;
        default:
            name->setString("unknown");
            break;
    }
}

void ScrubAudioCHOP::execute(CHOP_Output* output, const OP_Inputs* inputs, void*) {
    const char* moviefileinPath = inputs->getParString("Moviefileintop");
    const OP_TOPInput* moviefileinTop = inputs->getParTOP("Moviefileintop");
    const char* filePath = inputs->getParFilePath("File");
    const float gain = static_cast<float>(inputs->getParDouble("Gain"));
    const double pitchScale = inputs->getParDouble("Pitchscale");
    const double cueParam = clampDouble(inputs->getParDouble("Cueposition"), 0.0, 1.0);
    const bool useSync = inputs->getParInt("Usesync") != 0;
    const double syncStrength = clampDouble(inputs->getParDouble("Syncstrength"), 0.0, 1.0);
    const bool reset = inputs->getParInt("Reset") != 0;
    const bool loop = inputs->getParInt("Loop") != 0;
    const int interp = inputs->getParInt("Interpolation");

    // The TOP parameter is intentionally kept as a dependency/reference hook.
    // TouchDesigner C++ OP inputs expose TOP texture metadata, but not the
    // referenced TOP's parameter table, so the moviefileinTOP file parameter
    // cannot be read here without a Python callback layer.
    (void)moviefileinPath;
    (void)moviefileinTop;

    if (filePath && myLoadedPath != filePath) {
        loadAudioFile(filePath);
    }
    const bool doReset = reset || myResetPending;
    if (doReset) {
        myPlayhead = 0.0;
        myLastPitch = 0.0;
        myLastEffectivePitch = 0.0f;
        myResetPending = false;
    }
    const bool holdResetCook = doReset;

    const OP_CHOPInput* input = inputs->getNumInputs() > 0 ? inputs->getInputCHOP(0) : nullptr;
    const int pitchIndex = findChannel(input, "pitch", 0);
    const int positionIndex = findChannel(input, "position", 1);
    const float* pitchData = pitchIndex >= 0 ? input->getChannelData(pitchIndex) : nullptr;
    const float* positionData = positionIndex >= 0 ? input->getChannelData(positionIndex) : nullptr;

    if (myLoaded && useSync && positionData && input->numSamples > 0) {
        const float positionMs = positionData[input->numSamples - 1];
        if (std::isfinite(positionMs) && positionMs >= 0.0f) {
            resyncToPositionMs(positionMs, syncStrength);
        }
    }

    const double outputSampleRate = output->sampleRate > 0.0f ? output->sampleRate : 44100.0;
    myLastOutputSampleRate = outputSampleRate;
    const int inputSamples = input ? input->numSamples : 0;
    const bool pitchIsBlockValue = !pitchData || inputSamples <= 1;
    const double blockPitch = pitchData ? pitchData[std::max(0, inputSamples - 1)] : 0.0;

    myPeak = 0.0f;
    const double frameCount = myChannels > 0 ? static_cast<double>(myAudio.size() / myChannels) : 0.0;
    myLastWrapped = 0;

    if (std::abs(cueParam - myLastCueParam) > 0.0000001) {
        myCuePosition = cueParam;
        myLastCueParam = cueParam;
    }
    if (myCueSetPending) {
        myCuePosition = frameCount > 1.0
                ? clampDouble(myPlayhead / (frameCount - 1.0), 0.0, 1.0)
                : 0.0;
        myLastCueParam = myCuePosition;
        myCueSetPending = false;
    }
    if (myCueJumpPending) {
        myPlayhead = frameCount > 1.0 ? myCuePosition * (frameCount - 1.0) : 0.0;
        myLastPitch = 0.0;
        myLastEffectivePitch = 0.0f;
        myCueJumpPending = false;
    }

    for (int i = 0; i < output->numSamples; ++i) {
        float pitch = 0.0f;
        if (pitchData) {
            if (pitchIsBlockValue) {
                const double t = output->numSamples > 1
                        ? static_cast<double>(i + 1) / static_cast<double>(output->numSamples)
                        : 1.0;
                pitch = static_cast<float>(myLastPitch + (blockPitch - myLastPitch) * t);
            } else {
                const int sourceIndex = output->numSamples > 1
                        ? static_cast<int>(std::round(
                                  static_cast<double>(i) * static_cast<double>(inputSamples - 1) /
                                  static_cast<double>(output->numSamples - 1)))
                        : inputSamples - 1;
                pitch = pitchData[sourceIndex];
            }
            pitch = holdResetCook ? 0.0f : static_cast<float>(static_cast<double>(pitch) * pitchScale);
        }

        float left = 0.0f;
        float right = 0.0f;
        if (myLoaded && frameCount > 1.0) {
            if (interp == 1) {
                left = sampleAtHermite(myPlayhead, 0);
                right = sampleAtHermite(myPlayhead, 1);
            } else {
                left = sampleAtLinear(myPlayhead, 0);
                right = sampleAtLinear(myPlayhead, 1);
            }
            left *= gain;
            right *= gain;

            myPeak = std::max(myPeak, std::max(std::abs(left), std::abs(right)));
            if (std::abs(left) > 0.99f || std::abs(right) > 0.99f) {
                ++myClipCount;
            }

            myLastEffectivePitch = pitch;
            myPlayhead += static_cast<double>(pitch) * mySourceSampleRate / outputSampleRate;
            myLastPreWrapPlayhead = myPlayhead;

            if (loop && frameCount > 1.0) {
                if (myPlayhead < 0.0 || myPlayhead >= frameCount) {
                    myPlayhead = std::fmod(myPlayhead, frameCount);
                    if (myPlayhead < 0.0) {
                        myPlayhead += frameCount;
                    }
                    ++myLoopCount;
                    myLastWrapped = 1;
                }
            } else {
                myPlayhead = clampDouble(myPlayhead, 0.0, frameCount - 1.0);
            }
            myLastPostWrapPlayhead = myPlayhead;
            myLastRawPlayheadFraction = frameCount > 1.0
                    ? myPlayhead / (frameCount - 1.0)
                    : 0.0;
        }

        output->channels[0][i] = left;
        output->channels[1][i] = right;
    }

    myLastPitch = holdResetCook ? 0.0 : blockPitch;
}

void ScrubAudioCHOP::pulsePressed(const char* name, void*) {
    if (name && std::strcmp(name, "Reset") == 0) {
        myResetPending = true;
    } else if (name && std::strcmp(name, "Cuepulse") == 0) {
        myCueJumpPending = true;
    } else if (name && std::strcmp(name, "Cuesetpulse") == 0) {
        myCueSetPending = true;
    }
}

int32_t ScrubAudioCHOP::getNumInfoCHOPChans(void*) {
    return 20;
}

void ScrubAudioCHOP::getInfoCHOPChan(int index, OP_InfoCHOPChan* chan, void*) {
    const double frameCount = myChannels > 0
            ? static_cast<double>(myAudio.size() / myChannels)
            : 0.0;
    const double playheadSeconds = mySourceSampleRate > 0.0
            ? myPlayhead / mySourceSampleRate
            : 0.0;
    const double playheadFraction = frameCount > 1.0
            ? clampDouble(myPlayhead / (frameCount - 1.0), 0.0, 1.0)
            : 0.0;

    switch (index) {
        case 0:
            chan->name->setString("loaded");
            chan->value = myLoaded ? 1.0f : 0.0f;
            break;
        case 1:
            chan->name->setString("load_failed");
            chan->value = myLoadFailed ? 1.0f : 0.0f;
            break;
        case 2:
            chan->name->setString("source_rate");
            chan->value = static_cast<float>(mySourceSampleRate);
            break;
        case 3:
            chan->name->setString("frames");
            chan->value = static_cast<float>(myAudio.size() / std::max(1, myChannels));
            break;
        case 4:
            chan->name->setString("duration");
            chan->value = static_cast<float>(myDurationSeconds);
            break;
        case 5:
            chan->name->setString("playhead");
            chan->value = static_cast<float>(myPlayhead);
            break;
        case 6:
            chan->name->setString("load_count");
            chan->value = static_cast<float>(myLoadCount);
            break;
        case 7:
            chan->name->setString("peak");
            chan->value = myPeak;
            break;
        case 8:
            chan->name->setString("clip_count");
            chan->value = static_cast<float>(myClipCount);
            break;
        case 9:
            chan->name->setString("playhead_seconds");
            chan->value = static_cast<float>(playheadSeconds);
            break;
        case 10:
            chan->name->setString("playhead_fraction");
            chan->value = static_cast<float>(playheadFraction);
            break;
        case 11:
            chan->name->setString("effective_pitch");
            chan->value = myLastEffectivePitch;
            break;
        case 12:
            chan->name->setString("output_rate");
            chan->value = static_cast<float>(myLastOutputSampleRate);
            break;
        case 13:
            chan->name->setString("loop_count");
            chan->value = static_cast<float>(myLoopCount);
            break;
        case 14:
            chan->name->setString("wrapped");
            chan->value = static_cast<float>(myLastWrapped);
            break;
        case 15:
            chan->name->setString("pre_wrap_playhead");
            chan->value = static_cast<float>(myLastPreWrapPlayhead);
            break;
        case 16:
            chan->name->setString("post_wrap_playhead");
            chan->value = static_cast<float>(myLastPostWrapPlayhead);
            break;
        case 17:
            chan->name->setString("raw_playhead_fraction");
            chan->value = static_cast<float>(myLastRawPlayheadFraction);
            break;
        case 18:
            chan->name->setString("cue_position");
            chan->value = static_cast<float>(myCuePosition);
            break;
        case 19:
            chan->name->setString("moviefileintop_supported");
            chan->value = 0.0f;
            break;
    }
}

void ScrubAudioCHOP::setupParameters(OP_ParameterManager* manager, void*) {
    {
        OP_StringParameter sp;
        sp.name = "Moviefileintop";
        sp.label = "Moviefilein TOP";
        sp.defaultValue = "";
        OP_ParAppendResult res = manager->appendTOP(sp);
        assert(res == OP_ParAppendResult::Success);
    }
    {
        OP_StringParameter sp;
        sp.name = "File";
        sp.label = "File";
        sp.defaultValue = "";
        OP_ParAppendResult res = manager->appendFile(sp);
        assert(res == OP_ParAppendResult::Success);
    }
    {
        OP_NumericParameter np;
        np.name = "Gain";
        np.label = "Gain";
        np.defaultValues[0] = 1.0;
        np.minSliders[0] = 0.0;
        np.maxSliders[0] = 2.0;
        np.minValues[0] = 0.0;
        np.maxValues[0] = 16.0;
        np.clampMins[0] = true;
        np.clampMaxes[0] = true;
        OP_ParAppendResult res = manager->appendFloat(np);
        assert(res == OP_ParAppendResult::Success);
    }
    {
        OP_NumericParameter np;
        np.name = "Pitchscale";
        np.label = "Pitch Scale";
        np.defaultValues[0] = 1.0;
        np.minSliders[0] = 0.9;
        np.maxSliders[0] = 1.1;
        np.minValues[0] = 0.25;
        np.maxValues[0] = 4.0;
        np.clampMins[0] = true;
        np.clampMaxes[0] = true;
        OP_ParAppendResult res = manager->appendFloat(np);
        assert(res == OP_ParAppendResult::Success);
    }
    {
        OP_NumericParameter np;
        np.name = "Cueposition";
        np.label = "Cue Position";
        np.defaultValues[0] = 0.0;
        np.minSliders[0] = 0.0;
        np.maxSliders[0] = 1.0;
        np.minValues[0] = 0.0;
        np.maxValues[0] = 1.0;
        np.clampMins[0] = true;
        np.clampMaxes[0] = true;
        OP_ParAppendResult res = manager->appendFloat(np);
        assert(res == OP_ParAppendResult::Success);
    }
    {
        OP_NumericParameter np;
        np.name = "Cuepulse";
        np.label = "Cue Jump";
        np.defaultValues[0] = 0.0;
        OP_ParAppendResult res = manager->appendPulse(np);
        assert(res == OP_ParAppendResult::Success);
    }
    {
        OP_NumericParameter np;
        np.name = "Cuesetpulse";
        np.label = "Cue Set";
        np.defaultValues[0] = 0.0;
        OP_ParAppendResult res = manager->appendPulse(np);
        assert(res == OP_ParAppendResult::Success);
    }
    {
        OP_NumericParameter np;
        np.name = "Useinputrate";
        np.label = "Use Input Rate";
        np.defaultValues[0] = 1.0;
        OP_ParAppendResult res = manager->appendToggle(np);
        assert(res == OP_ParAppendResult::Success);
    }
    {
        OP_NumericParameter np;
        np.name = "Outputrate";
        np.label = "Output Rate";
        np.defaultValues[0] = 44100.0;
        np.minSliders[0] = 22050.0;
        np.maxSliders[0] = 96000.0;
        np.minValues[0] = 8000.0;
        np.maxValues[0] = 192000.0;
        np.clampMins[0] = true;
        np.clampMaxes[0] = true;
        OP_ParAppendResult res = manager->appendFloat(np);
        assert(res == OP_ParAppendResult::Success);
    }
    {
        OP_StringParameter sp;
        sp.name = "Leftname";
        sp.label = "Left Name";
        sp.defaultValue = "left";
        OP_ParAppendResult res = manager->appendString(sp);
        assert(res == OP_ParAppendResult::Success);
    }
    {
        OP_StringParameter sp;
        sp.name = "Rightname";
        sp.label = "Right Name";
        sp.defaultValue = "right";
        OP_ParAppendResult res = manager->appendString(sp);
        assert(res == OP_ParAppendResult::Success);
    }
    {
        OP_StringParameter sp;
        sp.name = "Interpolation";
        sp.label = "Interpolation";
        sp.defaultValue = kInterpNames[0];
        OP_ParAppendResult res = manager->appendMenu(sp, 2, kInterpNames, kInterpLabels);
        assert(res == OP_ParAppendResult::Success);
    }
    {
        OP_NumericParameter np;
        np.name = "Loop";
        np.label = "Loop";
        np.defaultValues[0] = 0.0;
        OP_ParAppendResult res = manager->appendToggle(np);
        assert(res == OP_ParAppendResult::Success);
    }
    {
        OP_NumericParameter np;
        np.name = "Usesync";
        np.label = "Use Position Sync";
        np.defaultValues[0] = 0.0;
        OP_ParAppendResult res = manager->appendToggle(np);
        assert(res == OP_ParAppendResult::Success);
    }
    {
        OP_NumericParameter np;
        np.name = "Syncstrength";
        np.label = "Sync Strength";
        np.defaultValues[0] = 0.02;
        np.minSliders[0] = 0.0;
        np.maxSliders[0] = 0.2;
        np.minValues[0] = 0.0;
        np.maxValues[0] = 1.0;
        np.clampMins[0] = true;
        np.clampMaxes[0] = true;
        OP_ParAppendResult res = manager->appendFloat(np);
        assert(res == OP_ParAppendResult::Success);
    }
    {
        OP_NumericParameter np;
        np.name = "Reset";
        np.label = "Reset";
        np.defaultValues[0] = 0.0;
        OP_ParAppendResult res = manager->appendPulse(np);
        assert(res == OP_ParAppendResult::Success);
    }
}

void ScrubAudioCHOP::unloadAudio() {
    myAudio.clear();
    myLoadedPath.clear();
    myChannels = kOutputChannels;
    mySourceSampleRate = 44100.0;
    myPlayhead = 0.0;
    myLastPitch = 0.0;
    myDurationSeconds = 0.0;
    myLoaded = false;
    myResetPending = false;
    myCueJumpPending = false;
    myCueSetPending = false;
    myLastEffectivePitch = 0.0f;
    myLastPreWrapPlayhead = 0.0;
    myLastPostWrapPlayhead = 0.0;
    myLastRawPlayheadFraction = 0.0;
    myCuePosition = 0.0;
    myLastCueParam = 0.0;
    myLastWrapped = 0;
}

bool ScrubAudioCHOP::loadAudioFile(const char* path) {
    unloadAudio();
    myLoadFailed = false;
    myLoadedPath = path ? path : "";
    ++myLoadCount;

    if (myLoadedPath.empty()) {
        return false;
    }

#ifndef _WIN32
    myLoadFailed = true;
    return false;
#else
    if (!myMfStarted) {
        myLoadFailed = true;
        return false;
    }

    std::wstring widePath = utf8ToWide(myLoadedPath.c_str());
    if (widePath.empty()) {
        myLoadFailed = true;
        return false;
    }

    using Microsoft::WRL::ComPtr;
    ComPtr<IMFSourceReader> reader;
    HRESULT hr = MFCreateSourceReaderFromURL(widePath.c_str(), nullptr, reader.GetAddressOf());
    if (FAILED(hr)) {
        myLoadFailed = true;
        return false;
    }

    reader->SetStreamSelection(MF_SOURCE_READER_ALL_STREAMS, FALSE);
    hr = reader->SetStreamSelection(MF_SOURCE_READER_FIRST_AUDIO_STREAM, TRUE);
    if (FAILED(hr)) {
        myLoadFailed = true;
        return false;
    }

    ComPtr<IMFMediaType> requestedType;
    hr = MFCreateMediaType(requestedType.GetAddressOf());
    if (FAILED(hr)) {
        myLoadFailed = true;
        return false;
    }
    requestedType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
    requestedType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_Float);
    requestedType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, kOutputChannels);
    hr = reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, requestedType.Get());
    if (FAILED(hr)) {
        requestedType.Reset();
        hr = MFCreateMediaType(requestedType.GetAddressOf());
        if (FAILED(hr)) {
            myLoadFailed = true;
            return false;
        }
        requestedType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
        requestedType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
        requestedType->SetUINT32(MF_MT_AUDIO_NUM_CHANNELS, kOutputChannels);
        hr = reader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, requestedType.Get());
        if (FAILED(hr)) {
            myLoadFailed = true;
            return false;
        }
    }

    ComPtr<IMFMediaType> actualType;
    hr = reader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, actualType.GetAddressOf());
    if (FAILED(hr)) {
        myLoadFailed = true;
        return false;
    }

    GUID subtype = {};
    UINT32 channels = 0;
    UINT32 sampleRate = 0;
    UINT32 bitsPerSample = 0;
    actualType->GetGUID(MF_MT_SUBTYPE, &subtype);
    actualType->GetUINT32(MF_MT_AUDIO_NUM_CHANNELS, &channels);
    actualType->GetUINT32(MF_MT_AUDIO_SAMPLES_PER_SECOND, &sampleRate);
    actualType->GetUINT32(MF_MT_AUDIO_BITS_PER_SAMPLE, &bitsPerSample);

    if (channels == 0 || sampleRate == 0) {
        myLoadFailed = true;
        return false;
    }
    if (bitsPerSample == 0) {
        bitsPerSample = IsEqualGUID(subtype, MFAudioFormat_Float) ? 32 : 16;
    }

    mySourceSampleRate = static_cast<double>(sampleRate);
    myChannels = kOutputChannels;

    while (true) {
        DWORD streamIndex = 0;
        DWORD flags = 0;
        LONGLONG timestamp = 0;
        ComPtr<IMFSample> sample;
        hr = reader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM,
                0,
                &streamIndex,
                &flags,
                &timestamp,
                sample.GetAddressOf());
        if (FAILED(hr)) {
            myLoadFailed = true;
            myAudio.clear();
            return false;
        }
        if (flags & MF_SOURCE_READERF_ENDOFSTREAM) {
            break;
        }
        if (!sample) {
            continue;
        }

        ComPtr<IMFMediaBuffer> buffer;
        hr = sample->ConvertToContiguousBuffer(buffer.GetAddressOf());
        if (FAILED(hr)) {
            continue;
        }

        BYTE* data = nullptr;
        DWORD maxLength = 0;
        DWORD currentLength = 0;
        hr = buffer->Lock(&data, &maxLength, &currentLength);
        if (FAILED(hr) || !data || currentLength == 0) {
            continue;
        }

        const UINT32 bytesPerInputSample = bitsPerSample / 8;
        const UINT32 bytesPerInputFrame = channels * bytesPerInputSample;
        const UINT32 frames = bytesPerInputFrame > 0 ? currentLength / bytesPerInputFrame : 0;
        const size_t start = myAudio.size();
        myAudio.resize(start + static_cast<size_t>(frames) * kOutputChannels);

        for (UINT32 frame = 0; frame < frames; ++frame) {
            float left = 0.0f;
            float right = 0.0f;
            if (IsEqualGUID(subtype, MFAudioFormat_Float) && bitsPerSample == 32) {
                const float* in = reinterpret_cast<const float*>(data) + static_cast<size_t>(frame) * channels;
                left = in[0];
                right = channels > 1 ? in[1] : left;
            } else if (IsEqualGUID(subtype, MFAudioFormat_PCM) && bitsPerSample == 16) {
                const int16_t* in = reinterpret_cast<const int16_t*>(data) + static_cast<size_t>(frame) * channels;
                left = static_cast<float>(in[0]) / 32768.0f;
                right = channels > 1 ? static_cast<float>(in[1]) / 32768.0f : left;
            }
            const size_t outIndex = start + static_cast<size_t>(frame) * kOutputChannels;
            myAudio[outIndex] = left;
            myAudio[outIndex + 1] = right;
        }

        buffer->Unlock();
    }

    myLoaded = !myAudio.empty();
    myLoadFailed = !myLoaded;
    myDurationSeconds = myLoaded
            ? static_cast<double>(myAudio.size() / kOutputChannels) / mySourceSampleRate
            : 0.0;
    myPlayhead = 0.0;
    myLastPitch = 0.0;
    return myLoaded;
#endif
}

float ScrubAudioCHOP::sampleAtLinear(double frame, int channel) const {
    const int64_t frameCount = static_cast<int64_t>(myAudio.size() / myChannels);
    if (frameCount <= 0) {
        return 0.0f;
    }
    frame = clampDouble(frame, 0.0, static_cast<double>(frameCount - 1));
    const int64_t i0 = static_cast<int64_t>(std::floor(frame));
    const int64_t i1 = std::min<int64_t>(i0 + 1, frameCount - 1);
    const float frac = static_cast<float>(frame - static_cast<double>(i0));
    const float s0 = myAudio[static_cast<size_t>(i0) * myChannels + channel];
    const float s1 = myAudio[static_cast<size_t>(i1) * myChannels + channel];
    return s0 + frac * (s1 - s0);
}

float ScrubAudioCHOP::sampleAtHermite(double frame, int channel) const {
    const int64_t frameCount = static_cast<int64_t>(myAudio.size() / myChannels);
    if (frameCount <= 0) {
        return 0.0f;
    }
    frame = clampDouble(frame, 0.0, static_cast<double>(frameCount - 1));
    const int64_t i0 = static_cast<int64_t>(std::floor(frame));
    const int64_t im1 = std::max<int64_t>(i0 - 1, 0);
    const int64_t i1 = std::min<int64_t>(i0 + 1, frameCount - 1);
    const int64_t i2 = std::min<int64_t>(i0 + 2, frameCount - 1);
    const float frac = static_cast<float>(frame - static_cast<double>(i0));
    return hermite4(frac,
            myAudio[static_cast<size_t>(im1) * myChannels + channel],
            myAudio[static_cast<size_t>(i0) * myChannels + channel],
            myAudio[static_cast<size_t>(i1) * myChannels + channel],
            myAudio[static_cast<size_t>(i2) * myChannels + channel]);
}

void ScrubAudioCHOP::resyncToPositionMs(double positionMs, double strength) {
    if (!myLoaded || strength <= 0.0) {
        return;
    }
    const double frameCount = static_cast<double>(myAudio.size() / myChannels);
    const double target = clampDouble(positionMs * mySourceSampleRate / 1000.0, 0.0, frameCount - 1.0);
    myPlayhead += (target - myPlayhead) * strength;
}
