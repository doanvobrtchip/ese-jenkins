/**
 * interactive_viewport.h
 * $Id$
 * \file interactive_viewport.h
 * \brief interactive_viewport.h
 * \date 2013-12-15 13:09GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FT800EMUQT_INTERACTIVE_VIEWPORT_H
#define FT800EMUQT_INTERACTIVE_VIEWPORT_H

// STL includes
#include <vector>

// Qt includes

// Emulator includes

// Project includes
#include "emulator_viewport.h"

namespace FT800EMUQT {

class MainWindow;

/**
 * InteractiveViewport
 * \brief InteractiveViewport
 * \date 2013-12-15 13:09GMT
 * \author Jan Boon (Kaetemi)
 */
class InteractiveViewport : public EmulatorViewport
{
	Q_OBJECT

public:
	InteractiveViewport(MainWindow *parent);
	virtual ~InteractiveViewport();

	// Graphics callback synchronized to the emulator thread, use to get debug information for a frame
	virtual void graphics();

	// Graphics callback synchronized to Qt thread, use to overlay graphics
	virtual void graphics(QImage *image);

private:
	MainWindow *m_MainWindow;

	bool m_TraceEnabled;
	uint32_t m_TraceX, m_TraceY;
	std::vector<int> m_TraceStack;

private:
	InteractiveViewport(const InteractiveViewport &);
	InteractiveViewport &operator=(const InteractiveViewport &);
	
}; /* class InteractiveViewport */

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_INTERACTIVE_VIEWPORT_H */

/* end of file */
