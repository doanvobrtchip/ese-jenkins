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

#ifndef FTEDITOR_INTERACTIVE_WIDGETS_H
#define FTEDITOR_INTERACTIVE_WIDGETS_H

#pragma warning(disable : 26812)
#pragma warning(disable : 26495)
#pragma warning(disable : 26444)

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
#include <QPushButton>
#include <QFileDialog>

// Emulator includes
#include <bt8xxemu_inttypes.h>
#include <bt8xxemu_diag.h>

// Project includes
#include "main_window.h"
#include "interactive_properties.h"
#include "dl_parser.h"
#include "dl_editor.h"
#include "undo_stack_disabler.h"
#include "constant_mapping.h"
#include "constant_common.h"

class SupportCopyPasteSpinBox : public QSpinBox
{
	Q_OBJECT

public:
	SupportCopyPasteSpinBox(QWidget *parent = nullptr)
		: QSpinBox(parent)
	{
	}

public slots:
	void cut()
	{
		this->lineEdit()->cut();
	}

	void copy()
	{
		this->lineEdit()->copy();
	}

	void paste()
	{
		this->lineEdit()->paste();
	}
};

class SupportCopyPasteDoubleSpinBox : public QDoubleSpinBox
{
	Q_OBJECT

public:
	SupportCopyPasteDoubleSpinBox(QWidget *parent = nullptr)
	    : QDoubleSpinBox(parent)
	{
	}

public slots:
	void cut()
	{
		this->lineEdit()->cut();
	}

	void copy()
	{
		this->lineEdit()->copy();
	}

	void paste()
	{
		this->lineEdit()->paste();
	}
};

namespace FTEDITOR
{

extern BT8XXEMU_Emulator *g_Emulator;
extern BT8XXEMU_Flash *g_Flash;

class MainWindow;
class DlEditor;

extern int g_PropertiesWidgetCombineId;

class InteractiveProperties::PropertiesWidget
{
public:
	PropertiesWidget(InteractiveProperties *parent, const QString &undoMessage)
	    : m_InteractiveProperties(parent)
	    , m_CombineId(g_PropertiesWidgetCombineId)
	    , m_UndoMessage(undoMessage)
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

	const DlParsed &getLine() const
	{
		return m_InteractiveProperties->m_LineEditor->getLine(m_InteractiveProperties->m_LineNumber);
	}

	InteractiveProperties *m_InteractiveProperties;
	int m_CombineId;
	QString m_UndoMessage;
};

////////////////////////////////////////////////////////////////////////
class InteractiveProperties::PropertiesSpinBoxAlphaHex : public UndoStackDisabler<SupportCopyPasteSpinBox>, public PropertiesWidget
{
	Q_OBJECT

public:
	PropertiesSpinBoxAlphaHex(InteractiveProperties *parent, const QString &undoMessage, int index)
	    : UndoStackDisabler<SupportCopyPasteSpinBox>(parent)
	    , PropertiesWidget(parent, undoMessage)
	    , m_Index(index)
	    , m_SoftMod(false)
	{
		m_SoftMod = true;
		setUndoStack(parent->m_MainWindow->undoStack());
		setKeyboardTracking(false);
		setMinimum(0);
		setMaximum(255);
		setSingleStep(1);
		connect(this, SIGNAL(valueChanged(int)), this, SLOT(updateValue(int)));
	}

	virtual ~PropertiesSpinBoxAlphaHex()
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
		if (m_SoftMod)
			return;
		m_SoftMod = true;
		setValue(getLine().Parameter[m_Index].U >> 24);
		m_SoftMod = false;
		//printf("bye");
	}

private slots:
	void updateValue(int value)
	{
		//printf("updateValue\n");
		if (m_SoftMod)
			return;
		m_SoftMod = true;
		//printf("PropertiesSpinBox::updateValue(value)\n");
		DlParsed parsed = getLine();
		parsed.Parameter[m_Index].U = (parsed.Parameter[m_Index].U & 0xFFFFFF) | (value << 24);
		setLine(parsed);
		m_SoftMod = false;
	}

private:
	int m_Index;
	bool m_SoftMod;
};

////////////////////////////////////////////////////////////////////////

class InteractiveProperties::PropertiesSpinBoxAddress : public UndoStackDisabler<SupportCopyPasteSpinBox>, public PropertiesWidget
{
	Q_OBJECT

public:
	PropertiesSpinBoxAddress(InteractiveProperties *parent, const QString &undoMessage, int index, bool negative)
	    : UndoStackDisabler<SupportCopyPasteSpinBox>(parent)
	    , PropertiesWidget(parent, undoMessage)
	    , m_Index(index)
	    , m_SoftMod(false)
		, m_ReadOut(false)
	{
		m_SoftMod = true;
		setUndoStack(parent->m_MainWindow->undoStack());
		setKeyboardTracking(false);
		setMinimum(negative ? -addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G_END) + 4 : 0);
		setMaximum(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G_END) - 4);
		setSingleStep(4);
		connect(this, SIGNAL(valueChanged(int)), this, SLOT(updateValue(int)));
	}

	virtual ~PropertiesSpinBoxAddress()
	{
	}

	void setReadOut(bool readOut)
	{
		m_ReadOut = true;
		setReadOnly(readOut);
	}

	void done()
	{
		m_SoftMod = false;
		modifiedEditorLine();
	}

	virtual void modifiedEditorLine()
	{
		//printf("modifiedEditorLine %i\n", getLine().Parameter[m_Index].I);
		if (m_SoftMod)
			return;
		m_SoftMod = true;
		int v = m_ReadOut
			? getLine().ReadOut[m_Index].I
			: getLine().Parameter[m_Index].I;
		if (value() != v)
			setValue(v);
		m_SoftMod = false;
		//printf("bye");
	}

private slots:
	void updateValue(int value)
	{
		//printf("updateValue\n");
		if (m_ReadOut)
			return;
		if (m_SoftMod)
			return;
		m_SoftMod = true;
		//printf("PropertiesSpinBox::updateValue(value)\n");
		DlParsed parsed = getLine();
		parsed.Parameter[m_Index].I = value; // (value & 0x7FFFFFFC);
		setLine(parsed);
		m_SoftMod = false;
	}

private:
	int m_Index;
	bool m_SoftMod;
	bool m_ReadOut;

};

////////////////////////////////////////////////////////////////////////

class InteractiveProperties::PropertiesSpinBoxAddressFlash : public UndoStackDisabler<SupportCopyPasteSpinBox>, public PropertiesWidget
{
	Q_OBJECT

public:
	PropertiesSpinBoxAddressFlash(InteractiveProperties *parent, const QString &undoMessage, int index)
	    : UndoStackDisabler<SupportCopyPasteSpinBox>(parent)
	    , PropertiesWidget(parent, undoMessage)
	    , m_Index(index)
	    , m_SoftMod(false)
	{
		m_SoftMod = true;
		setUndoStack(parent->m_MainWindow->undoStack());
		setKeyboardTracking(false);
		setMaximum(g_Flash ? (int)BT8XXEMU_Flash_size(g_Flash) : (0x7FFFFF * 32));
		setSingleStep(64);
		connect(this, SIGNAL(valueChanged(int)), this, SLOT(updateValue(int)));
	}

	virtual ~PropertiesSpinBoxAddressFlash()
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
		if (m_SoftMod)
			return;
		m_SoftMod = true;
		setValue(getLine().Parameter[m_Index].I);
		m_SoftMod = false;
		//printf("bye");
	}

private slots:
	void updateValue(int value)
	{
		//printf("updateValue\n");
		if (m_SoftMod)
			return;
		m_SoftMod = true;
		//printf("PropertiesSpinBox::updateValue(value)\n");
		DlParsed parsed = getLine();
		parsed.Parameter[m_Index].I = value; // (value & 0x7FFFFFFC);
		setLine(parsed);
		m_SoftMod = false;
	}

private:
	int m_Index;
	bool m_SoftMod;
};

////////////////////////////////////////////////////////////////////////

class InteractiveProperties::PropertiesSpinBoxAddressFlashOpt : public UndoStackDisabler<SupportCopyPasteSpinBox>, public PropertiesWidget
{
	Q_OBJECT

public:
	PropertiesSpinBoxAddressFlashOpt(InteractiveProperties *parent, const QString &undoMessage, int index, bool negative)
	    : UndoStackDisabler<SupportCopyPasteSpinBox>(parent)
	    , PropertiesWidget(parent, undoMessage)
	    , m_Index(index)
	    , m_SoftMod(false)
	    , m_Negative(negative)
	{
		m_SoftMod = true;
		setUndoStack(parent->m_MainWindow->undoStack());
		setKeyboardTracking(false);
		setMinimum(m_Negative ? ~(addressMask(FTEDITOR_CURRENT_DEVICE) >> 1) : 0);
		setMaximum(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G_END) - 4);
		setSingleStep(4);
		connect(this, SIGNAL(valueChanged(int)), this, SLOT(updateValue(int)));
	}

	virtual ~PropertiesSpinBoxAddressFlashOpt()
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
		if (m_SoftMod)
			return;
		m_SoftMod = true;
		int addr = getLine().Parameter[m_Index].I;
		if ((addr & 0x800000) && (addr > 0))
		{
			setMinimum(0);
			setMaximum(g_Flash ? (int)BT8XXEMU_Flash_size(g_Flash) : (0x7FFFFF * 32));
			setSingleStep(64);
		}
		else
		{
			setMinimum(m_Negative ? ~(addressMask(FTEDITOR_CURRENT_DEVICE) >> 1) : 0);
			setMaximum(FTEDITOR::addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G_END) - 4);
			setSingleStep(4);
		}
		setValue(((addr & 0x800000) && (addr > 0)) ? (addr & 0x7FFFFF) * 32 : addr);
		m_SoftMod = false;
		//printf("bye");
	}

private slots:
	void updateValue(int value)
	{
		//printf("updateValue\n");
		if (m_SoftMod)
			return;
		m_SoftMod = true;
		//printf("PropertiesSpinBox::updateValue(value)\n");
		DlParsed parsed = getLine();
		int addr = getLine().Parameter[m_Index].I;
		addr = ((addr & 0x800000) && (addr > 0)) ? 0x800000 | ((value / 32) & 0x7FFFFF) : value;
		parsed.Parameter[m_Index].I = addr;
		setLine(parsed);
		m_SoftMod = false;
	}

private:
	int m_Index;
	bool m_SoftMod;
	bool m_Negative;
};

////////////////////////////////////////////////////////////////////////

class InteractiveProperties::PropertiesSpinBox : public UndoStackDisabler<SupportCopyPasteSpinBox>, public PropertiesWidget
{
	Q_OBJECT

public:
	PropertiesSpinBox(InteractiveProperties *parent, const QString &undoMessage, int index)
		: UndoStackDisabler<SupportCopyPasteSpinBox>(parent)
		, PropertiesWidget(parent, undoMessage)
		, m_Index(index)
		, m_SoftMod(false)
		, m_ReadOut(false)
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

	void setReadOut(bool readOut)
	{
		m_ReadOut = readOut;
		setReadOnly(readOut);
	}

	void done()
	{
		m_SoftMod = false;
		modifiedEditorLine();
	}

	virtual void modifiedEditorLine()
	{
		//printf("modifiedEditorLine %i\n", getLine().Parameter[m_Index].I);
		if (m_SoftMod)
			return;
		m_SoftMod = true;
		int v = m_ReadOut
			? getLine().ReadOut[m_Index].I
			: getLine().Parameter[m_Index].I;
		if (value() != v)
			setValue(v);
		m_SoftMod = false;
		//printf("bye");
	}

private slots:
	void updateValue(int value)
	{
		//printf("updateValue\n");
		if (m_ReadOut)
			return;
		if (m_SoftMod)
			return;
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
	bool m_ReadOut;

};

////////////////////////////////////////////////////////////////////////

class InteractiveProperties::PropertiesSpinBoxUInt32 : public UndoStackDisabler<SupportCopyPasteDoubleSpinBox>, public PropertiesWidget
{
	Q_OBJECT

public:
	PropertiesSpinBoxUInt32(InteractiveProperties *parent, const QString &undoMessage, int index)
	    : UndoStackDisabler<SupportCopyPasteDoubleSpinBox>(parent)
	    , PropertiesWidget(parent, undoMessage)
	    , m_Index(index)
	    , m_SoftMod(false)
	{
		m_SoftMod = true;
		setUndoStack(parent->m_MainWindow->undoStack());
		setKeyboardTracking(false);
		connect(this, SIGNAL(valueChanged(double)), this, SLOT(updateValue(double)));
	}

	virtual ~PropertiesSpinBoxUInt32()
	{
	}

	void done()
	{
		m_SoftMod = false;
		modifiedEditorLine();
	}

	virtual void modifiedEditorLine()
	{
		if (m_SoftMod)
			return;
		m_SoftMod = true;

		uint32_t t = getLine().Parameter[m_Index].U;
		setValue(t);
		m_SoftMod = false;
	}

	QString textFromValue(double val) const
	{
		return QString::number((int64_t)val);
	}

private slots:
	void updateValue(double value)
	{
		if (m_SoftMod)
			return;
		m_SoftMod = true;
		DlParsed parsed = getLine();
		parsed.Parameter[m_Index].U = value;
		setLine(parsed);
		m_SoftMod = false;
	}

private:
	int m_Index;
	bool m_SoftMod;
};

////////////////////////////////////////////////////////////////////////

class InteractiveProperties::PropertiesDoubleSpinBox : public UndoStackDisabler<SupportCopyPasteDoubleSpinBox>, public PropertiesWidget
{
	Q_OBJECT

public:
	PropertiesDoubleSpinBox(InteractiveProperties *parent, const QString &undoMessage, int index)
	    : UndoStackDisabler<SupportCopyPasteDoubleSpinBox>(parent)
	    , PropertiesWidget(parent, undoMessage)
	    , m_Index(index)
	    , m_SoftMod(false)
	{
		m_SoftMod = true;
		setUndoStack(parent->m_MainWindow->undoStack());
		setKeyboardTracking(false);
		setMinimum(0x80000000);
		setMaximum(0x7FFFFFFF);
		connect(this, SIGNAL(valueChanged(double)), this, SLOT(updateValue(double)));
	}

	virtual ~PropertiesDoubleSpinBox()
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
		if (m_SoftMod)
			return;
		m_SoftMod = true;
		setValue(getLine().Parameter[m_Index].I);
		m_SoftMod = false;
		//printf("bye");
	}

protected slots:
	virtual void updateValue(double value)
	{
		//printf("updateValue\n");
		if (m_SoftMod)
			return;
		m_SoftMod = true;
		//printf("PropertiesSpinBox::updateValue(value)\n");
		DlParsed parsed = getLine();
		parsed.Parameter[m_Index].I = value;
		setLine(parsed);
		m_SoftMod = false;
	}

protected:
	int m_Index;
	bool m_SoftMod;
};

////////////////////////////////////////////////////////////////////////

class InteractiveProperties::PropertiesSpinBox16 : public InteractiveProperties::PropertiesDoubleSpinBox
{
public:
	PropertiesSpinBox16(InteractiveProperties *parent, const QString &undoMessage, int index)
	    : PropertiesDoubleSpinBox(parent, undoMessage, index)
	{
	}

	virtual ~PropertiesSpinBox16()
	{
	}

protected:
	virtual QString textFromValue(double value) const
	{
		return QString::number((float)value / 16.f);
	}

	virtual double valueFromText(const QString &text) const
	{
		return (int)floorf((text.toFloat() * 16.f) + 0.5f);
	}
};

////////////////////////////////////////////////////////////////////////

class InteractiveProperties::PropertiesSpinBox256 : public InteractiveProperties::PropertiesDoubleSpinBox
{
public:
	PropertiesSpinBox256(InteractiveProperties *parent, const QString &undoMessage, int index)
	    : PropertiesDoubleSpinBox(parent, undoMessage, index)
	{
	}

	virtual ~PropertiesSpinBox256()
	{
	}

protected:
	virtual QString textFromValue(double value) const
	{
		return QString::number((double)value / 256.0);
	}

	virtual double valueFromText(const QString &text) const
	{
		return (int)floor((text.toDouble() * 256.0) + 0.5);
	}
};

////////////////////////////////////////////////////////////////////////

class InteractiveProperties::PropertiesSpinBoxForBitmapTransform : public InteractiveProperties::PropertiesDoubleSpinBox
{
	Q_OBJECT
public:
	PropertiesSpinBoxForBitmapTransform(InteractiveProperties *parent, const QString &undoMessage, int index)
	    : PropertiesDoubleSpinBox(parent, undoMessage, index)
	{
		int p = getLine().Parameter[m_Index - 1].I;

		p ? setDecimals(15) : setDecimals(8);
	}

	virtual ~PropertiesSpinBoxForBitmapTransform()
	{
	}

protected slots:
	virtual void updateValue(double value)
	{
		int p = getLine().Parameter[m_Index - 1].I;
		p ? setDecimals(15) : setDecimals(8);
		PropertiesDoubleSpinBox::updateValue(value);
	}

protected:
	virtual QString textFromValue(double value) const
	{
		int p = getLine().Parameter[m_Index - 1].I;

		if (p)
		{
			return QString::number((double)value / 32768.0, 'g', 16);
		}
		return QString::number((double)value / 256.0, 'g', 16);
	}

	virtual double valueFromText(const QString &text) const
	{
		int p = getLine().Parameter[m_Index - 1].I;

		if (p)
		{
			return (text.toDouble() * 32768.0);
		}
		return (text.toDouble() * 256.0);
	}
};

////////////////////////////////////////////////////////////////////////

class InteractiveProperties::PropertiesSpinBox65536 : public InteractiveProperties::PropertiesDoubleSpinBox
{
public:
	PropertiesSpinBox65536(InteractiveProperties *parent, const QString &undoMessage, int index)
	    : PropertiesDoubleSpinBox(parent, undoMessage, index)
	{
	}

	virtual ~PropertiesSpinBox65536()
	{
	}

protected:
	virtual QString textFromValue(double value) const
	{
		return QString::number((double)value / 65536.0);
	}

	virtual double valueFromText(const QString &text) const
	{
		return (int)floor((text.toDouble() * 65536.0) + 0.5);
	}
};

////////////////////////////////////////////////////////////////////////

class InteractiveProperties::PropertiesSpinBoxAngle65536 : public InteractiveProperties::PropertiesDoubleSpinBox
{
public:
	PropertiesSpinBoxAngle65536(InteractiveProperties *parent, const QString &undoMessage, int index)
	    : PropertiesDoubleSpinBox(parent, undoMessage, index)
	{
	}

	virtual ~PropertiesSpinBoxAngle65536()
	{
	}

protected:
	virtual QString textFromValue(double value) const
	{
		return QString::number(value * 360.0 / 65536.0);
	}

	virtual double valueFromText(const QString &text) const
	{
		return (int)floor((text.toDouble() * (65536.0 / 360.0)) + 0.5);
	}
};

////////////////////////////////////////////////////////////////////////

class InteractiveProperties::PropertiesSpinBoxVertexFormat : public InteractiveProperties::PropertiesDoubleSpinBox
{
public:
	PropertiesSpinBoxVertexFormat(InteractiveProperties *parent, const QString &undoMessage, int index)
	    : PropertiesDoubleSpinBox(parent, undoMessage, index)
	{
	}

	virtual ~PropertiesSpinBoxVertexFormat()
	{
	}

protected:
	virtual QString textFromValue(double value) const
	{
		const DlState &state = m_InteractiveProperties->m_LineEditor->getState(m_InteractiveProperties->m_LineNumber);
		float factor = (float)(1 << state.Graphics.VertexFormat);
		return QString::number((float)value / factor);
	}

	virtual double valueFromText(const QString &text) const
	{
		const DlState &state = m_InteractiveProperties->m_LineEditor->getState(m_InteractiveProperties->m_LineNumber);
		float factor = (float)(1 << state.Graphics.VertexFormat);
		return (int)floorf((text.toFloat() * factor) + 0.5f);
	}
};

////////////////////////////////////////////////////////////////////////

class InteractiveProperties::PropertiesCheckBox : public QCheckBox, public PropertiesWidget
{
	Q_OBJECT

public:
	PropertiesCheckBox(InteractiveProperties *parent, const QString &undoMessage, int index, uint32_t flag)
	    : QCheckBox(parent)
	    , PropertiesWidget(parent, undoMessage)
	    , m_Index(index)
	    , m_Flag(flag)
	    , m_SoftMod(false)
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
		if (m_SoftMod)
			return;
		m_SoftMod = true;

		uint32_t v = getLine().Parameter[m_Index].U;
		bool isChecked = v & 0x80000000 ? Qt::Unchecked : ((v & m_Flag) == m_Flag ? Qt::Checked : Qt::Unchecked);
		setChecked(isChecked);
		m_SoftMod = false;
	}

private slots:
	void updateValue(int value)
	{
		if (m_SoftMod)
			return;
		m_SoftMod = true;
		// printf("PropertiesCheckBox::updateValue(value)\n");
		DlParsed parsed = getLine();
		uint32_t v = parsed.Parameter[m_Index].U;

		if ((v & 0x80000000) == 0)
		{
			v = (value == Qt::Checked) ? (v | m_Flag) : (v & ~m_Flag);
		}

		if ((v & OPT_FORMAT) == 0)
			parsed.VarArgCount = 0;

		parsed.Parameter[m_Index].U = v;
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
	PropertiesLineEdit(InteractiveProperties *parent, const QString &undoMessage, int index)
	    : UndoStackDisabler<QLineEdit>(parent)
	    , PropertiesWidget(parent, undoMessage)
	    , m_Index(index)
	    , m_SoftMod(false)
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
		if (m_SoftMod)
			return;
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
		if (m_SoftMod)
			return;
		m_SoftMod = true;
		// printf("PropertiesLineEdit::updateValue(value)\n");
		DlParsed parsed = getLine();
		QByteArray ba = text.toUtf8();
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
	PropertiesLineEditChar(InteractiveProperties *parent, const QString &undoMessage, int index)
	    : UndoStackDisabler<QLineEdit>(parent)
	    , PropertiesWidget(parent, undoMessage)
	    , m_Index(index)
	    , m_SoftMod(false)
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
		if (m_SoftMod)
			return;
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
		if (m_SoftMod)
			return;
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
	PropertiesSlider(InteractiveProperties *parent, const QString &undoMessage, int index)
	    : QSlider(Qt::Horizontal, parent)
	    , PropertiesWidget(parent, undoMessage)
	    , m_Index(index)
	    , m_SoftMod(false)
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
		if (m_SoftMod)
			return;
		m_SoftMod = true;
		setValue(getLine().Parameter[m_Index].I);
		m_SoftMod = false;
	}

private slots:
	void updateValue(int value)
	{
		if (m_SoftMod)
			return;
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
	PropertiesSliderDyn(InteractiveProperties *parent, const QString &undoMessage, int index, int maxim)
	    : QSlider(Qt::Horizontal, parent)
	    , PropertiesWidget(parent, undoMessage)
	    , m_Index(index)
	    , m_SoftMod(false)
	    , m_Maxim(maxim)
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
		if (m_SoftMod)
			return;
		m_SoftMod = true;
		setMaximum(getLine().Parameter[m_Maxim].I);
		setValue(getLine().Parameter[m_Index].I);
		m_SoftMod = false;
	}

private slots:
	void updateValue(int value)
	{
		if (m_SoftMod)
			return;
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
	PropertiesSliderDynSubClip(InteractiveProperties *parent, const QString &undoMessage, int index, int clip, int maxim)
	    : QSlider(Qt::Horizontal, parent)
	    , PropertiesWidget(parent, undoMessage)
	    , m_Index(index)
	    , m_Clip(clip)
	    , m_SoftMod(false)
	    , m_Maxim(maxim)
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
		if (m_SoftMod)
			return;
		m_SoftMod = true;
		setMaximum(getLine().Parameter[m_Maxim].I);
		setValue(getLine().Parameter[m_Index].I);
		m_SoftMod = false;
	}

private slots:
	void updateValue(int value)
	{
		if (m_SoftMod)
			return;
		m_SoftMod = true;
		// printf("PropertiesSlider::updateValue(value)\n");
		DlParsed parsed = getLine();
		parsed.Parameter[m_Index].I = value;
		int maxim = parsed.Parameter[m_Maxim].I - parsed.Parameter[m_Index].I;
		if (parsed.Parameter[m_Clip].I > maxim)
			parsed.Parameter[m_Clip].I = maxim;
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
	PropertiesSliderDynSub(InteractiveProperties *parent, const QString &undoMessage, int index, int sub, int maxim)
	    : QSlider(Qt::Horizontal, parent)
	    , PropertiesWidget(parent, undoMessage)
	    , m_Index(index)
	    , m_SoftMod(false)
	    , m_Sub(sub)
	    , m_Maxim(maxim)
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
		if (m_SoftMod)
			return;
		m_SoftMod = true;
		setMaximum(getLine().Parameter[m_Maxim].I - getLine().Parameter[m_Sub].I);
		setValue(getLine().Parameter[m_Index].I);
		m_SoftMod = false;
	}

private slots:
	void updateValue(int value)
	{
		if (m_SoftMod)
			return;
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
	PropertiesColor(InteractiveProperties *parent, const QString &undoMessage, int r, int g, int b)
	    : QFrame(parent)
	    , PropertiesWidget(parent, undoMessage)
	    , m_R(r)
	    , m_G(g)
	    , m_B(b)
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
		if (event->button() != Qt::LeftButton)
			return;
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

class InteractiveProperties::PropertiesColorHex : public QFrame, public PropertiesWidget
{
	Q_OBJECT

public:
	PropertiesColorHex(InteractiveProperties *parent, const QString &undoMessage, int rgb)
	    : QFrame(parent)
	    , PropertiesWidget(parent, undoMessage)
	    , m_RGB(rgb)
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

	virtual ~PropertiesColorHex()
	{
	}

	virtual void modifiedEditorLine()
	{
		QPalette p = palette();
		const DlParsed &parsed = getLine();
		int r = (parsed.Parameter[m_RGB].I >> 16) & 0xFF;
		int g = (parsed.Parameter[m_RGB].I >> 8) & 0xFF;
		int b = parsed.Parameter[m_RGB].I & 0xFF;
		p.setColor(QPalette::Window, QColor(r, g, b));
		setPalette(p);
	}

	virtual void mousePressEvent(QMouseEvent *event)
	{
		if (event->button() != Qt::LeftButton)
			return;
		// printf("PropertiesColor::mousePressEvent(event)\n");
		QColor c = QColorDialog::getColor(palette().color(QPalette::Window), this);
		if (c.isValid())
		{
			DlParsed parsed = getLine();
			int rgb = c.red() << 16
			    | c.green() << 8
			    | c.blue();
			parsed.Parameter[m_RGB].U = (parsed.Parameter[m_RGB].U & 0xFF000000) | (rgb & 0xFFFFFF);
			setLine(parsed);
		}
	}

private:
	int m_RGB;
};

////////////////////////////////////////////////////////////////////////

class InteractiveProperties::PropertiesComboBox : public QComboBox, public PropertiesWidget
{
	Q_OBJECT

public:
	PropertiesComboBox(InteractiveProperties *parent, const QString &undoMessage, int index, int begin)
	    : QComboBox(parent)
	    , PropertiesWidget(parent, undoMessage)
	    , m_Index(index)
	    , m_Begin(begin)
	    , m_SoftMod(false)
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
		if (m_SoftMod)
			return;
		m_SoftMod = true;
		setCurrentIndex(getLine().Parameter[m_Index].U - m_Begin);
		m_SoftMod = false;
	}

private slots:
	void updateValue(int value)
	{
		if (m_SoftMod)
			return;
		if (value < 0)
			return;
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
	PropertiesRemapComboBox(InteractiveProperties *parent, const QString &undoMessage, int index, const int *toIntf, int toIntfSz, const int *toEnum, int toEnumSz)
	    : QComboBox(parent)
	    , PropertiesWidget(parent, undoMessage)
	    , m_Index(index)
	    , m_ToIntf(toIntf)
	    , m_ToIntfSz(toIntfSz)
	    , m_ToEnum(toEnum)
	    , m_ToEnumSz(toEnumSz)
	    , m_SoftMod(false)
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
		if (m_SoftMod)
			return;
		m_SoftMod = true;
		setCurrentIndex(m_ToIntf[getLine().Parameter[m_Index].U % m_ToIntfSz]);
		m_SoftMod = false;
	}

private slots:
	void updateValue(int value)
	{
		if (m_SoftMod)
			return;
		if (value < 0)
			return;
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

BT8XXEMU_FORCE_INLINE unsigned int mul255div63(const int &value)
{
	return ((value * 16575) + 65) >> 12;
}

BT8XXEMU_FORCE_INLINE unsigned int mul255div31(const int &value)
{
	return ((value * 8415) + 33) >> 10;
}

BT8XXEMU_FORCE_INLINE unsigned int mul255div15(const int &value)
{
	return ((value * 4335) + 17) >> 8;
}

class InteractiveProperties::PropertiesCaptureButton : public QPushButton, public PropertiesWidget
{
	Q_OBJECT

public:
	PropertiesCaptureButton(InteractiveProperties *parent, const QString &text, const QString &undoMessage)
	    : QPushButton(parent)
	    , PropertiesWidget(parent, undoMessage)
	{
		setText(text);
		connect(this, SIGNAL(clicked()), this, SLOT(actCapture()));
	}

	virtual ~PropertiesCaptureButton()
	{
	}

	virtual void modifiedEditorLine()
	{
	}

private slots:
	void actCapture()
	{
		const DlParsed &parsed = getLine();

		QString filterpng = tr("PNG image (*.png)");
		QString filterjpg = tr("JPG image (*.jpg)");
		QString filterargb4 = tr("Raw ARGB4 image (*.argb4)");
		QString filterargb8 = tr("Raw ARGB8 image (*.argb8)");
		QString filterrgb565 = tr("Raw RGB565 image (*.rgb565)");

		QString filter; // = filterpng + ";;" + filterjpg;

		uint8_t *ram = BT8XXEMU_getRam(g_Emulator);
		uint16_t *ram16 = reinterpret_cast<uint16_t *>(ram);
		int *ram32 = reinterpret_cast<int *>(ram);
		int sourceFormat;
		int memAddress;
		int imageWidth;
		int imageHeight;
		switch (parsed.IdLeft)
		{
		case FTEDITOR_CO_COMMAND:
			switch (parsed.IdRight | FTEDITOR_CO_COMMAND)
			{
			case CMD_SNAPSHOT:
				sourceFormat = ARGB4;
				filter = filterargb4 + ";;" + filterpng + ";; " + filterjpg;
				memAddress = parsed.Parameter[0].I;
				imageWidth = ram32[reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_HSIZE) >> 2];
				imageHeight = ram32[reg(FTEDITOR_CURRENT_DEVICE, FTEDITOR_REG_VSIZE) >> 2];
				break;
			case CMD_SNAPSHOT2:
				sourceFormat = parsed.Parameter[0].I;
				memAddress = parsed.Parameter[1].I;
				imageWidth = parsed.Parameter[4].I;
				imageHeight = parsed.Parameter[5].I;
				switch (parsed.Parameter[0].I)
				{
				case RGB565:
					filter = filterrgb565 + ";;" + filterpng + ";; " + filterjpg;
					break;
				case ARGB4:
					filter = filterargb4 + ";;" + filterpng + ";; " + filterjpg;
					break;
				case ARGB8_SNAPSHOT:
					filter = filterargb8 + ";;" + filterpng + ";; " + filterjpg;
					break;
				default:
					return;
				}
				break;
			default:
				return;
			}
			break;
		default:
			return;
		}

		int bpp;
		switch (sourceFormat)
		{
		case ARGB4:
		case RGB565:
			bpp = 8 * 2;
			break;
		case ARGB8_SNAPSHOT:
			bpp = 8 * 4;
			break;
		default:
			return;
		}

		bool alpha;
		switch (sourceFormat)
		{
		case ARGB4:
		case ARGB8_SNAPSHOT:
			alpha = true;
			break;
		default:
			alpha = false;
		}

		int memSize = (imageWidth * imageHeight * bpp) / 8;

		if (memSize <= 0)
			return;

		if (memSize > addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G_END) - memAddress)
			return;

		QString fileName = QFileDialog::getSaveFileName(this, text(), m_InteractiveProperties->m_MainWindow->getFileDialogPath(), filter, &filter);
		if (fileName.isNull())
			return;

		int targetFormat;
		// 0: no conversion
		// 1: to png
		// 2: to jpg
		if (filter == filterpng)
		{
			targetFormat = 1;
			if (!fileName.endsWith(".png"))
				fileName = fileName + ".png";
		}
		else if (filter == filterjpg)
		{
			targetFormat = 2;
			if (!fileName.endsWith(".jpg"))
				fileName = fileName + ".jpg";
		}
		else if (filter == filterargb4)
		{
			targetFormat = 0;
			if (!fileName.endsWith(".argb4"))
				fileName = fileName + ".argb4";
		}
		else if (filter == filterargb8)
		{
			targetFormat = 0;
			if (!fileName.endsWith(".argb8"))
				fileName = fileName + ".argb8";
		}
		else if (filter == filterrgb565)
		{
			targetFormat = 0;
			if (!fileName.endsWith(".rgb565"))
				fileName = fileName + ".rgb565";
		}
		else
		{
			targetFormat = 0;
		}

		switch (targetFormat)
		{
		case 0:
		{
			QFile file(fileName);
			file.open(QIODevice::WriteOnly);
			QDataStream out(&file);
			out.writeRawData((const char *)(void *)&ram[memAddress], memSize);
			break;
		}
		case 1:
		case 2:
		{
			QImage image(imageWidth, imageHeight, QImage::Format_RGB32);
			argb8888 *buffer = (argb8888 *)(void *)&ram[memAddress];
			switch (sourceFormat)
			{
			case ARGB4:
				for (int y = 0; y < imageHeight; ++y)
				{
					argb8888 *scanline = (argb8888 *)(void *)image.scanLine(y);
					uint16_t *input = (uint16_t *)(void *)&ram[memAddress + (y * (imageWidth * 2))];
					for (int x = 0; x < imageWidth; ++x)
					{
						uint16_t val = input[x];
						scanline[x] = (mul255div15(val >> 12) << 24)
						    | (mul255div15((val >> 8) & 0xF) << 16)
						    | (mul255div15((val >> 4) & 0xF) << 8)
						    | (mul255div15(val & 0xF));
					}
				}
				break;
			case RGB565:
				for (int y = 0; y < imageHeight; ++y)
				{
					argb8888 *scanline = (argb8888 *)(void *)image.scanLine(y);
					uint16_t *input = (uint16_t *)(void *)&ram[memAddress + (y * (imageWidth * 2))];
					for (int x = 0; x < imageWidth; ++x)
					{
						uint16_t val = input[x];
						scanline[x] = 0xFF000000 // todo opt
						    | (mul255div31(val >> 11) << 16)
						    | (mul255div63((val >> 5) & 0x3F) << 8)
						    | mul255div31(val & 0x1F);
					}
				}
				break;
			case ARGB8_SNAPSHOT:
				for (int y = 0; y < imageHeight; ++y)
					memcpy(image.scanLine(y), &buffer[y * imageWidth], sizeof(argb8888) * imageWidth);
				break;
			}
			image.save(fileName);
			break;
		}
		}

		// Save screenshot
		/*const QPixmap &pixmap = m_EmulatorViewport->getPixMap();
		pixmap.save(fileName);*/

		// TODO: Automatically add to content manager
	}
};

////////////////////////////////////////////////////////////////////////

} /* namespace FTEDITOR */

#endif /* #ifndef FTEDITOR_INTERACTIVE_WIDGETS_H */

/* end of file */
