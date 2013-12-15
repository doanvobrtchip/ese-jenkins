/**
 * interactive_viewport.cpp
 * $Id$
 * \file interactive_viewport.cpp
 * \brief interactive_viewport.cpp
 * \date 2013-12-15 13:09GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */
 
#include "interactive_viewport.h"

// STL includes

// Qt includes

// Emulator includes
#include <ft800emu_graphics_processor.h>

// Project includes
#include "main_window.h"
#include "dl_editor.h"
#include "code_editor.h"

namespace FT800EMUQT {

InteractiveViewport::InteractiveViewport(MainWindow *parent) 
	: EmulatorViewport(parent), m_MainWindow(parent)
{
	
}

InteractiveViewport::~InteractiveViewport()
{
	
}

// Graphics callback synchronized to the emulator thread, use to get debug information for a frame
void InteractiveViewport::graphics()
{
	FT800EMU::GraphicsProcessor.getDebugTrace(m_TraceEnabled, m_TraceX, m_TraceY);
	m_TraceStack.clear();
	if (m_TraceEnabled) FT800EMU::GraphicsProcessor.getDebugTrace(m_TraceStack);
}

// Graphics callback synchronized to Qt thread, use to overlay graphics
void InteractiveViewport::graphics(QImage *image)
{
	// Update frame dependent gui
	m_MainWindow->dlEditor()->codeEditor()->setTraceHighlights(m_TraceStack);

	// Draw image overlays
}

} /* namespace FT800EMUQT */

/* end of file */
