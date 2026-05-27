#include "VinylTimecodeCHOP.h"

#include <cstdio>
#include <cstring>
#include <algorithm>
#include <cmath>

// Include xwax C library
extern "C" {
#include "timecoder.h"
#include "pitch.h"
#include "pitch_kalman.h"
}

// Available timecode format names (from xwax)
static const char* TIMECODE_FORMATS[] = {
    "serato_2a",
    "serato_2b",
    "serato_cd",
    "traktor_a",
    "traktor_b",
    "mixvibes_v2",
    "mixvibes_7inch",
    "pioneer_a",
    "pioneer_b",
    nullptr
};

static const char* TIMECODE_LABELS[] = {
    "Serato 2nd Ed. Side A",
    "Serato 2nd Ed. Side B",
    "Serato CD",
    "Traktor Scratch Side A",
    "Traktor Scratch Side B",
    "MixVibes DVS V2",
    "MixVibes 7inch",
    "Pioneer RekordBox Side A",
    "Pioneer RekordBox Side B",
    nullptr
};

// This will correspond to the name of the Custom OP
extern "C"
{
    DLLEXPORT void FillCHOPPluginInfo(TD::CHOP_PluginInfo *info)
    {
        (void)info->setAPIVersion(TD::CHOPCPlusPlusAPIVersion);
        info->customOPInfo.opType->setString("Vinyltimecode");
        info->customOPInfo.opLabel->setString("Vinyl Timecode");
        info->customOPInfo.opIcon->setString("VTC");
        info->customOPInfo.minInputs = 1;
        info->customOPInfo.maxInputs = 1;
        info->customOPInfo.authorName->setString("Nobuyasu Fukazawa");
        info->customOPInfo.authorEmail->setString("yasu@skeletoncrew.co.jp");
        info->customOPInfo.cookOnStart = true;
    }

    DLLEXPORT TD::CHOP_CPlusPlusBase* CreateCHOPInstance(const TD::OP_NodeInfo* info)
    {
        return new VinylTimecodeCHOP(info);
    }

    DLLEXPORT void DestroyCHOPInstance(TD::CHOP_CPlusPlusBase* instance)
    {
        delete static_cast<VinylTimecodeCHOP*>(instance);
    }
};

VinylTimecodeCHOP::VinylTimecodeCHOP(const OP_NodeInfo* info)
    : myNodeInfo(info)
    , myTimecoder(nullptr)
    , myCurrentDef(nullptr)
    , myPosition(0.0)
    , myPitch(0.0)
    , myAlive(0)
    , myLastValidPosition(0.0)
    , myRawPosition(-1)
    , myLastNumSamples(0)
    , myPeakL(0.0f)
    , myPeakR(0.0f)
    , myClipCount(0)
    , myCurrentFormatIndex(0)
    , myCurrentSampleRate(44100.0)
    , myCurrentVinylSpeed(1.0)
    , myTimecoderInitialized(false)
{
}

VinylTimecodeCHOP::~VinylTimecodeCHOP()
{
    cleanupTimecoder();
}

void
VinylTimecodeCHOP::cleanupTimecoder()
{
    if (myTimecoder) {
        timecoder_clear(myTimecoder);
        delete myTimecoder;
        myTimecoder = nullptr;
    }
    myTimecoderInitialized = false;
}

void
VinylTimecodeCHOP::initializeTimecoder(const char* formatName, double sampleRate, double vinylSpeed)
{
    cleanupTimecoder();

    timecode_def* def = timecoder_find_definition(formatName);
    if (!def) {
        fprintf(stderr, "VinylTimecodeCHOP: Unknown timecode format '%s'\n", formatName);
        return;
    }

    myTimecoder = new timecoder;
    myCurrentDef = def;
    timecoder_init(myTimecoder, def, vinylSpeed, static_cast<unsigned int>(sampleRate), false, false);

    myCurrentSampleRate = sampleRate;
    myCurrentVinylSpeed = vinylSpeed;
    myTimecoderInitialized = true;
}

void
VinylTimecodeCHOP::getGeneralInfo(CHOP_GeneralInfo* ginfo, const OP_Inputs* inputs, void*)
{
    ginfo->cookEveryFrame = true;
    ginfo->cookEveryFrameIfAsked = true;
    ginfo->timeslice = false;
    ginfo->inputMatchIndex = 0;
}

bool
VinylTimecodeCHOP::getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs, void*)
{
    info->numChannels = 3;  // position, pitch, quality

    if (inputs->getNumInputs() > 0) {
        info->numSamples = inputs->getInputCHOP(0)->numSamples;
        info->sampleRate = static_cast<float>(inputs->getInputCHOP(0)->sampleRate);
    } else {
        info->numSamples = 1;
        info->sampleRate = 60.0f;
    }

    info->startIndex = 0;
    return true;
}

void
VinylTimecodeCHOP::getChannelName(int32_t index, OP_String* name, const OP_Inputs* inputs, void*)
{
    switch (index) {
        case 0: name->setString("position"); break;
        case 1: name->setString("pitch"); break;
        case 2: name->setString("quality"); break;
        default: name->setString("unknown"); break;
    }
}

void
VinylTimecodeCHOP::processAudioSamples(const float* leftChannel, const float* rightChannel,
                                        int numSamples, float gain, bool swapLR, bool invertL, bool invertR, double pitchScale)
{
    if (!myTimecoder || !myTimecoderInitialized) {
        return;
    }

    myAudioBuffer.resize(numSamples * 2);

    myPeakL = 0.0f;
    myPeakR = 0.0f;

    for (int i = 0; i < numSamples; i++) {
        float L = leftChannel[i] * gain;
        float R = rightChannel[i] * gain;

        if (swapLR) std::swap(L, R);
        if (invertL) L = -L;
        if (invertR) R = -R;

        // Track peak levels
        float absL = std::abs(L);
        float absR = std::abs(R);
        if (absL > myPeakL) myPeakL = absL;
        if (absR > myPeakR) myPeakR = absR;

        // Clip detection
        if (absL > 0.99f || absR > 0.99f) myClipCount++;

        // Clamp and convert to int16_t
        L = std::max(-1.0f, std::min(1.0f, L));
        R = std::max(-1.0f, std::min(1.0f, R));

        myAudioBuffer[i * 2] = static_cast<int16_t>(L * 32767.0f);
        myAudioBuffer[i * 2 + 1] = static_cast<int16_t>(R * 32767.0f);
    }

    timecoder_submit(myTimecoder, myAudioBuffer.data(), numSamples);

    myRawPosition = timecoder_get_position(myTimecoder, nullptr);
    myPitch = timecoder_get_pitch(myTimecoder) * pitchScale;

    if (myRawPosition >= 0) {
        myPosition = static_cast<double>(myRawPosition);
        myLastValidPosition = myPosition;
    } else {
        if (myLastNumSamples > 0 && std::abs(myPitch) > 0.01) {
            double positionChange = myPitch * numSamples * 1000.0 / myCurrentSampleRate;
            myPosition = myLastValidPosition + positionChange;
            myLastValidPosition = myPosition;
        } else {
            myPosition = myLastValidPosition;
        }
    }

    myLastNumSamples = numSamples;
    myAlive = (myTimecoder->valid_counter > 0) ? 1 : 0;
}

void
VinylTimecodeCHOP::execute(CHOP_Output* output, const OP_Inputs* inputs, void*)
{
    int formatIndex = inputs->getParInt("Format");
    const char* formatName = TIMECODE_FORMATS[formatIndex];
    float gain = static_cast<float>(inputs->getParDouble("Gain"));
    double timecoderRate = inputs->getParDouble("Timecoderate");
    double vinylSpeed = inputs->getParDouble("Vinylspeed");
    double pitchScale = inputs->getParDouble("Pitchscale");
    bool swapLR = inputs->getParInt("Swaplr") != 0;
    bool invertL = inputs->getParInt("Invertl") != 0;
    bool invertR = inputs->getParInt("Invertr") != 0;
    double sampleRate = 44100.0;

    const OP_CHOPInput* inputChop = inputs->getNumInputs() > 0 ? inputs->getInputCHOP(0) : nullptr;

    if (inputChop) {
        sampleRate = inputChop->sampleRate;
        const double effectiveTimecoderRate = timecoderRate > 0.0 ? timecoderRate : sampleRate;
        vinylSpeed = vinylSpeed > 0.0 ? vinylSpeed : 1.0;

        if (!myTimecoderInitialized ||
            formatIndex != myCurrentFormatIndex ||
            std::abs(effectiveTimecoderRate - myCurrentSampleRate) > 0.1 ||
            std::abs(vinylSpeed - myCurrentVinylSpeed) > 0.0001) {
            initializeTimecoder(formatName, effectiveTimecoderRate, vinylSpeed);
            myCurrentFormatIndex = formatIndex;
        }

        if (inputChop->numChannels >= 2 && myTimecoderInitialized) {
            const float* leftChannel = inputChop->getChannelData(0);
            const float* rightChannel = inputChop->getChannelData(1);
            processAudioSamples(leftChannel, rightChannel, inputChop->numSamples, gain, swapLR, invertL, invertR, pitchScale);
        }
    }

    for (int i = 0; i < output->numSamples; i++) {
        output->channels[0][i] = static_cast<float>(myPosition);
        output->channels[1][i] = static_cast<float>(myPitch);
        output->channels[2][i] = static_cast<float>(myAlive);
    }
}

int32_t
VinylTimecodeCHOP::getNumInfoCHOPChans(void*)
{
    return 14;
}

void
VinylTimecodeCHOP::getInfoCHOPChan(int index, OP_InfoCHOPChan* chan, void*)
{
    switch (index) {
        case 0:
            chan->name->setString("position");
            chan->value = static_cast<float>(myPosition);
            break;
        case 1:
            chan->name->setString("pitch");
            chan->value = static_cast<float>(myPitch);
            break;
        case 2:
            chan->name->setString("quality");
            chan->value = static_cast<float>(myAlive);
            break;
        case 3:
            chan->name->setString("initialized");
            chan->value = myTimecoderInitialized ? 1.0f : 0.0f;
            break;
        case 4:
            chan->name->setString("sample_rate");
            chan->value = static_cast<float>(myCurrentSampleRate);
            break;
        case 5:
            chan->name->setString("format_index");
            chan->value = static_cast<float>(myCurrentFormatIndex);
            break;
        case 6:
            chan->name->setString("valid_counter");
            chan->value = myTimecoder ? static_cast<float>(myTimecoder->valid_counter) : 0.0f;
            break;
        case 7:
            chan->name->setString("raw_position");
            chan->value = static_cast<float>(myRawPosition);
            break;
        case 8:
            chan->name->setString("forwards");
            chan->value = myTimecoder ? static_cast<float>(myTimecoder->forwards) : 1.0f;
            break;
        case 9:
            chan->name->setString("peak_l");
            chan->value = myPeakL;
            break;
        case 10:
            chan->name->setString("peak_r");
            chan->value = myPeakR;
            break;
        case 11:
            chan->name->setString("clip_count");
            chan->value = static_cast<float>(myClipCount);
            break;
        case 12:
            chan->name->setString("timecoder_rate");
            chan->value = static_cast<float>(myCurrentSampleRate);
            break;
        case 13:
            chan->name->setString("vinyl_speed");
            chan->value = static_cast<float>(myCurrentVinylSpeed);
            break;
    }
}

void
VinylTimecodeCHOP::setupParameters(OP_ParameterManager* manager, void*)
{
    // Format menu
    {
        OP_StringParameter sp;
        sp.name = "Format";
        sp.label = "Timecode Format";
        sp.defaultValue = TIMECODE_LABELS[0];

        int numFormats = 0;
        while (TIMECODE_FORMATS[numFormats] != nullptr) numFormats++;

        OP_ParAppendResult res = manager->appendMenu(sp, numFormats,
            TIMECODE_LABELS, TIMECODE_FORMATS);
        assert(res == OP_ParAppendResult::Success);
    }

    // Input Gain
    {
        OP_NumericParameter np;
        np.name = "Gain";
        np.label = "Input Gain";
        np.defaultValues[0] = 1.0;
        np.minSliders[0] = 0.1;
        np.maxSliders[0] = 20.0;
        np.minValues[0] = 0.1;
        np.maxValues[0] = 100.0;
        np.clampMins[0] = true;
        np.clampMaxes[0] = true;

        OP_ParAppendResult res = manager->appendFloat(np);
        assert(res == OP_ParAppendResult::Success);
    }

    // Timecoder sample rate override. 0 means use the input CHOP sample rate.
    {
        OP_NumericParameter np;
        np.name = "Timecoderate";
        np.label = "Timecoder Rate";
        np.defaultValues[0] = 0.0;
        np.minSliders[0] = 0.0;
        np.maxSliders[0] = 96000.0;
        np.minValues[0] = 0.0;
        np.maxValues[0] = 192000.0;
        np.clampMins[0] = true;
        np.clampMaxes[0] = true;

        OP_ParAppendResult res = manager->appendFloat(np);
        assert(res == OP_ParAppendResult::Success);
    }

    // Matches Mixxx/xwax speed normalization. Use 1.35 for 45 RPM.
    {
        OP_NumericParameter np;
        np.name = "Vinylspeed";
        np.label = "Vinyl Speed";
        np.defaultValues[0] = 1.0;
        np.minSliders[0] = 0.5;
        np.maxSliders[0] = 1.5;
        np.minValues[0] = 0.1;
        np.maxValues[0] = 4.0;
        np.clampMins[0] = true;
        np.clampMaxes[0] = true;

        OP_ParAppendResult res = manager->appendFloat(np);
        assert(res == OP_ParAppendResult::Success);
    }

    // Final correction after xwax pitch detection.
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

    // Swap L/R
    {
        OP_NumericParameter np;
        np.name = "Swaplr";
        np.label = "Swap L/R";
        np.defaultValues[0] = 0;

        OP_ParAppendResult res = manager->appendToggle(np);
        assert(res == OP_ParAppendResult::Success);
    }

    // Invert L
    {
        OP_NumericParameter np;
        np.name = "Invertl";
        np.label = "Invert L";
        np.defaultValues[0] = 0;

        OP_ParAppendResult res = manager->appendToggle(np);
        assert(res == OP_ParAppendResult::Success);
    }

    // Invert R
    {
        OP_NumericParameter np;
        np.name = "Invertr";
        np.label = "Invert R";
        np.defaultValues[0] = 0;

        OP_ParAppendResult res = manager->appendToggle(np);
        assert(res == OP_ParAppendResult::Success);
    }
}
