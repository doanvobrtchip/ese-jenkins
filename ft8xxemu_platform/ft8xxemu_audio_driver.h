/**
 * AudioDriverClass
 * $Id$
 * \file ft8xxemu_audio_driver.h
 * \brief AudioDriverClass
 * \date 2011-05-29 19:38GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FT8XXEMU_AUDIO_DRIVER_H
#define FT8XXEMU_AUDIO_DRIVER_H
// #include <...>

// System includes

// Project includes

namespace FT8XXEMU {

/**
 * AudioDriverClass
 * \brief AudioDriverClass
 * \date 2011-05-29 19:38GMT
 * \author Jan Boon (Kaetemi)
 */
class AudioDriverClass
{
public:
	AudioDriverClass() { }

	static void begin();
	static bool update();
	static void end();

	static int getFrequency(); // WASAPI doesn't resample
	static int getChannels(); // WASAPI doesn't adjust channels

	static void beginBuffer(short **buffer, int *samples);
	static void endBuffer();

private:
	AudioDriverClass(const AudioDriverClass &);
	AudioDriverClass &operator=(const AudioDriverClass &);

}; /* class AudioDriverClass */

extern AudioDriverClass AudioDriver;

extern void (*g_AudioProcess)(short *audioBuffer, int samples);

} /* namespace FT8XXEMU */

#endif /* #ifndef FT8XXEMU_AUDIO_DRIVER_H */

/* end of file */
