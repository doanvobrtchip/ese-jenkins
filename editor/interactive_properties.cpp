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
#include <vc.h>

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
			bool ok = false;
			// const uint32_t *p = m_DisplayListParsed[i].Parameter;
			if (parsed.IdLeft == FT800EMU_DL_VERTEX2F)
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
				setTitle("VERTEX2F");
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
				ok = true;
			}
			else if (parsed.IdLeft == FT800EMU_DL_VERTEX2II)
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
				setTitle("VERTEX2II");
				PropertiesSpinBox *propX = new PropertiesSpinBox(this, 0);
				propX->setMinimum(0);
				propX->setMaximum(512);
				PropertiesSpinBox *propY = new PropertiesSpinBox(this, 1);
				propY->setMinimum(0);
				propY->setMaximum(512);
				addLabeledWidget("XY: ", propX, propY);
				m_CurrentProperties.push_back(propX);
				m_CurrentProperties.push_back(propY);
				PropertiesSpinBox *propHandle = new PropertiesSpinBox(this, 2); // TODO: Handle combobox
				propHandle->setMinimum(0);
				propHandle->setMaximum(31);
				addLabeledWidget("Handle: ", propHandle);
				m_CurrentProperties.push_back(propHandle);
				PropertiesSpinBox *propCell = new PropertiesSpinBox(this, 3);
				propCell->setMinimum(0);
				propCell->setMaximum(255);
				addLabeledWidget("Cell: ", propCell);
				m_CurrentProperties.push_back(propCell);
				m_MainWindow->propertiesEditor()->setEditWidget(this, false, editor);
				ok = true;
			}
			else if (parsed.IdLeft == 0xFFFFFF00) switch (parsed.IdRight | 0xFFFFFF00)
			{
				case CMD_DLSTART:
				{
					m_MainWindow->propertiesEditor()->setInfo(tr(
						"<b>CMD_DLSTART</b>()<br>"
						"<br>"
						"When the co-processor engine executes this command, it waits until the display list is "
						"ready for writing, then sets REG_CMD_DL to zero."));
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
					ok = true;
					break;
				}
				case CMD_BGCOLOR:
				{
					m_MainWindow->propertiesEditor()->setInfo(tr(
						"<b>CMD_BGCOLOR</b>(<i>r</i>, <i>g</i>, <i>b</i>)<br>"
						"<b>rgb</b>: New background color, as a 32-bit RGB number. Red is the most significant 8 "
						"bits, blue is the least. So 0xff0000 is bright red.<br>"
						"Background color is applicable for things that the user cannot move. Example "
						"behind gauges and sliders etc.<br>"
						"<br>"
						"Set the background color."));
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
					ok = true;
					break;
				}
				case CMD_FGCOLOR:
				{
					m_MainWindow->propertiesEditor()->setInfo(tr(
						"<b>CMD_FGCOLOR</b>(<i>r</i>, <i>g</i>, <i>b</i>)<br>"
						"<b>rgb</b>: New foreground color, as a 32-bit RGB number. Red is the most significant 8 "
						"bits, blue is the least. So 0xff0000 is bright red.<br>"
						"Foreground color is applicable "
						"for things that the user can move such as handles and buttons "
						"(\"affordances\").<br>"
						"<br>"
						"Set the foreground color."));
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
						"<b>argb0</b>: Color of point 0, as a 32-bit ARGB number. R is the most significant 8 bits, B is "
						"the least. So 0xffff0000 is bright red.<br>"
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
					ok = true;
					break;
				}
				case CMD_TOGGLE:
				{
					m_MainWindow->propertiesEditor()->setInfo(tr(
						"<b>CMD_SCROLLBAR</b>(<i>x</i>, <i>y</i>, <i>w</i>, <i>f</i>, <i>options</i>, <i>state</i>, <i>s</i>)<br>"
						"<b>x</b>: x-coordinate of top-left of toggle, in pixels<br>"
						"<b>y</b>: y-coordinate of top-left of toggle, in pixels<br>"
						"<b>w</b>: width of toggle, in pixels<br>"
						"<b>f</b>: font to use for text, 0-31. See ROM and RAM Fonts<br>"
						"<b>state</b>: state of the toggle: 0 is off, 65535 is on.<br>"
						"<b>options</b>: By default the toggle bar is drawn with a 3D effect. OPT_FLAT removes the 3D "
						"effect<br>"
						"<b>s</b>: String label for toggle. A character value of 255 (in C it can be written as \xff) "
						"separates the two labels.<br>"
						"<br>"
						"Draw a toggle switch."));
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
						"Note that only one of CMD_SKETCH, CMD_SCREENSAVER, or CMD_SPINNER can be "
						"active at one time."));
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
					ok = true;
					break;
				}
				case CMD_STOP:
				{
					m_MainWindow->propertiesEditor()->setInfo(tr(
						"<b>CMD_STOP</b>()<br>"
						"<br>"
						"Stop any spinner, screensaver or sketch."));
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
					ok = true;
					break;
				}
				case CMD_MEMZERO:
				{
					m_MainWindow->propertiesEditor()->setInfo(tr(
						"<b>CMD_MEMSET</b>(<i>ptr</i>, <i>num</i>)<br>"
						"<b>ptr</b>: Starting address of the memory block<br>"
						"<b>num</b>: Number of bytes in the memory block<br>"
						"<br>"
						"Write zero to a block of memory."));
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
					ok = true;
					break;
				}
				case CMD_LOADIDENTITY:
				{
					m_MainWindow->propertiesEditor()->setInfo(tr(
						"<b>CMD_LOADIDENTITY</b>()<br>"
						"<br>"
						"Set the current matrix to identity."));
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
					ok = true;
					break;
				}
				case CMD_SETMATRIX:
				{
					m_MainWindow->propertiesEditor()->setInfo(tr(
						"<b>CMD_SETMATRIX</b>()<br>"
						"<br>"
						"Write the current matrix as a bitmap transform."));
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
						"Note that only one of CMD_SKETCH, CMD_SCREENSAVER, or CMD_SPINNER can be "
						"active at one time."));
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
						"Note that only one of CMD_SKETCH, CMD_SCREENSAVER, or CMD_SPINNER can be "
						"active at one time."));
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
					ok = true;
					break;
				}
				case CMD_LOGO:
				{
					m_MainWindow->propertiesEditor()->setInfo(tr(
						"<b>CMD_LOGO</b>()<br>"
						"<br>"
						"Play device logo animation."));
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
					ok = true;
					break;
				}
				case CMD_COLDSTART:
				{
					m_MainWindow->propertiesEditor()->setInfo(tr(
						"<b>CMD_LOGO</b>()<br>"
						"<br>"
						"Set co-processor engine state to default values."));
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
					ok = true;
					break;
				}
				case CMD_GRADCOLOR:
				{
					m_MainWindow->propertiesEditor()->setInfo(tr(
						"<b>CMD_GRADCOLOR</b>(<i>r</i>, <i>g</i>, <i>b</i>)<br>"
						"<b>rgb</b>: New highlight gradient color, as a 32-bit RGB number. Red is the most "
						"significant 8 bits, blue is the least. So 0xffff0000 is bright red.<br>"
						"<br>"
						"Set the 3D button highlight color."));
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
					ok = true;
					break;
				}
			}
			else switch (parsed.IdRight)
			{
				// ******************************************
				case FT800EMU_DL_DISPLAY:
				{
					m_MainWindow->propertiesEditor()->setInfo(tr(
						"<b>DISPLAY</b>()<br>"
						"<br>"
						"End the display list. FT800 will ignore all the commands following this command."));
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
					ok = true;
					break;
				}
				case FT800EMU_DL_BITMAP_LAYOUT:
				{
					m_MainWindow->propertiesEditor()->setInfo(tr(
						"<b>BITMAP_LAYOUT</b>(<i>format</i>, <i>linestride</i>, <i>height</i>)<br>"
						"<b>format</b>: Bitmap pixel format. The bitmap formats supported are L1, L4, L8, RGB332, ARGB2, ARGB4, ARGB1555, "
						"RGB565 and Palette.<br>"
						"<b>linestride</b>: Bitmap linestride, in bytes. For L1 format, the line stride must be a multiple of 8 bits; For L4 format the line "
						"stride must be multiple of 2 nibbles. (Aligned to byte).<br>"
						"<b>height</b>: Bitmap height, in lines<br>"
						"<br>"
						"Specify the source bitmap memory format and layout for the current handle."));
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
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
					m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
					ok = true;
					break;
				}
			}
			if (!ok)
			{
				m_MainWindow->propertiesEditor()->setInfo(tr("</i>Not yet implemented.</i>"));
				m_MainWindow->propertiesEditor()->setEditWidget(NULL, false, editor);
			}
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

void InteractiveProperties::modifiedEditorLine()
{
	printf("InteractiveProperties::modifiedEditorLine()\n");

	for (std::vector<PropertiesWidget *>::iterator it(m_CurrentProperties.begin()), end(m_CurrentProperties.end()); it != end; ++it)
	{
		(*it)->modifiedEditorLine();
	}
}

} /* namespace FT800EMUQT */

/* end of file */
