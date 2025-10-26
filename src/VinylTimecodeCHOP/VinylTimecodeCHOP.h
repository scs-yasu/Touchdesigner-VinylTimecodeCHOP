#ifndef __VinylTimecodeCHOP__
#define __VinylTimecodeCHOP__

#include "CHOP_CPlusPlusBase.h"
#include <vector>
#include <cstdint>

using namespace TD;

// Forward declare the C struct
struct timecoder;
struct timecode_def;

class VinylTimecodeCHOP : public CHOP_CPlusPlusBase
{
public:
    VinylTimecodeCHOP(const OP_NodeInfo* info);
    virtual ~VinylTimecodeCHOP();

    virtual void getGeneralInfo(CHOP_GeneralInfo*, const OP_Inputs*, void*) override;
    virtual bool getOutputInfo(CHOP_OutputInfo*, const OP_Inputs*, void*) override;
    virtual void getChannelName(int32_t index, OP_String* name, const OP_Inputs*, void*) override;
    virtual void execute(CHOP_Output*, const OP_Inputs*, void*) override;

    virtual int32_t getNumInfoCHOPChans(void*) override;
    virtual void getInfoCHOPChan(int index, OP_InfoCHOPChan* chan, void*) override;

    virtual void setupParameters(OP_ParameterManager* manager, void*) override;

private:
    void processAudioSamples(const float* leftChannel, const float* rightChannel, int numSamples);
    void initializeTimecoder(const char* formatName, double sampleRate);
    void cleanupTimecoder();

    const OP_NodeInfo* myNodeInfo;

    // xwax timecoder instance
    timecoder* myTimecoder;
    const timecode_def* myCurrentDef;

    // Audio buffer for int16_t samples
    std::vector<int16_t> myAudioBuffer;

    // Output values
    double myPosition;
    double myPitch;
    int32_t myAlive;

    // Last valid position (to handle -1 returns)
    double myLastValidPosition;
    signed int myRawPosition;  // Raw position from timecoder (-1 if unknown)

    // For pitch-based interpolation when raw_position is -1
    int myLastNumSamples;  // Number of samples in last process call

    // Current format index
    int32_t myCurrentFormatIndex;
    double myCurrentSampleRate;
    bool myTimecoderInitialized;
};

#endif
