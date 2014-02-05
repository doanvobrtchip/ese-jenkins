/**
 * undo_stack_disabler.h
 * $Id$
 * \file undo_stack_disabler.h
 * \brief undo_stack_disabler.h
 * \date 2014-02-15 18:16GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#ifndef FT800EMUQT_UNDO_STACK_DISABLER_H
#define FT800EMUQT_UNDO_STACK_DISABLER_H

// STL includes

// Qt includes
#include <QWidget>
#include <QUndoStack>
#include <QKeyEvent>
#include <QMenu>
#include <QString>

// Emulator includes

// Project includes

namespace FT800EMUQT {

/**
 * Disables local undo stack and re-routes key press events for undo and redo to global undo stack.
 * \brief UndoStackDisabler
 * \date 2014-02-15 18:16GMT
 * \author Jan Boon (Kaetemi)
 */
template<class T>
class UndoStackDisabler : public T
{
public:
	UndoStackDisabler(QWidget *parent) : T(parent), m_UndoStack(NULL)
	{

	}

	virtual ~UndoStackDisabler()
	{

	}

protected:
	virtual void keyPressEvent(QKeyEvent *e)
	{
		if ((e->modifiers() == Qt::ControlModifier)
			&& (e->key() == Qt::Key_Z))
		{
			if (m_UndoStack)
				m_UndoStack->undo();
		}
		else if ((e->modifiers() == Qt::ControlModifier)
			&& (e->key() == Qt::Key_Y))
		{
			if (m_UndoStack)
				m_UndoStack->redo();
		}
		else
		{
			T::keyPressEvent(e);
		}
	}

	virtual void contextMenuEvent(QContextMenuEvent *event)
	{
		// TODO: Create new context menu with proper undo/redo
		QMenu* menu = new QMenu(this);
		if (m_UndoStack)
		{
			menu->addAction(m_UndoStack->createUndoAction(menu));
			menu->addAction(m_UndoStack->createRedoAction(menu));
			menu->addSeparator();
		}
		QAction *actCut = new QAction(QWidget::tr("Cut"), menu);
		QWidget::connect(actCut, SIGNAL(triggered()), this, SLOT(cut()));
		menu->addAction(actCut);
		QAction *actCopy = new QAction(QWidget::tr("Copy"), menu);
		QWidget::connect(actCopy, SIGNAL(triggered()), this, SLOT(copy()));
		menu->addAction(actCopy);
		QAction *actPaste = new QAction(QWidget::tr("Paste"), menu);
		QWidget::connect(actPaste, SIGNAL(triggered()), this, SLOT(paste()));
		menu->addAction(actPaste);
		menu->addSeparator();
		QAction *actSelectAll = new QAction(QWidget::tr("Select All"), menu);
		QWidget::connect(actSelectAll, SIGNAL(triggered()), this, SLOT(selectAll()));
		menu->addAction(actSelectAll);
		menu->exec(event->globalPos());
		delete menu;
	}

public:
	void setUndoStack(QUndoStack *undoStack)
	{
		m_UndoStack = undoStack;
	}

protected:
	QUndoStack *m_UndoStack;

private:
	UndoStackDisabler(const UndoStackDisabler &);
	UndoStackDisabler &operator=(const UndoStackDisabler &);

}; /* class UndoStackDisabler */

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_UNDO_STACK_DISABLER_H */

/* end of file */
