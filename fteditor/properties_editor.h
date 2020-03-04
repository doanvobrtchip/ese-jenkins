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

#ifndef FTEDITOR_PROPERTIES_EDITOR_H
#define FTEDITOR_PROPERTIES_EDITOR_H

#pragma warning(disable : 26812)
#pragma warning(disable : 26495)
#pragma warning(disable : 26444)

// STL includes

// Qt includes
#include <QWidget>

// Emulator includes

// Project includes

class QLabel;
class QGroupBox;
class QVBoxLayout;

namespace FTEDITOR {
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

	void setInfo(QString message);
	void setEditWidget(QWidget *widget, bool own, QWidget *setter);
	void setEditWidgets(const std::vector<QWidget *> &widgets, bool own, QWidget *setter);
	QWidget *getEditWidgetSetter() const { return m_CurrentEditWidgetSetter; }

	void surpressSet(bool surpress) { m_SurpressSet = surpress; }

	void translate();

signals:
	void setterChanged(QWidget *setter);

private:
	QGroupBox *m_InfoGroupBox;
	QLabel *m_InfoLabel;
	QVBoxLayout *m_EditLayout;
	bool m_LayoutInserted;
	QVBoxLayout *m_GlobalLayout;
	std::vector<QWidget *> m_CurrentEditWidgets;
	bool m_OwnCurrentEditWidget;
	QWidget *m_CurrentEditWidgetSetter;
	bool m_SurpressSet;

private:
	PropertiesEditor(const PropertiesEditor &);
	PropertiesEditor &operator=(const PropertiesEditor &);

}; /* class PropertiesEditor */

} /* namespace FTEDITOR */

#endif /* #ifndef FTEDITOR_PROPERTIES_EDITOR_H */

/* end of file */
