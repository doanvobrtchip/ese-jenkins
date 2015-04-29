/**
 * dl_editor.h
 * $Id$
 * \file dl_editor.h
 * \brief dl_editor.h
 * \date 2013-11-05 09:02GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FTEDITOR_DL_EDITOR_H
#define FTEDITOR_DL_EDITOR_H

// STL includes

// Qt includes
#include <QWidget>
#include <QMutex>
#include <QStringList>

// Emulator includes
#include <ft8xxemu_inttypes.h>

// Project includes
#include "dl_parser.h"
#include "dl_state.h"

class CodeEditor;
class QUndoStack;
class QTextBlock;
class QCompleter;
class QStringListModel;

#define FTEDITOR_DL_SIZE 2048
#define FT800EMU_MACRO_SIZE 2

namespace FTEDITOR {

class MainWindow;
class DlHighlighter;
class PropertiesEditor;

/**
 * DlEditor
 * \brief DlEditor
 * \date 2013-11-05 09:02GMT
 * \author Jan Boon (Kaetemi)
 */
class DlEditor : public QWidget
{
	Q_OBJECT

public:
	DlEditor(MainWindow *parent, bool coprocessor = false);
	virtual ~DlEditor();

	void setUndoStack(QUndoStack *undo_stack);
	void setPropertiesEditor(PropertiesEditor *props) { m_PropertiesEditor = props; }
	void setModeMacro();

	void bindCurrentDevice();

	void clearUndoStack();
	void clear();

	void lockDisplayList(); // mutex
	void unlockDisplayList(); // mutex
	inline uint32_t *getDisplayList() { return m_DisplayListShared; }
	inline const DlParsed *getDisplayListParsed() { return m_DisplayListParsed; }
	inline bool isDisplayListModified() { bool result = m_DisplayListModified; m_DisplayListModified = false; return result; }

	void reloadDisplayList(bool fromEmulator); // reloads the entire display list from m_DisplayListShared, must be called inside mutex!!!

	// Replace a line (creates undo stack), used for example from the interactive viewport
	void replaceLine(int line, const DlParsed &parsed, int combineId = -1, const QString &message = QString());
	void removeLine(int line);
	const DlParsed &getLine(int line) const;
	// Move cursor to line
	void selectLine(int line);
	void insertLine(int line, const DlParsed &parsed);
	int getLineCount();

	bool isCoprocessor() const { return m_ModeCoprocessor; }
	bool isMacro() const { return m_ModeMacro; }

	CodeEditor *codeEditor() { return m_CodeEditor; }
	const DlState &state() { if (m_InvalidState) { processState(); } return m_State; }

private slots:
	void documentContentsChange(int position, int charsRemoved, int charsAdded);
	void documentBlockCountChanged(int newBlockCount);
	void editorCursorPositionChanged();

public slots:
	void frame();

private:
	void parseLine(QTextBlock block);
	void editingLine(QTextBlock block);

	void processState();

	MainWindow *m_MainWindow;
	CodeEditor *m_CodeEditor;
	DlHighlighter *m_DlHighlighter;
	uint32_t m_DisplayListShared[FTEDITOR_DL_SIZE]; // display list that is to be used by the thread forwarding to the emulator, todo: internal copy to compare when coprocessor changes stuff
	DlParsed m_DisplayListParsed[FTEDITOR_DL_SIZE]; // parsed version of the display list
	bool m_DisplayListModified; // flagged whenever the emulator needs to refresh the display list from m_DisplayListShared
	QMutex m_Mutex;
	bool m_Reloading;
	QCompleter *m_Completer;
	QStringListModel *m_CompleterModel;
	QStringList m_CompleterIdentifiers;
	QStringList m_CompleterParams;
	bool m_CompleterIdentifiersActive;

	DlState m_State;

	PropertiesEditor *m_PropertiesEditor;
	int m_PropLine;
	int m_PropIdLeft;
	int m_PropIdRight;
	bool m_PropIdValid;

	bool m_ModeMacro;
	bool m_ModeCoprocessor;

	bool m_EditingInteractive;

	bool m_InvalidState;

private:
	DlEditor(const DlEditor &);
	DlEditor &operator=(const DlEditor &);

}; /* class DlEditor */

} /* namespace FTEDITOR */

#endif /* #ifndef FTEDITOR_DL_EDITOR_H */

/* end of file */
