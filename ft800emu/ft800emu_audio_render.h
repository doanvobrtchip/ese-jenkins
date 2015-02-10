/**
 * AudioRenderClass
 * $Id$
 * \file ft800emu_audio_render.h
 * \brief AudioRenderClass
 * \date 2013-10-17 23:44GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FT800EMU_AUDIO_RENDER_H
#define FT800EMU_AUDIO_RENDER_H
// #include <...>

// System includes

// Project includes

namespace FT800EMU {

/**
 * AudioRenderClass
 * \brief AudioRenderClass
 * \date 2013-10-17 23:44GMT
 * \author Jan Boon (Kaetemi)
 */
class AudioRenderClass
{
public:
	AudioRenderClass() { }

	static void begin();
	static void end();

	static void playbackPlay();
	static void process();
	static void process(short *audioBuffer, int samples);

private:
	AudioRenderClass(const AudioRenderClass &);
	AudioRenderClass &operator=(const AudioRenderClass &);

}; /* class AudioRenderClass */

extern AudioRenderClass AudioRender;

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_AUDIO_RENDER_H */

/* end of file */
