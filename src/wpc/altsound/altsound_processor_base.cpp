// ---------------------------------------------------------------------------
// altsound_processor_base.cpp
// 06/23/23 - Dave Roscoe
//
// Base class implementation for AltsoundProcessor, which encapsulates
// command execution, and specialization in support of all AltSound formats
// ---------------------------------------------------------------------------
// license:<TODO>
// ---------------------------------------------------------------------------
#include "altsound_processor_base.hpp"

#include "altsound_logger.hpp"

extern AltsoundLogger alog;
extern StreamArray channel_stream;
extern float master_vol;
extern float global_vol;

// ---------------------------------------------------------------------------
// CTOR/DTOR
// ---------------------------------------------------------------------------

AltsoundProcessorBase::AltsoundProcessorBase(const std::string& game_name_in)
: game_name(game_name_in)
{
}

AltsoundProcessorBase::~AltsoundProcessorBase()
{
	// clean up stored steam objects
	for (auto& stream : channel_stream) {
		delete stream;
		stream = nullptr;
	}
}

// ---------------------------------------------------------------------------

bool AltsoundProcessorBase::findFreeChannel(unsigned int& channel_out)
{
	ALT_INFO(0, "BEGIN: AltsoundProcessorBase::findFreeChannel()");
	INDENT;

	auto it = std::find(channel_stream.begin(), channel_stream.end(), nullptr);
	if (it != channel_stream.end()) {
		channel_out = std::distance(channel_stream.begin(), it);
		ALT_INFO(1, "Found free channel: %02u", channel_out);
		
		OUTDENT;
		ALT_DEBUG(0, "END: AltsoundProcessorBase::findFreeChannel()");
		return true;
	}
	ALT_ERROR(1, "No free channels available!");
	
	OUTDENT;
	ALT_DEBUG(0, "END AltsoundProcessorBase::findFreeChannel()");
	return false;
}

// ----------------------------------------------------------------------------

bool AltsoundProcessorBase::setStreamVolume(HSTREAM stream_in, const float vol_in)
{
	ALT_DEBUG(0, "BEGIN: AltsoundProcessorBase::setVolume()");
	INDENT;

	if (stream_in == BASS_NO_STREAM)
		return true;

	float new_vol = vol_in * global_vol * master_vol;
	ALT_INFO(1, "Setting volume for stream %u", stream_in);
	ALT_DEBUG(1, "SAMPLE_VOL:%.02f  GLOBAL_VOL:%.02f  MASTER_VOL:%.02f", vol_in,
		      global_vol, master_vol);
	bool success = BASS_ChannelSetAttribute(stream_in, BASS_ATTRIB_VOL, new_vol);

	if (!success) {
		ALT_ERROR(1, "FAILED BASS_ChannelSetAttribute(BASS_ATTRIB_VOL)");
	}
	else {
		ALT_INFO(1, "SUCCESS BASS_ChannelSetAttribute(BASS_ATTRIB_VOL)");
	}

	OUTDENT;
	ALT_DEBUG(0, "END: AltsoundProcessorBase::setVolume()");
	return success;
}

// ----------------------------------------------------------------------------

bool AltsoundProcessorBase::createStream(void* syncproc_in, AltsoundStreamInfo* stream_out)
{
	ALT_DEBUG(0, "BEGIN AltsoundProcessorBase::createStream()");
	INDENT;

	std::string short_path = getShortPath(stream_out->sample_path); // supports logging
	unsigned int ch_idx;
	
	if (!ALT_CALL(findFreeChannel(ch_idx))) {
		ALT_ERROR(1, "FAILED AltsoundProcessorBase::findFreeChannel()");
		
		OUTDENT;
		ALT_DEBUG(0, "END AltsoundProcessorBase::createStream()");
		return false;
	}

	stream_out->channel_idx = ch_idx; // store channel assignment
	bool loop = stream_out->loop;

  // Create playback stream
	HSTREAM hstream = BASS_StreamCreateFile(FALSE, stream_out->sample_path.c_str(), 0, 0, loop ? BASS_SAMPLE_LOOP : 0);

	if (hstream == BASS_NO_STREAM) {
		// Failed to create stream
		ALT_ERROR(1, "FAILED BASS_StreamCreateFile(%s): %s", short_path.c_str(), get_bass_err());
		
		OUTDENT;
		ALT_DEBUG(0, "END: AltsoundProcessorBase::createStream()");
		return false;
	}

	// Set callback to execute when sample playback ends
	SYNCPROC* callback = static_cast<SYNCPROC*>(syncproc_in);
	HSYNC hsync = 0;

	if (callback) {
		// Set sync to execute callback when sample playback ends
		hsync = BASS_ChannelSetSync(hstream, BASS_SYNC_END | BASS_SYNC_ONETIME, 0,
			                              callback, stream_out);
		if (!hsync) {
			// Failed to set sync
			ALT_ERROR(1, "FAILED BASS_ChannelSetSync(): STREAM: %u ERROR: %s", hstream, get_bass_err());
			freeStream(hstream);
			
			OUTDENT;
			ALT_DEBUG(0, "END: AltsoundProcessorBase::createStream()");
			return false;
		}
	}
	ALT_INFO(1, "Successfully created stream(%u) on channel(%02d)", hstream, ch_idx);

	stream_out->hstream = hstream; // store hstream
	stream_out->hsync = hsync; // store hsync

	OUTDENT;
	ALT_DEBUG(0, "END: AltsoundProcessorBase::createStream()");
	return true;
}

// ----------------------------------------------------------------------------

bool AltsoundProcessorBase::freeStream(const HSTREAM hstream_in)
{
	ALT_INFO(0, "BEGIN AltsoundProcessorBase::freeStream()");
	INDENT;

	bool success = BASS_StreamFree(hstream_in);
	if (!success) {
		ALT_ERROR(1, "FAILED BASS_StreamFree(%u)", hstream_in);
	}
	else {
		ALT_INFO(1, "Successfully free'd stream(%u)", hstream_in);
	}

	OUTDENT;
	ALT_DEBUG(0, "END AltsoundProcessorBase::freeStream()");
	return success;
}

// ----------------------------------------------------------------------------

bool AltsoundProcessorBase::stopStream(HSTREAM hstream_in)
{
	ALT_DEBUG(0, "BEGIN: AltsoundProcessorBase::stopStream()");
	INDENT;

	bool success = false;

	if (hstream_in != BASS_NO_STREAM) {
		if (BASS_ChannelStop(hstream_in)) {
			success = freeStream(hstream_in);
			if (!success) {
				ALT_ERROR(0, "FAILED AltsoundProcessorBase::freeStream()");
			}
			else {
				ALT_INFO(0, "Successfully stopped stream(%u)", hstream_in);
			}
		}
		else {
			ALT_ERROR(0, "FAILED BASS_ChannelStop(%u): %s", hstream_in, get_bass_err());
		}
	}

	OUTDENT;
	ALT_DEBUG(0, "END: AltsoundProcessorBase::stopStream()");
	return success;
}

// ----------------------------------------------------------------------------

bool AltsoundProcessorBase::stopAllStreams()
{
	ALT_DEBUG(0, "BEGIN AltsoundProcessorBase::stopAllStreams()");
	INDENT;

	bool success = true;

	for (auto stream : channel_stream) {
		if (!stream)
			continue;

		if (!stopStream(stream->hstream)) {
			success = false;
			ALT_ERROR(0, "FAILED stopStream(%u)", stream->hstream);
		}
	}
	
	ALT_DEBUG(0, "END AltsoundProcessorBase::stopAllStreams()");
	return success;
}

// ---------------------------------------------------------------------------
// Helper function to remove major path from filenames.  Returns just:
// <ROM shortname>/<rest of path>
// ---------------------------------------------------------------------------

std::string AltsoundProcessorBase::getShortPath(const std::string& path_in)
{
	std::string tmp_str = strstr(path_in.c_str(), game_name.c_str());
	return tmp_str.empty() ? path_in : tmp_str;
}

