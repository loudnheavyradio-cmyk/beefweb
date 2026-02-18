#include "player.hpp"

namespace msrv::player_foobar2000 {

namespace {

RawMetadataResult extractRawMetadata(const metadb_handle_ptr& handle)
{
    RawMetadataResult result;

    metadb_info_container::ptr infoRef;

    if (!handle->get_info_ref(infoRef))
        return result;

    const file_info& info = infoRef->info();

    // Music tags (multi-value support)
    for (t_size i = 0; i < info.meta_get_count(); i++)
    {
        const char* name = info.meta_enum_name(i);
        std::vector<std::string> values;

        for (t_size j = 0; j < info.meta_enum_value_count(i); j++)
        {
            values.emplace_back(info.meta_enum_value(i, j));
        }

        result.tags[name] = std::move(values);
    }

    // Technical info (codec, samplerate, bitspersample, channels, bitrate, etc.)
    for (t_size i = 0; i < info.info_get_count(); i++)
    {
        const char* name = info.info_enum_name(i);
        const char* value = info.info_enum_value(i);
        result.techInfo[name] = value;
    }

    return result;
}

} // anonymous namespace

RawMetadataResult PlayerImpl::getRawMetadata(const PlaylistRef& plref, int32_t index)
{
    auto playlist = playlists_->getIndex(plref);

    metadb_handle_ptr handle;

    if (!playlistManager_->playlist_get_item_handle(handle, playlist, index))
        throw InvalidRequestException("playlist item index is out of range");

    return extractRawMetadata(handle);
}

RawMetadataResult PlayerImpl::getPlayingRawMetadata()
{
    metadb_handle_ptr handle;

    if (!playbackControl_->get_now_playing(handle))
        return {};

    return extractRawMetadata(handle);
}

}
