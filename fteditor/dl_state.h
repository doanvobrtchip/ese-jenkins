/*
Copyright (C) 2015  Future Technology Devices International Ltd
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#ifndef FTEDITOR_DL_STATE_H
#define FTEDITOR_DL_STATE_H

namespace FTEDITOR {

struct DlParsed;

struct DlState
{
	DlState();

	int Cell;
	int BitmapHandle;
	int VertexFormat; // Shift = (4 - VertexFormat)
	int VertexTranslateX;
	int VertexTranslateY;

	static void process(int deviceIntf, DlState &state, const int line, const DlParsed *displayList, const bool coprocessor);
};

} /* namespace FTEDITOR */

#endif /* #ifndef FTEDITOR_DL_STATE_H */

/* end of file */
