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

#ifndef FTEDITOR_INTERACTIVE_PROPERTIES_H
#define FTEDITOR_INTERACTIVE_PROPERTIES_H

#pragma warning(disable : 26812)
#pragma warning(disable : 26495)
#pragma warning(disable : 26444)

// STL includes
#include <vector>

// Qt includes
#include <QGroupBox>

// Emulator includes
#include <bt8xxemu_inttypes.h>

// Project includes

class QComboBox;

namespace FTEDITOR {

class MainWindow;
class DlEditor;
struct DlParsed;

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

	void bindCurrentDevice();

	// Called by a editor when the active line changes
	void setEditorLine(DlEditor *editor, int line);
	void modifiedEditorLine();

	void setProperties(int idLeft, int idRight, DlEditor *editor);

private:
	void addLabeledWidget(const QString &label, QWidget *widget);
	void addLabeledWidget(const QString &label, QWidget *widget0, QWidget *widget1);

	void addXY(int x, int y, int minim, int maxim, QString label = QString("XY: "));
	void addWH(int w, int h, int minim, int maxim);
	void addXY16(int x, int y, int minim, int maxim);
	void addXYVertexFormat(int x, int y, int minim, int maxim);
	void addHandle(int handle, bool font = false);
	void addCell(int cell);
	void addOptions(int options, uint32_t flags, bool flatOnly = false, bool noClock = false);
	void addCharacter(int character);
	void addText(int text);
	void addStream(int stream);
	void addValueSlider(int val, int maxim);
	void addValueSliderDyn(int val, int maxim);
	void addValueSliderDynSub(int val, int sub, int maxim);
	void addSizeSubDynSlider(int size, int clip, int maxim);
	void addRangeMaximum(int range, int maxim);
	void addWidth(int width, int minim, int maxim);
	void addRadius(int width, int minim, int maxim);
	void addSpinBox(int index, int minim, int maxim, const QString &label, const QString &undoMessage, bool readOnly = false);
	void addSpinBoxUInt32(int index, double minim, double maxim, const QString &label, const QString &undoMessage);
	void addSpinBox16(int index, int minim, int maxim, const QString &label, const QString &undoMessage);
	void addSpinBox256(int index, int minim, int maxim, const QString &label, const QString &undoMessage);
	void addSpinBoxForBitmapTransform(int index, int minim, int maxim, const QString &label, const QString &undoMessage);
	void addSpinBox65536(int index, int minim, int maxim, const QString &label, const QString &undoMessage);
	void addSpinBoxAngle65536(int index, int minim, int maxim, const QString &label, const QString &undoMessage);
	void addSpinBoxVertexFormat(int index, int minim, int maxim, const QString &label, const QString &undoMessage);
	void addColor(int r, int g, int b);
	void addColorHex(int rgb);
	void addCheckBox(int index, const QString &label, const QString &undoMessage);
	void addWidth16(int width, int minim, int maxim, bool size);
	void addAlpha(int alpha);
	void addAlphaHex(int alpha);
	void addByteFlag(int flag, const QString &undoMessage);
	// void addAddress(int address);
	QComboBox *addComboBox(int index, const char **items, int nb, const QString &label, const QString &undoMessage);
	QComboBox *addComboBox(int index, const char **items, int begin, int end, const QString &label, const QString &undoMessage);
	QComboBox *addComboBox(int index, const int *toEnum, int toEnumSz, const int *toIntf, const char **toString, int toIntfStringSz, const QString &label, const QString &undoMessage);
	void addBlendFunction(int blend, const QString &label, const QString &undoMessage);
	void addCompareFunction(int compare);
	void addStencilOperation(int operation, const QString &label, const QString &undoMessage);
	void addPrimitive(int primitive);
	void addBitmapFormat(int format);
	void addBitmapFormatCmdSketch(int format);
	void addExtFormat(int format);
	void addSwizzle(int swizzle, const QString &label, const QString &undoMessage);
	void addBitmapWrap(int wrap, const QString &label, const QString &undoMessage);
	void addBitmapFilter(int filter);
	void addAnimLoop(int loop);
	void addAddressFlashOpt(int address, bool negative);
	void addAddressFlash(int address);
	void addAddress(int index, bool negative, bool readOnly = false);
	void addAddress64ByteAligned(int index);
	void addMemorySize(int size);
	void addCaptureButton(const QString &text, const QString &undoMessage);
	void addHelp(const QString &text);

private slots:
	void propertiesSetterChanged(QWidget *setter);

private:
	class PropertiesWidget;
	class PropertiesSpinBoxAlphaHex;
	class PropertiesSpinBoxAddress;
	class PropertiesSpinBoxAddressFlash;
	class PropertiesSpinBoxAddressFlashOpt;
	class PropertiesSpinBox;
	class PropertiesSpinBoxUInt32;
	class PropertiesDoubleSpinBox;
	class PropertiesSpinBox16;
	class PropertiesSpinBox256;
	class PropertiesSpinBoxForBitmapTransform;
	class PropertiesSpinBox65536;
	class PropertiesSpinBoxAngle65536;
	class PropertiesSpinBoxVertexFormat;
	class PropertiesCheckBox;
	class PropertiesLineEdit;
	class PropertiesLineEditChar;
	class PropertiesSlider;
	class PropertiesSliderDyn;
	class PropertiesSliderDynSub;
	class PropertiesSliderDynSubClip;
	class PropertiesColor;
	class PropertiesColorHex;
	class PropertiesComboBox;
	class PropertiesRemapComboBox;
	class PropertiesCaptureButton;

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

} /* namespace FTEDITOR */

#endif /* #ifndef FTEDITOR_INTERACTIVE_PROPERTIES_H */

/* end of file */
