/**
 * dl_editor.cpp
 * $Id$
 * \file dl_editor.cpp
 * \brief dl_editor.cpp
 * \date 2013-11-05 09:02GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#include "dl_editor.h"

// STL includes

// Qt includes
#include <QVBoxLayout>
#include <QTextBlock>
#include <QCompleter>
#include <QStringListModel>
#include <QAbstractItemView>

// Emulator includes
#include <ft800emu_graphics_processor.h>
#include <vc.h>

// Project includes
#include "main_window.h"
#include "interactive_viewport.h"
#include "code_editor.h"
#include "dl_highlighter.h"
#include "properties_editor.h"

using namespace std;

namespace FT800EMUQT {

DlEditor::DlEditor(MainWindow *parent, bool coprocessor) : QWidget(parent), m_MainWindow(parent), m_Reloading(false), m_CompleterIdentifiersActive(true), 
m_PropertiesEditor(NULL), m_PropLine(-1), m_PropIdLeft(-1), m_PropIdRight(-1), m_ModeMacro(false), m_ModeCoprocessor(coprocessor)
{
	m_DisplayListShared[0] = DISPLAY();
	
	m_CodeEditor = new CodeEditor();
	m_CodeEditor->setMaxLinesNotice(FT800EMU_DL_SIZE);
	// m_CodeEditor->setReadOnly(true);
	// m_CodeEditor->setFocusPolicy(Qt::NoFocus);
	// m_CommandInput = new QLineEdit();

	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(m_CodeEditor);
	// layout->addWidget(m_CommandInput);
	setLayout(layout);
	
	m_DlHighlighter = new DlHighlighter(m_CodeEditor->document(), coprocessor);

	// connect(m_CommandInput, SIGNAL(returnPressed()), this, SLOT(returnPressed()));
	
	connect(m_CodeEditor->document(), SIGNAL(contentsChange(int, int, int)), this, SLOT(documentContentsChange(int, int, int)));
	connect(m_CodeEditor->document(), SIGNAL(blockCountChanged(int)), this, SLOT(documentBlockCountChanged(int)));
	connect(m_CodeEditor, SIGNAL(cursorPositionChanged()), this, SLOT(editorCursorPositionChanged()));
	
	DlParser::getIdentifiers(m_CompleterIdentifiers, m_ModeCoprocessor);
	DlParser::getParams(m_CompleterParams, m_ModeCoprocessor);
	
	m_CompleterModel = new QStringListModel(m_CodeEditor);
	m_CompleterModel->setStringList(m_CompleterIdentifiers);
	
	m_Completer = new QCompleter(m_CompleterModel, m_CodeEditor);
	m_CodeEditor->setCompleter(m_Completer);
}

DlEditor::~DlEditor()
{

}

void DlEditor::setUndoStack(QUndoStack *undo_stack)
{
	m_CodeEditor->setUndoStack(undo_stack);
}

void DlEditor::clearUndoStack()
{
	m_CodeEditor->document()->clearUndoRedoStacks();
}

void DlEditor::setModeMacro()
{
	m_ModeMacro = true;
	m_CodeEditor->setMaxLinesNotice(FT800EMU_MACRO_SIZE);
	/*QFontMetrics m(m_CodeEditor->font()) ;
	int height = m.lineSpacing() ;
	m_CodeEditor->setFixedHeight(3 * height) ;*/
}

void DlEditor::clear()
{
	m_CodeEditor->clear();
	lockDisplayList();
	for (int i = 0; i < FT800EMU_DL_SIZE; ++i)
	{
		m_DisplayListShared[i] = DISPLAY();
	}
	m_DisplayListModified = true;
	m_CodeEditor->textCursor().setPosition(0);
	unlockDisplayList();
}

void DlEditor::lockDisplayList()
{
	m_Mutex.lock();
}

void DlEditor::unlockDisplayList()
{
	m_Mutex.unlock();
}

// reloads the entire display list from m_DisplayListShared
void DlEditor::reloadDisplayList(bool fromEmulator)
{
	bool firstLine = true;
	m_Reloading = true;
	if (!fromEmulator)
	{
		m_DisplayListModified = true;
	}
	int dcount = 0;
	for (int i = 0; i < (m_ModeMacro ? FT800EMU_MACRO_SIZE : FT800EMU_DL_SIZE); ++i)
	{
		if (!m_ModeMacro && m_DisplayListShared[i] == DISPLAY() && dcount > 0)
		{
			++dcount;
		}
		else
		{
			if (dcount > 0)
			{
				for (int dc = 1; dc < dcount; ++dc)
				{
					if (firstLine) firstLine = false;
					else m_CodeEditor->textCursor().insertText("\n");
				}
				dcount = 0;
			}
			if (m_DisplayListShared[i] == DISPLAY())
			{
				++dcount;
			}
			QString line = DlParser::toString(m_DisplayListShared[i]);
			m_DisplayListParsed[i] = DlParsed();
			// verify parsing ->
			DlParser::parse(m_DisplayListParsed[i], line, m_ModeCoprocessor);
			uint32_t compiled = DlParser::compile(m_DisplayListParsed[i]);
			if (compiled != m_DisplayListShared[i])
			{
				QByteArray chars = line.toLatin1();
				const char *src = chars.constData();
				printf("Parser bug at dl %i: '%s' -> expect %u, compiled %u\n", i, src, m_DisplayListShared[i], compiled);
			}
			// <- verify parsing
			if (firstLine) firstLine = false;
			else m_CodeEditor->textCursor().insertText("\n");
			m_CodeEditor->textCursor().insertText(line);
		}
	}
	m_Reloading = false;
}

void DlEditor::editorCursorPositionChanged()
{
	QTextBlock block = m_CodeEditor->document()->findBlock(m_CodeEditor->textCursor().position());
	
	// switch between auto completers
	if (m_DisplayListParsed[block.blockNumber()].ValidId && m_CompleterIdentifiersActive)
	{
		m_CompleterIdentifiersActive = false;
		m_CompleterModel->setStringList(m_CompleterParams);
		m_Completer->popup()->hide();
	}
	else if (!m_DisplayListParsed[block.blockNumber()].ValidId && !m_CompleterIdentifiersActive)
	{
		m_CompleterIdentifiersActive = true;
		m_CompleterModel->setStringList(m_CompleterIdentifiers);
		m_Completer->popup()->hide();
	}
	
	editingLine(block);
}

void DlEditor::documentContentsChange(int position, int charsRemoved, int charsAdded)
{
	if (m_Reloading)
		return;
	
	QTextBlock block = m_CodeEditor->document()->findBlock(position);
	
	lockDisplayList();
	parseLine(block);
	m_DisplayListModified = true;
	unlockDisplayList();
}

void DlEditor::documentBlockCountChanged(int newBlockCount)
{
	if (m_Reloading)
		return;
	
	lockDisplayList();
	for (int i = 0; i < newBlockCount && i < (m_ModeMacro ? FT800EMU_MACRO_SIZE : FT800EMU_DL_SIZE); ++i)
	{
		parseLine(m_CodeEditor->document()->findBlockByNumber(i));
	}
	for (int i = newBlockCount; i < (m_ModeMacro ? FT800EMU_MACRO_SIZE : FT800EMU_DL_SIZE); ++i)
	{
		m_DisplayListParsed[i] = DlParsed();
		m_DisplayListShared[i] = DISPLAY();
	}
	m_DisplayListModified = true;
	unlockDisplayList();
	
	editorCursorPositionChanged();
}

void DlEditor::parseLine(QTextBlock block)
{
	QString line = block.text();
	int i = block.blockNumber();
	m_DisplayListParsed[i] = DlParsed();
	DlParser::parse(m_DisplayListParsed[i], line, m_ModeCoprocessor);
	m_DisplayListShared[i] = DlParser::compile(m_DisplayListParsed[i]);
	
	// check for misformed lines and do a no-op (todo: mark them)
	if (m_DisplayListShared[i] == DISPLAY() && !m_DisplayListParsed[i].ValidId)
	{
		m_DisplayListShared[i] = JUMP(i + 1);
	}
}

void DlEditor::replaceLine(int line, const DlParsed &parsed)
{
	uint32_t compiled = DlParser::compile(parsed);
	QString linestr = DlParser::toString(compiled);
	QTextCursor c = m_CodeEditor->textCursor();
	c.setPosition(m_CodeEditor->document()->findBlockByNumber(line).position());
	//m_CodeEditor->setTextCursor(c);
	c.select(QTextCursor::LineUnderCursor);
	c.insertText(linestr);
}

const DlParsed &DlEditor::getLine(int line) const
{
	return m_DisplayListParsed[line];
}

void DlEditor::selectLine(int line)
{
	QTextCursor c = m_CodeEditor->textCursor();
	c.setPosition(m_CodeEditor->document()->findBlockByNumber(line).position());
	m_CodeEditor->setTextCursor(c);
}

void DlEditor::editingLine(QTextBlock block)
{
	// update properties editor
	if (m_PropertiesEditor->getEditWidgetSetter() != this
		|| block.blockNumber() != m_PropLine
		|| m_DisplayListParsed[block.blockNumber()].IdLeft != m_PropIdLeft 
		|| m_DisplayListParsed[block.blockNumber()].IdRight != m_PropIdRight
		|| m_DisplayListParsed[block.blockNumber()].ValidId != m_PropIdValid)
	{
		m_PropLine = block.blockNumber();
		m_PropIdLeft = m_DisplayListParsed[m_PropLine].IdLeft;
		m_PropIdRight = m_DisplayListParsed[m_PropLine].IdRight;
		m_PropIdValid = m_DisplayListParsed[m_PropLine].ValidId;
		if (m_PropIdValid)
		{
			m_MainWindow->viewport()->setEditorLine(this, m_PropLine);
			bool ok = false;
			// const uint32_t *p = m_DisplayListParsed[i].Parameter;
			if (m_DisplayListParsed[m_PropLine].IdLeft == FT800EMU_DL_VERTEX2F)
			{
				m_PropertiesEditor->setInfo(tr(
					"<b>VERTEX2F</b>(<i>x</i>, <i>y</i>)<br>"
					"<b>x</b>: Signed x-coordinate in 1/16 pixel precision<br>"
					"<b>y</b>: Signed x-coordinate in 1/16 pixel precision<br>"
					"<br>"
					"Start the operation of graphics primitives at the specified screen coordinate, in 1/16th "
					"pixel precision.<br>"
					"<br>"
					"The range of coordinates can be from -16384 to +16383 in terms of 1/16 th pixel "
					"units. Please note the negative x coordinate value means the coordinate in the left "
					"virtual screen from (0, 0), while the negative y coordinate value means the "
					"coordinate in the upper virtual screen from (0, 0). If drawing on the negative "
					"coordinate position, the drawing operation will not be visible."));
				m_PropertiesEditor->setEditWidget(NULL, false, this);
				ok = true;
			}
			else if (m_DisplayListParsed[m_PropLine].IdLeft == FT800EMU_DL_VERTEX2II)
			{
				m_PropertiesEditor->setInfo(tr(
					"<b>VERTEX2II</b>(<i>x</i>, <i>y</i>, <i>handle</i>, </i>cell</i>)<br>"
					"<b>x</b>: x-coordinate in pixels<br>"
					"<b>y</b>: y-coordinate in pixels<br>"
					"<b>handle</b>: Bitmap handle. The valid range is from 0 to 31. From 16 to 31, the bitmap "
					"handle is dedicated to the FT800 built-in font.<br>"
					"<b>cell</b>: Cell number. Cell number is the index of bitmap with same bitmap layout and "
					"format. For example, for handle 31, the cell 65 means the character \"A\" in "
					"the largest built in font.<br>"
					"<br>"
					"Start the operation of graphics primitive at the specified coordinates. The handle and cell "
					"parameters will be ignored unless the graphics primitive is specified as bitmap by "
					"command BEGIN, prior to this command."));
				m_PropertiesEditor->setEditWidget(NULL, false, this);
				ok = true;
			}
			else if (m_DisplayListParsed[m_PropLine].IdLeft == 0xFFFFFF00) switch (m_DisplayListParsed[m_PropLine].IdRight)
			{
				// ...
			}
			else switch (m_DisplayListParsed[m_PropLine].IdRight)
			{
				case FT800EMU_DL_DISPLAY:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>DISPLAY</b>()<br>"
						"<br>"
						"End the display list. FT800 will ignore all the commands following this command."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_BITMAP_SOURCE:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>BITMAP_SOURCE</b>(<i>addr</i>)<br>"
						"<b>addr</b>: Bitmap address in graphics SRAM FT800, aligned with respect to the bitmap "
						"format.<br>"
						"For example, if the bitmap format is RGB565/ARGB4/ARGB1555, the bitmap "
						"source shall be aligned to 2 bytes.<br>"
						"<br>"
						"Specify the source address of bitmap data in FT800 graphics memory RAM_G."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_CLEAR_COLOR_RGB:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>CLEAR_COLOR_RGB</b>(<i>red</i>, <i>green</i>, <i>blue</i>)<br>"
						"<b>red</b>: Red value used when the color buffer is cleared. The initial value is 0<br>"
						"<b>green</b>: Green value used when the color buffer is cleared. The initial value is 0<br>"
						"<b>blue</b>: Blue value used when the color buffer is cleared. The initial value is 0<br>"
						"<br>"
						"Sets the color values used by a following CLEAR."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_TAG:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>TAG</b>(<i>s</i>)<br>"
						"<b>s</b>: Tag value. Valid value range is from 1 to 255.<br>"
						"<br>"
						"Attach the tag value for the following graphics objects drawn on the screen. The initial "
						"tag buffer value is 255."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_COLOR_RGB:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>COLOR_RGB</b>(<i>red</i>, <i>green</i>, <i>blue</i>)<br>"
						"<b>red</b>: Red value for the current color. The initial value is 255<br>"
						"<b>green</b>: Green value for the current color. The initial value is 255<br>"
						"<b>blue</b>: Blue value for the current color. The initial value is 255<br>"
						"<br>"
						"Sets red, green and blue values of the FT800 color buffer which will be applied to the "
						"following draw operation."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_BITMAP_HANDLE:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>BITMAP_HANDLE</b>(<i>handle</i>)<br>"
						"<b>handle</b>: Bitmap handle. The initial value is 0. The valid value range is from 0 to 31.<br>"
						"<br>"
						"Specify the bitmap handle.<br>"
						"<br>"
						"Handles 16 to 31 are defined by the FT800 for built-in font and handle 15 is "
						"defined in the co-processor engine commands CMD_GRADIENT, CMD_BUTTON and "
						"CMD_KEYS. Users can define new bitmaps using handles from 0 to 14. If there is "
						"no co-processor engine command CMD_GRADIENT, CMD_BUTTON and CMD_KEYS in "
						"the current display list, users can even define a bitmap using handle 15."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_CELL:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>CELL</b>(<i>cell</i>)<br>"
						"<b>cell</b>: Bitmap cell number. The initial value is 0<br>"
						"<br>"
						"Specify the bitmap cell number for the VERTEX2F command."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_BITMAP_LAYOUT:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>BITMAP_LAYOUT</b>(<i>format</i>, <i>linestride</i>, <i>height</i>)<br>"
						"<b>format</b>: Bitmap pixel format. The bitmap formats supported are L1, L4, L8, RGB332, ARGB2, ARGB4, ARGB1555, "
						"RGB565 and Palette.<br>"
						"<b>linestride</b>: Bitmap linestride, in bytes. For L1 format, the line stride must be a multiple of 8 bits; For L4 format the line "
						"stride must be multiple of 2 nibbles. (Aligned to byte).<br>"
						"<b>height</b>: Bitmap height, in lines<br>"
						"<br>"
						"Specify the source bitmap memory format and layout for the current handle."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_BITMAP_SIZE:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>BITMAP_SIZE</b>(<i>filter</i>, <i>wrapx</i>, <i>wrapy</i>, <i>width</i>, <i>height</i>)<br>"
						"<b>filter</b>: Bitmap filtering mode, one of NEAREST or BILINEAR<br>"
						"<b>wrapx</b>: Bitmap x wrap mode, one of REPEAT or BORDER<br>"
						"<b>wrapy</b>: Bitmap y wrap mode, one of REPEAT or BORDER<br>"
						"<b>width</b>: Drawn bitmap width, in pixels<br>"
						"<b>height</b>: Drawn bitmap height, in pixels<br>"
						"<br>"
						"Specify the screen drawing of bitmaps for the current handle."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_ALPHA_FUNC:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>ALPHA_FUNC</b>(<i>func</i>, <i>ref</i>)<br>"
						"<b>func</b>: Specifies the test function, one of NEVER, LESS, LEQUAL, GREATER, GEQUAL, "
						"EQUAL, NOTEQUAL, or ALWAYS. The initial value is ALWAYS (7)<br>"
						"<b>ref</b>: Specifies the reference value for the alpha test. The initial value is 0<br>"
						"<br>"
						"Specify the alpha test function."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_STENCIL_FUNC:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>STENCIL_FUNC</b>(<i>func</i>, <i>ref</i>, <i>mask</i>)<br>"
						"<b>func</b>: Specifies the test function, one of NEVER, LESS, LEQUAL, GREATER, GEQUAL, "
						"EQUAL, NOTEQUAL, or ALWAYS. The initial value is ALWAYS. <br>"
						"<b>ref</b>: Specifies the reference value for the stencil test. The initial value is 0<br>"
						"<b>mask</b>: Specifies a mask that is ANDed with the reference value and the stored stencil "
						"value. The initial value is 255<br>"
						"<br>"
						"Set function and reference value for stencil testing."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_BLEND_FUNC:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>BLEND_FUNC</b>(<i>src</i>, <i>dst</i>)<br>"
						"<b>src</b>: Specifies how the source blending factor is computed. One of ZERO, ONE, SRC_ALPHA, DST_ALPHA, ONE_MINUS_SRC_ALPHA or ONE_MINUS_DST_ALPHA. The initial value is SRC_ALPHA (2).<br>"
						"<b>dst</b>: Specifies how the destination blending factor is computed, one of the same "
						"constants as src. The initial value is ONE_MINUS_SRC_ALPHA(4)<br>"
						"<br>"
						"Specify pixel arithmetic."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_STENCIL_OP:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>STENCIL_OP</b>(<i>sfail</i>, <i>spass</i>)<br>"
						"<b>sfail</b>: Specifies the action to take when the stencil test fails, one of KEEP, ZERO, "
						"REPLACE, INCR, DECR, and INVERT. The initial value is KEEP (1)<br>"
						"<b>spass</b>: Specifies the action to take when the stencil test passes, one of the same "
						"constants as sfail. The initial value is KEEP (1)<br>"
						"<br>"
						"Set stencil test actions."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_POINT_SIZE:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>POINT_SIZE</b>(<i>size</i>)<br>"
						"<b>size</b>: Point radius in 1/16 pixel. The initial value is 16.<br>"
						"<br>"
						"Sets the size of drawn points. The width is the distance from the center of the point "
						"to the outermost drawn pixel, in units of 1/16 pixels. The valid range is from 16 to "
						"8191 with respect to 1/16th pixel unit."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_LINE_WIDTH:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>LINE_WIDTH</b>(<i>width</i>)<br>"
						"<b>width</b>: Line width in 1/16 pixel. The initial value is 16.<br>"
						"<br>"
						"Sets the width of drawn lines. The width is the distance from the center of the line to "
						"the outermost drawn pixel, in units of 1/16 pixel. The valid range is from 16 to 4095 "
						"in terms of 1/16th pixel units."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_CLEAR_COLOR_A:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>CLEAR_COLOR_A</b>(<i>alpha</i>)<br>"
						"<b>alpha</b>: Alpha value used when the color buffer is cleared. The initial value is 0.<br>"
						"<br>"
						"Specify clear value for the alpha channel."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_COLOR_A:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>COLOR_A</b>(<i>alpha</i>)<br>"
						"<b>alpha</b>: Alpha for the current color. The initial value is 255<br>"
						"<br>"
						"Set the current color alpha."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_CLEAR_STENCIL:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>CLEAR_STENCIL</b>(<i>s</i>)<br>"
						"<b>s</b>: Value used when the stencil buffer is cleared. The initial value is 0<br>"
						"<br>"
						"Specify clear value for the stencil buffer."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_CLEAR_TAG:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>CLEAR_TAG</b>(<i>s</i>)<br>"
						"<b>s</b>: Value used when the tag buffer is cleared. The initial value is 0<br>"
						"<br>"
						"Specify clear value for the tag buffer."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_STENCIL_MASK:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>STENCIL_MASK</b>(<i>mask</i>)<br>"
						"<b>mask</b>: The mask used to enable writing stencil bits. The initial value is 255<br>"
						"<br>"
						"Control the writing of individual bits in the stencil planes."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_TAG_MASK:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>TAG_MASK</b>(<i>mask</i>)<br>"
						"<b>mask</b>: Allow updates to the tag buffer. The initial value is one and it means the tag "
						"buffer of the FT800 is updated with the value given by the TAG command. "
						"Therefore, the following graphics objects will be attached to the tag value "
						"given by the TAG command.<br>"
						"The value zero means the tag buffer of the FT800 is set as the default value,"
						"rather than the value given by TAG command in the display list.<br>"
						"<br>"
						"Control the writing of the tag buffer."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_BITMAP_TRANSFORM_A:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>BITMAP_TRANSFORM_A</b>(<i>a</i>)<br>"
						"<b>a</b>: Coefficient A of the bitmap transform matrix, in signed 8.8 bit fixed-point "
						"form. The initial value is 256<br>"
						"<br>"
						"Specify the A coefficient of the bitmap transform matrix."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_BITMAP_TRANSFORM_B:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>BITMAP_TRANSFORM_B</b>(<i>b</i>)<br>"
						"<b>b</b>: Coefficient B of the bitmap transform matrix, in signed 8.8 bit fixed-point "
						"form. The initial value is 0<br>"
						"<br>"
						"Specify the B coefficient of the bitmap transform matrix."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_BITMAP_TRANSFORM_C:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>BITMAP_TRANSFORM_C</b>(<i>c</i>)<br>"
						"<b>c</b>: Coefficient C of the bitmap transform matrix, in signed 15.8 bit fixed-point "
						"form. The initial value is 0<br>"
						"<br>"
						"Specify the C coefficient of the bitmap transform matrix."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_BITMAP_TRANSFORM_D:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>BITMAP_TRANSFORM_D</b>(<i>d</i>)<br>"
						"<b>d</b>: Coefficient D of the bitmap transform matrix, in signed 8.8 bit fixed-point "
						"form. The initial value is 0<br>"
						"<br>"
						"Specify the D coefficient of the bitmap transform matrix."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_BITMAP_TRANSFORM_E:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>BITMAP_TRANSFORM_E</b>(<i>e</i>)<br>"
						"<b>e</b>: Coefficient E of the bitmap transform matrix, in signed 8.8 bit fixed-point "
						"form. The initial value is 256<br>"
						"<br>"
						"Specify the E coefficient of the bitmap transform matrix."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_BITMAP_TRANSFORM_F:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>BITMAP_TRANSFORM_F</b>(<i>f</i>)<br>"
						"<b>f</b>: Coefficient F of the bitmap transform matrix, in signed 15.8 bit fixed-point "
						"form. The initial value is 0<br>"
						"<br>"
						"Specify the F coefficient of the bitmap transform matrix."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_SCISSOR_XY:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>SCISSOR_XY</b>(<i>x</i>, <i>y</i>)<br>"
						"<b>x</b>: The x coordinate of the scissor clip rectangle, in pixels. The initial value is 0<br>"
						"<b>y</b>: The y coordinate of the scissor clip rectangle, in pixels. The initial value is 0<br>"
						"<br>"
						"Sets the top-left position of the scissor clip rectangle, which limits the drawing area."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_SCISSOR_SIZE:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>SCISSOR_SIZE</b>(<i>width</i>, <i>height</i>)<br>"
						"<b>width</b>: The width of the scissor clip rectangle, in pixels. The initial value is 512. "
						"The valid value range is from 0 to 512.<br>"
						"<b>height</b>: The height of the scissor clip rectangle, in pixels. The initial value is 512. "
						"The valid value range is from 0 to 512.<br>"
						"<br>"
						"Sets the width and height of the scissor clip rectangle, which limits the drawing area."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_CALL:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>CALL</b>(<i>dest</i>)<br>"
						"<b>dest</b>: The destination address in RAM_DL which the display command is to be "
						"switched. FT800 has the stack to store the return address. To come back to "
						"the next command of source address, the RETURN command can help.<br>"
						"<br>"
						"Execute a sequence of commands at another location in the display list<br>"
						"<br>"
						"CALL and RETURN have a 4 level stack in addition to the current pointer. Any "
						"additional CALL/RETURN done will lead to unexpected behavior."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_JUMP:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>JUMP</b>(<i>dest</i>)<br>"
						"<b>dest</b>: Display list address to be jumped.<br>"
						"<br>"
						"Execute commands at another location in the display list."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_BEGIN:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>BEGIN</b>(<i>prim</i>)<br>"
						"<b>prim</b>: Graphics primitive. The valid value is defined as: BITMAPS, POINTS, LINES, LINE_STRIP, EDGE_STRIP_R, EDGE_STRIP_L, EDGE_STRIP_A, EDGE_STRIP_B, RECTS<br>"
						"<br>"
						"Begin drawing a graphics primitive."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_COLOR_MASK:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>COLOR_MASK</b>(<i>r</i>, <i>g</i>, <i>b</i>, <i>a</i>)<br>"
						"<b>r</b>: Enable or disable the red channel update of the FT800 color buffer. The initial value is 1 and means enable.<br>"
						"<b>g</b>: Enable or disable the green channel update of the FT800 color buffer. The initial value is 1 and means enable.<br>"
						"<b>b</b>: Enable or disable the blue channel update of the FT800 color buffer. The initial value is 1 and means enable.<br>"
						"<b>a</b>: Enable or disable the alpha channel update of the FT800 color buffer. The initial value is 1 and means enable.<br>"
						"<br>"
						"The color mask controls whether the color values of a pixel are updated. Sometimes "
						"it is used to selectively update only the red, green, blue or alpha channels of the "
						"image. More often, it is used to completely disable color updates while updating the "
						"tag and stencil buffers."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_END:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>END</b>()<br>"
						"<br>"
						"End drawing a graphics primitive.<br>"
						"<br>"
						"It is recommended to have an END for each BEGIN. Whereas advanced users can "
						"avoid the usage of END in order to save extra graphics instructions in the display list "
						"RAM."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_SAVE_CONTEXT:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>SAVE_CONTEXT</b>()<br>"
						"<br>"
						"Push the current graphics context on the context stack.<br>"
						"<br>"
						"Any extra SAVE_CONTEXT will throw away the earliest saved context."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_RESTORE_CONTEXT:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>RESTORE_CONTEXT</b>()<br>"
						"<br>"
						"Restore the current graphics context from the context stack.<br>"
						"<br>"
						"Any extra RESTORE_CONTEXT will load the default values into the present context."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_RETURN:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>RETURN</b>()<br>"
						"<br>"
						"Return from a previous CALL command.<br>"
						"<br>"
						"CALL and RETURN have 4 levels of stack in addition to the current pointer. Any additional CALL/RETURN done will lead to unexpected behavior."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_MACRO:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>MACRO</b>(<i>m</i>)<br>"
						"<b>m</b>: Macro register to read. Value 0 means the FT800 will fetch the command "
						"from REG_MACRO_0 to execute. Value 1 means the FT800 will fetch the "
						"command from REG_MACRO_1 to execute. The content of REG_MACRO_0 or "
						"REG_MACRO_1 shall be a valid display list command, otherwise the behavior "
						"is undefined.<br>"
						"<br>"
						"Execute a single command from a macro register."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
				case FT800EMU_DL_CLEAR:
				{
					m_PropertiesEditor->setInfo(tr(
						"<b>CLEAR</b>(<i>c</i>, <i>s</i>, <i>t</i>)<br>"
						"<b>c</b>: Clear color buffer. Setting this bit to 1 will clear the color buffer of the FT800 "
						"to the preset value. Setting this bit to 0 will maintain the color buffer of the "
						"FT800 with an unchanged value. The preset value is defined in command "
						"CLEAR_COLOR_RGB for RGB channel and CLEAR_COLOR_A for alpha channel.<br>"
						"<b>s</b>: Clear stencil buffer. Setting this bit to 1 will clear the stencil buffer of the "
						"FT800 to the preset value. "
						"Setting this bit to 0 will maintain the stencil "
						"buffer of the FT800 with an unchanged value. The preset value is defined in "
						"command CLEAR_STENCIL.<br>"
						"<b>t</b>: Clear tag buffer. Setting this bit to 1 will clear the tag buffer of the FT800 to "
						"the preset value. Setting this bit to 0 will maintain the tag buffer of the "
						"FT800 with an unchanged value. The preset value is defined in command "
						"CLEAR_TAG.<br>"
						"<br>"
						"Clear buffers to preset values."));
					m_PropertiesEditor->setEditWidget(NULL, false, this);
					ok = true;
					break;
				}
			}
			if (!ok)
			{
				m_PropertiesEditor->setInfo(tr("</i>Not yet implemented.</i>"));
				m_PropertiesEditor->setEditWidget(NULL, false, this);
			}
		}
		else
		{
			m_MainWindow->viewport()->unsetEditorLine();
			if (m_DisplayListParsed[m_PropLine].IdText.size() > 0)
			{
				QString message;
				message.sprintf(tr("Unknown command '<i>%s</i>'").toUtf8().constData(), m_DisplayListParsed[m_PropLine].IdText.c_str());
				m_PropertiesEditor->setInfo(message);
			}
			else
			{
				m_PropertiesEditor->setInfo(QString());
			}
			m_PropertiesEditor->setEditWidget(NULL, false, this);
		}
	}
}

void DlEditor::frame()
{
	// FIXME DL/CMD Mapping
	// update current step highlight
	int idx = FT800EMU::GraphicsProcessor.getDebugLimiterEffective() ? FT800EMU::GraphicsProcessor.getDebugLimiterIndex() : -1;
	if (idx > 0 && m_ModeCoprocessor)
	{
		if (idx < FT800EMU_DL_SIZE)
		{
			idx = m_MainWindow->getDlCmd()[idx];
		}
		else
		{
			idx = -1;
		}
	}
	m_CodeEditor->setStepHighlight(idx);
}

} /* namespace FT800EMUQT */

/* end of file */
