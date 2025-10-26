#include "VinylTimecodeCHOP.h"

#include <cstdio>
#include <cstring>
#include <algorithm>
#include <cmath>

// Include xwax C library
extern "C" {
#include "timecoder.h"
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
        info->apiVersion = TD::CHOPCPlusPlusAPIVersion;
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
    , myCurrentFormatIndex(0)
    , myCurrentSampleRate(44100.0)
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
VinylTimecodeCHOP::initializeTimecoder(const char* formatName, double sampleRate)
{
    // Clean up existing timecoder if any
    cleanupTimecoder();

    // Find the timecode definition
    timecode_def* def = timecoder_find_definition(formatName);

    if (!def) {
        fprintf(stderr, "VinylTimecodeCHOP: Unknown timecode format '%s'\n", formatName);
        return;
    }

    // Create new timecoder
    myTimecoder = new timecoder;
    myCurrentDef = def;

    // Initialize with the definition
    // Parameters: timecoder, def, speed (1.0 for 33 1/3 RPM), sample_rate, phono, use_legacy_pitch_filter
    timecoder_init(myTimecoder, def, 1.0, static_cast<unsigned int>(sampleRate), false, false);

    myCurrentSampleRate = sampleRate;
    myTimecoderInitialized = true;
}

void
VinylTimecodeCHOP::getGeneralInfo(CHOP_GeneralInfo* ginfo, const OP_Inputs* inputs, void*)
{
    ginfo->cookEveryFrame = false;
    ginfo->cookEveryFrameIfAsked = true;
    ginfo->timeslice = false;
    ginfo->inputMatchIndex = 0;
}

bool
VinylTimecodeCHOP::getOutputInfo(CHOP_OutputInfo* info, const OP_Inputs* inputs, void*)
{
    info->numChannels = 3;  // position, pitch, quality

    // Match input sample count if input exists
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
        case 0:
            name->setString("position");
            break;
        case 1:
            name->setString("pitch");
            break;
        case 2:
            name->setString("quality");
            break;
        default:
            name->setString("unknown");
            break;
    }
}

void
VinylTimecodeCHOP::processAudioSamples(const float* leftChannel, const float* rightChannel, int numSamples)
{
    if (!myTimecoder || !myTimecoderInitialized) {
        return;
    }

    // Prepare interleaved int16_t buffer
    myAudioBuffer.resize(numSamples * 2);

    for (int i = 0; i < numSamples; i++) {
        // Clamp and convert to int16_t
        float leftSample = std::max(-1.0f, std::min(1.0f, leftChannel[i]));
        float rightSample = std::max(-1.0f, std::min(1.0f, rightChannel[i]));

        myAudioBuffer[i * 2] = static_cast<int16_t>(leftSample * 32767.0f);
        myAudioBuffer[i * 2 + 1] = static_cast<int16_t>(rightSample * 32767.0f);
    }

    // Submit to timecoder
    timecoder_submit(myTimecoder, myAudioBuffer.data(), numSamples);

    // Get the latest values
    myRawPosition = timecoder_get_position(myTimecoder, nullptr);
    myPitch = timecoder_get_pitch(myTimecoder);

    // Handle -1 (position unknown) with pitch-based interpolation
    if (myRawPosition >= 0) {
        // Valid position received, update both current and last valid
        myPosition = static_cast<double>(myRawPosition);
        myLastValidPosition = myPosition;
    } else {
        // Position is -1 (unknown)
        // Use pitch-based interpolation for smooth movement
        if (myLastNumSamples > 0 && std::abs(myPitch) > 0.01) {
            // Estimate position change based on pitch and samples
            // pitch is in relative speed (1.0 = normal, -1.0 = reverse at normal speed)
            double positionChange = myPitch * numSamples;
            myPosition = myLastValidPosition + positionChange;

            // Update last valid position for next iteration
            myLastValidPosition = myPosition;
        } else {
            // No pitch information or stopped, keep the last valid position
            myPosition = myLastValidPosition;
        }
    }

    // Store sample count for next iteration
    myLastNumSamples = numSamples;

    // Quality based on valid_counter (number of successful error checks)
    myAlive = (myTimecoder->valid_counter > 0) ? 1 : 0;
}

void
VinylTimecodeCHOP::execute(CHOP_Output* output, const OP_Inputs* inputs, void*)
{
    // Get parameters - menu parameter returns index
    int formatIndex = inputs->getParInt("Format");
    const char* formatName = TIMECODE_FORMATS[formatIndex];
    double sampleRate = 44100.0; // Default

    // Get input CHOP
    const OP_CHOPInput* inputChop = inputs->getNumInputs() > 0 ? inputs->getInputCHOP(0) : nullptr;

    if (inputChop) {
        sampleRate = inputChop->sampleRate;

        // Reinitialize if format or sample rate changed
        if (!myTimecoderInitialized ||
            formatIndex != myCurrentFormatIndex ||
            std::abs(sampleRate - myCurrentSampleRate) > 0.1) {

            initializeTimecoder(formatName, sampleRate);
            myCurrentFormatIndex = formatIndex;
        }

        // Process audio if we have at least 2 channels (L/R)
        if (inputChop->numChannels >= 2 && myTimecoderInitialized) {
            const float* leftChannel = inputChop->getChannelData(0);
            const float* rightChannel = inputChop->getChannelData(1);

            processAudioSamples(leftChannel, rightChannel, inputChop->numSamples);
        }
    }

    // Output the current values
    for (int i = 0; i < output->numSamples; i++) {
        output->channels[0][i] = static_cast<float>(myPosition);
        output->channels[1][i] = static_cast<float>(myPitch);
        output->channels[2][i] = static_cast<float>(myAlive);
    }
}

int32_t
VinylTimecodeCHOP::getNumInfoCHOPChans(void*)
{
    return 9;  // Added raw_position channel
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
            chan->name->setString("timecode_ticker");
            chan->value = myTimecoder ? static_cast<float>(myTimecoder->timecode_ticker) : 0.0f;
            break;
        case 8:
            chan->name->setString("raw_position");
            chan->value = static_cast<float>(myRawPosition);
            break;
    }
}

void
VinylTimecodeCHOP::setupParameters(OP_ParameterManager* manager, void*)
{
    // Format menu parameter
    {
        OP_StringParameter sp;
        sp.name = "Format";
        sp.label = "Timecode Format";
        sp.defaultValue = TIMECODE_LABELS[0];

        // Count number of formats
        int numFormats = 0;
        while (TIMECODE_FORMATS[numFormats] != nullptr) {
            numFormats++;
        }

        OP_ParAppendResult res = manager->appendMenu(sp, numFormats,
            TIMECODE_LABELS,
            TIMECODE_FORMATS);

        assert(res == OP_ParAppendResult::Success);
    }
}
