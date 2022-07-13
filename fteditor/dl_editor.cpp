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

#pragma warning(disable : 26812) // Unscoped enum
#pragma warning(disable : 26495) // Uninitialized member
#pragma warning(disable : 26444) // Unnamed objects

#define NOMINMAX
#include "dl_editor.h"

// STL includes
#include <algorithm>

// Qt includes
#include <QApplication>
#include <QVBoxLayout>
#include <QTextBlock>
#include <QCompleter>
#include <QStringListModel>
#include <QAbstractItemView>
#include <QTextStream>
#include <QtEndian>
#include <QFileDialog>
#include <QStandardPaths>
#include <QRegularExpression>

// Emulator includes
#include <bt8xxemu_diag.h>

// Project includes
#include "main_window.h"
#include "interactive_viewport.h"
#include "code_editor.h"
#include "dl_highlighter.h"
#include "properties_editor.h"
#include "toolbox.h"
#include "interactive_properties.h"
#include "constant_common.h"

namespace FTEDITOR
{

extern BT8XXEMU_Emulator *g_Emulator;
extern BT8XXEMU_Flash *g_Flash;

extern int g_StepCmdLimit;

DlEditor::DlEditor(MainWindow *parent, bool coprocessor)
    : QWidget(parent)
    , m_MainWindow(parent)
    , m_Reloading(false)
    , m_CompleterIdentifiersActive(true)
    , m_PropertiesEditor(NULL)
    , m_PropLine(-1)
    , m_PropIdLeft(-1)
    , m_PropIdRight(-1)
    , m_ModeMacro(false)
    , m_ModeCoprocessor(coprocessor)
    , m_EditingInteractive(false)
    , m_CompleterModel(NULL)
{
	m_DisplayListShared[0] = DISPLAY();

	m_CodeEditor = new CodeEditor();
	m_CodeEditor->setMaxLinesNotice(FTEDITOR_DL_SIZE);
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

	bindCurrentDevice();

	m_CompleterModel = new QStringListModel(m_CodeEditor);
	m_CompleterModel->setStringList(m_CompleterIdentifiers);

	m_Completer = new QCompleter(m_CompleterModel, m_CodeEditor);
	m_CodeEditor->setCompleter(m_Completer);

	for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
	{
		m_DisplayListParsed[i] = DlParsed();
		m_DisplayListParsed[i].ValidId = false;
		m_DisplayListShared[i] = DISPLAY();
	}

	if (m_ModeCoprocessor)
	{
	    m_CoproCmdArgName
	    = {
		      { "CMD_DLSTART", {} },
		      { "CMD_INTERRUPT", { "milliseconds: %1" } },
		      { "CMD_COLDSTART", {} },
		      { "CMD_APPEND", { "<u>pointer: %1", "number of bytes: %1" } },
		      { "CMD_REGREAD", { "<u>pointer: %1", "result: %1" } },
		      { "CMD_MEMWRITE", { "<u>pointer: %1", "number of bytes: %1" } },
		      { "CMD_INFLATE", { "<u>pointer: %1" } },
		      { "CMD_INFLATE2", { "<u>pointer: %1", "options: %1" } },
		      { "CMD_LOADIMAGE", { "<u>pointer: %1", "options: %1" } },
		      { "CMD_MEDIAFIFO", { "<u>pointer: %1", "size: %1" } },
		      { "CMD_PLAYVIDEO", { "options: %1" } },
		      { "CMD_VIDEOSTART", {} },
		      { "CMD_VIDEOFRAME", { "<u>destination: %1", "<u>pointer: %1" } },
		      { "CMD_MEMCRC", { "<u>pointer: %1", "number of bytes: %1", "result: %1" } },
		      { "CMD_MEMZERO", { "<u>pointer: %1", "number of bytes: %1" } },
		      { "CMD_MEMSET", { "<u>pointer: %1", "value: %1", "number of bytes: %1" } },
		      { "CMD_MEMCPY", { "<u>destination: %1", "<u>source: %1", "number of bytes: %1" } },
		      { "CMD_BUTTON", { "<i>", "x: %1, y: %2", "width: %1, height: %2", "font: %1, options: %2", "string: %1" } },
		      { "CMD_CLOCK", { "<i>", "x: %1, y: %2", "radius: %1, options: %2", "hour: %1, minutes: %2", "second: %1, milliseconds: %2" } },
		      { "CMD_FGCOLOR", { "<u>color: %1" } },
		      { "CMD_BGCOLOR", { "<u>color: %1" } },
		      { "CMD_GRADCOLOR", { "<u>color: %1" } },
		      { "CMD_GAUGE", { "<i>", "x: %1, y: %2", "radius: %1, options: %2", "major: %1, minor: %2", "value: %1, range: %2" } },
		      { "CMD_GRADIENT", { "<i>", "x0: %1, y0: %2", "<u>rgb0: %1", "x1: %1, y1: %2", "<u>rgb1: %1" } },
		      { "CMD_GRADIENTA", { "<i>", "x0: %1, y0: %2", "<u>argb0: %1", "x1: %1, y1: %2", "<u>argb1: %1" } },
		      { "CMD_KEYS", { "<i>", "x: %1, y: %2", "width: %1, height: %2", "font: %1, options: %2", "string: %1" } },
		      { "CMD_PROGRESS", { "<i>", "x: %1, y: %2", "width: %1, height: %2", "options: %1, value: %2", "range: %1" } },
		      { "CMD_SCROLLBAR", { "<i>", "x: %1, y: %2", "width: %1, height: %2", "options: %1, value: %2", "size: %1, range: %2" } },
		      { "CMD_SLIDER", { "<i>", "x: %1, y: %2", "width: %1, height: %2", "options: %1, value: %2", "range: %1" } },
		      { "CMD_DIAL", { "<i>", "x: %1, y: %2", "radius: %1, options: %2", "value: %1" } },
		      { "CMD_TOGGLE", { "<i>", "x: %1, y: %2", "width: %1, font: %2", "options: %1, state: %2", "string: %1" } },
		      { "CMD_FILLWIDTH", { "fill width: %1" } },
		      { "CMD_TEXT", { "<i>", "x: %1, y: %2", "font: %1, options: %2", "string: %1" } },
		      { "CMD_SETBASE", { "numeric base: %1" } },
		      { "CMD_NUMBER", { "<i>", "x: %1, y: %2", "font: %1, options: %2", "number: %1" } },
		      { "CMD_LOADIDENTIY", {} },
		      { "CMD_SETMATRIX", {} },
		      { "CMD_GETMATRIX", { "<i>", "a: %1", "b: %1", "c: %1", "d: %1", "e: %1", "f: %1" } },
		      { "CMD_GETPTR", { "<u>result: %1" } },
		      { "CMD_GETPROPS", { "<u>pointer: %1", "width: %1", "height: %1" } },
		      { "CMD_SCALE", { "<i>", "x scale factor: %1", "y scale factor: %1" } },
		      { "CMD_ROTATE", { "<i>", "clockwise rotation angle: %1" } },
		      { "CMD_ROTATEAROUND", { "<i>", "center of rotation/scaling, x-coordinate: %1", "center of rotation/scaling, y-coordinate: %1", "clockwise rotation angle: %1", "scale factor: %1" } },
		      { "CMD_TRANSLATE", { "<i>", "x translate factor: %1", "y translate factor: %1" } },
		      { "CMD_CALIBRATE", { "result: %1" } },
		      { "CMD_SETROTATE", { "rotation: %1" } },
		      { "CMD_SPINNER", { "<i>", "x: %1, y: %2", "style: %1, scale: %2" } },
		      { "CMD_SCREENSAVER", {} },
		      { "CMD_SKETCH", { "<i>", "x: %1, y: %2", "width: %1, height: %2", "<u>pointer: %1", "format: %1" } },
		      { "CMD_STOP", {} },
		      { "CMD_SETFONT", { "font: %1", "<u>pointer: %1" } },
		      { "CMD_SETFONT2", { "font: %1", "<u>pointer: %1", "first character: %1" } },
		      { "CMD_SETSCRATCH", { "bitmap handle: %1" } },
		      { "CMD_ROMFONT", { "font: %1", "ROM slot: %1" } },
		      { "CMD_RESETFONTS", {} },
		      { "CMD_TRACK", { "<i>", "x: %1, y: %2", "width: %1, height: %2", "tag: %1" } },
		      { "CMD_SNAPSHOT", { "<u>pointer: %1" } },
		      { "CMD_SNAPSHOT2", { "<i>", "<u>format: %1", "<u>pointer: %1", "x: %1, y: %2", "width: %1, height: %2" } },
		      { "CMD_SETBITMAP", { "<u>source: %1", "format: %1, width: %2", "height: %1" } },
		      { "CMD_LOGO", {} },
		      { "CMD_FLASHERASE", {} },
		      { "CMD_FLASHWRITE", { "<u>pointer: %1", "number of bytes: %1" } },
		      { "CMD_FLASHREAD", { "<u>destination: %1", "<u>source: %1", "number of bytes: %1" } },
		      { "CMD_APPENDF", { "<u>pointer: %1", "number of bytes: %1" } },
		      { "CMD_FLASHUPDATE", { "<u>destination: %1", "<u>source: %1", "number of bytes: %1" } },
		      { "CMD_FLASHDETACH", {} },
		      { "CMD_FLASHATTACH", {} },
		      { "CMD_FLASHFAST", { "result: %1" } },
		      { "CMD_FLASHSPIDESEL", {} },
		      { "CMD_FLASHSPITX", { "number of bytes: %1" } },
		      { "CMD_FLASHSPIRX", { "<u>pointer: %1", "number of bytes: %1" } },
		      { "CMD_CLEARCACHE", {} },
		      { "CMD_FLASHSOURCE", { "<u>pointer: %1" } },
		      { "CMD_VIDEOSTARTF", {} },
		      { "CMD_ANIMSTART", { "<i>", "animation channel: %1", "<u>animation object pointer: %1", "<u>loop flag: %1" } },			  
			  { "CMD_ANIMSTARTRAM", { "<i>", "animation channel: %1", "<u>animation object pointer: %1", "<u>loop flag: %1" } },
			  { "CMD_RUNANIM", { "<u>wait mask: %1", "<i>play: %1" } },
		      { "CMD_ANIMSTOP", { "<i>", "animation channel: %1" } },
		      { "CMD_ANIMXY", { "<i>", "animation channel: %1", "x: %1, y: %2" } },
		      { "CMD_ANIMDRAW", { "<i>", "animation channel: %1" } },
		      { "CMD_ANIMFRAME", { "<i>", "x: %1, y: %2", "<u>animation object pointer: %1", "<u>frame: %1" } },
			  { "CMD_ANIMFRAMERAM", { "<i>", "x: %1, y: %2", "<u>animation object pointer: %1", "<u>frame: %1" } },
		      { "CMD_SYNC", {} },
		      { "CMD_BITMAP_TRANSFORM", { "<i>", "x0: %1", "y0: %1", "x1: %1", "y1: %1", "x2: %1", "y2: %1", "tx0: %1", "ty0: %1", "tx1: %1", "ty1: %1", "tx2: %1", "ty2: %1", "result: %1" } },
			  
			  { "CMD_WAIT", { "<u>us: %1" } },
			  { "CMD_NEWLIST", { "<u>memory address of command list: %1" } },
			  { "CMD_ENDLIST", {} },
			  { "CMD_CALLLIST", { "<u>memory address of command list: %1"} },
			  { "CMD_RETURN", {} },
			  { "CMD_APILEVEL", { "<u>level: %1" } },
			  { "CMD_CALIBRATESUB", { "<u>x: %1", "<u>y: %1", "<u>w: %1", "<u>h: %1", "<u>result: %1" } },
			  { "CMD_TESTCARD", {} },
			  { "CMD_FONTCACHE", { "<u>font handle: %1", "<i>start of cache area: %1", "<u>size of cache area: %1" } },
			  { "CMD_FONTCACHEQUERY", { "<u>total: %1", "<i>used: %1" } },
			  { "CMD_GETIMAGE", { "<u>bitmap address: %1", "<u>bitmap format: %1", "<u>width: %1", "<u>height: %1", "<u>palette: %1" } },
	      };
	}
}

DlEditor::~DlEditor()
{
}

void DlEditor::bindCurrentDevice()
{
	m_CompleterIdentifiers.clear();
	m_CompleterParams.clear();

	DlParser::getIdentifiers(FTEDITOR_CURRENT_DEVICE, m_CompleterIdentifiers, m_ModeCoprocessor);
	DlParser::getParams(FTEDITOR_CURRENT_DEVICE, m_CompleterParams, m_ModeCoprocessor);

	if (m_CompleterModel)
	{
		if (m_CompleterIdentifiersActive)
			m_CompleterModel->setStringList(m_CompleterIdentifiers);
		else
			m_CompleterModel->setStringList(m_CompleterParams);
		m_Completer->popup()->hide();
	}

	lockDisplayList();
	for (int i = 0; i < m_CodeEditor->document()->blockCount(); ++i)
	{
		parseLine(m_CodeEditor->document()->findBlockByNumber(i));
	}
	m_DisplayListModified = true;
	unlockDisplayList();

	m_DlHighlighter->rehighlight();
	m_InvalidState = true;
}

void DlEditor::setUndoStack(QUndoStack *undo_stack)
{
	m_CodeEditor->setUndoStack(undo_stack);
}

void DlEditor::clearUndoStack()
{
	m_CodeEditor->document()->clearUndoRedoStacks();
}

bool DlEditor::eventFilter(QObject *watched, QEvent *event)
{
	if (watched == m_CodeEditor && m_CodeEditor->blockCount() > 1)
	{
		if (event->type() == QKeyEvent::KeyPress)
		{
			QKeyEvent *ke = static_cast<QKeyEvent *>(event);
			if (ke->key() == Qt::Key_Return || ke->key() == Qt::Key_Enter)
			{
				return true; // do not process this event further
			}
		}
		return false; // process this event further
	}
	else
	{
		// pass the event on to the parent class
		return QWidget::eventFilter(watched, event);
	}
}

int DlEditor::getVertextFormat(int line)
{	
	if (line > getLineCount())
		line = getLineCount();
	DlState state = getState(line);
	return state.Graphics.VertexFormat;
}

void DlEditor::setModeMacro()
{
	m_ModeMacro = true;
	m_CodeEditor->setMaxLinesNotice(FT800EMU_MACRO_SIZE);
	m_CodeEditor->installEventFilter(this);
}

void DlEditor::clear()
{
	m_CodeEditor->clear();
	lockDisplayList();
	for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
	{
		m_DisplayListShared[i] = DISPLAY();
	}
	m_DisplayListModified = true;
	m_CodeEditor->textCursor().setPosition(0);
	unlockDisplayList();
	m_InvalidState = true;
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
	m_CodeEditor->setInteractiveDelete(false);

	bool firstLine = true;
	m_Reloading = true;
	if (!fromEmulator)
	{
		m_DisplayListModified = true;
	}
	int dcount = 0;
	for (int i = 0; i < (m_ModeMacro ? FT800EMU_MACRO_SIZE : FTEDITOR_DL_SIZE); ++i)
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
					if (firstLine)
						firstLine = false;
					else
						m_CodeEditor->textCursor().insertText("\n");
				}
				dcount = 0;
			}
			if (m_DisplayListShared[i] == DISPLAY())
			{
				++dcount;
			}
			QString line = DlParser::toString(FTEDITOR_CURRENT_DEVICE, m_DisplayListShared[i]);
			m_DisplayListParsed[i] = DlParsed();
			// verify parsing ->
			DlParser::parse(FTEDITOR_CURRENT_DEVICE, m_DisplayListParsed[i], line, m_ModeCoprocessor);
			uint32_t compiled = DlParser::compile(FTEDITOR_CURRENT_DEVICE, m_DisplayListParsed[i]);
			if (compiled != m_DisplayListShared[i])
			{
				QByteArray chars = line.toLatin1();
				const char *src = chars.constData();
#if _DEBUG
				printf("Parser bug at dl %i: '%s' -> expect %u, compiled %u\n", i, src, m_DisplayListShared[i], compiled);
#endif
			}
			// <- verify parsing
			if (firstLine)
				firstLine = false;
			else
				m_CodeEditor->textCursor().insertText("\n");
			m_CodeEditor->textCursor().insertText(line);
		}
	}
	m_Reloading = false;
}

void DlEditor::editorCursorPositionChanged()
{
	m_CodeEditor->setInteractiveDelete(m_EditingInteractive);

	if (QApplication::instance()->closingDown())
		return;

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
	m_CodeEditor->setInteractiveDelete(m_EditingInteractive);

	//printf("contents change %i %i\n", charsRemoved, charsAdded);

	if (QApplication::instance()->closingDown())
		return;

	if (m_Reloading)
		return;

	int charsEdited = std::max(charsRemoved, charsAdded);
	QTextBlock firstBlock = m_CodeEditor->document()->findBlock(position);
	int blockNb = firstBlock.blockNumber();
	int firstPosition = firstBlock.position();
	charsEdited += (position - firstPosition);

	//int count = 0;

	lockDisplayList();
	while (charsEdited > 0)
	{
		if (blockNb < m_CodeEditor->document()->blockCount())
		{
			QTextBlock block = m_CodeEditor->document()->findBlockByNumber(blockNb);
			parseLine(block);
			charsEdited -= block.length();
			++blockNb;
			//++count;
		}
		else
		{
			break;
		}
	}
	m_DisplayListModified = true;
	unlockDisplayList();
}

void DlEditor::saveCoprocessorCmd(bool isBigEndian)
{
	static QString dirPath = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
	QString fileName = QFileDialog::getSaveFileName(m_MainWindow, tr("Save Coprocessor Command"), dirPath, tr("Coprocessor Command Files(*.txt)"));

	QDir saveDir(fileName);
	saveDir.cdUp();
	dirPath = saveDir.absolutePath();

	int blockCount = 0;
	uint32_t val = 0;
	QString line("");
	QTextBlock block;

	QFile f(fileName);

	if (!f.open(QIODevice::WriteOnly | QIODevice::Text))
	{
		return;
	}

	QTextStream ts(&f);
	ts.setAutoDetectUnicode(true);
#if QT_VERSION_MAJOR < 6
	ts.setCodec("utf-8");
#else
    ts.setEncoding(QStringConverter::Utf8);
#endif
	std::vector<uint32_t> comp;

	lockDisplayList();
	for (int i = 0; i < m_CodeEditor->document()->blockCount(); i++)
	{
		DlParsed dl;

		block = m_CodeEditor->document()->findBlockByNumber(i);
		line = block.text();
		DlParser::parse(FTEDITOR_CURRENT_DEVICE, dl, line, true);

		if (!dl.ValidId)
			continue;

		if (dl.IdLeft != FTEDITOR_CO_COMMAND)
		{
			val = DlParser::compile(FTEDITOR_CURRENT_DEVICE, dl);
			val = isBigEndian ? qToBigEndian(val) : val;
			ts << QString("0x%1   // %2\n").arg(val, 8, 16, QChar('0')).arg(line);
		}
		else
		{
			comp.clear();
			DlParser::compile(FTEDITOR_CURRENT_DEVICE, comp, dl);
			val = (0xFFFFFF00 | dl.IdRight);
			val = isBigEndian ? qToBigEndian(val) : val;
			ts << QString("0x%1   // %2\n").arg(val, 8, 16, QChar('0')).arg(line);

			QStringList argNames = m_CoproCmdArgName.value(QString(dl.IdText.c_str()));
			bool isSigned = (argNames.size() > 0 && argNames[0] == "<i>");
			int j = 0;
			int x = 0, y = 0;
			QString argItem("");

			if (isSigned)
				argNames.removeFirst();

			for (int i = 0; i < comp.size(); i++)
			{
				if (i == comp.size() - 1 && argNames.indexOf(QRegularExpression("^string.*")) != -1)
				{
					argNames.append("end string");
				}

				if (argNames.size() < i + 1)
				{
					if (argNames.indexOf(QRegularExpression("^string.*")) != -1)
					{
						argNames.append("(string continue)");
					}
					else
					{
						argNames.append("");
					}
				}

				val = isBigEndian ? qToBigEndian(comp.at(i)) : comp.at(i);
				ts << QString("0x%1   //    ").arg(val, 8, 16, QChar('0'));

				argItem = argNames.at(i);

				if (argItem.startsWith("string"))
				{
					QString s = QString("\"%1\"").arg(dl.StringParameter.c_str());
					s.replace("\n", "\\n");
					ts << argItem.arg(s);
				}
				else if (argItem.startsWith("end string"))
				{
					ts << argItem;
				}
				else
				{
					if (argItem.startsWith("<u>"))
					{
						argItem.remove(0, 3);
						QString h = QString("0x%1").arg(dl.Parameter[j++].U, 8, 16, QChar('0'));
						ts << argItem.arg(h);
					}
					else if (argItem.indexOf(",") != -1)
					{
						x = isSigned ? dl.Parameter[j++].I : dl.Parameter[j++].U;
						y = isSigned ? dl.Parameter[j++].I : dl.Parameter[j++].U;
						ts << argItem.arg(x).arg(y);
					}
					else
					{
						x = isSigned ? dl.Parameter[j++].I : dl.Parameter[j++].U;
						ts << argItem.arg(x);
					}
				}
				
				ts << '\n';
			}
		}
	}
	unlockDisplayList();

	ts.flush();
	f.close();
}

bool DlEditor::isInvalid(void)
{
	for (int i = 0; i < getLineCount(); i++)
	{
		if (getLine(i).ValidId)
		{
			return false;
		}
	}
	return true;
}

void DlEditor::documentBlockCountChanged(int newBlockCount)
{
	m_CodeEditor->setInteractiveDelete(m_EditingInteractive);

	//printf("blockcount change\n");

	if (QApplication::instance()->closingDown())
		return;

	if (m_Reloading)
		return;

	lockDisplayList();
	for (int i = 0; i < newBlockCount && i < (m_ModeMacro ? FT800EMU_MACRO_SIZE : FTEDITOR_DL_SIZE); ++i)
	{
		parseLine(m_CodeEditor->document()->findBlockByNumber(i));
	}
	for (int i = newBlockCount; i < (m_ModeMacro ? FT800EMU_MACRO_SIZE : FTEDITOR_DL_SIZE); ++i)
	{
		m_DisplayListParsed[i] = DlParsed();
		m_DisplayListParsed[i].ValidId = false;
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

	if (i >= FTEDITOR_DL_SIZE)
		return;

	if (!m_InvalidState)
		m_InvalidState = DlState::requiresProcessing(m_DisplayListParsed[i]);

	m_DisplayListParsed[i] = DlParsed();
	DlParser::parse(FTEDITOR_CURRENT_DEVICE, m_DisplayListParsed[i], line, m_ModeCoprocessor);
	m_DisplayListShared[i] = DlParser::compile(FTEDITOR_CURRENT_DEVICE, m_DisplayListParsed[i]);

	// check for misformed lines and do a no-op (todo: mark them)
	if (m_DisplayListShared[i] == DISPLAY() && !m_DisplayListParsed[i].ValidId)
	{
		m_DisplayListShared[i] = JUMP(i + 1);
	}

	if (!m_InvalidState)
		m_InvalidState = DlState::requiresProcessing(m_DisplayListParsed[i]);
}

void DlEditor::replaceLine(int line, const DlParsed &parsed, int combineId, const QString &message)
{
	m_EditingInteractive = true;
	if (combineId >= 0)
		m_CodeEditor->setUndoCombine(combineId, message);
	QString linestr = DlParser::toString(FTEDITOR_CURRENT_DEVICE, parsed);
	QTextCursor c = m_CodeEditor->textCursor();
	c.setPosition(m_CodeEditor->document()->findBlockByNumber(line).position());
	//m_CodeEditor->setTextCursor(c);
	//c.select(QTextCursor::BlockUnderCursor);
	c.movePosition(QTextCursor::EndOfBlock, QTextCursor::KeepAnchor);
	c.insertText(linestr);
	// editorCursorPositionChanged() needed? // VERIFY
	if (combineId >= 0)
		m_CodeEditor->endUndoCombine();
	m_EditingInteractive = false;
}

void DlEditor::removeLine(int line)
{
	m_EditingInteractive = true;
	QTextCursor c = m_CodeEditor->textCursor();
	c.setPosition(m_CodeEditor->document()->findBlockByNumber(line).position());
	c.movePosition(QTextCursor::NextBlock, QTextCursor::KeepAnchor);
	c.insertText("");
	m_EditingInteractive = false;
}

void DlEditor::removeAll()
{
	m_EditingInteractive = true;
	QTextCursor c = m_CodeEditor->textCursor();
	c.setPosition(0);
	c.movePosition(QTextCursor::End, QTextCursor::KeepAnchor);
	c.insertText("");
	m_EditingInteractive = false;
}

int DlEditor::getLineCount()
{
	return m_CodeEditor->document()->blockCount();
}

void DlEditor::insertLine(int line, const DlParsed &parsed)
{
	m_EditingInteractive = true;
	QString linestr = DlParser::toString(FTEDITOR_CURRENT_DEVICE, parsed);
	QTextCursor c = m_CodeEditor->textCursor();
	if (line == 0)
	{
		c.setPosition(0);
		c.insertText(linestr + "\n");
	}
	else
	{
		while (line > m_CodeEditor->document()->lineCount()) {
			c.insertText("\n");
		}
		int pos = m_CodeEditor->document()->findBlockByNumber(line - 1).position();
		c.setPosition(pos);
		c.movePosition(QTextCursor::EndOfBlock, QTextCursor::MoveAnchor);
		c.insertText("\n" + linestr);
	}
	m_EditingInteractive = false;
}

const DlParsed &DlEditor::getLine(int line) const
{
	return m_DisplayListParsed[line];
}

QString DlEditor::getLineText(int line) const
{
	return m_CodeEditor->document()->findBlockByNumber(line).text();
}

void DlEditor::selectLine(int line)
{
	m_EditingInteractive = true;
	QTextCursor c = m_CodeEditor->textCursor();
	c.setPosition(m_CodeEditor->document()->findBlockByNumber(line).position());
	m_CodeEditor->setTextCursor(c);
	editingLine(m_CodeEditor->document()->findBlockByNumber(line));
	// editorCursorPositionChanged() instead of editingLine? // VERIFY
	m_EditingInteractive = false;
}

void DlEditor::editingLine(QTextBlock block)
{
	// update properties editor
	m_CodeEditor->setInteractiveDelete(m_EditingInteractive);
	if (m_PropertiesEditor->getEditWidgetSetter() != this
	    || block.blockNumber() != m_PropLine
	    || m_DisplayListParsed[block.blockNumber()].IdLeft != m_PropIdLeft
	    || m_DisplayListParsed[block.blockNumber()].IdRight != m_PropIdRight
	    || m_DisplayListParsed[block.blockNumber()].ValidId != m_PropIdValid
	    || !m_DisplayListParsed[block.blockNumber()].ValidId) // Necessary for the "Unknown command" info message
	{
		m_PropLine = block.blockNumber();
		if (m_PropLine < 0) // ?
		{
			m_PropertiesEditor->setInfo(QString());
			m_PropertiesEditor->setEditWidget(NULL, false, this);
			return;
		}
		m_PropIdLeft = m_DisplayListParsed[m_PropLine].IdLeft;
		m_PropIdRight = m_DisplayListParsed[m_PropLine].IdRight;
		m_PropIdValid = m_DisplayListParsed[m_PropLine].ValidId;
		m_MainWindow->toolbox()->setEditorLine(this, m_PropLine);
		m_MainWindow->viewport()->setEditorLine(this, m_PropLine);
		m_MainWindow->interactiveProperties()->setEditorLine(this, m_PropLine);
	}
	else
	{
		m_MainWindow->interactiveProperties()->modifiedEditorLine();
	}

	m_MainWindow->focusProperties();
}

void DlEditor::processState()
{
	if (!m_ModeMacro)
	{
		printf("DEBUG: Process state\n");
		DlState::process(FTEDITOR_CURRENT_DEVICE, m_State, m_PropLine, m_DisplayListParsed, m_ModeCoprocessor);
	}
	m_InvalidState = false;
}

void DlEditor::frame()
{
	// FIXME DL/CMD Mapping
	// update current step highlight
	int idx = BT8XXEMU_getDebugLimiterEffective(g_Emulator) ? BT8XXEMU_getDebugLimiterIndex(g_Emulator) : -1;
	if (idx > 0 && m_ModeCoprocessor)
	{
		if (idx < FTEDITOR_DL_SIZE)
		{
			idx = m_MainWindow->getDlCmd()[idx];
		}
		else
		{
			idx = -1;
		}
	}
	if (idx < 0 && m_ModeCoprocessor)
	{
		idx = (g_StepCmdLimit - 1);
	}
	m_CodeEditor->setStepHighlight(idx);
}

} /* namespace FTEDITOR */

/* end of file */
