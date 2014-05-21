/**
 * inspector.h
 * $Id$
 * \file inspector.h
 * \brief inspector.h
 * \date 2014-01-29 16:53GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#ifndef FT800EMUQT_INSPECTOR_H
#define FT800EMUQT_INSPECTOR_H

// STL includes
#include <vector>

// Qt includes
#include <QWidget>

// Emulator includes
#include "ft800emu_inttypes.h"

// Project includes
#include "dl_editor.h"

class QTreeWidget;
class QTreeWidgetItem;

namespace FT800EMUQT {

class MainWindow;

/**
 * Inspector
 * \brief Inspector
 * \date 2014-01-29 16:53GMT
 * \author Jan Boon (Kaetemi)
 */
class Inspector : public QWidget
{
	Q_OBJECT

public:
	Inspector(MainWindow *parent);
	virtual ~Inspector();

	void frameEmu();
	void frameQt();

private:
	void initDisplayReg();
	void releaseDisplayReg();

	MainWindow *m_MainWindow;

	QTreeWidget *m_DisplayList;
	QTreeWidgetItem *m_DisplayListItems[FT800EMU_DL_SIZE];
	uint32_t m_DisplayListCopy[FT800EMU_DL_SIZE];
	bool m_DisplayListUpdate[FT800EMU_DL_SIZE];

	QTreeWidget *m_Registers;
	std::vector<QTreeWidgetItem *> m_RegisterItems;
	std::vector<uint32_t> m_RegisterCopy;

private:
	Inspector(const Inspector &);
	Inspector &operator=(const Inspector &);

}; /* class Inspector */

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_INSPECTOR_H */

/* end of file */
