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

#ifndef FT800EMUQT_DL_EDITOR_H
#define FT800EMUQT_DL_EDITOR_H

// STL includes

// Qt includes
#include <QWidget>
#include <QMutex>

// Emulator includes
#define NOMINMAX
#include <ft800emu_inttypes.h>

// Project includes

class CodeEditor;
class QUndoStack;

#define FT800EMU_DL_SIZE 2048

namespace FT800EMUQT {
	class DlHighlighter;

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
	DlEditor(QWidget *parent);
	virtual ~DlEditor();
	
	void setUndoStack(QUndoStack *undo_stack);
	
	void clearUndoStack();
	void clear();
	
	void lockDisplayList(); // mutex
	void unlockDisplayList(); // mutex
	inline uint32_t *getDisplayList() { return m_DisplayListShared; }
	inline bool isDisplayListModified() { return m_DisplayListModified; }
	
	void reloadDisplayList(); // reloads the entire display list from m_DisplayListShared

// private slots:
//	void returnPressed();

private:
	CodeEditor *m_CodeEditor;
	DlHighlighter *m_DlHighlighter;
	uint32_t m_DisplayListShared[FT800EMU_DL_SIZE]; // display list that is to be used by the thread forwarding to the emulator
	bool m_DisplayListModified; // flagged whenever the emulator needs to refresh the display list from m_DisplayListShared
	QMutex m_Mutex;

private:
	DlEditor(const DlEditor &);
	DlEditor &operator=(const DlEditor &);
	
}; /* class DlEditor */

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_DL_EDITOR_H */

/* end of file */
