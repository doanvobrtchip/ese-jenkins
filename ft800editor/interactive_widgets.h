/**
 * interactive_widgets.h
 * $Id$
 * \file interactive_widgets.h
 * \brief interactive_widgets.h
 * \date 2014-03-04 22:58GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#ifndef FT800EMUQT_INTERACTIVE_WIDGETS_H
#define FT800EMUQT_INTERACTIVE_WIDGETS_H

// STL includes
#include <stdio.h>
#include <math.h>

// Qt includes
#include <QSpinBox>
#include <QCheckBox>
#include <QLineEdit>
#include <QSlider>
#include <QFrame>
#include <QColorDialog>
#include <QComboBox>

// Emulator includes
#include <ft8xxemu_inttypes.h>

// Project includes
#include "main_window.h"
#include "interactive_properties.h"
#include "dl_parser.h"
#include "dl_editor.h"
#include "undo_stack_disabler.h"
#include "constant_mapping.h"

namespace FT800EMUQT {

class MainWindow;
class DlEditor;

extern int g_PropertiesWidgetCombineId;

class InteractiveProperties::PropertiesWidget
{
public:
	PropertiesWidget(InteractiveProperties *parent, const QString &undoMessage) : m_InteractiveProperties(parent), m_CombineId(g_PropertiesWidgetCombineId), m_UndoMessage(undoMessage)
	{
		++g_PropertiesWidgetCombineId;
	}

	virtual ~PropertiesWidget()
	{

	}

	virtual void modifiedEditorLine() = 0;

protected:
	void setLine(const DlParsed &parsed)
	{
		m_InteractiveProperties->m_LineEditor->replaceLine(m_InteractiveProperties->m_LineNumber, parsed, m_CombineId, m_UndoMessage);
	}

	const DlParsed &getLine()
	{
		return m_InteractiveProperties->m_LineEditor->getLine(m_InteractiveProperties->m_LineNumber);
	}

	InteractiveProperties *m_InteractiveProperties;
	int m_CombineId;
	QString m_UndoMessage;

};

////////////////////////////////////////////////////////////////////////

class InteractiveProperties::PropertiesSpinBoxAddress : public UndoStackDisabler<QSpinBox>, public PropertiesWidget
{
	Q_OBJECT

public:
	PropertiesSpinBoxAddress(InteractiveProperties *parent, const QString &undoMessage, int index) : UndoStackDisabler<QSpinBox>(parent), PropertiesWidget(parent, undoMessage), m_Index(index), m_SoftMod(false)
	{
		m_SoftMod = true;
		setUndoStack(parent->m_MainWindow->undoStack());
		setKeyboardTracking(false);
		setMinimum(0);
		setMaximum(ram(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G_END) - 4);
		setSingleStep(4);
		connect(this, SIGNAL(valueChanged(int)), this, SLOT(updateValue(int)));
	}

	virtual ~PropertiesSpinBoxAddress()
	{

	}

	void done()
	{
		m_SoftMod = false;
		modifiedEditorLine();
	}

	virtual void modifiedEditorLine()
	{
		//printf("modifiedEditorLine %i\n", getLine().Parameter[m_Index].I);
		if (m_SoftMod) return;
		m_SoftMod = true;
		setValue(getLine().Parameter[m_Index].I);
		m_SoftMod = false;
		//printf("bye");
	}

private slots:
	void updateValue(int value)
	{
		//printf("updateValue\n");
		if (m_SoftMod) return;
		m_SoftMod = true;
		//printf("PropertiesSpinBox::updateValue(value)\n");
		DlParsed parsed = getLine();
		parsed.Parameter[m_Index].I = (value & 0x7FFFFFFC);
		setLine(parsed);
		m_SoftMod = false;
	}

private:
	int m_Index;
	bool m_SoftMod;

};

////////////////////////////////////////////////////////////////////////

class InteractiveProperties::PropertiesSpinBox : public UndoStackDisabler<QSpinBox>, public PropertiesWidget
{
	Q_OBJECT

public:
	PropertiesSpinBox(InteractiveProperties *parent, const QString &undoMessage, int index) : UndoStackDisabler<QSpinBox>(parent), PropertiesWidget(parent, undoMessage), m_Index(index), m_SoftMod(false)
	{
		m_SoftMod = true;
		setUndoStack(parent->m_MainWindow->undoStack());
		setKeyboardTracking(false);
		setMinimum(0x80000000);
		setMaximum(0x7FFFFFFF);
		connect(this, SIGNAL(valueChanged(int)), this, SLOT(updateValue(int)));
	}

	virtual ~PropertiesSpinBox()
	{

	}

	void done()
	{
		m_SoftMod = false;
		modifiedEditorLine();
	}

	virtual void modifiedEditorLine()
	{
		//printf("modifiedEditorLine %i\n", getLine().Parameter[m_Index].I);
		if (m_SoftMod) return;
		m_SoftMod = true;
		setValue(getLine().Parameter[m_Index].I);
		m_SoftMod = false;
		//printf("bye");
	}

private slots:
	void updateValue(int value)
	{
		//printf("updateValue\n");
		if (m_SoftMod) return;
		m_SoftMod = true;
		//printf("PropertiesSpinBox::updateValue(value)\n");
		DlParsed parsed = getLine();
		parsed.Parameter[m_Index].I = value;
		setLine(parsed);
		m_SoftMod = false;
	}

private:
	int m_Index;
	bool m_SoftMod;

};

////////////////////////////////////////////////////////////////////////

class InteractiveProperties::PropertiesSpinBox16 : public InteractiveProperties::PropertiesSpinBox
{
public:
	PropertiesSpinBox16(InteractiveProperties *parent, const QString &undoMessage, int index) : PropertiesSpinBox(parent, undoMessage, index)
	{

	}

	virtual ~PropertiesSpinBox16()
	{

	}

protected:
	virtual QString textFromValue(int value) const
	{
		return QString::number((float)value / 16.f);
	}

	virtual int valueFromText(const QString &text) const
	{
		return (int)floorf((text.toFloat() * 16.f) + 0.5f);
	}
};

////////////////////////////////////////////////////////////////////////

class InteractiveProperties::PropertiesSpinBox256 : public InteractiveProperties::PropertiesSpinBox
{
public:
	PropertiesSpinBox256(InteractiveProperties *parent, const QString &undoMessage, int index) : PropertiesSpinBox(parent, undoMessage, index)
	{

	}

	virtual ~PropertiesSpinBox256()
	{

	}

protected:
	virtual QString textFromValue(int value) const
	{
		return QString::number((double)value / 256.0);
	}

	virtual int valueFromText(const QString &text) const
	{
		return (int)floor((text.toDouble() * 256.0) + 0.5);
	}
};

////////////////////////////////////////////////////////////////////////

class InteractiveProperties::PropertiesSpinBox65536 : public InteractiveProperties::PropertiesSpinBox
{
public:
	PropertiesSpinBox65536(InteractiveProperties *parent, const QString &undoMessage, int index) : PropertiesSpinBox(parent, undoMessage, index)
	{

	}

	virtual ~PropertiesSpinBox65536()
	{

	}

protected:
	virtual QString textFromValue(int value) const
	{
		return QString::number((double)value / 65536.0);
	}

	virtual int valueFromText(const QString &text) const
	{
		return (int)floor((text.toDouble() * 65536.0) + 0.5);
	}
};

////////////////////////////////////////////////////////////////////////

class InteractiveProperties::PropertiesSpinBoxAngle65536 : public InteractiveProperties::PropertiesSpinBox
{
public:
	PropertiesSpinBoxAngle65536(InteractiveProperties *parent, const QString &undoMessage, int index) : PropertiesSpinBox(parent, undoMessage, index)
	{

	}

	virtual ~PropertiesSpinBoxAngle65536()
	{

	}

protected:
	virtual QString textFromValue(int value) const
	{
		return QString::number((double)value / (65536.0 / 360.0));
	}

	virtual int valueFromText(const QString &text) const
	{
		return (int)floor((text.toDouble() * (65536.0 / 360.0)) + 0.5);
	}
};

////////////////////////////////////////////////////////////////////////

class InteractiveProperties::PropertiesCheckBox : public QCheckBox, public PropertiesWidget
{
	Q_OBJECT

public:
	PropertiesCheckBox(InteractiveProperties *parent, const QString &undoMessage, int index, uint32_t flag) : QCheckBox(parent), PropertiesWidget(parent, undoMessage), m_Index(index), m_Flag(flag), m_SoftMod(false)
	{
		m_CombineId = -1;
		connect(this, SIGNAL(stateChanged(int)), this, SLOT(updateValue(int)));
		modifiedEditorLine();
	}

	virtual ~PropertiesCheckBox()
	{

	}

	virtual void modifiedEditorLine()
	{
		if (m_SoftMod) return;
		m_SoftMod = true;
		setChecked(((getLine().Parameter[m_Index].U & m_Flag) == m_Flag) ? Qt::Checked : Qt::Unchecked);
		m_SoftMod = false;
	}

private slots:
	void updateValue(int value)
	{
		if (m_SoftMod) return;
		m_SoftMod = true;
		// printf("PropertiesCheckBox::updateValue(value)\n");
		DlParsed parsed = getLine();
		parsed.Parameter[m_Index].U = (value == Qt::Checked)
			? (parsed.Parameter[m_Index].U | m_Flag)
			: (parsed.Parameter[m_Index].U & ~m_Flag);
		setLine(parsed);
		m_SoftMod = false;
	}

private:
	int m_Index;
	uint32_t m_Flag;
	bool m_SoftMod;
};

class InteractiveProperties::PropertiesLineEdit : public UndoStackDisabler<QLineEdit>, public PropertiesWidget
{
	Q_OBJECT

public:
	PropertiesLineEdit(InteractiveProperties *parent, const QString &undoMessage, int index) : UndoStackDisabler<QLineEdit>(parent), PropertiesWidget(parent, undoMessage), m_Index(index), m_SoftMod(false)
	{
		setUndoStack(parent->m_MainWindow->undoStack());
		connect(this, SIGNAL(textEdited(const QString &)), this, SLOT(updateValue(const QString &)));
		modifiedEditorLine();
	}

	virtual ~PropertiesLineEdit()
	{

	}

	virtual void modifiedEditorLine()
	{
		if (m_SoftMod) return;
		m_SoftMod = true;
		const DlParsed &parsed = getLine();
		std::string str;
		DlParser::escapeString(str, parsed.StringParameter);
		setText(str.c_str());
		m_SoftMod = false;
	}

private slots:
	void updateValue(const QString &text)
	{
		if (m_SoftMod) return;
		m_SoftMod = true;
		// printf("PropertiesLineEdit::updateValue(value)\n");
		DlParsed parsed = getLine();
		QByteArray ba = text.toLatin1();
		std::string str = std::string(ba.data());
		DlParser::unescapeString(parsed.StringParameter, str);
		setLine(parsed);
		m_SoftMod = false;
	}

private:
	int m_Index;
	bool m_SoftMod;
};

class InteractiveProperties::PropertiesLineEditChar : public UndoStackDisabler<QLineEdit>, public PropertiesWidget
{
	Q_OBJECT

public:
	PropertiesLineEditChar(InteractiveProperties *parent, const QString &undoMessage, int index) : UndoStackDisabler<QLineEdit>(parent), PropertiesWidget(parent, undoMessage), m_Index(index), m_SoftMod(false)
	{
		setUndoStack(parent->m_MainWindow->undoStack());
		connect(this, SIGNAL(textEdited(const QString &)), this, SLOT(updateValue(const QString &)));
		modifiedEditorLine();
	}

	virtual ~PropertiesLineEditChar()
	{

	}

	virtual void modifiedEditorLine()
	{
		if (m_SoftMod) return;
		m_SoftMod = true;
		const DlParsed &parsed = getLine();
		std::string str;
		if (parsed.Parameter[m_Index].U & 0xFF)
		{
			DlParser::escapeString(str, std::string(1, (unsigned char)(parsed.Parameter[m_Index].U & 0xFF)));
			setText(str.c_str());
		}
		else
		{
			setText("");
		}
		m_SoftMod = false;
	}

private slots:
	void updateValue(const QString &text)
	{
		if (m_SoftMod) return;
		m_SoftMod = true;
		// printf("PropertiesLineEdit::updateValue(value)\n");
		DlParsed parsed = getLine();
		QByteArray ba = text.toLatin1();
		std::string str = std::string(ba.data());
		std::string dst;
		DlParser::unescapeString(dst, str);
		if (dst.size())
		{
			parsed.Parameter[m_Index].U = (parsed.Parameter[m_Index].U & ~0xFF) | (uint32_t)(unsigned char)dst[0];
		}
		else
		{
			parsed.Parameter[m_Index].U = (parsed.Parameter[m_Index].U & ~0xFF);
		}
		setLine(parsed);
		m_SoftMod = false;
	}

private:
	int m_Index;
	bool m_SoftMod;
};

////////////////////////////////////////////////////////////////////////

class InteractiveProperties::PropertiesSlider : public QSlider, public PropertiesWidget
{
	Q_OBJECT

public:
	PropertiesSlider(InteractiveProperties *parent, const QString &undoMessage, int index) : QSlider(Qt::Horizontal, parent), PropertiesWidget(parent, undoMessage), m_Index(index), m_SoftMod(false)
	{
		m_SoftMod = true;
		connect(this, SIGNAL(valueChanged(int)), this, SLOT(updateValue(int)));
		setMinimum(0x80000000);
		setMaximum(0x7FFFFFFF);
	}

	virtual ~PropertiesSlider()
	{

	}

	void ready()
	{
		m_SoftMod = false;
		modifiedEditorLine();
	}

	virtual void modifiedEditorLine()
	{
		if (m_SoftMod) return;
		m_SoftMod = true;
		setValue(getLine().Parameter[m_Index].I);
		m_SoftMod = false;
	}

private slots:
	void updateValue(int value)
	{
		if (m_SoftMod) return;
		m_SoftMod = true;
		// printf("PropertiesSlider::updateValue(value)\n");
		DlParsed parsed = getLine();
		parsed.Parameter[m_Index].I = value;
		setLine(parsed);
		m_SoftMod = false;
	}

private:
	int m_Index;
	bool m_SoftMod;

};

////////////////////////////////////////////////////////////////////////

class InteractiveProperties::PropertiesSliderDyn : public QSlider, public PropertiesWidget
{
	Q_OBJECT

public:
	PropertiesSliderDyn(InteractiveProperties *parent, const QString &undoMessage, int index, int maxim) : QSlider(Qt::Horizontal, parent), PropertiesWidget(parent, undoMessage), m_Index(index), m_SoftMod(false), m_Maxim(maxim)
	{
		m_SoftMod = true;
		connect(this, SIGNAL(valueChanged(int)), this, SLOT(updateValue(int)));
	}

	void ready()
	{
		m_SoftMod = false;
		modifiedEditorLine();
	}

	virtual ~PropertiesSliderDyn()
	{

	}

	virtual void modifiedEditorLine()
	{
		if (m_SoftMod) return;
		m_SoftMod = true;
		setMaximum(getLine().Parameter[m_Maxim].I);
		setValue(getLine().Parameter[m_Index].I);
		m_SoftMod = false;
	}

private slots:
	void updateValue(int value)
	{
		if (m_SoftMod) return;
		m_SoftMod = true;
		// printf("PropertiesSlider::updateValue(value)\n");
		DlParsed parsed = getLine();
		parsed.Parameter[m_Index].I = value;
		setLine(parsed);
		m_SoftMod = false;
	}

private:
	int m_Index;
	int m_Maxim;
	bool m_SoftMod;

};

////////////////////////////////////////////////////////////////////////

class InteractiveProperties::PropertiesSliderDynSubClip : public QSlider, public PropertiesWidget
{
	Q_OBJECT

public:
	PropertiesSliderDynSubClip(InteractiveProperties *parent, const QString &undoMessage, int index, int clip, int maxim) : QSlider(Qt::Horizontal, parent), PropertiesWidget(parent, undoMessage), m_Index(index), m_Clip(clip), m_SoftMod(false), m_Maxim(maxim)
	{
		m_SoftMod = true;
		connect(this, SIGNAL(valueChanged(int)), this, SLOT(updateValue(int)));
	}

	virtual ~PropertiesSliderDynSubClip()
	{

	}

	void ready()
	{
		m_SoftMod = false;
		modifiedEditorLine();
	}

	virtual void modifiedEditorLine()
	{
		if (m_SoftMod) return;
		m_SoftMod = true;
		setMaximum(getLine().Parameter[m_Maxim].I);
		setValue(getLine().Parameter[m_Index].I);
		m_SoftMod = false;
	}

private slots:
	void updateValue(int value)
	{
		if (m_SoftMod) return;
		m_SoftMod = true;
		// printf("PropertiesSlider::updateValue(value)\n");
		DlParsed parsed = getLine();
		parsed.Parameter[m_Index].I = value;
		int maxim = parsed.Parameter[m_Maxim].I - parsed.Parameter[m_Index].I;
		if (parsed.Parameter[m_Clip].I > maxim) parsed.Parameter[m_Clip].I = maxim;
		setLine(parsed);
		m_SoftMod = false;
	}

private:
	int m_Index;
	int m_Clip;
	int m_Maxim;
	bool m_SoftMod;

};

////////////////////////////////////////////////////////////////////////

class InteractiveProperties::PropertiesSliderDynSub : public QSlider, public PropertiesWidget
{
	Q_OBJECT

public:
	PropertiesSliderDynSub(InteractiveProperties *parent, const QString &undoMessage, int index, int sub, int maxim) : QSlider(Qt::Horizontal, parent), PropertiesWidget(parent, undoMessage), m_Index(index), m_SoftMod(false), m_Sub(sub), m_Maxim(maxim)
	{
		m_SoftMod = true;
		connect(this, SIGNAL(valueChanged(int)), this, SLOT(updateValue(int)));
	}

	virtual ~PropertiesSliderDynSub()
	{

	}

	void ready()
	{
		m_SoftMod = false;
		modifiedEditorLine();
	}

	virtual void modifiedEditorLine()
	{
		if (m_SoftMod) return;
		m_SoftMod = true;
		setMaximum(getLine().Parameter[m_Maxim].I - getLine().Parameter[m_Sub].I);
		setValue(getLine().Parameter[m_Index].I);
		m_SoftMod = false;
	}

private slots:
	void updateValue(int value)
	{
		if (m_SoftMod) return;
		m_SoftMod = true;
		// printf("PropertiesSlider::updateValue(value)\n");
		DlParsed parsed = getLine();
		parsed.Parameter[m_Index].I = value;
		setLine(parsed);
		m_SoftMod = false;
	}

private:
	int m_Index;
	int m_Sub;
	int m_Maxim;
	bool m_SoftMod;

};

////////////////////////////////////////////////////////////////////////

class InteractiveProperties::PropertiesColor : public QFrame, public PropertiesWidget
{
	Q_OBJECT

public:
	PropertiesColor(InteractiveProperties *parent, const QString &undoMessage, int r, int g, int b) : QFrame(parent), PropertiesWidget(parent, undoMessage), m_R(r), m_G(g), m_B(b)
	{
		//setWidth(48);
		//setHeight(26);
		QPalette p = palette();
		p.setColor(QPalette::WindowText, p.color(QPalette::Shadow));
		p.setColor(QPalette::Window, Qt::black);
		setPalette(p);
		setAutoFillBackground(true);
		setFrameStyle(QFrame::Panel | QFrame::Plain);
		modifiedEditorLine();
	}

	virtual ~PropertiesColor()
	{

	}

	virtual void modifiedEditorLine()
	{
		QPalette p = palette();
		const DlParsed &parsed = getLine();
		p.setColor(QPalette::Window, QColor(parsed.Parameter[m_R].I, parsed.Parameter[m_G].I, parsed.Parameter[m_B].I));
		setPalette(p);
	}

	virtual void mousePressEvent(QMouseEvent *event)
	{
		if (event->button() != Qt::LeftButton) return;
		// printf("PropertiesColor::mousePressEvent(event)\n");
		QColor c = QColorDialog::getColor(palette().color(QPalette::Window), this);
		if (c.isValid())
		{
			DlParsed parsed = getLine();
			parsed.Parameter[m_R].I = c.red();
			parsed.Parameter[m_G].I = c.green();
			parsed.Parameter[m_B].I = c.blue();
			setLine(parsed);
		}
	}

private:
	int m_R;
	int m_G;
	int m_B;

};

////////////////////////////////////////////////////////////////////////

class InteractiveProperties::PropertiesComboBox : public QComboBox, public PropertiesWidget
{
	Q_OBJECT

public:
	PropertiesComboBox(InteractiveProperties *parent, const QString &undoMessage, int index, int begin) : QComboBox(parent), PropertiesWidget(parent, undoMessage), m_Index(index), m_Begin(begin), m_SoftMod(false)
	{
		m_SoftMod = true;
		connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(updateValue(int)));
	}

	virtual ~PropertiesComboBox()
	{

	}

	void ready()
	{
		m_SoftMod = false;
		modifiedEditorLine();
	}

	virtual void modifiedEditorLine()
	{
		if (m_SoftMod) return;
		m_SoftMod = true;
		setCurrentIndex(getLine().Parameter[m_Index].U - m_Begin);
		m_SoftMod = false;
	}

private slots:
	void updateValue(int value)
	{
		if (m_SoftMod) return;
		if (value < 0) return;
		m_SoftMod = true;
		//printf("PropertiesSlider::updateValue(value) %i\n", value);
		DlParsed parsed = getLine();
		parsed.Parameter[m_Index].U = value + m_Begin;
		setLine(parsed);
		m_SoftMod = false;
	}

private:
	int m_Index;
	int m_Begin;
	bool m_SoftMod;

};

////////////////////////////////////////////////////////////////////////

class InteractiveProperties::PropertiesRemapComboBox : public QComboBox, public PropertiesWidget
{
	Q_OBJECT

public:
	PropertiesRemapComboBox(InteractiveProperties *parent, const QString &undoMessage, int index, const int *toIntf, int toIntfSz, const int *toEnum, int toEnumSz) : QComboBox(parent), PropertiesWidget(parent, undoMessage), m_Index(index), m_ToIntf(toIntf), m_ToIntfSz(toIntfSz), m_ToEnum(toEnum), m_ToEnumSz(toEnumSz), m_SoftMod(false)
	{
		m_SoftMod = true;
		connect(this, SIGNAL(currentIndexChanged(int)), this, SLOT(updateValue(int)));
	}

	virtual ~PropertiesRemapComboBox()
	{

	}

	void ready()
	{
		m_SoftMod = false;
		modifiedEditorLine();
	}

	virtual void modifiedEditorLine()
	{
		if (m_SoftMod) return;
		m_SoftMod = true;
		setCurrentIndex(m_ToIntf[getLine().Parameter[m_Index].U % m_ToIntfSz]);
		m_SoftMod = false;
	}

	private slots:
	void updateValue(int value)
	{
		if (m_SoftMod) return;
		if (value < 0) return;
		m_SoftMod = true;
		//printf("PropertiesSlider::updateValue(value) %i\n", value);
		DlParsed parsed = getLine();
		parsed.Parameter[m_Index].U = m_ToEnum[value % m_ToEnumSz];
		setLine(parsed);
		m_SoftMod = false;
	}

private:
	int m_Index;
	const int *m_ToIntf;
	int m_ToIntfSz;
	const int *m_ToEnum;
	int m_ToEnumSz;
	bool m_SoftMod;

};

////////////////////////////////////////////////////////////////////////

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_INTERACTIVE_WIDGETS_H */

/* end of file */
