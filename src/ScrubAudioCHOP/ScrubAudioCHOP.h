#pragma once

#include "CHOP_CPlusPlusBase.h"

#include <cstdint>
#include <string>
#include <vector>

using namespace TD;

class ScrubAudioCHOP : public CHOP_CPlusPlusBase {
public:
    explicit ScrubAudioCHOP(const OP_NodeInfo* info);
    ~ScrubAudioCHOP() override;

    void getGeneralInfo(CHOP_GeneralInfo*, const OP_Inputs*, void*) override;
    bool getOutputInfo(CHOP_OutputInfo*, const OP_Inputs*, void*) override;
    void getChannelName(int32_t index, OP_String* name, const OP_Inputs*, void*) override;
    void execute(CHOP_Output*, const OP_Inputs*, void*) override;
    void pulsePressed(const char* name, void*) override;

    int32_t getNumInfoCHOPChans(void*) override;
    void getInfoCHOPChan(int index, OP_InfoCHOPChan* chan, void*) override;
    void setupParameters(OP_ParameterManager* manager, void*) override;

private:
    bool loadAudioFile(const char* path);
    void unloadAudio();
    float sampleAtLinear(double frame, int channel) const;
    float sampleAtHermite(double frame, int channel) const;
    void resyncToPositionMs(double positionMs, double strength);

    const OP_NodeInfo* myNodeInfo;

    std::string myLoadedPath;
    std::vector<float> myAudio;
    int32_t myChannels;
    double mySourceSampleRate;
    double myPlayhead;
    double myLastPitch;
    double myDurationSeconds;
    bool myLoaded;
    bool myLoadFailed;
    bool myMfStarted;
    bool myComInitialized;
    bool myResetPending;
    bool myCueJumpPending;
    bool myCueSetPending;
    int32_t myLoadCount;
    int32_t myLoopCount;
    int32_t myClipCount;
    float myPeak;
    float myLastEffectivePitch;
    double myLastOutputSampleRate;
    double myLastPreWrapPlayhead;
    double myLastPostWrapPlayhead;
    double myLastRawPlayheadFraction;
    double myCuePosition;
    double myLastCueParam;
    int32_t myLastWrapped;
};
