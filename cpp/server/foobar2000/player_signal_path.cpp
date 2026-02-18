#include "player.hpp"

namespace msrv {
namespace player_foobar2000 {

namespace {

const char* replayGainSourceModeToString(int mode)
{
    switch (mode)
    {
    case t_replaygain_config::source_mode_none:              return "none";
    case t_replaygain_config::source_mode_track:             return "track";
    case t_replaygain_config::source_mode_album:             return "album";
    case t_replaygain_config::source_mode_byPlaybackOrder:   return "byPlaybackOrder";
    default:                                                  return "unknown";
    }
}

const char* replayGainProcessingModeToString(int mode)
{
    switch (mode)
    {
    case t_replaygain_config::processing_mode_none:          return "none";
    case t_replaygain_config::processing_mode_gain:          return "gain";
    case t_replaygain_config::processing_mode_gain_and_peak: return "gainAndPeak";
    case t_replaygain_config::processing_mode_peak:          return "peak";
    default:                                                  return "unknown";
    }
}

bool isExclusiveOutput(const char* backendName, const char* deviceName)
{
    // Check backend name for known exclusive patterns
    if (backendName)
    {
        pfc::string8 backend(backendName);
        if (backend.find_first("WASAPI (event)") != pfc::infinite_size)
            return true;
        if (backend.find_first("WASAPI (push)") != pfc::infinite_size)
            return true;
        if (backend.find_first("ASIO") != pfc::infinite_size)
            return true;
    }

    // foobar2000 v2 wraps outputs under "Default" backend
    // but marks exclusive devices with [exclusive] in device name
    if (deviceName)
    {
        pfc::string8 device(deviceName);
        if (device.find_first("[exclusive]") != pfc::infinite_size)
            return true;
    }

    return false;
}

} // anonymous namespace

SignalPathInfo PlayerImpl::getSignalPath()
{
    SignalPathInfo info;

    // === SOURCE (from currently playing track) ===
    metadb_handle_ptr handle;
    if (playbackControl_->get_now_playing(handle))
    {
        metadb_info_container::ptr infoRef;
        handle->get_info_ref(infoRef);
        const file_info& fi = infoRef->info();

        auto sr = fi.info_get("samplerate");
        if (sr) info.source.sampleRate = atoi(sr);

        auto bps = fi.info_get("bitspersample");
        if (bps) info.source.bitDepth = atoi(bps);

        auto ch = fi.info_get("channels");
        if (ch) info.source.channels = atoi(ch);

        auto codec = fi.info_get("codec");
        if (codec) info.source.codec = codec;

        auto br = fi.info_get("bitrate");
        if (br) info.source.bitrate = atoi(br);

        auto enc = fi.info_get("encoding");
        if (enc) info.source.encoding = enc;

        // ReplayGain per-track values
        auto rg = fi.get_replaygain();
        if (rg.is_track_gain_present())
            info.replayGain.trackGain = rg.m_track_gain;
        if (rg.is_track_peak_present())
            info.replayGain.trackPeak = rg.m_track_peak;
        if (rg.is_album_gain_present())
            info.replayGain.albumGain = rg.m_album_gain;
        if (rg.is_album_peak_present())
            info.replayGain.albumPeak = rg.m_album_peak;
    }

    // === REPLAY GAIN (global settings) ===
    try
    {
        auto rgManager = replaygain_manager::get();
        t_replaygain_config rgConfig;
        rgManager->get_core_settings(rgConfig);
        info.replayGain.sourceMode = replayGainSourceModeToString(rgConfig.m_source_mode);
        info.replayGain.processingMode = replayGainProcessingModeToString(rgConfig.m_processing_mode);
    }
    catch (...) {}

    // === DSP CHAIN ===
    try
    {
        auto dspManager = dsp_config_manager::get();
        dsp_chain_config_impl chain;
        dspManager->get_core_settings(chain);

        for (t_size i = 0; i < chain.get_count(); i++)
        {
            DspInfo dsp;
            const auto& preset = chain.get_item(i);
            pfc::string8 name;
            if (dsp_entry::g_name_from_guid(name, preset.get_owner()))
                dsp.name = name.get_ptr();
            else
                dsp.name = "Unknown DSP";
            dsp.active = true;
            info.dspChain.push_back(std::move(dsp));
        }
    }
    catch (...) {}

    // === OUTPUT ===
    try
    {
        auto config = outputManager_->getCoreConfig();

        // Get the output backend name (e.g. "WASAPI", "WASAPI (event)", "DS", "ASIO")
        output_entry::ptr entry;
        if (output_entry::g_find(config.m_output, entry))
        {
            const char* backendName = entry->get_name();
            info.output.backend = backendName ? backendName : "Unknown";

            // Get device name
            pfc::string8 deviceName = entry->get_device_name(config.m_device);
            if (deviceName.get_length() > 0)
                info.output.device = deviceName.get_ptr();
            else
                info.output.device = info.output.backend;

            info.output.exclusive = isExclusiveOutput(backendName, info.output.device.c_str());
        }

        info.output.bufferLength = config.m_buffer_length;
        info.output.configBitDepth = static_cast<int32_t>(config.m_bitDepth);
        info.output.useDither = (config.m_flags & outputCoreConfig_t::flagUseDither) != 0;
    }
    catch (...) {}

    return info;
}

}
}
