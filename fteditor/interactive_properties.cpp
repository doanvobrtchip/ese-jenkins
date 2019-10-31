/*
Copyright (C) 2013-2015  Future Technology Devices International Ltd
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#include "interactive_properties.h"

// STL includes
#include <stdio.h>

// Qt includes
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QSpinBox>
#include <QLabel>

// Emulator includes

// Project includes
#include "main_window.h"
#include "dl_editor.h"
#include "properties_editor.h"
#include "interactive_widgets.h"
#include "constant_mapping.h"
#include "constant_common.h"

namespace FTEDITOR {

static int s_CoordMin[FTEDITOR_DEVICE_NB] = {
	-1024,
	-1024,
	-4096,
	-4096,
	-4096,
	-4096,
	-4096,
	-4096,
};

static int s_CoordMax[FTEDITOR_DEVICE_NB] = {
	1023,
	1023,
	4095,
	4095,
	4095,
	4095,
	4095,
	4095,
};

static int s_ScreenCoordWHMax[FTEDITOR_DEVICE_NB] = {
	512,
	512,
	2048,
	2048,
	2048,
	2048,
	2048,
	2048,
};

#define FTEDITOR_COORD_MIN s_CoordMin[FTEDITOR_CURRENT_DEVICE]
#define FTEDITOR_COORD_MAX s_CoordMax[FTEDITOR_CURRENT_DEVICE]
#define FTEDITOR_SCREENCOORDWH_MIN 0
#define FTEDITOR_SCREENCOORDWH_MAX s_ScreenCoordWHMax[FTEDITOR_CURRENT_DEVICE]
#define FTEDITOR_SCREENCOORDXY_MIN 0
#define FTEDITOR_SCREENCOORDXY_MAX (FTEDITOR_SCREENCOORDWH_MAX - 1)
#define FTEDITOR_BITMAPHANDLE_MAX 15
#define FTEDITOR_FONTHANDLE_MAX 31

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

void InteractiveProperties::addXY(int x, int y, int minim, int maxim, QString label)
{
	PropertiesSpinBox *propX = new PropertiesSpinBox(this, "Set x position", x);
	propX->setMinimum(minim);
	propX->setMaximum(maxim);
	PropertiesSpinBox *propY = new PropertiesSpinBox(this, "Set y position", y);
	propY->setMinimum(minim);
	propY->setMaximum(maxim);
	addLabeledWidget(label, propX, propY);
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

void InteractiveProperties::addXYVertexFormat(int x, int y, int minim, int maxim)
{
	PropertiesSpinBoxVertexFormat *propX = new PropertiesSpinBoxVertexFormat(this, "Set x position", x);
	propX->setMinimum(minim);
	propX->setMaximum(maxim);
	PropertiesSpinBoxVertexFormat *propY = new PropertiesSpinBoxVertexFormat(this, "Set y position", y);
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

void InteractiveProperties::addOptions(int options, uint32_t flags, bool flatOnly, bool noClock)
{
#define ADD_OPTIONS_CHECKBOX(opt) \
		{ \
			PropertiesCheckBox *chb = new PropertiesCheckBox(this, "Set " #opt, options, opt); \
			addLabeledWidget(#opt ": ", chb); \
			m_CurrentProperties.push_back(chb); \
		}
	/*
	#define OPT_MONO             1UL
	#define OPT_NODL             2UL
	#define OPT_FLASH            64UL -- BT815
	#define OPT_SIGNED           256UL <- special case (when CMD_NUMBER, probably whenever no NOBACK)
	#define OPT_FLAT             256UL
	#define OPT_CENTERX          512UL
	#define OPT_CENTERY          1024UL
	#define OPT_CENTER           1536UL ----
	#define OPT_RIGHTX           2048UL
	#define OPT_NOBACK           4096UL
	#define OPT_FILL             8192UL -- BT815
	#define OPT_NOTICKS          8192UL
	#define OPT_NOHM             16384UL
	#define OPT_NOPOINTER        16384UL <- special case (when NOHM this but not NOSECS)
	#define OPT_NOSECS           32768UL
	#define OPT_NOHANDS          49152UL ---- */
	if (flags & OPT_MONO)
	{
		ADD_OPTIONS_CHECKBOX(OPT_MONO);
	}
	if (flags & OPT_NODL)
	{
		ADD_OPTIONS_CHECKBOX(OPT_NODL);
	}
	if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815)
	{
		if (flags & OPT_FLASH)
		{
			ADD_OPTIONS_CHECKBOX(OPT_FLASH);
		}
        if (flags & OPT_OVERLAY)
        {
            ADD_OPTIONS_CHECKBOX(OPT_OVERLAY);
        }
	}
	if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810)
	{
		if (flags & OPT_NOTEAR)
		{
			ADD_OPTIONS_CHECKBOX(OPT_NOTEAR);
		}
		if (flags & OPT_FULLSCREEN)
		{
			ADD_OPTIONS_CHECKBOX(OPT_FULLSCREEN);
		}
		if (flags & OPT_MEDIAFIFO)
		{
			ADD_OPTIONS_CHECKBOX(OPT_MEDIAFIFO);
		}
		if (flags & OPT_SOUND)
		{
			ADD_OPTIONS_CHECKBOX(OPT_SOUND);
		}
	}
	if (flags & OPT_FLAT)
	{
		if (flags & OPT_NOBACK || flatOnly)
		{
			ADD_OPTIONS_CHECKBOX(OPT_FLAT);

			if (!noClock && flags & OPT_NOBACK)
			{
				ADD_OPTIONS_CHECKBOX(OPT_NOBACK);
			}
		}
		else
		{
			ADD_OPTIONS_CHECKBOX(OPT_SIGNED);
		}
	}
	if (flags & OPT_CENTERX)
	{
		ADD_OPTIONS_CHECKBOX(OPT_CENTERX);
	}
	if (flags & OPT_CENTERY)
	{
		ADD_OPTIONS_CHECKBOX(OPT_CENTERY);
	}
	if (flags & OPT_RIGHTX)
	{
		ADD_OPTIONS_CHECKBOX(OPT_RIGHTX);
	}
	if (noClock)
	{
		if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815)
		{
			if (flags & OPT_FORMAT)
			{
				ADD_OPTIONS_CHECKBOX(OPT_FORMAT);
			}
			if (flags & OPT_FILL)
			{
				ADD_OPTIONS_CHECKBOX(OPT_FILL);
			}
		}
	}
	else
	{
		if (flags & OPT_NOTICKS)
		{
			ADD_OPTIONS_CHECKBOX(OPT_NOTICKS);
		}
	}
	if (flags & OPT_NOHM)
	{
		if (flags & OPT_NOSECS)
		{
			ADD_OPTIONS_CHECKBOX(OPT_NOHM);
			ADD_OPTIONS_CHECKBOX(OPT_NOSECS);
		}
		else
		{
			ADD_OPTIONS_CHECKBOX(OPT_NOPOINTER);
		}
	}
	#undef ADD_OPTIONS_CHECKBOX
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

void InteractiveProperties::addStream(int stream)
{
	// TODO: More appropriate GUI
	PropertiesLineEdit *propText = new PropertiesLineEdit(this, "Set stream", stream);
	addLabeledWidget("Stream: ", propText);
	m_CurrentProperties.push_back(propText);

	// Help message
	addHelp(tr("<p><i>Direct filepath</i>:<br>Enter a relative or absolute filepath. String formatted in backslash escape format, use \\\\ for regular backslash.</p>"
		"<p><i>Content Manager</i>:<br>Enter the full content name suffixed with .raw or .bin (eg. content/catvideo.raw) for raw or compressed format respectively.</p>"));
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

void InteractiveProperties::addSpinBox16(int index, int minim, int maxim, const QString &label, const QString &undoMessage)
{
	PropertiesSpinBox16 *prop = new PropertiesSpinBox16(this, undoMessage, index);
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

void InteractiveProperties::addSpinBoxForBitmapTransform(int index, int minim, int maxim, const QString &label, const QString &undoMessage)
{
	PropertiesSpinBoxForBitmapTransform *prop = new PropertiesSpinBoxForBitmapTransform(this, undoMessage, index);
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
	prop->setSingleStep(65536 / 360.0);
	addLabeledWidget(label, prop);
	m_CurrentProperties.push_back(prop);
	prop->done();
}

void InteractiveProperties::addSpinBoxVertexFormat(int index, int minim, int maxim, const QString &label, const QString &undoMessage)
{
	PropertiesSpinBoxVertexFormat *prop = new PropertiesSpinBoxVertexFormat(this, undoMessage, index);
	prop->setMinimum(minim);
	prop->setMaximum(maxim);
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

void InteractiveProperties::addColorHex(int rgb)
{
	PropertiesColorHex *prop = new PropertiesColorHex(this, "Set color", rgb);
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

void InteractiveProperties::addAlphaHex(int alpha)
{
	PropertiesSpinBoxAlphaHex *prop = new PropertiesSpinBoxAlphaHex(this, tr("Set alpha"), alpha);
	addLabeledWidget(tr("Alpha") + ": ", prop);
	m_CurrentProperties.push_back(prop);
	prop->done();
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

QComboBox *InteractiveProperties::addComboBox(int index, const char **items, int nb, const QString &label, const QString &undoMessage)
{
	return addComboBox(index, items, 0, nb, label, undoMessage);
}

QComboBox *InteractiveProperties::addComboBox(int index, const char **items, int begin, int end, const QString &label, const QString &undoMessage)
{
	PropertiesComboBox *prop = new PropertiesComboBox(this, undoMessage, index, begin);
	for (int i = begin; i < end; ++i)
		prop->addItem(items[i]);
	addLabeledWidget(label, prop);
	m_CurrentProperties.push_back(prop);
	prop->ready();
	return prop;
}

QComboBox *InteractiveProperties::addComboBox(int index, const int *toEnum, int toEnumSz, const int *toIntf, const char **toString, int toIntfStringSz, const QString &label, const QString &undoMessage)
{
	PropertiesRemapComboBox *prop = new PropertiesRemapComboBox(this, undoMessage, index, toIntf, toIntfStringSz, toEnum, toEnumSz);
	for (int i = 0; i < toEnumSz; ++i)
		prop->addItem(toString[toEnum[i]]);
	addLabeledWidget(label, prop);
	m_CurrentProperties.push_back(prop);
	prop->ready();
	return prop;
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
	addComboBox(format, 
		g_BitmapFormatFromIntf[FTEDITOR_CURRENT_DEVICE],
		g_BitmapFormatIntfNb[FTEDITOR_CURRENT_DEVICE],
		g_BitmapFormatToIntf[FTEDITOR_CURRENT_DEVICE],
		g_BitmapFormatToString[FTEDITOR_CURRENT_DEVICE],
		g_BitmapFormatEnumNb[FTEDITOR_CURRENT_DEVICE],
		tr("Format") + ": ", tr("Set bitmap format"));
}

void InteractiveProperties::addExtFormat(int format)
{
	if (!g_ExtFormatIntfNb[FTEDITOR_CURRENT_DEVICE])
	{
		addBitmapFormat(format);
		return;
	}
	QComboBox *comboBox = addComboBox(format,
		g_ExtFormatFromIntf[FTEDITOR_CURRENT_DEVICE],
		g_ExtFormatIntfNb[FTEDITOR_CURRENT_DEVICE],
		g_ExtFormatToIntf[FTEDITOR_CURRENT_DEVICE],
		g_BitmapFormatToString[FTEDITOR_CURRENT_DEVICE],
		g_BitmapFormatEnumNb[FTEDITOR_CURRENT_DEVICE],
		tr("Format") + ": ", tr("Set extended bitmap format"));
	for (int i = 0; i < comboBox->count(); ++i)
		comboBox->setItemText(i, comboBox->itemText(i).replace("COMPRESSED_RGBA_", ""));
}

void InteractiveProperties::addSwizzle(int swizzle, const QString &label, const QString &undoMessage)
{
	addComboBox(swizzle, g_DlEnumSwizzle, DL_ENUM_SWIZZLE_NB, label, undoMessage);
}

void InteractiveProperties::addBitmapWrap(int wrap, const QString &label, const QString &undoMessage)
{
	addComboBox(wrap, g_DlEnumBitmapWrap, DL_ENUM_BITMAP_WRAP_NB, label, undoMessage);
}

void InteractiveProperties::addBitmapFilter(int filter)
{
	addComboBox(filter, g_DlEnumBitmapFilter, DL_ENUM_BITMAP_FILTER_NB, tr("Filter") + ": ", tr("Set bitmap filter"));
}

void InteractiveProperties::addAnimLoop(int loop)
{
	addComboBox(loop, g_DlEnumAnimLoop, DL_ENUM_BITMAP_FILTER_NB, tr("Loop") + ": ", tr("Set animation loop"));
}

void InteractiveProperties::addAddressFlashOpt(int address, bool negative)
{
	; {
		PropertiesSpinBoxAddressFlashOpt *prop = new PropertiesSpinBoxAddressFlashOpt(this, tr("Set address"), address, negative);
		addLabeledWidget("Address: ", prop);
		m_CurrentProperties.push_back(prop);
		prop->done();
	}
	; {
		PropertiesCheckBox *prop = new PropertiesCheckBox(this, tr("Set flash"), address, 0x800000);
		addLabeledWidget("Flash: ", prop);
		m_CurrentProperties.push_back(prop);
	}
}

void InteractiveProperties::addAddressFlash(int address)
{
	PropertiesSpinBoxAddressFlash *prop = new PropertiesSpinBoxAddressFlash(this, tr("Set flash address"), address);
	addLabeledWidget("Flash address: ", prop);
	m_CurrentProperties.push_back(prop);
	prop->done();
}

void InteractiveProperties::addAddress(int address, bool negative)
{
	PropertiesSpinBoxAddress *prop = new PropertiesSpinBoxAddress(this, tr("Set address"), address, negative);
	addLabeledWidget("Address: ", prop);
	m_CurrentProperties.push_back(prop);
	prop->done();
}

void InteractiveProperties::addMemorySize(int size)
{
	PropertiesSpinBoxAddress *prop = new PropertiesSpinBoxAddress(this, tr("Set size"), size, false);
	addLabeledWidget("Size: ", prop);
	m_CurrentProperties.push_back(prop);
	prop->done();
}

void InteractiveProperties::addCaptureButton(const QString &text, const QString &undoMessage)
{
	PropertiesCaptureButton *prop = new PropertiesCaptureButton(this, text, undoMessage);
	((QVBoxLayout *)layout())->addWidget(prop);
	m_CurrentWidgets.push_back(prop);
	m_CurrentProperties.push_back(prop);
}

void InteractiveProperties::addHelp(const QString &text)
{
	QLabel *prop = new QLabel();
	prop->setTextFormat(Qt::RichText);
	prop->setWordWrap(true);
	prop->setText(text);
	((QVBoxLayout *)layout())->addWidget(prop);
	m_CurrentWidgets.push_back(prop);
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

void InteractiveProperties::bindCurrentDevice()
{ 
	if (m_MainWindow->propertiesEditor()->getEditWidgetSetter() == m_LineEditor)
		setEditorLine(m_LineEditor, m_LineNumber); 
}

void InteractiveProperties::setProperties(int idLeft, int idRight, DlEditor *editor)
{
	bool ok = false;
	// const uint32_t *p = m_DisplayListParsed[i].Parameter;
	if (idLeft == FTEDITOR_DL_VERTEX2F)
	{
		m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_VERTEX2F."));
		if (editor)
		{
			setTitle("VERTEX2F");
			addXYVertexFormat(0, 1, -16384, 16383);
			m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
		}
		ok = true;
	}
	else if (idLeft == FTEDITOR_DL_VERTEX2II)
	{
		m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_VERTEX2II."));
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
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_DLSTART."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_SWAP:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_SWAP."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_INTERRUPT:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_INTERRUPT."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_BGCOLOR:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_BGCOLOR."));
			if (editor)
			{
				setTitle("CMD_BGCOLOR");
				addColorHex(0);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_FGCOLOR:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_FGCOLOR."));
			if (editor)
			{
				setTitle("CMD_FGCOLOR");
				addColorHex(0);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_GRADIENT:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_GRADIENT."));
			if (editor)
			{
				setTitle("CMD_GRADIENT");
				addXY(0, 1, FTEDITOR_COORD_MIN, FTEDITOR_COORD_MAX);
				addColorHex(2);
				addXY(3, 4, FTEDITOR_COORD_MIN, FTEDITOR_COORD_MAX);
				addColorHex(5);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_TEXT:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_TEXT."));
			if (editor)
			{
				setTitle("CMD_TEXT");
				addXY(0, 1, FTEDITOR_COORD_MIN, FTEDITOR_COORD_MAX);
				addHandle(2, true);
				addOptions(3, OPT_CENTER | OPT_RIGHTX | OPT_FILL | OPT_FORMAT, false, true);
				addText(4);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_BUTTON:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_BUTTON."));
			if (editor)
			{
				setTitle("CMD_BUTTON");
				addXY(0, 1, FTEDITOR_COORD_MIN, FTEDITOR_COORD_MAX);
				addWH(2, 3, 0, 1023);
				addHandle(4, true);
				addOptions(5, OPT_FLAT | OPT_FILL | OPT_FORMAT, true, true);
				addText(6);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_KEYS:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_KEYS."));
			if (editor)
			{
				setTitle("CMD_KEYS");
				addXY(0, 1, FTEDITOR_COORD_MIN, FTEDITOR_COORD_MAX);
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
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_PROGRESS."));
			if (editor)
			{
				setTitle("CMD_PROGRESS");
				addXY(0, 1, FTEDITOR_COORD_MIN, FTEDITOR_COORD_MAX);
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
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_SLIDER."));
			if (editor)
			{
				setTitle("CMD_SLIDER");
				addXY(0, 1, FTEDITOR_COORD_MIN, FTEDITOR_COORD_MAX);
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
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_SCROLLBAR."));
			if (editor)
			{
				setTitle("CMD_SCROLLBAR");
				addXY(0, 1, FTEDITOR_COORD_MIN, FTEDITOR_COORD_MAX);
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
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_TOGGLE."));
			if (editor)
			{
				setTitle("CMD_TOGGLE");
				addXY(0, 1, FTEDITOR_COORD_MIN, FTEDITOR_COORD_MAX);
				addWidth(2, 0, 1023);
				addHandle(3, true);
				addOptions(4, OPT_FLAT | OPT_FORMAT, false, true);
				addValueSlider(5, 0xFFFF);
				addText(6); // TODO: Separate
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_GAUGE:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_GAUGE"));
			if (editor)
			{
				setTitle("CMD_GAUGE");
				addXY(0, 1, FTEDITOR_COORD_MIN, FTEDITOR_COORD_MAX);
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
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_CLOCK."));
			if (editor)
			{
				setTitle("CMD_CLOCK");
				addXY(0, 1, FTEDITOR_COORD_MIN, FTEDITOR_COORD_MAX);
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
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_CALIBRATE."));
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
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_SPINNER."));
			if (editor)
			{
				setTitle("CMD_SPINNER");
				addXY(0, 1, FTEDITOR_COORD_MIN, FTEDITOR_COORD_MAX);
				addSpinBox(2, 0, 3, "Style: ", "Edit style"); // TODO: ComboBox
				addSpinBox(3, 0, 1023, "Scale: ", "Edit scale");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_STOP:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_STOP."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_MEMSET:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_MEMSET."));
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
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_MEMZERO."));
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
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_MEMCPY."));
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
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_APPEND."));
			if (editor)
			{
				setTitle("CMD_APPEND");
				addAddress(0, false);
				addSpinBox(1, 0, 0x7FFFFFFF, "Num: ", "Set num");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_SNAPSHOT:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_SNAPSHOT."));
			if (editor)
			{
				setTitle("CMD_SNAPSHOT");
				addAddress(0, false);
				addCaptureButton(tr("Capture Snapshot"), tr("Capture snapshot"));
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_GETPROPS:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_GETPROPS."));
			if (editor)
			{
				setTitle("CMD_GETPROPS");
				addSpinBox(0, 0, 65535, "Pointer:", "Set pointer");
				addSpinBox(1, 0, 65535, "Width:", "Set width");
				addSpinBox(2, 0, 65535, "Height:", "Set height");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_LOADIMAGE:
		{
			if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810)
				m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_LOADIMAGE_FT810."));
			else
				m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_LOADIMAGE."));
			if (editor)
			{
				setTitle("CMD_LOADIMAGE");
				addAddress(0, false);
				if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815)
					addOptions(1, OPT_NODL | OPT_MONO | OPT_MEDIAFIFO | OPT_FULLSCREEN | OPT_FLASH);
				else
					addOptions(1, OPT_NODL | OPT_MONO | OPT_MEDIAFIFO | OPT_FULLSCREEN);
				addStream(2);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_LOADIDENTITY:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_LOADIDENTITY."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_TRANSLATE:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_TRANSLATE."));
			if (editor)
			{
				setTitle("CMD_TRANSLATE");
				addSpinBox65536(0, 0x80000000, 0x7FFFFFFF, "X: ", tr("Set x translation"));
				addSpinBox65536(1, 0x80000000, 0x7FFFFFFF, "Y: ", tr("Set y translation"));
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_SCALE:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_SCALE."));
			if (editor)
			{
				setTitle("CMD_SCALE");
				addSpinBox65536(0, 0x80000000, 0x7FFFFFFF, "X: ", tr("Set x scale"));
				addSpinBox65536(1, 0x80000000, 0x7FFFFFFF, "Y: ", tr("Set y scale"));
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_ROTATE:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_ROTATE."));
			if (editor)
			{
				setTitle("CMD_ROTATE");
				addSpinBoxAngle65536(0, 0x80000000, 0x7FFFFFFF, tr("Angle") + ": ", tr("Set angle"));
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_SETMATRIX:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_SETMATRIX."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_SETFONT:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_SETFONT."));
			if (editor)
			{
				setTitle("CMD_SETFONT");
				addSpinBox(0, 0, FTEDITOR_FONTHANDLE_MAX, "Font: ", "Set font");
				addAddress(1, true);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_TRACK:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_TRACK."));
			if (editor)
			{
				setTitle("CMD_TRACK");
				addXY(0, 1, FTEDITOR_SCREENCOORDXY_MIN, FTEDITOR_SCREENCOORDXY_MAX);
				addWH(2, 3, FTEDITOR_SCREENCOORDWH_MIN, FTEDITOR_SCREENCOORDWH_MAX);
				addSpinBox(4, 0, 255, "Tag: ", "Set tag");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_DIAL:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_DIAL."));
			if (editor)
			{
				setTitle("CMD_DIAL");
				addXY(0, 1, FTEDITOR_COORD_MIN, FTEDITOR_COORD_MAX);
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
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_NUMBER."));
			if (editor)
			{
				setTitle("CMD_NUMBER");
				addXY(0, 1, FTEDITOR_COORD_MIN, FTEDITOR_COORD_MAX);
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
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_SCREENSAVER."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_SKETCH:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_SKETCH."));
			if (editor)
			{
				setTitle("CMD_SKETCH");
				addXY(0, 1, FTEDITOR_COORD_MIN, FTEDITOR_COORD_MAX);
				addWH(2, 3, 0, 1023);
				addAddress(4, false);
				addBitmapFormat(5);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_CSKETCH:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_CSKETCH."));
			if (editor)
			{
				setTitle("CMD_SKETCH");
				addXY(0, 1, FTEDITOR_COORD_MIN, FTEDITOR_COORD_MAX);
				addWH(2, 3, 0, 1023);
				addAddress(4, false);
				addBitmapFormat(5);
				addSpinBox(6, 0, 0xFFFF, "Freq: ", "Edit frequency");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_LOGO:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_LOGO."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_COLDSTART:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_COLDSTART."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_GRADCOLOR:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_GRADCOLOR."));
			if (editor)
			{
				setTitle("CMD_GRADCOLOR");
				addColorHex(0);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
#if 0
		case CMD_SETROTATE:
		{
			static const char *rotations[] = {
				"Landscape",
				"Inverted Landscape",
				"Portrait",
				"Inverted Portrait",
				"Mirrored Landscape",
				"Mirrored Inverted Landscape",
				"Mirorred Portrait",
				"Mirrored Inverted Portrait",
			};
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_SETROTATE."));
			if (editor)
			{
				setTitle("CMD_SETROTATE");
				addComboBox(0, rotations, 8, "Rotation: ", "Set rotation");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
#endif
		case CMD_SNAPSHOT2:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_SNAPSHOT2."));
			if (editor)
			{
				setTitle("CMD_SNAPSHOT2");
				addComboBox(0, 
					g_SnapshotFormatFromIntf[FTEDITOR_CURRENT_DEVICE],
					g_SnapshotFormatIntfNb[FTEDITOR_CURRENT_DEVICE],
					g_SnapshotFormatToIntf[FTEDITOR_CURRENT_DEVICE],
					g_BitmapFormatToString[FTEDITOR_CURRENT_DEVICE],
					g_BitmapFormatEnumNb[FTEDITOR_CURRENT_DEVICE],
					tr("Format") + ": ", tr("Set snapshot format"));
				addAddress(1, false);
				addXY(2, 3, FTEDITOR_SCREENCOORDXY_MIN, FTEDITOR_SCREENCOORDXY_MAX);
				addWH(4, 5, FTEDITOR_SCREENCOORDWH_MIN, FTEDITOR_SCREENCOORDWH_MAX);
				addCaptureButton(tr("Capture Snapshot"), tr("Capture snapshot"));
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_SETBASE:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_SETBASE."));
			if (editor)
			{
				setTitle("CMD_SETBASE");
				addSpinBox(0, 2, 36, tr("Base") + ": ", tr("Set the base"));
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_MEDIAFIFO:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_MEDIAFIFO."));
			if (editor)
			{
				setTitle("CMD_MEDIAFIFO");
				addAddress(0, false);
				addMemorySize(1);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_PLAYVIDEO:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_PLAYVIDEO."));
			if (editor)
			{
				setTitle("CMD_PLAYVIDEO");
                if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815)
                {
                    addOptions(0, OPT_NOTEAR | OPT_FULLSCREEN | OPT_MEDIAFIFO | OPT_SOUND | OPT_FLASH | OPT_OVERLAY | OPT_NODL);
                }
                else
                {
                    addOptions(0, OPT_NOTEAR | OPT_FULLSCREEN | OPT_MEDIAFIFO | OPT_SOUND);
                }
				addStream(1);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_SETFONT2:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_SETFONT2."));
			if (editor)
			{
				setTitle("CMD_SETFONT2");
				addSpinBox(0, 0, FTEDITOR_FONTHANDLE_MAX, tr("Font") + ": ", tr("Set font"));
				addAddress(1, true);
				addSpinBox(2, 0, 255, tr("First character") + ": ", tr("Set first character"));
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_SETSCRATCH:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_SETSCRATCH."));
			if (editor)
			{
				setTitle("CMD_SETSCRATCH");
				addSpinBox(0, 0, 31, tr("Handle") + ": ", tr("Set handle"));
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		/*
		/*s_CmdIdMap["CMD_INT_RAMSHARED"] = CMD_INT_RAMSHARED & 0xFF;
		s_CmdParamCount[CMD_INT_RAMSHARED & 0xFF] = 0; // undocumented
		s_CmdParamString[CMD_INT_RAMSHARED & 0xFF] = false;*/
		/*s_CmdIdMap["CMD_INT_SWLOADIMAGE"] = CMD_INT_SWLOADIMAGE & 0xFF;
		s_CmdParamCount[CMD_INT_SWLOADIMAGE & 0xFF] = 0; // undocumented
		s_CmdParamString[CMD_INT_SWLOADIMAGE & 0xFF] = false;*/
		case CMD_ROMFONT:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_ROMFONT."));
			if (editor)
			{
				setTitle("CMD_ROMFONT");
				addSpinBox(0, 0, FTEDITOR_FONTHANDLE_MAX, tr("Font") + ": ", tr("Set font"));
				addSpinBox(1, 16, 34, tr("ROM Slot") + ": ", tr("Set ROM slot"));
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_VIDEOSTART:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_VIDEOSTART."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		/*
		s_CmdIdMap["CMD_VIDEOSTART"] = CMD_VIDEOSTART & 0xFF;
		s_CmdParamCount[CMD_VIDEOSTART & 0xFF] = 0;
		s_CmdParamString[CMD_VIDEOSTART & 0xFF] = false;
		s_CmdIdMap["CMD_VIDEOFRAME"] = CMD_VIDEOFRAME & 0xFF;
		s_CmdParamCount[CMD_VIDEOFRAME & 0xFF] = 2;
		s_CmdParamString[CMD_VIDEOFRAME & 0xFF] = false;*/
		/*s_CmdIdMap["CMD_SYNC"] = CMD_SYNC & 0xFF;
		s_CmdParamCount[CMD_SYNC & 0xFF] = 0; // undocumented
		s_CmdParamString[CMD_SYNC & 0xFF] = false;*/
		case CMD_SETBITMAP:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_SETBITMAP."));
			if (editor)
			{
				setTitle("CMD_SETBITMAP");
				if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815)
					addAddressFlashOpt(0, true);
				else
					addAddress(0, true);
				addExtFormat(1);
				addWH(2, 3, FTEDITOR_SCREENCOORDWH_MIN, FTEDITOR_SCREENCOORDWH_MAX);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_FLASHERASE:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_FLASHERASE."));
			if (editor)
			{
				setTitle("CMD_FLASHERASE");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_FLASHWRITE:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_FLASHWRITE."));
			if (editor)
			{
				setTitle("CMD_FLASHWRITE");
				addAddressFlash(0);
				addMemorySize(1);
				addStream(2);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_FLASHREAD:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_FLASHREAD."));
			if (editor)
			{
				setTitle("CMD_FLASHREAD");
				addAddress(0, false);
				addAddressFlash(1);
				addMemorySize(2);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_FLASHUPDATE:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_FLASHUPDATE."));
			if (editor)
			{
				setTitle("CMD_FLASHUPDATE");
				addAddressFlash(0);
				addAddress(1, false);
				addMemorySize(2);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_FLASHDETACH:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_FLASHDETACH."));
			if (editor)
			{
				setTitle("CMD_FLASHDETACH");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_FLASHATTACH:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_FLASHATTACH."));
			if (editor)
			{
				setTitle("CMD_FLASHATTACH");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_FLASHFAST:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_FLASHFAST."));
			if (editor)
			{
				setTitle("CMD_FLASHFAST");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_FLASHSPIDESEL:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_FLASHSPIDESEL."));
			if (editor)
			{
				setTitle("CMD_FLASHSPIDESEL");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		// case CMD_FLASHSPITX:
		// case CMD_FLASHSPIRX:
		case CMD_FLASHSOURCE:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_FLASHSOURCE."));
			if (editor)
			{
				setTitle("CMD_FLASHSOURCE");
				addAddressFlash(0);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_CLEARCACHE:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_CLEARCACHE."));
			if (editor)
			{
				setTitle("CMD_CLEARCACHE");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
        case CMD_INFLATE:
        {
            m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_INFLATE."));
            if (editor)
            {
                setTitle("CMD_INFLATE");
                addAddress(0, false);
                addStream(1);
                m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
            }
            ok = true;
            break;
        }
		case CMD_INFLATE2:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_INFLATE2."));
			if (editor)
			{
				setTitle("CMD_INFLATE2");
				addAddress(0, false);
				addOptions(1, OPT_MEDIAFIFO | OPT_FLASH);
				addStream(2); // Stream not applicable when OPT_FLASH given
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_ROTATEAROUND:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_ROTATEAROUND."));
			if (editor)
			{
				setTitle("CMD_ROTATEAROUND");
				addXY(0, 1, FTEDITOR_COORD_MIN, FTEDITOR_COORD_MAX);
				addSpinBoxAngle65536(2, 0x80000000UL, 0x7FFFFFFFUL, tr("Angle") + ": ", tr("Set angle"));
				addSpinBox65536(3, 0x80000000UL, 0x7FFFFFFFUL, tr("Scale") + ": ", tr("Set scale"));
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_RESETFONTS:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_RESETFONTS."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_ANIMSTART:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_ANIMSTART."));
			if (editor)
			{
				setTitle("CMD_ANIMSTART");
				addSpinBox(0, -1, 31, tr("Channel") + ": ", tr("Set channel"));
				addAddressFlash(1);
				addAnimLoop(2);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_ANIMSTOP:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_ANIMSTOP."));
			if (editor)
			{
				setTitle("CMD_ANIMSTOP");
				addSpinBox(0, -1, 31, tr("Channel") + ": ", tr("Set channel"));
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_ANIMXY:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_ANIMXY."));
			if (editor)
			{
				setTitle("CMD_ANIMXY");
				addSpinBox(0, -1, 31, tr("Channel") + ": ", tr("Set channel"));
				addXY(1, 2, FTEDITOR_COORD_MIN, FTEDITOR_COORD_MAX);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_ANIMDRAW:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_ANIMDRAW."));
			if (editor)
			{
				setTitle("CMD_ANIMDRAW");
				addSpinBox(0, -1, 31, tr("Channel") + ": ", tr("Set channel"));
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_GRADIENTA:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_GRADIENTA."));
			if (editor)
			{
				setTitle("CMD_GRADIENTA");
				addXY(0, 1, FTEDITOR_COORD_MIN, FTEDITOR_COORD_MAX);
				addAlphaHex(2);
				addColorHex(2);
				addXY(3, 4, FTEDITOR_COORD_MIN, FTEDITOR_COORD_MAX);
				addAlphaHex(5);
				addColorHex(5);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_FILLWIDTH:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_FILLWIDTH."));
			if (editor)
			{
				setTitle("CMD_FILLWIDTH");
				addWidth(0, 0, FTEDITOR_COORD_MAX);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_APPENDF:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_APPENDF."));
			if (editor)
			{
				setTitle("CMD_APPENDF");
				addAddressFlash(0);
				addMemorySize(1);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case CMD_ANIMFRAME:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_ANIMFRAME."));
			if (editor)
			{
				setTitle("CMD_ANIMFRAME");
				addXY(0, 1, FTEDITOR_COORD_MIN, FTEDITOR_COORD_MAX);
				addAddressFlash(2);
				addSpinBox(3, 0, 0x7FFFFFFF, tr("Frame") + ": ", tr("Set frame"));
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		// case CMD_NOP:
		// case CMD_SHA1:
		// case CMD_HMAC:
		// case CMD_LAST_:
		case CMD_VIDEOSTARTF:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_VIDEOSTARTF."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
        case CMD_VIDEOFRAME:
        {
            m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_VIDEOFRAME."));
            if (editor)
            {
                setTitle("CMD_VIDEOFRAME");
                addAddress(0, false);
                addAddress(1, false);
                m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
            }
            ok = true;
            break;
        }
        case CMD_BITMAP_TRANSFORM:
        {
            m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CMD_BITMAP_TRANSFORM."));
            if (editor)
            {
                setTitle("CMD_BITMAP_TRANSFORM");
                addXY(0, 1, FTEDITOR_COORD_MIN, FTEDITOR_COORD_MAX, "X0 Y0: ");
                addXY(2, 3, FTEDITOR_COORD_MIN, FTEDITOR_COORD_MAX, "X1 Y1: ");
                addXY(4, 5, FTEDITOR_COORD_MIN, FTEDITOR_COORD_MAX, "X2 Y2: ");
                addXY(6, 7, FTEDITOR_COORD_MIN, FTEDITOR_COORD_MAX, "TX0 TY0: ");
                addXY(8, 9, FTEDITOR_COORD_MIN, FTEDITOR_COORD_MAX, "TX1 TY1: ");
                addXY(10, 11, FTEDITOR_COORD_MIN, FTEDITOR_COORD_MAX, "TX2 TY2: ");
                m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
            }
            ok = true;
            break;
        }
	}
	else switch (idRight)
	{
		// ******************************************
		case FTEDITOR_DL_DISPLAY:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_DISPLAY."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_BITMAP_SOURCE:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_BITMAP_SOURCE."));
			if (editor)
			{
				setTitle("BITMAP_SOURCE");
				if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815)
					addAddressFlashOpt(0, true);
				else
					addAddress(0, true);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_CLEAR_COLOR_RGB:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CLEAR_COLOR_RGB."));
			if (editor)
			{
				setTitle("CLEAR_COLOR_RGB");
				addColor(0, 1, 2);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_TAG:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_TAG."));
			if (editor)
			{
				setTitle("TAG");
				addSpinBox(0, 0, 255, "Value: ", "Set value");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_COLOR_RGB:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_COLOR_RGB."));
			if (editor)
			{
				setTitle("COLOR_RGB");
				addColor(0, 1, 2);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_BITMAP_HANDLE:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_BITMAP_HANDLE."));
			if (editor)
			{
				setTitle("BITMAP_HANDLE");
				addHandle(0);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_CELL:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CELL."));
			if (editor)
			{
				setTitle("CELL");
				addCell(0);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_BITMAP_LAYOUT:
		{
			if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810)
				m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_BITMAP_LAYOUT_FT810."));
			else
				m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_BITMAP_LAYOUT."));
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
		case FTEDITOR_DL_BITMAP_SIZE:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_BITMAP_SIZE."));
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
		case FTEDITOR_DL_ALPHA_FUNC:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_ALPHA_FUNC."));
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
		case FTEDITOR_DL_STENCIL_FUNC:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_STENCIL_FUNC."));
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
		case FTEDITOR_DL_BLEND_FUNC:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_BLEND_FUNC."));
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
		case FTEDITOR_DL_STENCIL_OP:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_STENCIL_OP."));
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
		case FTEDITOR_DL_POINT_SIZE:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_POINT_SIZE."));
			if (editor)
			{
				setTitle("POINT_SIZE");
				addWidth16(0, 0, 8191, true);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_LINE_WIDTH:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_LINE_WIDTH."));
			if (editor)
			{
				setTitle("LINE_WIDTH");
				addWidth16(0, 0, 8191, false);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_CLEAR_COLOR_A:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CLEAR_COLOR_A."));
			if (editor)
			{
				setTitle("CLEAR_COLOR_A");
				addAlpha(0);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_COLOR_A:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_COLOR_A."));
			if (editor)
			{
				setTitle("COLOR_A");
				addAlpha(0);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_CLEAR_STENCIL:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CLEAR_STENCIL."));
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
		case FTEDITOR_DL_CLEAR_TAG:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CLEAR_TAG."));
			if (editor)
			{
				setTitle("CLEAR_TAG");
				addSpinBox(0, 0, 255, "Value: ", "Set value");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_STENCIL_MASK:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_STENCIL_MASK."));
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
		case FTEDITOR_DL_TAG_MASK:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_TAG_MASK."));
			if (editor)
			{
				setTitle("TAG_MASK");
				InteractiveProperties::addCheckBox(0, "Mask: ", "Set mask");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_BITMAP_TRANSFORM_A:
		{
			if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815)
				m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_BITMAP_TRANSFORM_A_BT815."));
			else
				m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_BITMAP_TRANSFORM_A."));
			if (editor)
			{
				setTitle("BITMAP_TRANSFORM_A");
				if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815)
				{
					addCheckBox(0, "Precision: ", "Set transform precision A");
					addSpinBoxForBitmapTransform(1, 0xFFFF0000, 0xFFFF, "A: ", "Set transform A"); // TODO: Precision-aware SpinBox
				}
				else
				{
					addSpinBox256(0, 0xFFFF0000, 0xFFFF, "A: ", "Set transform A");
				}
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_BITMAP_TRANSFORM_B:
		{
			if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815)
				m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_BITMAP_TRANSFORM_B_BT815."));
			else
				m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_BITMAP_TRANSFORM_B."));
			if (editor)
			{
				setTitle("BITMAP_TRANSFORM_B");
				if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815)
				{
					addCheckBox(0, "Precision: ", "Set transform precision B");
					addSpinBoxForBitmapTransform(1, 0xFFFF0000, 0xFFFF, "B: ", "Set transform B"); // TODO: Precision-aware SpinBox
				}
				else
				{
					addSpinBox256(0, 0xFFFF0000, 0xFFFF, "B: ", "Set transform B");
				}
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_BITMAP_TRANSFORM_C:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_BITMAP_TRANSFORM_C."));
			if (editor)
			{
				setTitle("BITMAP_TRANSFORM_C");
				addSpinBox256(0, 0xFF800000, 0x7FFFFF, "C: ", "Set transform c");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_BITMAP_TRANSFORM_D:
		{
			if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815)
				m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_BITMAP_TRANSFORM_D_BT815."));
			else
				m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_BITMAP_TRANSFORM_D."));
			if (editor)
			{
				setTitle("BITMAP_TRANSFORM_D");
				if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815)
				{
					addCheckBox(0, "Precision: ", "Set transform precision D");
					addSpinBoxForBitmapTransform(1, 0xFFFF0000, 0xFFFF, "D: ", "Set transform D"); // TODO: Precision-aware SpinBox
				}
				else
				{
					addSpinBox256(0, 0xFFFF0000, 0xFFFF, "D: ", "Set transform D");
				}
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_BITMAP_TRANSFORM_E:
		{
			if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815)
				m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_BITMAP_TRANSFORM_E_BT815."));
			else
				m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_BITMAP_TRANSFORM_E."));
			if (editor)
			{
				setTitle("BITMAP_TRANSFORM_E");
				if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_BT815)
				{
					addCheckBox(0, "Precision: ", "Set transform precision E");
					addSpinBoxForBitmapTransform(1, 0xFFFF0000, 0xFFFF, "E: ", "Set transform E"); // TODO: Precision-aware SpinBox
				}
				else
				{
					addSpinBox256(0, 0xFFFF0000, 0xFFFF, "E: ", "Set transform E");
				}
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_BITMAP_TRANSFORM_F:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_BITMAP_TRANSFORM_F."));
			if (editor)
			{
				setTitle("BITMAP_TRANSFORM_F");
				addSpinBox256(0, 0xFF800000, 0x7FFFFF, "F: ", "Set transform f");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_SCISSOR_XY:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_SCISSOR_XY."));
			if (editor)
			{
				setTitle("SCISSOR_XY");
				addXY(0, 1, FTEDITOR_SCREENCOORDXY_MIN, FTEDITOR_SCREENCOORDXY_MAX);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_SCISSOR_SIZE:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_SCISSOR_SIZE."));
			if (editor)
			{
				setTitle("SCISSOR_SIZE");
				addWH(0, 1, FTEDITOR_SCREENCOORDWH_MIN, FTEDITOR_SCREENCOORDWH_MAX);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_CALL:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CALL."));
			if (editor)
			{
				setTitle("CALL");
				addSpinBox(0, 0, 2047, "Dest: ", "Set dest");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_JUMP:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_JUMP."));
			if (editor)
			{
				setTitle("JUMP");
				addSpinBox(0, 0, 2047, "Dest: ", "Set dest");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_BEGIN:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_BEGIN."));
			if (editor)
			{
				setTitle("BEGIN");
				addPrimitive(0);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_COLOR_MASK:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_COLOR_MASK."));
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
		case FTEDITOR_DL_END:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_END."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_SAVE_CONTEXT:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_SAVE_CONTEXT."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_RESTORE_CONTEXT:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_RESTORE_CONTEXT."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_RETURN:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_RETURN."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_MACRO:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_MACRO."));
			if (editor)
			{
				setTitle("MACRO");
				addSpinBox(0, 0, 1, "Macro: ", "Set macro");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_CLEAR:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_CLEAR."));
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
		case FTEDITOR_DL_VERTEX_FORMAT:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_VERTEX_FORMAT."));
			if (editor)
			{
				setTitle("VERTEX_FORMAT");
				addSpinBox(0, 0, 4, "Fractional: ", "Set the number of fractional bits");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_BITMAP_LAYOUT_H:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_BITMAP_LAYOUT_H."));
			if (editor)
			{
				setTitle("BITMAP_LAYOUT_H");
				addSpinBox(0, 0, 0x3, "Stride: ", "Set bitmap line stride high bits");
				addSpinBox(1, 0, 0x3, "Height: ", "Set bitmap layout height high bits");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_BITMAP_SIZE_H:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_BITMAP_SIZE_H."));
			if (editor)
			{
				setTitle("BITMAP_SIZE_H");
				addSpinBox(0, 0, 0x3, "Width: ", "Set bitmap width high bits");
				addSpinBox(1, 0, 0x3, "Height: ", "Set bitmap height high bits");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_PALETTE_SOURCE:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_PALETTE_SOURCE."));
			if (editor)
			{
				setTitle("PALETTE_SOURCE");
				addAddress(0, false);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_VERTEX_TRANSLATE_X:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_VERTEX_TRANSLATE_X."));
			if (editor)
			{
				setTitle("VERTEX_TRANSLATE_X");
				addSpinBox16(0, -32768, 32767, "X: ", "Set x translation");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_VERTEX_TRANSLATE_Y:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_VERTEX_TRANSLATE_Y."));
			if (editor)
			{
				setTitle("VERTEX_TRANSLATE_Y");
				addSpinBox16(0, -32768, 32767, "Y: ", "Set y translation");
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_NOP:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_NOP."));
			if (editor)
			{
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_BITMAP_EXT_FORMAT:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_BITMAP_EXT_FORMAT."));
			if (editor)
			{
				setTitle("BITMAP_EXT_FORMAT");
				addExtFormat(0);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_BITMAP_SWIZZLE:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_BITMAP_SWIZZLE_FORMAT."));
			if (editor)
			{
				setTitle("BITMAP_SWIZZLE");
				addSwizzle(0, "R: ", tr("Set swizzle R"));
				addSwizzle(1, "G: ", tr("Set swizzle G"));
				addSwizzle(2, "B: ", tr("Set swizzle B"));
				addSwizzle(3, "A: ", tr("Set swizzle A"));
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
			}
			ok = true;
			break;
		}
		case FTEDITOR_DL_INT_FRR:
		{
			m_MainWindow->propertiesEditor()->setInfo(tr("DESCRIPTION_INT_FRR."));
			if (editor)
			{
				setTitle("INT_FRR");
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

} /* namespace FTEDITOR */

/* end of file */
