/**
 * AudioDriverClass
 * $Id$
 * \file ft800emu_audio_driver.h
 * \brief AudioDriverClass
 * \date 2011-05-29 19:38GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2011  Jan Boon (Kaetemi)
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FT800EMU_AUDIO_DRIVER_H
#define FT800EMU_AUDIO_DRIVER_H
// #include <...>

// System includes

// Project includes

namespace FT800EMU {

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

	static int getFrequency(); // for now because in wasapi i can't have it automagically resample :(

	static void beginBuffer(short **buffer, int *samples);
	static void endBuffer();

private:
	AudioDriverClass(const AudioDriverClass &);
	AudioDriverClass &operator=(const AudioDriverClass &);

}; /* class AudioDriverClass */

extern AudioDriverClass AudioDriver;

} /* namespace FT800EMU */

#endif /* #ifndef FT800EMU_AUDIO_DRIVER_H */

/* end of file */
