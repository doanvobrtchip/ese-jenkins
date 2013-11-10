/**
 * properties_editor.h
 * $Id$
 * \file properties_editor.h
 * \brief properties_editor.h
 * \date 2013-11-10 09:43GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2013  Future Technology Devices International Ltd
 */

#ifndef FT800EMUQT_PROPERTIES_EDITOR_H
#define FT800EMUQT_PROPERTIES_EDITOR_H

// STL includes

// Qt includes
#include <QWidget>

// Emulator includes

// Project includes

namespace FT800EMUQT {
	class DlHighlighter;

/**
 * PropertiesEditor
 * \brief PropertiesEditor
 * \date 2013-11-10 09:43GMT
 * \author Jan Boon (Kaetemi)
 */
class PropertiesEditor : public QWidget
{
	Q_OBJECT
	
public:
	PropertiesEditor(QWidget *parent);
	virtual ~PropertiesEditor();

private:
	PropertiesEditor(const PropertiesEditor &);
	PropertiesEditor &operator=(const PropertiesEditor &);
	
}; /* class PropertiesEditor */

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_PROPERTIES_EDITOR_H */

/* end of file */
