/**
 * interactive_properties.cpp
 * $Id$
 * \file interactive_properties.cpp
 * \brief interactive_properties.cpp
 * \date 2014-03-04 22:58GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#include "interactive_properties.h"

// STL includes
#include <stdio.h>

// Qt includes
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>

// Emulator includes
#include <ft800emu_vc.h>

// Project includes
#include "main_window.h"
#include "dl_editor.h"
#include "properties_editor.h"
#include "interactive_widgets.h"

using namespace std;

namespace FT800EMUQT {

InteractiveProperties::InteractiveProperties(MainWindow *parent) : QGroupBox(parent), m_MainWindow(parent),
	m_LineEditor(NULL), m_LineNumber(0)
{
	QVBoxLayout *layout = new QVBoxLayout();
	setLayout(layout);
	connect(m_MainWindow->propertiesEditor(), SIGNAL(setterChanged(QWidget *)), this, SLOT(propertiesSetterChanged(QWidget *)));
}

InteractiveProperties::~InteractiveProperties()
{

}

void InteractiveProperties::clear()
{
	// printf("InteractiveProperties::clear()\n");

	for (std::vector<QWidget *>::iterator it(m_CurrentWidgets.begin()), end(m_CurrentWidgets.end()); it != end; ++it)
	{
		delete (*it);
	}
	m_CurrentWidgets.clear();
	m_CurrentProperties.clear();
	for (std::vector<QLayout *>::iterator it(m_CurrentLayouts.begin()), end(m_CurrentLayouts.end()); it != end; ++it)
	{
		delete (*it);
	}
	m_CurrentLayouts.clear();
}

void InteractiveProperties::propertiesSetterChanged(QWidget *setter)
{
	// printf("InteractiveProperties::propertiesSetterChanged(setter)\n");

	if (m_LineEditor && setter != m_LineEditor)
	{
		clear();
	}
}

void InteractiveProperties::addLabeledWidget(const QString &label, QWidget *widget)
{
	QHBoxLayout *hbox = new QHBoxLayout();
	QLabel *l = new QLabel(this);
	l->setText(label);
	hbox->addWidget(l);
	hbox->addWidget(widget);
	m_CurrentWidgets.push_back(l);
	m_CurrentWidgets.push_back(widget);
	m_CurrentLayouts.push_back(hbox);
	((QVBoxLayout *)layout())->addLayout(hbox);
}

void InteractiveProperties::addLabeledWidget(const QString &label, QWidget *widget0, QWidget *widget1)
{
	QHBoxLayout *hbox = new QHBoxLayout();
	QLabel *l = new QLabel(this);
	l->setText(label);
	hbox->addWidget(l);
	hbox->addWidget(widget0);
	hbox->addWidget(widget1);
	m_CurrentWidgets.push_back(l);
	m_CurrentWidgets.push_back(widget0);
	m_CurrentWidgets.push_back(widget1);
	m_CurrentLayouts.push_back(hbox);
	((QVBoxLayout *)layout())->addLayout(hbox);
}

void InteractiveProperties::addXY(int x, int y, int minim, int maxim)
{
	PropertiesSpinBox *propX = new PropertiesSpinBox(this, "Set x position", x);
	propX->setMinimum(minim);
	propX->setMaximum(maxim);
	PropertiesSpinBox *propY = new PropertiesSpinBox(this, "Set y position", y);
	propY->setMinimum(minim);
	propY->setMaximum(maxim);
	addLabeledWidget("XY: ", propX, propY);
	m_CurrentProperties.push_back(propX);
	m_CurrentProperties.push_back(propY);
	propX->done();
	propY->done();
}

void InteractiveProperties::addWH(int w, int h, int minim, int maxim)
{
	PropertiesSpinBox *propW = new PropertiesSpinBox(this, "Set width", w);
	propW->setMinimum(minim);
	propW->setMaximum(maxim);
	PropertiesSpinBox *propH = new PropertiesSpinBox(this, "Set height", h);
	propH->setMinimum(minim);
	propH->setMaximum(maxim);
	addLabeledWidget("WH: ", propW, propH);
	m_CurrentProperties.push_back(propW);
	m_CurrentProperties.push_back(propH);
	propW->done();
	propH->done();
}

void InteractiveProperties::addXY16(int x, int y, int minim, int maxim)
{
	PropertiesSpinBox16 *propX = new PropertiesSpinBox16(this, "Set x position", x);
	propX->setMinimum(minim);
	propX->setMaximum(maxim);
	PropertiesSpinBox16 *propY = new PropertiesSpinBox16(this, "Set y position", y);
	propY->setMinimum(minim);
	propY->setMaximum(maxim);
	addLabeledWidget("XY: ", propX, propY);
	m_CurrentProperties.push_back(propX);
	m_CurrentProperties.push_back(propY);
	propX->done();
	propY->done();
}

void InteractiveProperties::addHandle(int handle, bool font)
{
	PropertiesSpinBox *propHandle = new PropertiesSpinBox(this, font ? "Set font" : "Set handle", handle); // TODO: Handle combobox
	propHandle->setMinimum(0);
	propHandle->setMaximum(31);
	addLabeledWidget(font ? "Font: " : "Handle: ", propHandle);
	m_CurrentProperties.push_back(propHandle);
	propHandle->done();
}

void InteractiveProperties::addCell(int cell)
{
	PropertiesSpinBox *propCell = new PropertiesSpinBox(this, "Set cell", cell);
	propCell->setMinimum(0);
	propCell->setMaximum(255);
	addLabeledWidget("Cell: ", propCell);
	m_CurrentProperties.push_back(propCell);
	propCell->done();
}

void InteractiveProperties::addOptions(int options, uint32_t flags, bool flatOnly)
{
	/*
	#define OPT_MONO             1UL
	#define OPT_NODL             2UL
	#define OPT_SIGNED           256UL <- special case (when CMD_NUMBER, probably whenever no NOBACK)
	#define OPT_FLAT             256UL
	#define OPT_CENTERX          512UL
	#define OPT_CENTERY          1024UL
	#define OPT_CENTER           1536UL ----
	#define OPT_RIGHTX           2048UL
	#define OPT_NOBACK           4096UL
	#define OPT_NOTICKS          8192UL
	#define OPT_NOHM             16384UL
	#define OPT_NOPOINTER        16384UL <- special case (when NOHM this but not NOSECS)
	#define OPT_NOSECS           32768UL
	#define OPT_NOHANDS          49152UL ---- */
	if (flags & OPT_MONO)
	{
		PropertiesCheckBox *chb = new PropertiesCheckBox(this, "Set OPT_MONO", options, OPT_MONO);
		addLabeledWidget("OPT_MONO: ", chb);
		m_CurrentProperties.push_back(chb);
	}
	if (flags & OPT_NODL)
	{
		PropertiesCheckBox *chb = new PropertiesCheckBox(this, "Set OPT_NODL", options, OPT_NODL);
		addLabeledWidget("OPT_NODL: ", chb);
		m_CurrentProperties.push_back(chb);
	}
	if (flags & OPT_FLAT)
	{
		if (flags & OPT_NOBACK || flatOnly)
		{
			PropertiesCheckBox *chb0 = new PropertiesCheckBox(this, "Set OPT_FLAT", options, OPT_FLAT);
			addLabeledWidget("OPT_FLAT: ", chb0);
			m_CurrentProperties.push_back(chb0);
			if (flags & OPT_NOBACK)
			{
				PropertiesCheckBox *chb1 = new PropertiesCheckBox(this, "Set OPT_NOBACK", options, OPT_NOBACK);
				addLabeledWidget("OPT_NOBACK: ", chb1);
				m_CurrentProperties.push_back(chb1);
			}
		}
		else
		{
			PropertiesCheckBox *chb = new PropertiesCheckBox(this, "Set OPT_SIGNED", options, OPT_SIGNED);
			addLabeledWidget("OPT_SIGNED: ", chb);
			m_CurrentProperties.push_back(chb);
		}
	}
	if (flags & OPT_CENTERX)
	{
		PropertiesCheckBox *chb = new PropertiesCheckBox(this, "Set OPT_CENTERX", options, OPT_CENTERX);
		addLabeledWidget("OPT_CENTERX: ", chb);
		m_CurrentProperties.push_back(chb);
	}
	if (flags & OPT_CENTERY)
	{
		PropertiesCheckBox *chb = new PropertiesCheckBox(this, "Set OPT_CENTERY", options, OPT_CENTERY);
		addLabeledWidget("OPT_CENTERY: ", chb);
		m_CurrentProperties.push_back(chb);
	}
	if (flags & OPT_RIGHTX)
	{
		PropertiesCheckBox *chb = new PropertiesCheckBox(this, "Set OPT_RIGHTX", options, OPT_RIGHTX);
		addLabeledWidget("OPT_RIGHTX: ", chb);
		m_CurrentProperties.push_back(chb);
	}
	if (flags & OPT_NOTICKS)
	{
		PropertiesCheckBox *chb = new PropertiesCheckBox(this, "Set OPT_NOTICKS", options, OPT_NOTICKS);
		addLabeledWidget("OPT_NOTICKS: ", chb);
		m_CurrentProperties.push_back(chb);
	}
	if (flags & OPT_NOHM)
	{
		if (flags & OPT_NOSECS)
		{
			PropertiesCheckBox *chb0 = new PropertiesCheckBox(this, "Set OPT_NOHM", options, OPT_NOHM);
			addLabeledWidget("OPT_NOHM: ", chb0);
			m_CurrentProperties.push_back(chb0);
			PropertiesCheckBox *chb1 = new PropertiesCheckBox(this, "Set OPT_NOSECS", options, OPT_NOSECS);
			addLabeledWidget("OPT_NOSECS: ", chb1);
			m_CurrentProperties.push_back(chb1);
		}
		else
		{
			PropertiesCheckBox *chb = new PropertiesCheckBox(this, "Set OPT_NOPOINTER", options, OPT_NOPOINTER);
			addLabeledWidget("OPT_NOPOINTER: ", chb);
			m_CurrentProperties.push_back(chb);
		}
	}
}

void InteractiveProperties::addCharacter(int character)
{
	PropertiesLineEditChar *propCharacter = new PropertiesLineEditChar(this, "Set character", character);
	addLabeledWidget("Character: ", propCharacter);
	m_CurrentProperties.push_back(propCharacter);
}

void InteractiveProperties::addText(int text)
{
	PropertiesLineEdit *propText = new PropertiesLineEdit(this, "Set text", text);
	addLabeledWidget("Text: ", propText);
	m_CurrentProperties.push_back(propText);
}

void InteractiveProperties::addValueSlider(int val, int maxim)
{
	PropertiesSlider *propVal = new PropertiesSlider(this, "Set value", val);
	propVal->setMinimum(0);
	propVal->setMaximum(maxim);
	addLabeledWidget("Value: ", propVal);
	m_CurrentProperties.push_back(propVal);
	propVal->ready();
}

void InteractiveProperties::addValueSliderDyn(int val, int maxim)
{
	PropertiesSliderDyn *propVal = new PropertiesSliderDyn(this, "Set value", val, maxim);
	addLabeledWidget("Value: ", propVal);
	m_CurrentProperties.push_back(propVal);
	propVal->ready();
}

void InteractiveProperties::addValueSliderDynSub(int val, int sub, int maxim)
{
	PropertiesSliderDynSub *propVal = new PropertiesSliderDynSub(this, "Set value", val, sub, maxim);
	addLabeledWidget("Value: ", propVal);
	m_CurrentProperties.push_back(propVal);
	propVal->ready();
}

void InteractiveProperties::addSizeSubDynSlider(int size, int clip, int maxim)
{
	PropertiesSliderDynSubClip *propSize = new PropertiesSliderDynSubClip(this, "Set size", size, clip, maxim);
	addLabeledWidget("Size: ", propSize);
	m_CurrentProperties.push_back(propSize);
	propSize->ready();
}

void InteractiveProperties::addRangeMaximum(int range, int maxim)
{
	PropertiesSpinBox *propRange = new PropertiesSpinBox(this, "Set range", range);
	propRange->setMinimum(0);
	propRange->setMaximum(maxim);
	addLabeledWidget("Range: ", propRange);
	m_CurrentProperties.push_back(propRange);
	propRange->done();
}

void InteractiveProperties::addWidth(int width, int minim, int maxim)
{
	PropertiesSpinBox *propWidth = new PropertiesSpinBox(this, "Set width", width);
	propWidth->setMinimum(minim);
	propWidth->setMaximum(maxim);
	addLabeledWidget("Width: ", propWidth);
	m_CurrentProperties.push_back(propWidth);
	propWidth->done();
}

void InteractiveProperties::addRadius(int radius, int minim, int maxim)
{
	PropertiesSpinBox *propRadius = new PropertiesSpinBox(this, "Set radius", radius);
	propRadius->setMinimum(minim);
	propRadius->setMaximum(maxim);
	addLabeledWidget("Radius: ", propRadius);
	m_CurrentProperties.push_back(propRadius);
	propRadius->done();
}

void InteractiveProperties::addSpinBox(int index, int minim, int maxim, const QString &label, const QString &undoMessage)
{
	PropertiesSpinBox *prop = new PropertiesSpinBox(this, undoMessage, index);
	prop->setMinimum(minim);
	prop->setMaximum(maxim);
	addLabeledWidget(label, prop);
	m_CurrentProperties.push_back(prop);
	prop->done();
}

void InteractiveProperties::addSpinBox256(int index, int minim, int maxim, const QString &label, const QString &undoMessage)
{
	PropertiesSpinBox256 *prop = new PropertiesSpinBox256(this, undoMessage, index);
	prop->setMinimum(minim);
	prop->setMaximum(maxim);
	prop->setSingleStep(256);
	addLabeledWidget(label, prop);
	m_CurrentProperties.push_back(prop);
	prop->done();
}

void InteractiveProperties::addSpinBox65536(int index, int minim, int maxim, const QString &label, const QString &undoMessage)
{
	PropertiesSpinBox65536 *prop = new PropertiesSpinBox65536(this, undoMessage, index);
	prop->setMinimum(minim);
	prop->setMaximum(maxim);
	prop->setSingleStep(65536);
	addLabeledWidget(label, prop);
	m_CurrentProperties.push_back(prop);
	prop->done();
}

void InteractiveProperties::addSpinBoxAngle65536(int index, int minim, int maxim, const QString &label, const QString &undoMessage)
{
	PropertiesSpinBoxAngle65536 *prop = new PropertiesSpinBoxAngle65536(this, undoMessage, index);
	prop->setMinimum(minim);
	prop->setMaximum(maxim);
	prop->setSingleStep(65536 / 360);
	addLabeledWidget(label, prop);
	m_CurrentProperties.push_back(prop);
	prop->done();
}

void InteractiveProperties::addColor(int r, int g, int b)
{
	PropertiesColor *prop = new PropertiesColor(this, "Set color", r, g, b);
	addLabeledWidget("Color: ", prop);
	m_CurrentProperties.push_back(prop);
}

void InteractiveProperties::addCheckBox(int index, const QString &label, const QString &undoMessage)
{
	PropertiesCheckBox *prop = new PropertiesCheckBox(this, undoMessage, index, 0x01);
	addLabeledWidget(label, prop);
	m_CurrentProperties.push_back(prop);
}

void InteractiveProperties::addWidth16(int width, int minim, int maxim, bool size)
{
	PropertiesSpinBox16 *propWidth = new PropertiesSpinBox16(this, size ? "Set size" : "Set width", width);
	propWidth->setMinimum(minim);
	propWidth->setMaximum(maxim);
	addLabeledWidget(size ? "Size: " : "Width: ", propWidth);
	m_CurrentProperties.push_back(propWidth);
	propWidth->done();
}

void InteractiveProperties::addAlpha(int alpha)
{
	addSpinBox(alpha, 0, 255, "Alpha: ", "Set alpha");
}

void InteractiveProperties::addByteFlag(int flag, const QString &undoMessage)
{
	QHBoxLayout *hbox = new QHBoxLayout();
	{
		PropertiesCheckBox *chb = new PropertiesCheckBox(this, undoMessage, flag, 0x80);
		hbox->addWidget(chb);
		m_CurrentWidgets.push_back(chb);
		m_CurrentProperties.push_back(chb);
	}
	{
		PropertiesCheckBox *chb = new PropertiesCheckBox(this, undoMessage, flag, 0x40);
		hbox->addWidget(chb);
		m_CurrentWidgets.push_back(chb);
		m_CurrentProperties.push_back(chb);
	}
	{
		PropertiesCheckBox *chb = new PropertiesCheckBox(this, undoMessage, flag, 0x20);
		hbox->addWidget(chb);
		m_CurrentWidgets.push_back(chb);
		m_CurrentProperties.push_back(chb);
	}
	{
		PropertiesCheckBox *chb = new PropertiesCheckBox(this, undoMessage, flag, 0x10);
		hbox->addWidget(chb);
		m_CurrentWidgets.push_back(chb);
		m_CurrentProperties.push_back(chb);
	}
	{
		PropertiesCheckBox *chb = new PropertiesCheckBox(this, undoMessage, flag, 0x08);
		hbox->addWidget(chb);
		m_CurrentWidgets.push_back(chb);
		m_CurrentProperties.push_back(chb);
	}
	{
		PropertiesCheckBox *chb = new PropertiesCheckBox(this, undoMessage, flag, 0x04);
		hbox->addWidget(chb);
		m_CurrentWidgets.push_back(chb);
		m_CurrentProperties.push_back(chb);
	}
	{
		PropertiesCheckBox *chb = new PropertiesCheckBox(this, undoMessage, flag, 0x02);
		hbox->addWidget(chb);
		m_CurrentWidgets.push_back(chb);
		m_CurrentProperties.push_back(chb);
	}
	{
		PropertiesCheckBox *chb = new PropertiesCheckBox(this, undoMessage, flag, 0x01);
		hbox->addWidget(chb);
		m_CurrentWidgets.push_back(chb);
		m_CurrentProperties.push_back(chb);
	}
	m_CurrentLayouts.push_back(hbox);
	((QVBoxLayout *)layout())->addLayout(hbox);
}

void InteractiveProperties::addComboBox(int index, const char **items, int nb, const QString &label, const QString &undoMessage)
{
	addComboBox(index, items, 0, nb, label, undoMessage);
}

void InteractiveProperties::addComboBox(int index, const char **items, int begin, int end, const QString &label, const QString &undoMessage)
{
	PropertiesComboBox *prop = new PropertiesComboBox(this, undoMessage, index, begin);
	for (int i = begin; i < end; ++i)
		prop->addItem(items[i]);
	addLabeledWidget(label, prop);
	m_CurrentProperties.push_back(prop);
	prop->ready();
}

void InteractiveProperties::addBlendFunction(int blend, const QString &label, const QString &undoMessage)
{
	addComboBox(blend, g_DlEnumBlend, DL_ENUM_BLEND_NB, label, undoMessage);
}

void InteractiveProperties::addCompareFunction(int compare)
{
	addComboBox(compare, g_DlEnumCompare, DL_ENUM_COMPARE_NB, "Func: ", "Set func");
}

void InteractiveProperties::addStencilOperation(int operation, const QString &label, const QString &undoMessage)
{
	addComboBox(operation, g_DlEnumStencil, DL_ENUM_STENCIL_NB, label, undoMessage);
}

void InteractiveProperties::addPrimitive(int primitive)
{
	addComboBox(primitive, g_DlEnumPrimitive, 1, DL_ENUM_PRIMITIVE_NB, "Primitive: ", "Set primitive");
}

void InteractiveProperties::addBitmapFormat(int format)
{
	addComboBox(format, g_DlEnumBitmapFormat, DL_ENUM_BITMAP_FORMAT_NB, "Format: ", "Set bitmap format");
}

void InteractiveProperties::addBitmapWrap(int wrap, const QString &label, const QString &undoMessage)
{
	addComboBox(wrap, g_DlEnumBitmapWrap, DL_ENUM_BITMAP_WRAP_NB, label, undoMessage);
}

void InteractiveProperties::addBitmapFilter(int filter)
{
	addComboBox(filter, g_DlEnumBitmapFilter, DL_ENUM_BITMAP_FILTER_NB, "Filter: ", "Set bitmap filter");
}

void InteractiveProperties::addAddress(int address)
{
	PropertiesSpinBoxAddress *prop = new PropertiesSpinBoxAddress(this, "Set address", address);
	addLabeledWidget("Address: ", prop);
	m_CurrentProperties.push_back(prop);
	prop->done();
}

void InteractiveProperties::setEditorLine(DlEditor *editor, int line)
{
	// printf("InteractiveProperties::setEditorLine(...)\n");

	clear();

	m_LineNumber = line;
	m_LineEditor = editor;
	if (editor)
	{
		const DlParsed &parsed = editor->getLine(line);
		if (parsed.ValidId)
		{
			setProperties(parsed.IdLeft, parsed.IdRight, editor);
		}
		else
		{
			if (parsed.IdText.size() > 0)
			{
				QString message;
				message.sprintf(tr("Unknown command '<i>%s</i>'").toUtf8().constData(), parsed.IdText.c_str());
				m_MainWindow->propertiesEditor()->setInfo(message);
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			else if (m_MainWindow->propertiesEditor()->getEditWidgetSetter())
			{
				m_MainWindow->propertiesEditor()->setInfo(QString());
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
		}
	}
}

void InteractiveProperties::setProperties(int idLeft, int idRight, DlEditor *editor)
{
	bool ok = false;
	// const uint32_t *p = m_DisplayListParsed[i].Parameter;
	if (idLeft == FT800EMU_DL_VERTEX2F)
	{
		m_MainWindow->propertiesEditor()->setInfo(tr(
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
		if (editor)
		{
			setTitle("VERTEX2F");
			addXY16(0, 1, -16384, 16383);
			m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
		}
		ok = true;
	}
	else if (idLeft == FT800EMU_DL_VERTEX2II)
	{
		m_MainWindow->propertiesEditor()->setInfo(tr(
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
		if (editor)
		{
			setTitle("VERTEX2II");
			addXY(0, 1, 0, 511);
			addHandle(2);
			addCell(3);
			m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
		}
		ok = true;
	}
	else if (idLeft == 0xFFFFFF00) switch (idRight | 0xFFFFFF00)
	{
		case CMD_DLSTART:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_DLSTART</b>()<br>"
				"<br>"
				"When the co-processor engine executes this command, it waits until the display list is "
				"ready for writing, then sets REG_CMD_DL to zero."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_SWAP:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_SWAP</b>()<br>"
				"<br>"
				"When the co-processor engine executes this command, it requests a display list swap by "
				"writing to REG_DLSWAP."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_INTERRUPT:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_INTERRUPT</b>(<i>ms</i>)<br>"
				"<b>ms</b>: Delay before interrupt triggers, in milliseconds. The interrupt is guaranteed "
				"not to fire before this delay. If ms is zero, the interrupt fires immediately.<br>"
				"<br>"
				"When the co-processor engine executes this command, it triggers interrupt INT_CMDFLAG."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_BGCOLOR:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_BGCOLOR</b>(<i>r</i>, <i>g</i>, <i>b</i>)<br>"
				"<b>rgb</b>: New background color, as a 24-bit RGB number. Red is the most significant 8 "
				"bits, blue is the least. So 0xff0000 is bright red.<br>"
				"Background color is applicable for things that the user cannot move. Example "
				"behind gauges and sliders etc.<br>"
				"<br>"
				"Set the background color."));
			if (editor)
			{
				setTitle("CMD_BGCOLOR");
				addColor(0, 1, 2);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_FGCOLOR:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_FGCOLOR</b>(<i>r</i>, <i>g</i>, <i>b</i>)<br>"
				"<b>rgb</b>: New foreground color, as a 24-bit RGB number. Red is the most significant 8 "
				"bits, blue is the least. So 0xff0000 is bright red.<br>"
				"Foreground color is applicable "
				"for things that the user can move such as handles and buttons "
				"(\"affordances\").<br>"
				"<br>"
				"Set the foreground color."));
			if (editor)
			{
				setTitle("CMD_FGCOLOR");
				addColor(0, 1, 2);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_GRADIENT:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_GRADIENT</b>("
				"<i>x0</i>, <i>y0</i>, <i>a0</i>, <i>r0</i>, <i>g0</i>, <i>b0</i>, "
				"<i>x1</i>, <i>y1</i>, <i>a1</i>, <i>r1</i>, <i>g1</i>, <i>b1</i>)<br>"
				"<b>x0</b>: x-coordinate of point 0, in pixels<br>"
				"<b>y0</b>: y-coordinate of point 0, in pixels<br>"
				"<b>argb0</b>: Color of point 0, as a 24-bit RGB number. R is the most significant 8 bits, B is "
				"the least. So 0xff0000 is bright red.<br>"
				"<b>x1</b>: x-coordinate of point 1, in pixels<br>"
				"<b>y1</b>: y-coordinate of point 1, in pixels<br>"
				"<b>argb1</b>: Color of point 1.<br>"
				"<br>"
				"Draw a smooth color gradient.<br>"
				"<br>"
				"All the colours step values are calculated based on smooth curve interpolated from the "
				"RGB0 to RGB1 parameter. The smooth curve equation is independently calculated for all "
				"three colors and the equation used is R0 + t * (R1 - R0), where t is interpolated between "
				"0 and 1. Gradient must be used with Scissor function to get the intended gradient "
				"display."));
			if (editor)
			{
				setTitle("CMD_GRADIENT");
				addXY(0, 1, -1024, 1023);
				addColor(2, 3, 4);
				addXY(5, 6, -1024, 1023);
				addColor(7, 8, 9);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_TEXT:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_TEXT</b>(<i>x</i>, <i>y</i>, <i>font</i>, <i>options</i>, <i>s</i>)<br>"
				"<b>x</b>: x-coordinate of text base, in pixels<br>"
				"<b>y</b>: y-coordinate of text base, in pixels<br>"
				"<b>font</b>: Font to use for text, 0-31. See ROM and RAM Fonts<br>"
				"<b>options</b>: By default (x; y) is the top-left pixel of the text. OPT_CENTERX centers the "
				"text horizontally, OPT_CENTERY centers it vertically. OPT_CENTER centers the "
				"text in both directions. OPT_RIGHTX right-justifies the text, so that the x is "
				"the rightmost pixel.<br>"
				"<b>s</b>: text<br>"
				"<br>"
				"Draw text."));
			if (editor)
			{
				setTitle("CMD_TEXT");
				addXY(0, 1, -1024, 1023);
				addHandle(2, true);
				addOptions(3, OPT_CENTER | OPT_RIGHTX);
				addText(4);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_BUTTON:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_BUTTON</b>(<i>x</i>, <i>y</i>, <i>w</i>, <i>h</i>, <i>font</i>, <i>options</i>, <i>s</i>)<br>"
				"<b>x</b>: x-coordinate of button top-left, in pixels<br>"
				"<b>y</b>: y-coordinate of button top-left, in pixels<br>"
				"<b>w</b>: width of button, in pixels<br>"
				"<b>h</b>: height of button, in pixels<br>"
				"<b>font</b>: Font to use for text, 0-31. See ROM and RAM Fonts<br>"
				"<b>options</b>: By default the button is drawn with a 3D effect. OPT_FLAT removes the 3D "
				"effect.<br>"
				"<b>s</b>: button label<br>"
				"<br>"
				"Draw a button."));
			if (editor)
			{
				setTitle("CMD_BUTTON");
				addXY(0, 1, -1024, 1023);
				addWH(2, 3, 0, 1023);
				addHandle(4, true);
				addOptions(5, OPT_FLAT, true);
				addText(6);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_KEYS:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_KEYS</b>(<i>x</i>, <i>y</i>, <i>w</i>, <i>h</i>, <i>font</i>, <i>options</i>, <i>s</i>)<br>"
				"<b>x</b>: x-coordinate of keys top-left, in pixels<br>"
				"<b>y</b>: y-coordinate of keys top-left, in pixels<br>"
				"<b>w</b>: width of keys, in pixels<br>"
				"<b>h</b>: height of keys, in pixels<br>"
				"<b>font</b>: Font to use for keys, 0-31. See ROM and RAM Fonts<br>"
				"<b>options</b>: By default the keys are drawn with a 3D effect. OPT_FLAT removes the 3D "
				"effect. If OPT_CENTER is given the keys are drawn at minimum size centered "
				"within the w x h rectangle. Otherwise the keys are expanded so that they "
				"completely fill the available space. If an ASCII code is specified, that key is "
				"drawn 'pressed' - i.e. in background color with any 3D effect removed..<br>"
				"<b>s</b>: key labels, one character per key. The TAG value is set to the ASCII value of "
				"each key, so that key presses can be detected using the REG_TOUCH_TAG "
				"register.<br>"
				"<br>"
				"Draw a row of keys."));
			if (editor)
			{
				setTitle("CMD_KEYS");
				addXY(0, 1, -1024, 1023);
				addWH(2, 3, 0, 1023);
				addHandle(4, true);
				addOptions(5, OPT_FLAT | OPT_CENTERX, true);
				addCharacter(5);
				addText(6);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_PROGRESS:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_PROGRESS</b>(<i>x</i>, <i>y</i>, <i>w</i>, <i>h</i>, <i>options</i>, <i>val</i>, <i>range</i>)<br>"
				"<b>x</b>: x-coordinate of progress bar top-left, in pixels<br>"
				"<b>y</b>: y-coordinate of progress bar top-left, in pixels<br>"
				"<b>w</b>: width of progress bar, in pixels<br>"
				"<b>h</b>: height of progress bar, in pixels<br>"
				"<b>options</b>: By default the progress bar is drawn with a 3D effect. OPT_FLAT removes the "
				"3D effect<br>"
				"<b>val</b>: Displayed value of progress bar, between 0 and range inclusive<br>"
				"<b>range</b>: Maximum value<br>"
				"<br>"
				"Draw a progress bar."));
			if (editor)
			{
				setTitle("CMD_PROGRESS");
				addXY(0, 1, -1024, 1023);
				addWH(2, 3, 0, 1023);
				addOptions(4, OPT_FLAT, true);
				addValueSliderDyn(5, 6);
				addRangeMaximum(6, 0xFFFF);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_SLIDER:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_SLIDER</b>(<i>x</i>, <i>y</i>, <i>w</i>, <i>h</i>, <i>options</i>, <i>val</i>, <i>range</i>)<br>"
				"<b>x</b>: x-coordinate of slider top-left, in pixels<br>"
				"<b>y</b>: y-coordinate of slider top-left, in pixels<br>"
				"<b>w</b>: width of slider, in pixels<br>"
				"<b>h</b>: height of slider, in pixels<br>"
				"<b>options</b>: By default the slider is drawn with a 3D effect. OPT_FLAT removes the 3D "
				"effect<br>"
				"<b>val</b>: Displayed value of slider, between 0 and range inclusive<br>"
				"<b>range</b>: Maximum value<br>"
				"<br>"
				"Draw a slider."));
			if (editor)
			{
				setTitle("CMD_SLIDER");
				addXY(0, 1, -1024, 1023);
				addWH(2, 3, 0, 1023);
				addOptions(4, OPT_FLAT, true);
				addValueSliderDyn(5, 6);
				addRangeMaximum(6, 0xFFFF);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_SCROLLBAR:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_SCROLLBAR</b>(<i>x</i>, <i>y</i>, <i>w</i>, <i>h</i>, <i>options</i>, <i>val</i>, <i>size</i>, <i>range</i>)<br>"
				"<b>x</b>: x-coordinate of scroll bar top-left, in pixels<br>"
				"<b>y</b>: y-coordinate of scroll bar top-left, in pixels<br>"
				"<b>w</b>: width of scroll bar, in pixels. If width is greater, the scroll bar is drawn horizontally<br>"
				"<b>h</b>: height of scroll bar, in pixels. If height is greater, the scroll bar is drawn vertically<br>"
				"<b>options</b>: By default the scroll bar is drawn with a 3D effect. OPT_FLAT removes the 3D "
				"effect<br>"
				"<b>val</b>: Displayed value of scroll bar, between 0 and range inclusive<br>"
				"<b>size</b>: Size<br>"
				"<b>range</b>: Maximum value<br>"
				"<br>"
				"Draw a scroll bar."));
			if (editor)
			{
				setTitle("CMD_SCROLLBAR");
				addXY(0, 1, -1024, 1023);
				addWH(2, 3, 0, 1023);
				addOptions(4, OPT_FLAT, true);
				addValueSliderDynSub(5, 6, 7);
				addSizeSubDynSlider(6, 5, 7);
				addRangeMaximum(7, 0xFFFF);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_TOGGLE:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_TOGGLE</b>(<i>x</i>, <i>y</i>, <i>w</i>, <i>f</i>, <i>options</i>, <i>state</i>, <i>s</i>)<br>"
				"<b>x</b>: x-coordinate of top-left of toggle, in pixels<br>"
				"<b>y</b>: y-coordinate of top-left of toggle, in pixels<br>"
				"<b>w</b>: width of toggle, in pixels<br>"
				"<b>f</b>: font to use for text, 0-31. See ROM and RAM Fonts<br>"
				"<b>options</b>: By default the toggle bar is drawn with a 3D effect. OPT_FLAT removes the 3D "
				"effect<br>"
				"<b>state</b>: state of the toggle: 0 is off, 65535 is on.<br>"
				"<b>s</b>: String label for toggle. A character value of 255 (in C it can be written as \xff) "
				"separates the two labels.<br>"
				"<br>"
				"Draw a toggle switch."));
			if (editor)
			{
				setTitle("CMD_TOGGLE");
				addXY(0, 1, -1024, 1023);
				addWidth(2, 0, 1023);
				addHandle(3, true);
				addOptions(4, OPT_FLAT, true);
				addValueSlider(5, 0xFFFF);
				addText(6); // TODO: Separate
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_GAUGE:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_GAUGE</b>(<i>x</i>, <i>y</i>, <i>r</i>, <i>options</i>, <i>major</i>, <i>minor</i>, <i>val</i>, <i>range</i>)<br>"
				"<b>x</b>: X-coordinate of gauge center, in pixels<br>"
				"<b>y</b>: Y-coordinate of gauge center, in pixels<br>"
				"<b>r</b>: Radius of the gauge, in pixels<br>"
				"<b>options</b>: By default the gauge dial is drawn with a 3D effect. OPT_FLAT removes the "
				"3D effect. With option OPT_NOBACK, the background is not drawn. With "
				"option OPT_NOTICKS, the tick marks are not drawn. With option "
				"OPT_NOPOINTER, the pointer is not drawn.<br>"
				"<b>major</b>: Number of major subdivisions on the dial, 1-10<br>"
				"<b>minor</b>: Number of minor subdivisions on the dial, 1-10<br>"
				"<b>val</b>: Gauge indicated value, between 0 and range, inclusive<br>"
				"<b>range</b>: Maximum value<br>"
				"<br>"
				"Draw a gauge."));
			if (editor)
			{
				setTitle("CMD_GAUGE");
				addXY(0, 1, -1024, 1023);
				addRadius(2, 0, 1023);
				addOptions(3, OPT_FLAT | OPT_NOBACK | OPT_NOTICKS | OPT_NOPOINTER);
				addSpinBox(4, 1, 10, "Major: ", "Edit major");
				addSpinBox(5, 1, 10, "Minor: ", "Edit minor");
				addValueSliderDyn(6, 7);
				addRangeMaximum(7, 0xFFFF);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_CLOCK:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_CLOCK</b>(<i>x</i>, <i>y</i>, <i>r</i>, <i>options</i>, <i>h</i>, <i>m</i>, <i>s</i>, <i>ms</i>)<br>"
				"<b>x</b>: X-coordinate of clock center, in pixels<br>"
				"<b>y</b>: Y-coordinate of clock center, in pixels<br>"
				"<b>r</b>: Radius of the clock, in pixels<br>"
				"<b>options</b>: By default the clock dial is drawn with a 3D effect. OPT_FLAT removes the 3D "
				"effect. With option OPT_NOBACK, the background is not drawn. With option "
				"OPT_NOTICKS, the twelve hour ticks are not drawn. With option OPT_NOSECS, "
				"the seconds hand is not drawn. With option OPT_NOHANDS, no hands are "
				"drawn. With option OPT_NOHM, no hour and minutes hands are drawn.<br>"
				"<b>h</b>: hours<br>"
				"<b>m</b>: minutes<br>"
				"<b>s</b>: seconds<br>"
				"<b>ms</b>: milliseconds<br>"
				"<br>"
				"Draw a clock."));
			if (editor)
			{
				setTitle("CMD_CLOCK");
				addXY(0, 1, -1024, 1023);
				addRadius(2, 0, 1023);
				addOptions(3, OPT_FLAT | OPT_NOBACK | OPT_NOTICKS | OPT_NOHANDS);
				addSpinBox(4, 0, 23, "Hours: ", "Edit hours"); // TODO: Clock ?
				addSpinBox(5, 0, 59, "Minutes: ", "Edit minutes");
				addSpinBox(6, 0, 59, "Seconds: ", "Edit seconds");
				addSpinBox(7, 0, 999, "Millis: ", "Edit milliseconds");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_CALIBRATE:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_CALIBRATE</b>(<i>result</i>)<br>"
				"<b>result</b>: output parameter; written with 0 on failure<br>"
				"<br>"
				"The calibration procedure collects three touches from the touch screen, then computes "
				"and loads an appropriate matrix into REG_TOUCH_TRANSFORM_A-F. To use it, create a "
				"display list and then use CMD_CALIBRATE. The co-processor engine overlays the touch "
				"targets on the current display list, gathers the calibration input and updates "
				"REG_TOUCH_TRANSFORM_A-F."));
			if (editor)
			{
				// no properties
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_SPINNER:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_SPINNER</b>(<i>x</i>, <i>y</i>, <i>style</i>, <i>scale</i>)<br>"
				"<b>x</b>: x<br>"
				"<b>y</b>: y<br>"
				"<b>style</b>: style<br>"
				"<b>scale</b>: scale<br>"
				"<br>"
				"The spinner is an animated overlay that shows the user that some task is continuing. To "
				"trigger the spinner, create a display list and then use CMD_SPINNER. The co-processor "
				"engine overlays the spinner on the current display list, swaps the display list to make it "
				"visible, then continuously animates until it receives CMD_STOP. REG_MACRO_0 and "
				"REG_MACRO_1 registers are utilized to perform the animation kind of effect. The "
				"frequency of points movement is wrt display frame rate configured.<br>"
				"Typically for 480x272 display panels the display rate is ~60fps. For style 0 and 60fps, "
				"the point repeats the sequence within 2 seconds. For style 1 and 60fps, the point repeats "
				"the sequence within 1.25 seconds. For style 2 and 60fps, the clock hand repeats the "
				"sequence within 2 seconds. For style 3 and 60fps, the moving dots repeat the sequence "
				"within 1 second.<br>"
				"Note that only one of CMD_SKETCH, CMD_CSKETCH, CMD_SCREENSAVER, or CMD_SPINNER can be "
				"active at one time."));
			if (editor)
			{
				setTitle("CMD_SPINNER");
				addXY(0, 1, -1024, 1023);
				addSpinBox(2, 0, 3, "Style: ", "Edit style"); // TODO: ComboBox
				addSpinBox(3, 0, 1023, "Scale: ", "Edit scale");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_STOP:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_STOP</b>()<br>"
				"<br>"
				"Stop any spinner, screensaver or sketch."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_MEMSET:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_MEMSET</b>(<i>ptr</i>, <i>value</i>, <i>num</i>)<br>"
				"<b>ptr</b>: Starting address of the memory block<br>"
				"<b>value</b>: Value to be written to memory<br>"
				"<b>num</b>: Number of bytes in the memory block<br>"
				"<br>"
				"Fill memory with a byte value."));
			if (editor)
			{
				setTitle("CMD_MEMSET");
				addSpinBox(0, 0, 0x7FFFFFFF, "Address: ", "Set address");
				addSpinBox(1, 0x80000000, 0x7FFFFFFF, "Value: ", "Set value");
				addSpinBox(2, 0, 0x7FFFFFFF, "Num: ", "Set num");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_MEMZERO:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_MEMZERO</b>(<i>ptr</i>, <i>num</i>)<br>"
				"<b>ptr</b>: Starting address of the memory block<br>"
				"<b>num</b>: Number of bytes in the memory block<br>"
				"<br>"
				"Write zero to a block of memory."));
			if (editor)
			{
				setTitle("CMD_MEMZERO");
				addSpinBox(0, 0, 0x7FFFFFFF, "Address: ", "Set address");
				addSpinBox(1, 0, 0x7FFFFFFF, "Num: ", "Set num");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_MEMCPY:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_MEMCPY</b>(<i>dest</i>, <i>src</i>, <i>num</i>)<br>"
				"<b>dest</b>: address of the destination memory block<br>"
				"<b>src</b>: address of the source memory block<br>"
				"<b>num</b>: Number of bytes in the memory block<br>"
				"<br>"
				"Copy a block of memory."));
			if (editor)
			{
				setTitle("CMD_MEMCPY");
				addSpinBox(0, 0, 0x7FFFFFFF, "Dest: ", "Set dest");
				addSpinBox(1, 0, 0x7FFFFFFF, "Src: ", "Set src");
				addSpinBox(2, 0, 0x7FFFFFFF, "Num: ", "Set num");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_APPEND:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_APPEND</b>(<i>ptr</i>, <i>num</i>)<br>"
				"<b>ptr</b>: Start of source commands in main memory<br>"
				"<b>num</b>: Number of bytes to copy. This must be a multiple of 4<br>"
				"<br>"
				"Append memory to display list."));
			if (editor)
			{
				setTitle("CMD_APPEND");
				addAddress(0);
				addSpinBox(1, 0, 0x7FFFFFFF, "Num: ", "Set num");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_SNAPSHOT:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_SNAPSHOT</b>(<i>ptr</i>)<br>"
				"<b>ptr</b>: Snapshot destination address, in main memory<br>"
				"<br>"
				"Take a snapshot of the current screen."));
			if (editor)
			{
				setTitle("CMD_SNAPSHOT");
				addAddress(0);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_LOADIDENTITY:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_LOADIDENTITY</b>()<br>"
				"<br>"
				"Set the current matrix to identity."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_TRANSLATE:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_TRANSLATE</b>(<i>tx</i>, <i>ty</i>)<br>"
				"<b>tx</b>: x translate factor, in signed 16.16 bit fixed-point form.<br>"
				"<b>ty</b>: y translate factor, in signed 16.16 bit fixed-point form.<br>"
				"<br>"
				"Apply a translation to the current matrix."));
			if (editor)
			{
				setTitle("CMD_TRANSLATE");
				addSpinBox65536(0, 0x80000000, 0x7FFFFFFF, "X: ", "Set x translation");
				addSpinBox65536(1, 0x80000000, 0x7FFFFFFF, "Y: ", "Set y translation");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_SCALE:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_SCALE</b>(<i>sx</i>, <i>sy</i>)<br>"
				"<b>sx</b>: x scale factor, in signed 16.16 bit fixed-point form.<br>"
				"<b>sy</b>: y scale factor, in signed 16.16 bit fixed-point form.<br>"
				"<br>"
				"Apply a scale to the current matrix."));
			if (editor)
			{
				setTitle("CMD_SCALE");
				addSpinBox65536(0, 0x80000000, 0x7FFFFFFF, "X: ", "Set x scale");
				addSpinBox65536(1, 0x80000000, 0x7FFFFFFF, "Y: ", "Set y scale");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_ROTATE:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_ROTATE</b>(<i>a</i>)<br>"
				"<b>a</b>: Clockwise rotation angle, in units of 1/65536 of a circle.<br>"
				"<br>"
				"Apply a rotation to the current matrix."));
			if (editor)
			{
				setTitle("CMD_ROTATE");
				addSpinBoxAngle65536(0, 0x80000000, 0x7FFFFFFF, "X: ", "Set angle");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_SETMATRIX:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_SETMATRIX</b>()<br>"
				"<br>"
				"Write the current matrix as a bitmap transform."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_SETFONT:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_SETFONT</b>(<i>font</i>, <i>ptr</i>)<br>"
				"<b>font</b>: font<br>"
				"<b>ptr</b>: ptr<br>"
				"<br>"
				"To use a custom font with the co-processor engine objects, create the font definition "
				"data in FT800 RAM and issue CMD_SETFONT, as described in ROM and RAM Fonts."));
			if (editor)
			{
				setTitle("CMD_SETFONT");
				addSpinBox(0, 0, 15, "Font: ", "Set font");
				addAddress(1);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_TRACK:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_TRACK</b>(<i>x</i>, <i>y</i>, <i>w</i>, <i>h</i>, <i>tag</i>)<br>"
				"<b>x</b>: x-coordinate of track area top-left, in pixels<br>"
				"<b>y</b>: y-coordinate of track area top-left, in pixels<br>"
				"<b>w</b>: Width of track area, in pixels<br>"
				"<b>h</b>: Height of track area, in pixels.<br>"
				"A w and h of (1,1) means that the tracker is rotary, and reports an "
				"angle value in REG_TRACKER. (0,0) disables the tracker. Other values "
				"mean that the tracker is linear, and reports values along its length "
				"from 0 to 65535 in REG_TRACKER.<br>"
				"<b>tag</b>: tag for this track, 1-255<br>"
				"<br>"
				"The co-processor engine can assist the MCU in tracking touches on graphical objects. For "
				"example touches on dial objects can be reported as angles, saving MCU computation. To "
				"do this the MCU draws the object using a chosen tag value, and registers a track area for "
				"that tag.<br>"
				"From then on any touch on that object is reported in REG_TRACKER. "
				"The MCU can detect any touch on the object by reading the 32-bit value in "
				"REG_TRACKER. The low 8 bits give the current tag, or zero if there is no touch. The high "
				"sixteen bits give the tracked value.<br>"
				"For a rotary tracker - used for clocks, gauges and dials - this value is the angle of the "
				"touch point relative to the object center, in units of 1=65536 of a circle. 0 means that "
				"the angle is straight down, 0x4000 left, 0x8000 up, and 0xc000 right.<br>"
				"For a linear tracker - used for sliders and scrollbars - this value is the distance along the "
				"tracked object, from 0 to 65535."));
			if (editor)
			{
				setTitle("CMD_TRACK");
				addXY(0, 1, 0, 512);
				addWH(2, 3, 0, 512);
				addSpinBox(4, 0, 255, "Tag: ", "Set tag");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_DIAL:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_DIAL</b>(<i>x</i>, <i>y</i>, <i>r</i>, <i>options</i>, <i>val</i>)<br>"
				"<b>x</b>: x-coordinate of dial center, in pixels<br>"
				"<b>y</b>: y-coordinate of dial center, in pixels<br>"
				"<b>r</b>: radius of dial, in pixels<br>"
				"<b>options</b>: By default the dial is drawn with a 3D effect. OPT_FLAT removes the 3D "
				"effect<br>"
				"<b>val</b>: Displayed value of slider, between 0 and 65535 inclusive. 0 means that the "
				"dial points straight down, 0x4000 left, 0x8000 up, and 0xc000 right.<br>"
				"<br>"
				"Draw a rotary dial control."));
			if (editor)
			{
				setTitle("CMD_DIAL");
				addXY(0, 1, -1024, 1023);
				addRadius(2, 0, 1023);
				addOptions(3, OPT_FLAT, true);
				addValueSlider(4, 0xFFFF);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_NUMBER:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_NUMBER</b>(<i>x</i>, <i>y</i>, <i>font</i>, <i>options</i>, <i>n</i>)<br>"
				"<b>x</b>: x-coordinate of text base, in pixels<br>"
				"<b>y</b>: y-coordinate of text base, in pixels<br>"
				"<b>font</b>: Font to use for text, 0-31. See ROM and RAM Fonts<br>"
				"<b>options</b>: By default (x; y) is the top-left pixel of the text. OPT_CENTERX centers the "
				"text horizontally, OPT_CENTERY centers it vertically. OPT_CENTER centers the "
				"text in both directions. OPT_RIGHTX right-justifies the text, so that the x is "
				"the rightmost pixel. By default the number is displayed with no leading "
				"zeroes, but if a width 1-9 is specified in the options, then the number is "
				"padded if necessary with leading zeroes so that it has the given width. If "
				"OPT_SIGNED is given, the number is treated as signed, and prefixed by a "
				"minus sign if negative.<br>"
				"<b>n</b>: The number to display, either unsigned or signed 32-bit<br>"
				"<br>"
				"Draw text."));
			if (editor)
			{
				setTitle("CMD_TEXT");
				addXY(0, 1, -1024, 1023);
				addHandle(2, true);
				addOptions(3, OPT_CENTER | OPT_RIGHTX | OPT_SIGNED);
				addSpinBox(4, 0x80000000, 0x7FFFFFFF, "Number: ", "Edit number");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_SCREENSAVER:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_SCREENSAVER</b>()<br>"
				"<br>"
				"After the screensaver command, the co-processor engine continuously updates "
				"REG_MACRO_0 with VERTEX2F with varying (x; y) coordinates. With an appropriate "
				"display list, this causes a bitmap to move around the screen without any MCU work.<br>"
				"Command CMD_STOP stops the update process.<br>"
				"Note that only one of CMD_SKETCH, CMD_CSKETCH, CMD_SCREENSAVER, or CMD_SPINNER can be "
				"active at one time."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_SKETCH:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_SKETCH</b>(<i>x</i>, <i>y</i>, <i>w</i>, <i>h</i>, <i>ptr</i>, <i>format</i>)<br>"
				"<b>x</b>: x-coordinate of sketch area, in pixels<br>"
				"<b>y</b>: y-coordinate of sketch area, in pixels<br>"
				"<b>w</b>: Width of sketch area, in pixels<br>"
				"<b>h</b>: Height of sketch area, in pixels<br>"
				"<b>ptr</b>: Base address of sketch bitmap<br>"
				"<b>format</b>: Format of sketch bitmap, either L1 or L8<br>"
				"<br>"
				"After the sketch command, the co-processor engine continuously samples the touch "
				"inputs and paints pixels into a bitmap, according to the touch (x; y). This means that the "
				"user touch inputs are drawn into the bitmap without any need for MCU work.<br>Command "
				"CMD_STOP stops the update process.<br>"
				"Note that only one of CMD_SKETCH, CMD_CSKETCH, CMD_SCREENSAVER, or CMD_SPINNER can be "
				"active at one time."));
			if (editor)
			{
				setTitle("CMD_SKETCH");
				addXY(0, 1, -1024, 1023);
				addWH(2, 3, 0, 1023);
				addAddress(4);
				addBitmapFormat(5);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
#ifndef FT810EMU_MODE // Deprecated in FT810
		case CMD_CSKETCH:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_CSKETCH</b>(<i>x</i>, <i>y</i>, <i>w</i>, <i>h</i>, <i>ptr</i>, <i>format</i>, <i>freq</i>)<br>"
				"<b>x</b>: x-coordinate of sketch area, in pixels<br>"
				"<b>y</b>: y-coordinate of sketch area, in pixels<br>"
				"<b>w</b>: Width of sketch area, in pixels<br>"
				"<b>h</b>: Height of sketch area, in pixels<br>"
				"<b>ptr</b>: Base address of sketch bitmap<br>"
				"<b>format</b>: Format of sketch bitmap, either L1 or L8<br>"
				"<b>freq</b>: The oversampling frequency. The typical value is 1500 to make sure the "
				"lines are connected smoothly. The value zero means no oversampling operation<br>"
				"<br>"
				"This command is only valid for FT801 silicon. FT801 co-processor will oversample "
				"the coordinates reported by the capacitive touch panel in the frequency of �freq� and "
				"forms the lines with a smoother effect.<br>"
				"<br>"
				"After the sketch command, the co-processor engine continuously samples the touch "
				"inputs and paints pixels into a bitmap, according to the touch (x; y). This means that the "
				"user touch inputs are drawn into the bitmap without any need for MCU work.<br>Command "
				"CMD_STOP stops the update process.<br>"
				"Note that only one of CMD_SKETCH, CMD_CSKETCH, CMD_SCREENSAVER, or CMD_SPINNER can be "
				"active at one time."));
			if (editor)
			{
				setTitle("CMD_SKETCH");
				addXY(0, 1, -1024, 1023);
				addWH(2, 3, 0, 1023);
				addAddress(4);
				addBitmapFormat(5);
				addSpinBox(6, 0, 0xFFFF, "Freq: ", "Edit frequency");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
#endif
		case CMD_LOGO:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_LOGO</b>()<br>"
				"<br>"
				"Play device logo animation."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_COLDSTART:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_COLDSTART</b>()<br>"
				"<br>"
				"Set co-processor engine state to default values."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_GRADCOLOR:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CMD_GRADCOLOR</b>(<i>r</i>, <i>g</i>, <i>b</i>)<br>"
				"<b>rgb</b>: New highlight gradient color, as a 24-bit RGB number. Red is the most "
				"significant 8 bits, blue is the least. So 0xff0000 is bright red.<br>"
				"<br>"
				"Set the 3D button highlight color."));
			if (editor)
			{
				setTitle("CMD_GRADCOLOR");
				addColor(0, 1, 2);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
	}
	else switch (idRight)
	{
		// ******************************************
		case FT800EMU_DL_DISPLAY:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>DISPLAY</b>()<br>"
				"<br>"
				"End the display list. FT800 will ignore all the commands following this command."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_BITMAP_SOURCE:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>BITMAP_SOURCE</b>(<i>addr</i>)<br>"
				"<b>addr</b>: Bitmap address in graphics SRAM FT800, aligned with respect to the bitmap "
				"format.<br>"
				"For example, if the bitmap format is RGB565/ARGB4/ARGB1555, the bitmap "
				"source shall be aligned to 2 bytes.<br>"
				"<br>"
				"Specify the source address of bitmap data in FT800 graphics memory RAM_G."));
			if (editor)
			{
				setTitle("BITMAP_SOURCE");
				addAddress(0);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_CLEAR_COLOR_RGB:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CLEAR_COLOR_RGB</b>(<i>red</i>, <i>green</i>, <i>blue</i>)<br>"
				"<b>red</b>: Red value used when the color buffer is cleared. The initial value is 0<br>"
				"<b>green</b>: Green value used when the color buffer is cleared. The initial value is 0<br>"
				"<b>blue</b>: Blue value used when the color buffer is cleared. The initial value is 0<br>"
				"<br>"
				"Sets the color values used by a following CLEAR."));
			if (editor)
			{
				setTitle("CLEAR_COLOR_RGB");
				addColor(0, 1, 2);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_TAG:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>TAG</b>(<i>s</i>)<br>"
				"<b>s</b>: Tag value. Valid value range is from 1 to 255.<br>"
				"<br>"
				"Attach the tag value for the following graphics objects drawn on the screen. The initial "
				"tag buffer value is 255."));
			if (editor)
			{
				setTitle("TAG");
				addSpinBox(0, 0, 255, "Value: ", "Set value");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_COLOR_RGB:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>COLOR_RGB</b>(<i>red</i>, <i>green</i>, <i>blue</i>)<br>"
				"<b>red</b>: Red value for the current color. The initial value is 255<br>"
				"<b>green</b>: Green value for the current color. The initial value is 255<br>"
				"<b>blue</b>: Blue value for the current color. The initial value is 255<br>"
				"<br>"
				"Sets red, green and blue values of the FT800 color buffer which will be applied to the "
				"following draw operation."));
			if (editor)
			{
				setTitle("COLOR_RGB");
				addColor(0, 1, 2);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_BITMAP_HANDLE:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
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
			if (editor)
			{
				setTitle("BITMAP_HANDLE");
				addHandle(0);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_CELL:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CELL</b>(<i>cell</i>)<br>"
				"<b>cell</b>: Bitmap cell number. The initial value is 0<br>"
				"<br>"
				"Specify the bitmap cell number for the VERTEX2F command."));
			if (editor)
			{
				setTitle("CELL");
				addCell(0);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_BITMAP_LAYOUT:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>BITMAP_LAYOUT</b>(<i>format</i>, <i>linestride</i>, <i>height</i>)<br>"
				"<b>format</b>: Bitmap pixel format. The bitmap formats supported are L1, L4, L8, RGB332, ARGB2, ARGB4, ARGB1555, "
				"RGB565 and PALETTED.<br>"
				"<b>linestride</b>: Bitmap linestride, in bytes. For L1 format, the line stride must be a multiple of 8 bits; For L4 format the line "
				"stride must be multiple of 2 nibbles. (Aligned to byte).<br>"
				"<b>height</b>: Bitmap height, in lines<br>"
				"<br>"
				"Specify the source bitmap memory format and layout for the current handle."));
			if (editor)
			{
				setTitle("BITMAP_LAYOUT");
				addBitmapFormat(0);
				addSpinBox(1, 0, 0x7FFFFFFF, "Stride: ", "Set bitmap line stride");
				addSpinBox(2, 0, 0x7FFFFFFF, "Height: ", "Set bitmap layout height");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_BITMAP_SIZE:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>BITMAP_SIZE</b>(<i>filter</i>, <i>wrapx</i>, <i>wrapy</i>, <i>width</i>, <i>height</i>)<br>"
				"<b>filter</b>: Bitmap filtering mode, one of NEAREST or BILINEAR<br>"
				"<b>wrapx</b>: Bitmap x wrap mode, one of REPEAT or BORDER<br>"
				"<b>wrapy</b>: Bitmap y wrap mode, one of REPEAT or BORDER<br>"
				"<b>width</b>: Drawn bitmap width, in pixels<br>"
				"<b>height</b>: Drawn bitmap height, in pixels<br>"
				"<br>"
				"Specify the screen drawing of bitmaps for the current handle."));
			if (editor)
			{
				setTitle("BITMAP_SIZE");
				addBitmapFilter(0);
				addBitmapWrap(1, "Wrap X: ", "Set bitmap wrap x");
				addBitmapWrap(2, "Wrap Y: ", "Set bitmap wrap y");
				addSpinBox(3, 0, 0x7FFFFFFF, "Width: ", "Set bitmap width");
				addSpinBox(4, 0, 0x7FFFFFFF, "Height: ", "Set bitmap height");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_ALPHA_FUNC:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>ALPHA_FUNC</b>(<i>func</i>, <i>ref</i>)<br>"
				"<b>func</b>: Specifies the test function, one of NEVER, LESS, LEQUAL, GREATER, GEQUAL, "
				"EQUAL, NOTEQUAL, or ALWAYS. The initial value is ALWAYS (7)<br>"
				"<b>ref</b>: Specifies the reference value for the alpha test. The initial value is 0<br>"
				"<br>"
				"Specify the alpha test function."));
			if (editor)
			{
				setTitle("ALPHA_FUNC");
				addCompareFunction(0);
				addSpinBox(1, 0, 255, "Ref: ", "Set ref");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_STENCIL_FUNC:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>STENCIL_FUNC</b>(<i>func</i>, <i>ref</i>, <i>mask</i>)<br>"
				"<b>func</b>: Specifies the test function, one of NEVER, LESS, LEQUAL, GREATER, GEQUAL, "
				"EQUAL, NOTEQUAL, or ALWAYS. The initial value is ALWAYS. <br>"
				"<b>ref</b>: Specifies the reference value for the stencil test. The initial value is 0<br>"
				"<b>mask</b>: Specifies a mask that is ANDed with the reference value and the stored stencil "
				"value. The initial value is 255<br>"
				"<br>"
				"Set function and reference value for stencil testing."));
			if (editor)
			{
				setTitle("STENCIL_FUNC");
				addCompareFunction(0);
				addSpinBox(1, 0, 255, "Ref: ", "Set ref");
				addSpinBox(2, 0, 255, "Mask: ", "Set mask");
				addByteFlag(2, "Set mask");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_BLEND_FUNC:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>BLEND_FUNC</b>(<i>src</i>, <i>dst</i>)<br>"
				"<b>src</b>: Specifies how the source blending factor is computed. One of ZERO, ONE, SRC_ALPHA, DST_ALPHA, ONE_MINUS_SRC_ALPHA or ONE_MINUS_DST_ALPHA. The initial value is SRC_ALPHA (2).<br>"
				"<b>dst</b>: Specifies how the destination blending factor is computed, one of the same "
				"constants as src. The initial value is ONE_MINUS_SRC_ALPHA(4)<br>"
				"<br>"
				"Specify pixel arithmetic."));
			if (editor)
			{
				setTitle("BLEND_FUNC");
				addBlendFunction(0, "Src: ", "Set blend src");
				addBlendFunction(1, "Dst: ", "Set blend dst");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_STENCIL_OP:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>STENCIL_OP</b>(<i>sfail</i>, <i>spass</i>)<br>"
				"<b>sfail</b>: Specifies the action to take when the stencil test fails, one of KEEP, ZERO, "
				"REPLACE, INCR, DECR, and INVERT. The initial value is KEEP (1)<br>"
				"<b>spass</b>: Specifies the action to take when the stencil test passes, one of the same "
				"constants as sfail. The initial value is KEEP (1)<br>"
				"<br>"
				"Set stencil test actions."));
			if (editor)
			{
				setTitle("STENCIL_OP");
				addStencilOperation(0, "Fail: ", "Set stencil fail");
				addStencilOperation(1, "Pass: ", "Set stencil pass");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_POINT_SIZE:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>POINT_SIZE</b>(<i>size</i>)<br>"
				"<b>size</b>: Point radius in 1/16 pixel. The initial value is 16.<br>"
				"<br>"
				"Sets the size of drawn points. The width is the distance from the center of the point "
				"to the outermost drawn pixel, in units of 1/16 pixels. The valid range is from 16 to "
				"8191 with respect to 1/16th pixel unit."));
			if (editor)
			{
				setTitle("POINT_SIZE");
				addWidth16(0, 0, 8191, true);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_LINE_WIDTH:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>LINE_WIDTH</b>(<i>width</i>)<br>"
				"<b>width</b>: Line width in 1/16 pixel. The initial value is 16.<br>"
				"<br>"
				"Sets the width of drawn lines. The width is the distance from the center of the line to "
				"the outermost drawn pixel, in units of 1/16 pixel. The valid range is from 16 to 4095 "
				"in terms of 1/16th pixel units."));
			if (editor)
			{
				setTitle("LINE_WIDTH");
				addWidth16(0, 0, 8191, false);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_CLEAR_COLOR_A:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CLEAR_COLOR_A</b>(<i>alpha</i>)<br>"
				"<b>alpha</b>: Alpha value used when the color buffer is cleared. The initial value is 0.<br>"
				"<br>"
				"Specify clear value for the alpha channel."));
			if (editor)
			{
				setTitle("CLEAR_COLOR_A");
				addAlpha(0);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_COLOR_A:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>COLOR_A</b>(<i>alpha</i>)<br>"
				"<b>alpha</b>: Alpha for the current color. The initial value is 255<br>"
				"<br>"
				"Set the current color alpha."));
			if (editor)
			{
				setTitle("COLOR_A");
				addAlpha(0);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_CLEAR_STENCIL:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CLEAR_STENCIL</b>(<i>s</i>)<br>"
				"<b>s</b>: Value used when the stencil buffer is cleared. The initial value is 0<br>"
				"<br>"
				"Specify clear value for the stencil buffer."));
			if (editor)
			{
				setTitle("CLEAR_STENCIL");
				addSpinBox(0, 0, 255, "Value: ", "Set value");
				addByteFlag(0, "Set value");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_CLEAR_TAG:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CLEAR_TAG</b>(<i>s</i>)<br>"
				"<b>s</b>: Value used when the tag buffer is cleared. The initial value is 0<br>"
				"<br>"
				"Specify clear value for the tag buffer."));
			if (editor)
			{
				setTitle("CLEAR_TAG");
				addSpinBox(0, 0, 255, "Value: ", "Set value");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_STENCIL_MASK:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>STENCIL_MASK</b>(<i>mask</i>)<br>"
				"<b>mask</b>: The mask used to enable writing stencil bits. The initial value is 255<br>"
				"<br>"
				"Control the writing of individual bits in the stencil planes."));
			if (editor)
			{
				setTitle("STENCIL_MASK");
				addSpinBox(0, 0, 255, "Mask: ", "Set mask");
				InteractiveProperties::addByteFlag(0, "Set mask");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_TAG_MASK:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>TAG_MASK</b>(<i>mask</i>)<br>"
				"<b>mask</b>: Allow updates to the tag buffer. The initial value is one and it means the tag "
				"buffer of the FT800 is updated with the value given by the TAG command. "
				"Therefore, the following graphics objects will be attached to the tag value "
				"given by the TAG command.<br>"
				"The value zero means the tag buffer of the FT800 is set as the default value,"
				"rather than the value given by TAG command in the display list.<br>"
				"<br>"
				"Control the writing of the tag buffer."));
			if (editor)
			{
				setTitle("TAG_MASK");
				InteractiveProperties::addCheckBox(0, "Mask: ", "Set mask");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_BITMAP_TRANSFORM_A:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>BITMAP_TRANSFORM_A</b>(<i>a</i>)<br>"
				"<b>a</b>: Coefficient A of the bitmap transform matrix, in signed 8.8 bit fixed-point "
				"form. The initial value is 256<br>"
				"<br>"
				"Specify the A coefficient of the bitmap transform matrix."));
			if (editor)
			{
				setTitle("BITMAP_TRANSFORM_A");
				addSpinBox256(0, 0xFFFF0000, 0xFFFF, "A: ", "Set transform a");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_BITMAP_TRANSFORM_B:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>BITMAP_TRANSFORM_B</b>(<i>b</i>)<br>"
				"<b>b</b>: Coefficient B of the bitmap transform matrix, in signed 8.8 bit fixed-point "
				"form. The initial value is 0<br>"
				"<br>"
				"Specify the B coefficient of the bitmap transform matrix."));
			if (editor)
			{
				setTitle("BITMAP_TRANSFORM_B");
				addSpinBox256(0, 0xFFFF0000, 0xFFFF, "B: ", "Set transform b");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_BITMAP_TRANSFORM_C:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>BITMAP_TRANSFORM_C</b>(<i>c</i>)<br>"
				"<b>c</b>: Coefficient C of the bitmap transform matrix, in signed 15.8 bit fixed-point "
				"form. The initial value is 0<br>"
				"<br>"
				"Specify the C coefficient of the bitmap transform matrix."));
			if (editor)
			{
				setTitle("BITMAP_TRANSFORM_C");
				addSpinBox256(0, 0xFF800000, 0x7FFFFF, "C: ", "Set transform c");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_BITMAP_TRANSFORM_D:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>BITMAP_TRANSFORM_D</b>(<i>d</i>)<br>"
				"<b>d</b>: Coefficient D of the bitmap transform matrix, in signed 8.8 bit fixed-point "
				"form. The initial value is 0<br>"
				"<br>"
				"Specify the D coefficient of the bitmap transform matrix."));
			if (editor)
			{
				setTitle("BITMAP_TRANSFORM_D");
				addSpinBox256(0, 0xFFFF0000, 0xFFFF, "D: ", "Set transform d");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_BITMAP_TRANSFORM_E:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>BITMAP_TRANSFORM_E</b>(<i>e</i>)<br>"
				"<b>e</b>: Coefficient E of the bitmap transform matrix, in signed 8.8 bit fixed-point "
				"form. The initial value is 256<br>"
				"<br>"
				"Specify the E coefficient of the bitmap transform matrix."));
			if (editor)
			{
				setTitle("BITMAP_TRANSFORM_E");
				addSpinBox256(0, 0xFFFF0000, 0xFFFF, "E: ", "Set transform e");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_BITMAP_TRANSFORM_F:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>BITMAP_TRANSFORM_F</b>(<i>f</i>)<br>"
				"<b>f</b>: Coefficient F of the bitmap transform matrix, in signed 15.8 bit fixed-point "
				"form. The initial value is 0<br>"
				"<br>"
				"Specify the F coefficient of the bitmap transform matrix."));
			if (editor)
			{
				setTitle("BITMAP_TRANSFORM_F");
				addSpinBox256(0, 0xFF800000, 0x7FFFFF, "F: ", "Set transform f");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_SCISSOR_XY:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>SCISSOR_XY</b>(<i>x</i>, <i>y</i>)<br>"
				"<b>x</b>: The x coordinate of the scissor clip rectangle, in pixels. The initial value is 0<br>"
				"<b>y</b>: The y coordinate of the scissor clip rectangle, in pixels. The initial value is 0<br>"
				"<br>"
				"Sets the top-left position of the scissor clip rectangle, which limits the drawing area."));
			if (editor)
			{
				setTitle("SCISSOR_XY");
				addXY(0, 1, 0, 512);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_SCISSOR_SIZE:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>SCISSOR_SIZE</b>(<i>width</i>, <i>height</i>)<br>"
				"<b>width</b>: The width of the scissor clip rectangle, in pixels. The initial value is 512. "
				"The valid value range is from 0 to 512.<br>"
				"<b>height</b>: The height of the scissor clip rectangle, in pixels. The initial value is 512. "
				"The valid value range is from 0 to 512.<br>"
				"<br>"
				"Sets the width and height of the scissor clip rectangle, which limits the drawing area."));
			if (editor)
			{
				setTitle("SCISSOR_SIZE");
				addWH(0, 1, 0, 512);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_CALL:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>CALL</b>(<i>dest</i>)<br>"
				"<b>dest</b>: The destination address in RAM_DL which the display command is to be "
				"switched. FT800 has the stack to store the return address. To come back to "
				"the next command of source address, the RETURN command can help.<br>"
				"<br>"
				"Execute a sequence of commands at another location in the display list<br>"
				"<br>"
				"CALL and RETURN have a 4 level stack in addition to the current pointer. Any "
				"additional CALL/RETURN done will lead to unexpected behavior."));
			if (editor)
			{
				setTitle("CALL");
				addSpinBox(0, 0, 2047, "Dest: ", "Set dest");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_JUMP:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>JUMP</b>(<i>dest</i>)<br>"
				"<b>dest</b>: Display list address to be jumped.<br>"
				"<br>"
				"Execute commands at another location in the display list."));
			if (editor)
			{
				setTitle("JUMP");
				addSpinBox(0, 0, 2047, "Dest: ", "Set dest");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_BEGIN:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>BEGIN</b>(<i>prim</i>)<br>"
				"<b>prim</b>: Graphics primitive. The valid value is defined as: BITMAPS, POINTS, LINES, LINE_STRIP, EDGE_STRIP_R, EDGE_STRIP_L, EDGE_STRIP_A, EDGE_STRIP_B, RECTS<br>"
				"<br>"
				"Begin drawing a graphics primitive."));
			if (editor)
			{
				setTitle("BEGIN");
				addPrimitive(0);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_COLOR_MASK:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
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
			if (editor)
			{
				setTitle("COLOR_MASK");
				addCheckBox(0, "R: ", "Set red mask");
				addCheckBox(1, "G: ", "Set green mask");
				addCheckBox(2, "B: ", "Set blue mask");
				addCheckBox(3, "A: ", "Set alpha mask");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_END:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>END</b>()<br>"
				"<br>"
				"End drawing a graphics primitive.<br>"
				"<br>"
				"It is recommended to have an END for each BEGIN. Whereas advanced users can "
				"avoid the usage of END in order to save extra graphics instructions in the display list "
				"RAM."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_SAVE_CONTEXT:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>SAVE_CONTEXT</b>()<br>"
				"<br>"
				"Push the current graphics context on the context stack.<br>"
				"<br>"
				"Any extra SAVE_CONTEXT will throw away the earliest saved context."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_RESTORE_CONTEXT:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>RESTORE_CONTEXT</b>()<br>"
				"<br>"
				"Restore the current graphics context from the context stack.<br>"
				"<br>"
				"Any extra RESTORE_CONTEXT will load the default values into the present context."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_RETURN:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>RETURN</b>()<br>"
				"<br>"
				"Return from a previous CALL command.<br>"
				"<br>"
				"CALL and RETURN have 4 levels of stack in addition to the current pointer. Any additional CALL/RETURN done will lead to unexpected behavior."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_MACRO:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
				"<b>MACRO</b>(<i>m</i>)<br>"
				"<b>m</b>: Macro register to read. Value 0 means the FT800 will fetch the command "
				"from REG_MACRO_0 to execute. Value 1 means the FT800 will fetch the "
				"command from REG_MACRO_1 to execute. The content of REG_MACRO_0 or "
				"REG_MACRO_1 shall be a valid display list command, otherwise the behavior "
				"is undefined.<br>"
				"<br>"
				"Execute a single command from a macro register."));
			if (editor)
			{
				setTitle("MACRO");
				addSpinBox(0, 0, 1, "Macro: ", "Set macro");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FT800EMU_DL_CLEAR:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr(
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
			if (editor)
			{
				setTitle("CLEAR");
				addCheckBox(0, "Color: ", "Set color clear flag");
				addCheckBox(1, "Stencil: ", "Set stencil clear flag");
				addCheckBox(2, "Tag: ", "Set tag clear flag");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
	}
	if (!ok)
	{
		m_MainWindow->propertiesEditor()->setInfo(tr("</i>Not yet implemented.</i>"));
		m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
	}
	if (!editor)
	{
		m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
	}
}

void InteractiveProperties::modifiedEditorLine()
{
	// printf("InteractiveProperties::modifiedEditorLine()\n");

	for (std::vector<PropertiesWidget *>::iterator it(m_CurrentProperties.begin()), end(m_CurrentProperties.end()); it != end; ++it)
	{
		(*it)->modifiedEditorLine();
	}
}

} /* namespace FT800EMUQT */

/* end of file */