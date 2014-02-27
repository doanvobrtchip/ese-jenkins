/**
 * bitmap_setup.cpp
 * $Id$
 * \file bitmap_setup.cpp
 * \brief bitmap_setup.cpp
 * \date 2014-02-25 13:59GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#include "bitmap_setup.h"

// STL includes
#include <stdio.h>

// Qt includes
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QResizeEvent>
#include <QLabel>
#include <QGraphicsDropShadowEffect>
#include <QGroupBox>
#include <QSpinBox>
#include <QComboBox>
#include <QUndoCommand>

// Emulator includes
#include <ft800emu_graphics_processor.h>
#include <vc.h>

// Project includes
#include "properties_editor.h"
#include "content_manager.h"
#include "main_window.h"

using namespace std;

namespace FT800EMUQT {

////////////////////////////////////////////////////////////////////////
// BitmapWidget
////////////////////////////////////////////////////////////////////////

BitmapWidget::BitmapWidget(MainWindow *parent, int index) : QFrame(parent), m_MainWindow(parent), m_Index(index), m_PixmapOkay(false), m_ThreadRunning(false), m_ReloadRequested(false)
{
	QVBoxLayout *l = new QVBoxLayout();
	m_Label = new QLabel(this);
	l->addWidget(m_Label);
	setLayout(l);

	QVBoxLayout *layout = new QVBoxLayout();
	QLabel *label = new QLabel(this);
	label->setText(QString("<i>") + QString::number(index) + "<i> ");
	layout->addWidget(label);
	label->setWordWrap(true);
	label->setAlignment(Qt::AlignRight | Qt::AlignBottom);
	m_Label->setLayout(layout);

	/*QMargins cm = layout->contentsMargins();
	cm.setTop(cm.top() / 2);
	cm.setBottom(cm.bottom() / 2);
	cm.setLeft(cm.left() / 2);
	cm.setRight(cm.right() / 2);
	layout->setContentsMargins(cm);*/

	QMargins cm = layout->contentsMargins();
	label->setMargin(cm.left() / 4);
	layout->setContentsMargins(0, 0, 0, 0);
	l->setContentsMargins(0, 0, 0, 0);

	m_DefaultPalette = palette();
	m_SelectedPalette = m_DefaultPalette;
	m_SelectedPalette.setColor(QPalette::WindowText, Qt::red);

	deselect();

	QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect(this);

	effect->setBlurRadius(0);
	effect->setColor(palette().color(QPalette::Background));
	effect->setOffset(1,1);
	label->setGraphicsEffect(effect);

	m_ReloadThread = new BitmapWidgetThread();
	m_ReloadThread->m_BitmapWidget = this;
	connect(m_ReloadThread, SIGNAL(finished()), this, SLOT(threadFinished()));

	QSizePolicy policy(QSizePolicy::Ignored, QSizePolicy::Ignored);
	setSizePolicy(policy);
}

BitmapWidget::~BitmapWidget()
{

}

void BitmapWidget::mousePressEvent(QMouseEvent *event)
{
	m_MainWindow->bitmapSetup()->select(m_Index);
}

void BitmapWidget::resizeEvent(QResizeEvent *event)
{
	QFrame::resizeEvent(event);

	if (m_PixmapOkay)
	{
		// Rescale locally
		/*m_Pixmap = m_Pixmap.scaled(width(), height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		m_Label->setPixmap(m_Pixmap);
		m_Label->repaint();*/
	}

	// Reload from file
	reloadInternal();
}

void BitmapWidget::select()
{
	setPalette(m_SelectedPalette);
	setFrameStyle(QFrame::Panel | QFrame::Plain);
}

void BitmapWidget::deselect()
{
	setPalette(m_DefaultPalette);
	setFrameStyle(QFrame::Panel | QFrame::Sunken);
}

void BitmapWidget::setImage(const QString &name)
{
	printf("BitmapWidget::setImage(name)\n");

	// if (m_ImageName == name) return; // Also used for refreshing...

	m_ImageName = name;
	reloadInternal();
}

void BitmapWidget::unsetImage()
{
	printf("BitmapWidget::unsetImage(name)\n");

	m_ImageName = "";
	m_Pixmap = QPixmap();
	m_Label->setPixmap(m_Pixmap);
	m_Label->repaint();
	m_PixmapOkay = false;
}

void BitmapWidget::reloadInternal()
{
	// printf("BitmapWidget::reloadInternal()\n");

	m_Mutex.lock();
	m_ReloadRequested = true;
	m_ReloadWidth = m_Label->width();
	m_ReloadHeight = m_Label->height();
	m_ReloadName = m_ImageName;
	m_Mutex.unlock();
	if (!m_ThreadRunning && !m_ReloadName.isEmpty())
	{
		m_ThreadRunning = true;
		m_ReloadThread->start();
	}
}

void BitmapWidget::threadFinished()
{
	// printf("BitmapWidget::threadFinished()\n");

	m_ThreadRunning = false;
	if (!m_ImageName.isEmpty()) // may be empty after unset while thread running
	{
		if (m_ReloadSuccess)
		{
			m_Pixmap.convertFromImage(m_ReloadImage);
		}
		else
		{
			m_Pixmap = QPixmap();
		}
		m_Label->setPixmap(m_Pixmap);
		m_Label->repaint();
		m_PixmapOkay = m_ReloadSuccess;
	}
	if (m_ReloadRequested)
	{
		m_ThreadRunning = true;
		m_ReloadThread->start();
	}
}

void BitmapWidgetThread::run()
{
	// printf("BitmapWidgetThread::run()\n");

	m_BitmapWidget->m_Mutex.lock();
	m_BitmapWidget->m_ReloadRequested = false;
	int width = m_BitmapWidget->m_ReloadWidth;
	int height = m_BitmapWidget->m_ReloadHeight;
	QString name = m_BitmapWidget->m_ReloadName;
	m_BitmapWidget->m_Mutex.unlock();

	QImage image;
	m_BitmapWidget->m_ReloadSuccess = image.load(name + "_converted.png");
	if (!m_BitmapWidget->m_ReloadSuccess) { printf("Bitmap widget reload failed\n"); return; }
	m_BitmapWidget->m_ReloadImage = image.scaled(width, height, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation).copy(0, 0, width, height);
}

////////////////////////////////////////////////////////////////////////
// BitmapSetup
////////////////////////////////////////////////////////////////////////

void addLabeledWidget(QWidget *parent, QVBoxLayout *layout, const QString &label, QWidget *widget);
void addLabeledWidget(QWidget *parent, QVBoxLayout *layout, const QString &label, QWidget *widget0, QWidget *widget1);

BitmapSetup::BitmapSetup(MainWindow *parent) : QWidget(parent), m_MainWindow(parent), m_Selected(-1), m_ModificationNb(0), m_RebuildingPropSourceContent(false)
{
	QGridLayout *layout = new QGridLayout();

	for (int i = 0; i < 32; ++i)
	{
		m_Bitmaps[i] = new BitmapWidget(parent, i);
		layout->addWidget(m_Bitmaps[i], i / 4, i % 4);
		m_BitmapSource[i] = NULL;
	}

	layout->setRowStretch(32 / 4, 1);

	setLayout(layout);

	/*QMargins cm = layout->contentsMargins();
	cm.setTop(cm.top() / 2);
	cm.setBottom(cm.bottom() / 2);
	cm.setLeft(cm.left() / 2);
	cm.setRight(cm.right() / 2);
	layout->setContentsMargins(cm);*/

	connect(m_MainWindow->propertiesEditor(), SIGNAL(setterChanged(QWidget *)), this, SLOT(propertiesSetterChanged(QWidget *)));

	// Build properties windows
	m_PropSource = new QGroupBox(this);
	QVBoxLayout *sourceLayout = new QVBoxLayout();
	m_PropSource->setHidden(true);
	m_PropSource->setTitle(tr("Source"));
	m_PropSourceContent = new QComboBox(this);
	sourceLayout->addWidget(m_PropSourceContent);
	connect(m_PropSourceContent, SIGNAL(currentIndexChanged(int)), this, SLOT(propSourceContentChanged(int)));
	/*m_PropAddress = new QSpinBox(this);
	m_PropAddress->setMinimum(0);
	m_PropAddress->setMaximum(RAM_DL - 4);
	m_PropAddress->setSingleStep(4);
	m_PropAddress->setKeyboardTracking(false);
	sourceLayout->addWidget(m_PropAddress);*/
	m_PropSource->setLayout(sourceLayout);

	m_PropLayout = new QGroupBox(this);
	QVBoxLayout *layoutLayout = new QVBoxLayout();
	m_PropLayout->setHidden(true);
	m_PropLayout->setTitle(tr("Layout"));
	m_PropLayoutFormat = new QComboBox(this);
	m_PropLayoutFormat->addItem("ARGB1555");
	m_PropLayoutFormat->addItem("L1");
	m_PropLayoutFormat->addItem("L4");
	m_PropLayoutFormat->addItem("L8");
	m_PropLayoutFormat->addItem("RGB332");
	m_PropLayoutFormat->addItem("ARGB2");
	m_PropLayoutFormat->addItem("ARGB4");
	m_PropLayoutFormat->addItem("RGB565");
	m_PropLayoutFormat->addItem("PALETTED");
	addLabeledWidget(this, layoutLayout, tr("Format: "), m_PropLayoutFormat);
	// connect(m_PropLayoutFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(propLayoutFormatChanged(int)));
	m_PropLayoutStride = new QSpinBox(this);
	m_PropLayoutStride->setMinimum(0);
	m_PropLayoutStride->setMaximum(2048);
	m_PropLayoutStride->setSingleStep(1);
	// m_PropLayoutStride->setKeyboardTracking(false);
	addLabeledWidget(this, layoutLayout, tr("Stride: "), m_PropLayoutStride);
	// connect(m_PropLayoutStride, SIGNAL(valueChanged(int)), this, SLOT(propLayoutStrideChanged(int)));
	m_PropLayoutHeight = new QSpinBox(this);
	m_PropLayoutHeight->setMinimum(0);
	m_PropLayoutHeight->setMaximum(512);
	m_PropLayoutHeight->setSingleStep(1);
	// m_PropLayoutHeight->setKeyboardTracking(false);
	addLabeledWidget(this, layoutLayout, tr("Height: "), m_PropLayoutHeight);
	// connect(m_PropLayoutHeight, SIGNAL(valueChanged(int)), this, SLOT(propLayoutHeightChanged(int)));
	// TODO: Reset button
	m_PropLayout->setLayout(layoutLayout);

	m_PropSize = new QGroupBox(this);
	QVBoxLayout *sizeLayout = new QVBoxLayout();
	m_PropSize->setHidden(true);
	m_PropSize->setTitle(tr("Size"));
	m_PropSize->setLayout(sizeLayout);


	/*
	 *

	QGroupBox *m_PropLayout; // BITMAP_LAYOUT(format, linestride, height)
	QComboBox *m_PropLayoutFormat;
	QSpinBox *m_PropLayoutStride;
	QSpinBox *m_PropLayoutHeight;

	QGroupBox *m_PropSize; // BITMAP_SIZE(filter, wrapx, wrapy, width, height)
	QComboBox *m_PropSizeWrapX;
	QComboBox *m_PropSizeWrapY;
	QSpinBox *m_PropSizeWidth;
	QSpinBox *m_PropSizeHeight;
	 *
	 * */
}

BitmapSetup::~BitmapSetup()
{

}

void BitmapSetup::select(int i)
{
	printf("BitmapSetup::select(i)\n");

	if (m_Selected == i) return;

	// Update local GUI
	deselect();
	m_Bitmaps[i]->select();

	// Set new selection
	m_Selected = i;

	// TEST
	// m_Bitmaps[i]->setImage("photo_redglasses_2");

	// Construct properties GUI
	rebuildGUIInternal();
}

bool isValidContent(ContentInfo *info)
{
	return (info->MemoryLoaded) && (info->Converter == ContentInfo::Image || info->Converter == ContentInfo::Raw || info->Converter == ContentInfo::RawJpeg);
}

void BitmapSetup::rebuildGUIInternal()
{
	printf("BitmapSetup::rebuildGUIInternal()\n");
	// NOTE: Changes to ContentManager::contentInfos require rebuildGUIInternal() call

	// Construct properties GUI
	ContentManager *cm = m_MainWindow->contentManager();
	PropertiesEditor *props = m_MainWindow->propertiesEditor();
	bool hasLoadedContent = cm->getContentCount() > 0;
	std::vector<ContentInfo *> contentInfos;
	if (hasLoadedContent)
	{
		// Check if all content loaded
		cm->getContentInfos(contentInfos);
		hasLoadedContent = false;
		for (std::vector<ContentInfo *>::iterator it(contentInfos.begin()), end(contentInfos.end()); it != end; ++it)
		{
			if (isValidContent(*it))
			{
				// Found at least one content source that is loaded into RAM_G
				hasLoadedContent = true;
				break;
			}
		}
	}
	if (!hasLoadedContent)
	{
		// No loaded content, show a helpful message
		props->setInfo(tr("To add a new bitmap handle to the project, the bitmap must first be loaded into the device's global memory.<br><br>Use the <b>Content</b> tab to convert an image to the appropriate format and load it into device memory."));
		props->setEditWidget(NULL, false, NULL);
	}
	else
	{
		// Setup widgets
		std::vector<QWidget *> widgets;
		widgets.push_back(m_PropSource);
		widgets.push_back(m_PropLayout);
		widgets.push_back(m_PropSize);
		props->setEditWidgets(widgets, false, this);

		// Add contents to the combobox
		m_RebuildingPropSourceContent = true;
		m_PropSourceContent->clear();
		m_PropSourceContent->addItem("", qVariantFromValue((void *)(NULL)));
		for (std::vector<ContentInfo *>::iterator it(contentInfos.begin()), end(contentInfos.end()); it != end; ++it)
		{
			ContentInfo *info = *it;
			if (isValidContent(info))
			{
				m_PropSourceContent->addItem((*it)->DestName, qVariantFromValue((void *)info));
				// ref (ContentInfo *)(*it)->data(0, Qt::UserRole).value<void *>();
				// ref view->setData(0, Qt::UserRole, qVariantFromValue((void *)contentInfo));
			}
		}
		m_RebuildingPropSourceContent = false;

		// Refresh GUI contents
		refreshGUIInternal();
	}
}

void BitmapSetup::refreshViewInternal(int bitmapHandle)
{
	printf("BitmapSetup::refreshViewInternal(bitmapHandle)\n");

	// Update the preview icon
	ContentManager *cm = m_MainWindow->contentManager();
	if (cm->isValidContent(m_BitmapSource[bitmapHandle]) && isValidContent(m_BitmapSource[bitmapHandle]))
	{
		m_Bitmaps[bitmapHandle]->setImage(m_BitmapSource[bitmapHandle]->DestName);
	}
	else
	{
		m_Bitmaps[bitmapHandle]->unsetImage();
	}
}

void BitmapSetup::refreshGUIInternal(int bitmapHandle)
{
	// NOTE: Local changes (from undo stack) to BitmapSetup require refreshGUIInternal(bitmapHandle) if current
	if (bitmapHandle == m_Selected)
		refreshGUIInternal();
}

void BitmapSetup::refreshGUIInternal() // acts on m_Selected as bitmapHandle
{
	printf("BitmapSetup::refreshGUIInternal()\n");

	ContentManager *cm = m_MainWindow->contentManager();
	PropertiesEditor *props = m_MainWindow->propertiesEditor();

	bool sourceContentSet = false;
	for (int i = 0; i < m_PropSourceContent->count(); ++i)
	{
		ContentInfo *info = (ContentInfo *)m_PropSourceContent->itemData(i, Qt::UserRole).value<void *>();
		if (info == m_BitmapSource[m_Selected])
		{
			m_PropSourceContent->setCurrentIndex(i);
			sourceContentSet = true;
			break;
		}
	}
	if (!sourceContentSet)
	{
		// Content no longer exists
		m_RebuildingPropSourceContent = true;
		m_PropSourceContent->addItem(QString::number((intptr_t)m_BitmapSource[m_Selected]), qVariantFromValue((void *)m_BitmapSource[m_Selected]));
		m_RebuildingPropSourceContent = false;
		m_PropSourceContent->setCurrentIndex(m_PropSourceContent->count() - 1);
	}

	props->setInfo("<b>BITMAP_HANDLE</b>(" + QString::number(m_Selected) + ")");
}

void BitmapSetup::deselect()
{
	printf("BitmapSetup::deselect()\n");

	if (m_Selected == -1) return;

	for (int i = 0; i < 32; ++i)
		m_Bitmaps[i]->deselect();

	PropertiesEditor *props = m_MainWindow->propertiesEditor();
	if (props->getEditWidgetSetter() == this)
	{
		props->setInfo("");
		props->setEditWidget(NULL, false, NULL);
	}

	m_Selected = -1;
}

void BitmapSetup::propertiesSetterChanged(QWidget *setter)
{
	printf("BitmapSetup::propertiesSetterChanged(setter)\n");

	if (setter != this)
	{
		deselect();
	}
}

void BitmapSetup::resizeEvent(QResizeEvent *event)
{
	QWidget::resizeEvent(event);

	int width = m_Bitmaps[0]->width();

	for (int i = 0; i < 32 / 4; ++i)
	{
		((QGridLayout *)layout())->setRowMinimumHeight(i, width);
	}
}

/*void BitmapSetup::lockBitmaps()
{
	m_Mutex.lock();
}

void BitmapSetup::unlockBitmaps()
{
	m_Mutex.unlock();
}*/

void BitmapSetup::reloadContent(ContentInfo *info)
{
	// If currently selected GUI dependent, rebuild it completely
	if (m_Selected >= 0 && m_BitmapSource[m_Selected] == info)
		rebuildGUIInternal();

	// Refresh all of the thumbnails (even if not necessary... simplifies the code)
	for (int i = 0; i < 32; ++i)
	{
		if (m_BitmapSource[i] == info)
			refreshViewInternal(i);
	}
}

////////////////////////////////////////////////////////////////////////
// Undo/Redo
////////////////////////////////////////////////////////////////////////

class BitmapSetup::ChangeSourceContent : public QUndoCommand
{
public:
	ChangeSourceContent(BitmapSetup *bitmapSetup, int bitmapHandle, ContentInfo *value) :
		QUndoCommand(),
		m_BitmapSetup(bitmapSetup),
		m_BitmapHandle(bitmapHandle),
		m_OldValue(bitmapSetup->m_BitmapSource[bitmapHandle]),
		m_NewValue(value)
	{
		setText(tr("Bitmap Handle: Source content"));
	}

	virtual ~ChangeSourceContent()
	{

	}

	virtual void undo()
	{
		m_BitmapSetup->m_BitmapSource[m_BitmapHandle] = m_OldValue;
		m_BitmapSetup->refreshViewInternal(m_BitmapHandle);
		m_BitmapSetup->refreshGUIInternal(m_BitmapHandle);
	}

	virtual void redo()
	{
		m_BitmapSetup->m_BitmapSource[m_BitmapHandle] = m_NewValue;
		m_BitmapSetup->refreshViewInternal(m_BitmapHandle);
		m_BitmapSetup->refreshGUIInternal(m_BitmapHandle);
	}

	virtual int id() const
	{
		return 9065500;
	}

	virtual bool mergeWith(const QUndoCommand *command)
	{
		const ChangeSourceContent *c = static_cast<const ChangeSourceContent *>(command);

		if (c->m_BitmapHandle != m_BitmapHandle)
			return false;

		if (c->m_NewValue == m_OldValue)
			return false;

		m_NewValue = c->m_NewValue;
		return true;
	}

private:
	BitmapSetup *m_BitmapSetup;
	int m_BitmapHandle;
	ContentInfo *m_OldValue;
	ContentInfo *m_NewValue;
};

void BitmapSetup::changeSourceContent(int bitmapHandle, ContentInfo *value)
{
	printf("BitmapSetup::changeSourceContent(bitmapHandle, value)\n");

	// Create undo/redo
	ChangeSourceContent *changeSourceContent = new ChangeSourceContent(this, bitmapHandle, value);
	m_MainWindow->undoStack()->push(changeSourceContent);
}

void BitmapSetup::propSourceContentChanged(int value)
{
	printf("BitmapSetup::propSourceContentChanged(value)\n");

	if (m_RebuildingPropSourceContent)
	{
		printf("Rebuilding combo box, ignore\n");
		return;
	}

	ContentInfo *info = (ContentInfo *)m_PropSourceContent->itemData(value, Qt::UserRole).value<void *>();

	// printf("info: %p, selected: %i, selected info: %p\n", info, m_Selected, m_BitmapSource[m_Selected]);

	if (m_Selected >= 0 && m_BitmapSource[m_Selected] != info)
		changeSourceContent(m_Selected, info);
}

////////////////////////////////////////////////////////////////////////

} /* namespace FT800EMUQT */

/* end of file */
