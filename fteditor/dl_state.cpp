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

DlStateGraphics::DlStateGraphics() :
	Cell(0), BitmapHandle(0), VertexFormat(4), VertexTranslateX(0), VertexTranslateY(0)
{

}

DlStateRendering::DlStateRendering() :
	Primitive(0)
{

}

bool DlState::requiresProcessing(const DlParsed &pa)
{
	if (pa.ValidId)
	{
		switch (pa.IdLeft)
		{
		case FTEDITOR_DL_INSTRUCTION:
			switch (pa.IdRight)
			{
			case FTEDITOR_DL_CELL:
				return true;
			case FTEDITOR_DL_BITMAP_HANDLE:
				return true;
			case FTEDITOR_DL_JUMP:
				return true;
			case FTEDITOR_DL_BEGIN:
				return true;
			case FTEDITOR_DL_END:
				return true;
			case FTEDITOR_DL_CALL:
				return true;
			case FTEDITOR_DL_SAVE_CONTEXT:
				return true;
			case FTEDITOR_DL_RESTORE_CONTEXT:
				return true;
			case FTEDITOR_DL_RETURN:
				return true;
			case FTEDITOR_DL_VERTEX_FORMAT:
				return true;
			case FTEDITOR_DL_VERTEX_TRANSLATE_X:
				return true;
			case FTEDITOR_DL_VERTEX_TRANSLATE_Y:
				return true;
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
	return false;
}

void DlState::process(int deviceIntf, DlState *state, const int line, const DlParsed *displayList, const bool coprocessor)
{
	// TODO: Process macros?

	std::stack<DlStateGraphics> gsstack;
	std::stack<int> callstack;

	const int dlSize = displayListSize(deviceIntf);
	const int dlLimit = dlSize * 4;

	if (line < 0 || line >= dlSize)
		return;

	bool allowJump = !coprocessor;
	bool cSet = false;

	DlState gs;

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
						gs.Graphics.Cell = pa.Parameter[0].I;
						break;
					case FTEDITOR_DL_BITMAP_HANDLE:
						gs.Graphics.BitmapHandle = pa.Parameter[0].I;
						break;
					case FTEDITOR_DL_JUMP:
						if (allowJump)
						{
							c = pa.Parameter[0].I - 1;
						}
						break;
					case FTEDITOR_DL_BEGIN:
						gs.Rendering.Primitive = pa.Parameter[0].I;
						break;
					case FTEDITOR_DL_END:
						gs.Rendering.Primitive = 0;
						break;
					case FTEDITOR_DL_CALL:
						if (allowJump)
						{
							callstack.push(c);
							c = pa.Parameter[0].I - 1;
						}
						break;
					case FTEDITOR_DL_SAVE_CONTEXT:
						gsstack.push(gs.Graphics);
						break;
					case FTEDITOR_DL_RESTORE_CONTEXT:
						if (gsstack.empty())
						{
							gs.Graphics = DlStateGraphics();
						}
						else
						{
							gs.Graphics = gsstack.top();
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
						gs.Graphics.VertexFormat = pa.Parameter[0].I;
						break;
					case FTEDITOR_DL_VERTEX_TRANSLATE_X:
						gs.Graphics.VertexTranslateX = pa.Parameter[0].I;
						break;
					case FTEDITOR_DL_VERTEX_TRANSLATE_Y:
						gs.Graphics.VertexTranslateY = pa.Parameter[0].I;
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
			}
			state[c] = gs;
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
