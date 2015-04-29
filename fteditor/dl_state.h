/*
Copyright (C) 2015  Future Technology Devices International Ltd
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#ifndef FTEDITOR_DL_STATE_H
#define FTEDITOR_DL_STATE_H

namespace FTEDITOR {

struct DlParsed;

// TODO: OPTIMIZE:
// Have function to check if a DlParsed is state changing, check it before/after a display line is parsed
// Only invalidate the current state if a state changing DlParsed is being modified!

struct DlStateGraphics
{
	DlStateGraphics();

	int Cell;
	int BitmapHandle;
	int VertexFormat; // Shift = (4 - VertexFormat)
	int VertexTranslateX;
	int VertexTranslateY;
};

struct DlStateRendering
{
	DlStateRendering();

	int Primitive;
};

/*struct DlStateCoprocessor
{

};*/

struct DlState
{
	DlStateGraphics Graphics;
	DlStateRendering Rendering;
	// DlStateCoprocessor Coprocessor;

	static void process(int deviceIntf, DlState *state, const int line, const DlParsed *displayList, const bool coprocessor);
	static bool requiresProcessing(const DlParsed &pa);
};

} /* namespace FTEDITOR */

#endif /* #ifndef FTEDITOR_DL_STATE_H */

/* end of file */
