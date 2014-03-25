/**
 * interactive_properties.h
 * $Id$
 * \file interactive_properties.h
 * \brief interactive_properties.h
 * \date 2014-03-04 22:58GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#ifndef FT800EMUQT_INTERACTIVE_PROPERTIES_H
#define FT800EMUQT_INTERACTIVE_PROPERTIES_H

// STL includes
#include <vector>

// Qt includes
#include <QGroupBox>

// Emulator includes
#include <ft800emu_inttypes.h>

// Project includes

namespace FT800EMUQT {

class MainWindow;
class DlEditor;

/**
 * InteractiveProperties
 * \brief InteractiveProperties
 * \date 2014-03-04 22:58GMT
 * \author Jan Boon (Kaetemi)
 */
class InteractiveProperties : public QGroupBox
{
	Q_OBJECT

public:
	InteractiveProperties(MainWindow *parent);
	virtual ~InteractiveProperties();

	void clear();

	// Called by a editor when the active line changes
	void setEditorLine(DlEditor *editor, int line);
	void modifiedEditorLine();

private:
	void addLabeledWidget(const QString &label, QWidget *widget);
	void addLabeledWidget(const QString &label, QWidget *widget0, QWidget *widget1);

	void addXY(int x, int y, int minim, int maxim);
	void addWH(int w, int h, int minim, int maxim);
	void addXY16(int x, int y, int minim, int maxim);
	void addHandle(int handle, bool font = false);
	void addCell(int cell);
	void addOptions(int options, uint32_t flags, bool flatOnly = false);
	void addCharacter(int character);
	void addText(int text);
	void addValueSlider(int val, int maxim);
	void addValueSliderDyn(int val, int maxim);
	void addValueSliderDynSub(int val, int sub, int maxim);
	void addSizeSubDynSlider(int size, int clip, int maxim);
	void addRangeMaximum(int range, int maxim);
	void addWidth(int width, int minim, int maxim);
	void addRadius(int width, int minim, int maxim);
	void addSpinBox(int index, int minim, int maxim, const QString &label, const QString &undoMessage);
	void addSpinBox256(int index, int minim, int maxim, const QString &label, const QString &undoMessage);
	void addSpinBox65536(int index, int minim, int maxim, const QString &label, const QString &undoMessage);
	void addSpinBoxAngle65536(int index, int minim, int maxim, const QString &label, const QString &undoMessage);
	void addColor(int r, int g, int b);
	void addCheckBox(int index, const QString &label, const QString &undoMessage);
	void addWidth16(int width, int minim, int maxim, bool size);
	void addAlpha(int alpha);
	void addByteFlag(int flag, const QString &undoMessage);
	// void addAddress(int address);
	void addComboBox(int index, const char **items, int nb, const QString &label, const QString &undoMessage);
	void addComboBox(int index, const char **items, int begin, int end, const QString &label, const QString &undoMessage);
	void addBlendFunction(int blend, const QString &label, const QString &undoMessage);
	void addCompareFunction(int compare);
	void addStencilOperation(int operation, const QString &label, const QString &undoMessage);
	void addPrimitive(int primitive);
	void addBitmapFormat(int format);
	void addBitmapWrap(int wrap, const QString &label, const QString &undoMessage);
	void addBitmapFilter(int filter);
	void addAddress(int address);

private slots:
	void propertiesSetterChanged(QWidget *setter);

private:
	class PropertiesWidget;
	class PropertiesSpinBoxAddress;
	class PropertiesSpinBox;
	class PropertiesSpinBox16;
	class PropertiesSpinBox256;
	class PropertiesSpinBox65536;
	class PropertiesSpinBoxAngle65536;
	class PropertiesCheckBox;
	class PropertiesLineEdit;
	class PropertiesLineEditChar;
	class PropertiesSlider;
	class PropertiesSliderDyn;
	class PropertiesSliderDynSub;
	class PropertiesSliderDynSubClip;
	class PropertiesColor;
	class PropertiesComboBox;

	MainWindow *m_MainWindow;

	// Current line
	DlEditor *m_LineEditor;
	int m_LineNumber;

	std::vector<QLayout *> m_CurrentLayouts;
	std::vector<QWidget *> m_CurrentWidgets;
	std::vector<PropertiesWidget *> m_CurrentProperties;

private:
	InteractiveProperties(const InteractiveProperties &);
	InteractiveProperties &operator=(const InteractiveProperties &);

}; /* class InteractiveProperties */

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_INTERACTIVE_PROPERTIES_H */

/* end of file */
