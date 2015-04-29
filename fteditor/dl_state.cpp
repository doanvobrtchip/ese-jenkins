/*
Copyright (C) 2015  Future Technology Devices International Ltd
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#include "dl_state.h"

#include <stdio.h>
#include <stack>

#include "dl_parser.h"
#include "constant_mapping.h"
#include "constant_common.h"

namespace FTEDITOR {

DlState::DlState() :
	Cell(0), BitmapHandle(0), VertexFormat(4), VertexTranslateX(0), VertexTranslateY(0)
{

}

void DlState::process(int deviceIntf, DlState &state, const int line, const DlParsed *displayList, const bool coprocessor)
{
	// TODO: Process macros?

	std::stack<DlState> gsstack;
	std::stack<int> callstack;

	const int dlSize = displayListSize(deviceIntf);
	const int dlLimit = dlSize * 64;

	if (line < 0 || line >= dlSize)
		return;

	bool allowJump = !coprocessor;
	bool cSet = false;

	for (;;)
	{
		for (int c = 0, dlCount = 0; c < dlSize && dlCount < dlLimit; ++c, ++dlCount)
		{
			const DlParsed &pa = displayList[c];
			if (pa.ValidId)
			{
				switch (pa.IdLeft)
				{
				case FTEDITOR_DL_INSTRUCTION:
					switch (pa.IdRight)
					{
					case FTEDITOR_DL_CELL:
						state.Cell = pa.Parameter[0].I;
						break;
					case FTEDITOR_DL_BITMAP_HANDLE:
						state.BitmapHandle = pa.Parameter[0].I;
						break;
					case FTEDITOR_DL_JUMP:
						if (allowJump)
						{
							c = pa.Parameter[0].I - 1;
						}
						break;
					case FTEDITOR_DL_CALL:
						if (allowJump)
						{
							callstack.push(c);
							c = pa.Parameter[0].I - 1;
						}
						break;
					case FTEDITOR_DL_SAVE_CONTEXT:
						gsstack.push(state);
						break;
					case FTEDITOR_DL_RESTORE_CONTEXT:
						if (gsstack.empty())
						{
							state = DlState();
						}
						else
						{
							state = gsstack.top();
							gsstack.pop();
						}
						break;
					case FTEDITOR_DL_RETURN:
						if (allowJump)
						{
							if (callstack.empty())
							{
								c = dlSize;
							}
							else
							{
								c = callstack.top();
								callstack.pop();
							}
						}
						break;
					case FTEDITOR_DL_VERTEX_FORMAT:
						state.VertexFormat = pa.Parameter[0].I;
						break;
					case FTEDITOR_DL_VERTEX_TRANSLATE_X:
						state.VertexTranslateX = pa.Parameter[0].I;
						break;
					case FTEDITOR_DL_VERTEX_TRANSLATE_Y:
						state.VertexTranslateY = pa.Parameter[0].I;
						break;
					}
					break;
				case FTEDITOR_DL_VERTEX2F:
					break;
				case FTEDITOR_DL_VERTEX2II:
					break;
				case FTEDITOR_CO_COMMAND:
					break;
				}
			}
			if (c == line)
			{
				cSet = true;
				break;
			}
		}
		if (cSet)
		{
			break;
		}
		else
		{
			allowJump = false;
		}
	}
}

} /* namespace FTEDITOR */

/* end of file */
