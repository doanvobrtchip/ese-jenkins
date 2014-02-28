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
#include <QJsonArray>
#include <QJsonObject>
#include <QDrag>
#include <QMimeData>

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

BitmapWidget::BitmapWidget(MainWindow *parent, int index) : QFrame(parent), m_MainWindow(parent), m_Index(index), m_PixmapOkay(false), m_MouseDown(false), m_ThreadRunning(false), m_ReloadRequested(false)
{
	setMouseTracking(true);
	// setAcceptDrops(true); // TODO: Handle swapping!

	QVBoxLayout *l = new QVBoxLayout();
	m_Label = new QLabel(this);
	m_Label->setAttribute(Qt::WA_TransparentForMouseEvents);
	l->addWidget(m_Label);
	setLayout(l);

	QVBoxLayout *layout = new QVBoxLayout();
	QLabel *label = new QLabel(this);
	label->setText(QString("<i>") + QString::number(index) + "<i> ");
	label->setAttribute(Qt::WA_TransparentForMouseEvents);
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
	m_MouseDown = (event->button() == Qt::LeftButton);
}

void BitmapWidget::mouseReleaseEvent(QMouseEvent *event)
{
	m_MouseDown = false;
}

void BitmapWidget::mouseMoveEvent(QMouseEvent *event)
{
	if (m_MouseDown)
	{
		m_MouseDown = false;
		if (m_PixmapOkay)
		{
			printf("Bitmap handle drag from %p (parent %p)\n", this, m_MainWindow->bitmapSetup());
			QDrag *drag = new QDrag(m_MainWindow->bitmapSetup());
			drag->setPixmap(m_Pixmap);
			drag->setHotSpot(event->pos() - QPoint(1, 1));
			QMimeData *mimeData = new QMimeData();
			mimeData->setText(QString("BITMAP_HANDLE(") + QString::number(m_Index) + ")");
			drag->setMimeData(mimeData);
			drag->exec();
		}
	}
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
		memset(&m_BitmapInfo[i], 0, sizeof(FT800EMU::BitmapInfo));
		m_BitmapSourceExists[i] = false;
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
	connect(m_PropLayoutFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(propLayoutFormatChanged(int)));
	m_PropLayoutStride = new QSpinBox(this);
	m_PropLayoutStride->setMinimum(0);
	m_PropLayoutStride->setMaximum(2048);
	m_PropLayoutStride->setSingleStep(1);
	// m_PropLayoutStride->setKeyboardTracking(false);
	addLabeledWidget(this, layoutLayout, tr("Stride: "), m_PropLayoutStride);
	connect(m_PropLayoutStride, SIGNAL(valueChanged(int)), this, SLOT(propLayoutStrideChanged(int)));
	m_PropLayoutHeight = new QSpinBox(this);
	m_PropLayoutHeight->setMinimum(0);
	m_PropLayoutHeight->setMaximum(512);
	m_PropLayoutHeight->setSingleStep(1);
	// m_PropLayoutHeight->setKeyboardTracking(false);
	addLabeledWidget(this, layoutLayout, tr("Height: "), m_PropLayoutHeight);
	connect(m_PropLayoutHeight, SIGNAL(valueChanged(int)), this, SLOT(propLayoutHeightChanged(int)));
	// TODO: Reset button
	m_PropLayout->setLayout(layoutLayout);

	m_PropSize = new QGroupBox(this);
	QVBoxLayout *sizeLayout = new QVBoxLayout();
	m_PropSize->setHidden(true);
	m_PropSize->setTitle(tr("Size"));
	m_PropSizeFilter = new QComboBox(this);
	m_PropSizeFilter->addItem("NEAREST");
	m_PropSizeFilter->addItem("BILINEAR");
	addLabeledWidget(this, sizeLayout, tr("Filter: "), m_PropSizeFilter);
	connect(m_PropSizeFilter, SIGNAL(currentIndexChanged(int)), this, SLOT(propSizeFilterChanged(int)));
	m_PropSizeWrapX = new QComboBox(this);
	m_PropSizeWrapX->addItem("BORDER");
	m_PropSizeWrapX->addItem("REPEAT");
	addLabeledWidget(this, sizeLayout, tr("Wrap X: "), m_PropSizeWrapX);
	connect(m_PropSizeWrapX, SIGNAL(currentIndexChanged(int)), this, SLOT(propSizeWrapXChanged(int)));
	m_PropSizeWrapY = new QComboBox(this);
	m_PropSizeWrapY->addItem("BORDER");
	m_PropSizeWrapY->addItem("REPEAT");
	addLabeledWidget(this, sizeLayout, tr("Wrap Y: "), m_PropSizeWrapY);
	connect(m_PropSizeWrapY, SIGNAL(currentIndexChanged(int)), this, SLOT(propSizeWrapYChanged(int)));
	m_PropSizeWidth = new QSpinBox(this);
	m_PropSizeWidth->setMinimum(0);
	m_PropSizeWidth->setMaximum(512);
	m_PropSizeWidth->setSingleStep(1);
	// m_PropSizeWidth->setKeyboardTracking(false);
	addLabeledWidget(this, sizeLayout, tr("Width: "), m_PropSizeWidth);
	connect(m_PropSizeWidth, SIGNAL(valueChanged(int)), this, SLOT(propSizeWidthChanged(int)));
	m_PropSizeHeight = new QSpinBox(this);
	m_PropSizeHeight->setMinimum(0);
	m_PropSizeHeight->setMaximum(512);
	m_PropSizeHeight->setSingleStep(1);
	// m_PropSizeHeight->setKeyboardTracking(false);
	addLabeledWidget(this, sizeLayout, tr("Height: "), m_PropSizeHeight);
	connect(m_PropSizeHeight, SIGNAL(valueChanged(int)), this, SLOT(propSizeHeightChanged(int)));
	m_PropSize->setLayout(sizeLayout);
}

BitmapSetup::~BitmapSetup()
{

}

void BitmapSetup::clear()
{
	for (int i = 0; i < 32; ++i)
	{
		m_BitmapSourceExists[i] = false;
		m_BitmapSource[i] = NULL;
		memset(&m_BitmapInfo[i], 0, sizeof(FT800EMU::BitmapInfo));
		refreshGUIInternal(i);
		refreshViewInternal(i);
	}
}

QJsonArray BitmapSetup::toJson() const
{
	QJsonArray bitmaps;
	for (int i = 0; i < 32; ++i)
	{
		QJsonObject j;

		if (m_BitmapSource[i] && m_BitmapSourceExists[i])
		{
			// Source
			j["sourceContent"] = m_BitmapSource[i]->DestName;

			// Layout
			if (m_BitmapSource[i]->Converter != ContentInfo::Image)
			{
				j["layoutFormat"] = m_BitmapInfo[i].LayoutFormat;
				j["layoutStride"] = m_BitmapInfo[i].LayoutStride;
				j["layoutHeight"] = m_BitmapInfo[i].LayoutHeight;
			}

			// Size
			j["sizeFilter"] = m_BitmapInfo[i].SizeFilter;
			j["sizeWrapX"] = m_BitmapInfo[i].SizeWrapX;
			j["sizeWrapY"] = m_BitmapInfo[i].SizeWrapY;
			j["sizeWidth"] = m_BitmapInfo[i].SizeWidth;
			j["sizeHeight"] = m_BitmapInfo[i].SizeHeight;
		}

		bitmaps.push_back(j);
	}
	return bitmaps;
}

void BitmapSetup::fromJson(QJsonArray &bitmaps)
{
	for (int i = 0; i < 32; ++i)
	{
		QJsonObject j = bitmaps[i].toObject();

		// Source
		m_BitmapSourceExists[i] = false;
		m_BitmapSource[i] = m_MainWindow->contentManager()->find(j["sourceContent"].toString());
		m_BitmapSourceExists[i] = (m_BitmapSource[i] != NULL);

		if (m_BitmapSourceExists[i])
		{
			// Layout
			if (m_BitmapSource[i]->Converter == ContentInfo::Image)
			{
				m_MainWindow->contentManager()->cacheImageInfo(m_BitmapSource[i]);
				m_BitmapInfo[i].LayoutFormat = m_BitmapSource[i]->ImageFormat;
				m_BitmapInfo[i].LayoutStride = m_BitmapSource[i]->CachedImageStride;
				m_BitmapInfo[i].LayoutHeight = m_BitmapSource[i]->CachedImageHeight;
			}
			else
			{
				m_BitmapInfo[i].LayoutFormat = ((QJsonValue)j["layoutFormat"]).toVariant().toInt();
				m_BitmapInfo[i].LayoutStride = ((QJsonValue)j["layoutStride"]).toVariant().toInt();
				m_BitmapInfo[i].LayoutHeight = ((QJsonValue)j["layoutHeight"]).toVariant().toInt();
			}

			// Size
			m_BitmapInfo[i].SizeFilter = ((QJsonValue)j["sizeFilter"]).toVariant().toInt();
			m_BitmapInfo[i].SizeWrapX = ((QJsonValue)j["sizeWrapX"]).toVariant().toInt();
			m_BitmapInfo[i].SizeWrapY = ((QJsonValue)j["sizeWrapY"]).toVariant().toInt();
			m_BitmapInfo[i].SizeWidth = ((QJsonValue)j["sizeWidth"]).toVariant().toInt();
			m_BitmapInfo[i].SizeHeight = ((QJsonValue)j["sizeHeight"]).toVariant().toInt();
		}

		refreshGUIInternal(i);
		refreshViewInternal(i);
	}
	++m_ModificationNb;
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

	// Be helpful
	m_MainWindow->focusProperties();
}

void BitmapSetup::refreshViewInternal(int bitmapHandle)
{
	printf("BitmapSetup::refreshViewInternal(bitmapHandle)\n");

	// Update the preview icon
	ContentManager *cm = m_MainWindow->contentManager();
	if (m_BitmapSourceExists[bitmapHandle] && isValidContent(m_BitmapSource[bitmapHandle]))
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

static const char *formatOption[] = {
	"ARGB1555",
	"L1",
	"L4",
	"L8",
	"RGB332",
	"ARGB2",
	"ARGB4",
	"RGB565",
	"PALETTED"
};

static const char *wrapOption[] = {
	"BORDER",
	"REPEAT"
};

static const char *filterOption[] = {
	"NEAREST",
	"BILINEAR"
};

void BitmapSetup::refreshGUIInternal() // acts on m_Selected as bitmapHandle
{
	printf("BitmapSetup::refreshGUIInternal()\n");

	ContentManager *cm = m_MainWindow->contentManager();
	PropertiesEditor *props = m_MainWindow->propertiesEditor();

	// Source
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
		// Content no longer exists, add a false value
		m_RebuildingPropSourceContent = true;
		m_PropSourceContent->addItem(QString::number((intptr_t)m_BitmapSource[m_Selected]), qVariantFromValue((void *)m_BitmapSource[m_Selected]));
		m_RebuildingPropSourceContent = false;
		m_PropSourceContent->setCurrentIndex(m_PropSourceContent->count() - 1);
	}

	if (m_BitmapSource[m_Selected] && sourceContentSet)
	{
		if (m_BitmapSource[m_Selected]->Converter != ContentInfo::Image) // Image converter uses layout cached in ContentInfo directly
		{
			// Layout
			m_PropLayout->setVisible(true);
			m_PropLayoutFormat->setCurrentIndex(m_BitmapInfo[m_Selected].LayoutFormat);
			m_PropLayoutStride->setValue(m_BitmapInfo[m_Selected].LayoutStride);
			m_PropLayoutHeight->setValue(m_BitmapInfo[m_Selected].LayoutHeight);
		}
		else
		{
			m_PropLayout->setVisible(false);
		}

		// Size
		m_PropSize->setVisible(true);
		m_PropSizeFilter->setCurrentIndex(m_BitmapInfo[m_Selected].SizeFilter);
		m_PropSizeWrapX->setCurrentIndex(m_BitmapInfo[m_Selected].SizeWrapX);
		m_PropSizeWrapY->setCurrentIndex(m_BitmapInfo[m_Selected].SizeWrapY);
		m_PropSizeWidth->setValue(m_BitmapInfo[m_Selected].SizeWidth);
		m_PropSizeHeight->setValue(m_BitmapInfo[m_Selected].SizeHeight);
	}
	else
	{
		m_PropLayout->setVisible(false);
		m_PropSize->setVisible(false);
	}

	if (!sourceContentSet)
	{
		props->setInfo(tr("<b>Warning: </b>Selected content no longer exists in project"));
	}
	else
	{
		props->setInfo("<b>BITMAP_HANDLE</b>(" + QString::number(m_Selected) + ")<br>"
			+ "<b>BITMAP_SOURCE</b>(" + (m_BitmapSource[m_Selected] == NULL ? "..." : QString::number(m_BitmapSource[m_Selected]->MemoryAddress)) + ")<br>"
			+ "<b>BITMAP_LAYOUT</b>(" + formatOption[m_BitmapInfo[m_Selected].LayoutFormat] + ", " + QString::number(m_BitmapInfo[m_Selected].LayoutStride) + ", " + QString::number(m_BitmapInfo[m_Selected].LayoutHeight) + ")<br>"
			+ "<b>BITMAP_SIZE</b>(" + filterOption[m_BitmapInfo[m_Selected].SizeFilter] + ", " + wrapOption[m_BitmapInfo[m_Selected].SizeWrapX] + ", " + wrapOption[m_BitmapInfo[m_Selected].SizeWrapY] + ", " + QString::number(m_BitmapInfo[m_Selected].SizeWidth) + ", " + QString::number(m_BitmapInfo[m_Selected].SizeHeight) + ")<br>"
			);
	}
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

	bool exists = m_MainWindow->contentManager()->isValidContent(info);

	for (int i = 0; i < 32; ++i)
	{
		// Update valid state
		m_BitmapSourceExists[i] = exists;

		// Refresh all of the thumbnails (even if not necessary... simplifies the code)
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
		m_NewValue(value),
		m_OldInfo(bitmapSetup->m_BitmapInfo[bitmapHandle]),
		m_NewInfo(bitmapSetup->m_BitmapInfo[bitmapHandle])
	{
		ContentManager *cm = bitmapSetup->m_MainWindow->contentManager();
		if (!value)
		{
			memset(&m_NewInfo, 0, sizeof(FT800EMU::BitmapInfo));
		}
		else if (cm->isValidContent(value) && value->Converter == ContentInfo::Image && cm->cacheImageInfo(value))
		{
			printf("Replace bitmap info with cached info from new content\n");
			m_NewInfo.LayoutFormat = value->ImageFormat; //
			m_NewInfo.LayoutStride = value->CachedImageStride; //
			m_NewInfo.LayoutHeight = value->CachedImageHeight; //
			m_NewInfo.SizeWidth = value->CachedImageWidth;
			m_NewInfo.SizeHeight = value->CachedImageHeight;
		}
		setText(tr("Bitmap Handle: Source content"));
	}

	virtual ~ChangeSourceContent()
	{

	}

	virtual void undo()
	{
		ContentManager *cm = m_BitmapSetup->m_MainWindow->contentManager();
		m_BitmapSetup->m_BitmapSourceExists[m_BitmapHandle] = false;
		m_BitmapSetup->m_BitmapSource[m_BitmapHandle] = m_OldValue;
		m_BitmapSetup->m_BitmapSourceExists[m_BitmapHandle] = cm->isValidContent(m_OldValue);
		m_BitmapSetup->m_BitmapInfo[m_BitmapHandle] = m_OldInfo;
		++m_BitmapSetup->m_ModificationNb;
		m_BitmapSetup->refreshViewInternal(m_BitmapHandle);
		m_BitmapSetup->refreshGUIInternal(m_BitmapHandle);
	}

	virtual void redo()
	{
		ContentManager *cm = m_BitmapSetup->m_MainWindow->contentManager();
		m_BitmapSetup->m_BitmapSourceExists[m_BitmapHandle] = false;
		m_BitmapSetup->m_BitmapSource[m_BitmapHandle] = m_NewValue;
		m_BitmapSetup->m_BitmapSourceExists[m_BitmapHandle] = cm->isValidContent(m_NewValue);
		m_BitmapSetup->m_BitmapInfo[m_BitmapHandle] = m_NewInfo;
		++m_BitmapSetup->m_ModificationNb;
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
		m_NewInfo = c->m_NewInfo;
		return true;
	}

private:
	BitmapSetup *m_BitmapSetup;
	int m_BitmapHandle;
	ContentInfo *m_OldValue;
	ContentInfo *m_NewValue;
	FT800EMU::BitmapInfo m_OldInfo;
	FT800EMU::BitmapInfo m_NewInfo;
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

class BitmapSetup::ChangeLayoutFormat : public QUndoCommand
{
public:
	ChangeLayoutFormat(BitmapSetup *bitmapSetup, int bitmapHandle, int value) :
		QUndoCommand(),
		m_BitmapSetup(bitmapSetup),
		m_BitmapHandle(bitmapHandle),
		m_OldValue(bitmapSetup->m_BitmapInfo[bitmapHandle].LayoutFormat),
		m_NewValue(value)
	{
		setText(tr("Bitmap Handle: Layout format"));
	}

	virtual ~ChangeLayoutFormat()
	{

	}

	virtual void undo()
	{
		m_BitmapSetup->m_BitmapInfo[m_BitmapHandle].LayoutFormat = m_OldValue;
		++m_BitmapSetup->m_ModificationNb;
		m_BitmapSetup->refreshGUIInternal(m_BitmapHandle);
	}

	virtual void redo()
	{
		m_BitmapSetup->m_BitmapInfo[m_BitmapHandle].LayoutFormat = m_NewValue;
		++m_BitmapSetup->m_ModificationNb;
		m_BitmapSetup->refreshGUIInternal(m_BitmapHandle);
	}

	virtual int id() const
	{
		return 9065501;
	}

	virtual bool mergeWith(const QUndoCommand *command)
	{
		const ChangeLayoutFormat *c = static_cast<const ChangeLayoutFormat *>(command);

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
	int m_OldValue;
	int m_NewValue;
};

void BitmapSetup::changeLayoutFormat(int bitmapHandle, int value)
{
	printf("BitmapSetup::changeLayoutFormat(bitmapHandle, value)\n");

	// Create undo/redo
	ChangeLayoutFormat *changeLayoutFormat = new ChangeLayoutFormat(this, bitmapHandle, value);
	m_MainWindow->undoStack()->push(changeLayoutFormat);
}

void BitmapSetup::propLayoutFormatChanged(int value)
{
	if (m_Selected >= 0 && m_BitmapInfo[m_Selected].LayoutFormat != value)
		changeLayoutFormat(m_Selected, value);
}

////////////////////////////////////////////////////////////////////////

class BitmapSetup::ChangeLayoutStride : public QUndoCommand
{
public:
	ChangeLayoutStride(BitmapSetup *bitmapSetup, int bitmapHandle, int value) :
		QUndoCommand(),
		m_BitmapSetup(bitmapSetup),
		m_BitmapHandle(bitmapHandle),
		m_OldValue(bitmapSetup->m_BitmapInfo[bitmapHandle].LayoutStride),
		m_NewValue(value)
	{
		setText(tr("Bitmap Handle: Layout stride"));
	}

	virtual ~ChangeLayoutStride()
	{

	}

	virtual void undo()
	{
		m_BitmapSetup->m_BitmapInfo[m_BitmapHandle].LayoutStride = m_OldValue;
		++m_BitmapSetup->m_ModificationNb;
		m_BitmapSetup->refreshGUIInternal(m_BitmapHandle);
	}

	virtual void redo()
	{
		m_BitmapSetup->m_BitmapInfo[m_BitmapHandle].LayoutStride = m_NewValue;
		++m_BitmapSetup->m_ModificationNb;
		m_BitmapSetup->refreshGUIInternal(m_BitmapHandle);
	}

	virtual int id() const
	{
		return 9065502;
	}

	virtual bool mergeWith(const QUndoCommand *command)
	{
		const ChangeLayoutStride *c = static_cast<const ChangeLayoutStride *>(command);

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
	int m_OldValue;
	int m_NewValue;
};

void BitmapSetup::changeLayoutStride(int bitmapHandle, int value)
{
	printf("BitmapSetup::changeLayoutStride(bitmapHandle, value)\n");

	// Create undo/redo
	ChangeLayoutStride *changeLayoutStride = new ChangeLayoutStride(this, bitmapHandle, value);
	m_MainWindow->undoStack()->push(changeLayoutStride);
}

void BitmapSetup::propLayoutStrideChanged(int value)
{
	if (m_Selected >= 0 && m_BitmapInfo[m_Selected].LayoutStride != value)
		changeLayoutStride(m_Selected, value);
}

////////////////////////////////////////////////////////////////////////

class BitmapSetup::ChangeLayoutHeight : public QUndoCommand
{
public:
	ChangeLayoutHeight(BitmapSetup *bitmapSetup, int bitmapHandle, int value) :
		QUndoCommand(),
		m_BitmapSetup(bitmapSetup),
		m_BitmapHandle(bitmapHandle),
		m_OldValue(bitmapSetup->m_BitmapInfo[bitmapHandle].LayoutHeight),
		m_NewValue(value)
	{
		setText(tr("Bitmap Handle: Layout height"));
	}

	virtual ~ChangeLayoutHeight()
	{

	}

	virtual void undo()
	{
		m_BitmapSetup->m_BitmapInfo[m_BitmapHandle].LayoutHeight = m_OldValue;
		++m_BitmapSetup->m_ModificationNb;
		m_BitmapSetup->refreshGUIInternal(m_BitmapHandle);
	}

	virtual void redo()
	{
		m_BitmapSetup->m_BitmapInfo[m_BitmapHandle].LayoutHeight = m_NewValue;
		++m_BitmapSetup->m_ModificationNb;
		m_BitmapSetup->refreshGUIInternal(m_BitmapHandle);
	}

	virtual int id() const
	{
		return 9065503;
	}

	virtual bool mergeWith(const QUndoCommand *command)
	{
		const ChangeLayoutHeight *c = static_cast<const ChangeLayoutHeight *>(command);

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
	int m_OldValue;
	int m_NewValue;
};

void BitmapSetup::changeLayoutHeight(int bitmapHandle, int value)
{
	printf("BitmapSetup::changeLayoutHeight(bitmapHandle, value)\n");

	// Create undo/redo
	ChangeLayoutHeight *changeLayoutHeight = new ChangeLayoutHeight(this, bitmapHandle, value);
	m_MainWindow->undoStack()->push(changeLayoutHeight);
}

void BitmapSetup::propLayoutHeightChanged(int value)
{
	if (m_Selected >= 0 && m_BitmapInfo[m_Selected].LayoutHeight != value)
		changeLayoutHeight(m_Selected, value);
}

////////////////////////////////////////////////////////////////////////

class BitmapSetup::ChangeSizeFilter : public QUndoCommand
{
public:
	ChangeSizeFilter(BitmapSetup *bitmapSetup, int bitmapHandle, int value) :
		QUndoCommand(),
		m_BitmapSetup(bitmapSetup),
		m_BitmapHandle(bitmapHandle),
		m_OldValue(bitmapSetup->m_BitmapInfo[bitmapHandle].SizeFilter),
		m_NewValue(value)
	{
		setText(tr("Bitmap Handle: Size filter"));
	}

	virtual ~ChangeSizeFilter()
	{

	}

	virtual void undo()
	{
		m_BitmapSetup->m_BitmapInfo[m_BitmapHandle].SizeFilter = m_OldValue;
		++m_BitmapSetup->m_ModificationNb;
		m_BitmapSetup->refreshGUIInternal(m_BitmapHandle);
	}

	virtual void redo()
	{
		m_BitmapSetup->m_BitmapInfo[m_BitmapHandle].SizeFilter = m_NewValue;
		++m_BitmapSetup->m_ModificationNb;
		m_BitmapSetup->refreshGUIInternal(m_BitmapHandle);
	}

	virtual int id() const
	{
		return 9065504;
	}

	virtual bool mergeWith(const QUndoCommand *command)
	{
		const ChangeSizeFilter *c = static_cast<const ChangeSizeFilter *>(command);

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
	int m_OldValue;
	int m_NewValue;
};

void BitmapSetup::changeSizeFilter(int bitmapHandle, int value)
{
	printf("BitmapSetup::changeSizeFilter(bitmapHandle, value)\n");

	// Create undo/redo
	ChangeSizeFilter *changeSizeFilter = new ChangeSizeFilter(this, bitmapHandle, value);
	m_MainWindow->undoStack()->push(changeSizeFilter);
}

void BitmapSetup::propSizeFilterChanged(int value)
{
	if (m_Selected >= 0 && m_BitmapInfo[m_Selected].SizeFilter != value)
		changeSizeFilter(m_Selected, value);
}

////////////////////////////////////////////////////////////////////////

class BitmapSetup::ChangeSizeWrapX : public QUndoCommand
{
public:
	ChangeSizeWrapX(BitmapSetup *bitmapSetup, int bitmapHandle, int value) :
		QUndoCommand(),
		m_BitmapSetup(bitmapSetup),
		m_BitmapHandle(bitmapHandle),
		m_OldValue(bitmapSetup->m_BitmapInfo[bitmapHandle].SizeWrapX),
		m_NewValue(value)
	{
		setText(tr("Bitmap Handle: Size wrap x"));
	}

	virtual ~ChangeSizeWrapX()
	{

	}

	virtual void undo()
	{
		m_BitmapSetup->m_BitmapInfo[m_BitmapHandle].SizeWrapX = m_OldValue;
		++m_BitmapSetup->m_ModificationNb;
		m_BitmapSetup->refreshGUIInternal(m_BitmapHandle);
	}

	virtual void redo()
	{
		m_BitmapSetup->m_BitmapInfo[m_BitmapHandle].SizeWrapX = m_NewValue;
		++m_BitmapSetup->m_ModificationNb;
		m_BitmapSetup->refreshGUIInternal(m_BitmapHandle);
	}

	virtual int id() const
	{
		return 9065505;
	}

	virtual bool mergeWith(const QUndoCommand *command)
	{
		const ChangeSizeWrapX *c = static_cast<const ChangeSizeWrapX *>(command);

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
	int m_OldValue;
	int m_NewValue;
};

void BitmapSetup::changeSizeWrapX(int bitmapHandle, int value)
{
	printf("BitmapSetup::changeSizeWrapX(bitmapHandle, value)\n");

	// Create undo/redo
	ChangeSizeWrapX *changeSizeWrapX = new ChangeSizeWrapX(this, bitmapHandle, value);
	m_MainWindow->undoStack()->push(changeSizeWrapX);
}

void BitmapSetup::propSizeWrapXChanged(int value)
{
	if (m_Selected >= 0 && m_BitmapInfo[m_Selected].SizeWrapX != value)
		changeSizeWrapX(m_Selected, value);
}

////////////////////////////////////////////////////////////////////////

class BitmapSetup::ChangeSizeWrapY : public QUndoCommand
{
public:
	ChangeSizeWrapY(BitmapSetup *bitmapSetup, int bitmapHandle, int value) :
		QUndoCommand(),
		m_BitmapSetup(bitmapSetup),
		m_BitmapHandle(bitmapHandle),
		m_OldValue(bitmapSetup->m_BitmapInfo[bitmapHandle].SizeWrapY),
		m_NewValue(value)
	{
		setText(tr("Bitmap Handle: Size wrap y"));
	}

	virtual ~ChangeSizeWrapY()
	{

	}

	virtual void undo()
	{
		m_BitmapSetup->m_BitmapInfo[m_BitmapHandle].SizeWrapY = m_OldValue;
		++m_BitmapSetup->m_ModificationNb;
		m_BitmapSetup->refreshGUIInternal(m_BitmapHandle);
	}

	virtual void redo()
	{
		m_BitmapSetup->m_BitmapInfo[m_BitmapHandle].SizeWrapY = m_NewValue;
		++m_BitmapSetup->m_ModificationNb;
		m_BitmapSetup->refreshGUIInternal(m_BitmapHandle);
	}

	virtual int id() const
	{
		return 9065506;
	}

	virtual bool mergeWith(const QUndoCommand *command)
	{
		const ChangeSizeWrapY *c = static_cast<const ChangeSizeWrapY *>(command);

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
	int m_OldValue;
	int m_NewValue;
};

void BitmapSetup::changeSizeWrapY(int bitmapHandle, int value)
{
	printf("BitmapSetup::changeSizeWrapY(bitmapHandle, value)\n");

	// Create undo/redo
	ChangeSizeWrapY *changeSizeWrapY = new ChangeSizeWrapY(this, bitmapHandle, value);
	m_MainWindow->undoStack()->push(changeSizeWrapY);
}

void BitmapSetup::propSizeWrapYChanged(int value)
{
	if (m_Selected >= 0 && m_BitmapInfo[m_Selected].SizeWrapY != value)
		changeSizeWrapY(m_Selected, value);
}

////////////////////////////////////////////////////////////////////////

class BitmapSetup::ChangeSizeWidth : public QUndoCommand
{
public:
	ChangeSizeWidth(BitmapSetup *bitmapSetup, int bitmapHandle, int value) :
		QUndoCommand(),
		m_BitmapSetup(bitmapSetup),
		m_BitmapHandle(bitmapHandle),
		m_OldValue(bitmapSetup->m_BitmapInfo[bitmapHandle].SizeWidth),
		m_NewValue(value)
	{
		setText(tr("Bitmap Handle: Size width"));
	}

	virtual ~ChangeSizeWidth()
	{

	}

	virtual void undo()
	{
		m_BitmapSetup->m_BitmapInfo[m_BitmapHandle].SizeWidth = m_OldValue;
		++m_BitmapSetup->m_ModificationNb;
		m_BitmapSetup->refreshGUIInternal(m_BitmapHandle);
	}

	virtual void redo()
	{
		m_BitmapSetup->m_BitmapInfo[m_BitmapHandle].SizeWidth = m_NewValue;
		++m_BitmapSetup->m_ModificationNb;
		m_BitmapSetup->refreshGUIInternal(m_BitmapHandle);
	}

	virtual int id() const
	{
		return 9065507;
	}

	virtual bool mergeWith(const QUndoCommand *command)
	{
		const ChangeSizeWidth *c = static_cast<const ChangeSizeWidth *>(command);

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
	int m_OldValue;
	int m_NewValue;
};

void BitmapSetup::changeSizeWidth(int bitmapHandle, int value)
{
	printf("BitmapSetup::changeSizeWidth(bitmapHandle, value)\n");

	// Create undo/redo
	ChangeSizeWidth *changeSizeWidth = new ChangeSizeWidth(this, bitmapHandle, value);
	m_MainWindow->undoStack()->push(changeSizeWidth);
}

void BitmapSetup::propSizeWidthChanged(int value)
{
	if (m_Selected >= 0 && m_BitmapInfo[m_Selected].SizeWidth != value)
		changeSizeWidth(m_Selected, value);
}

////////////////////////////////////////////////////////////////////////

class BitmapSetup::ChangeSizeHeight : public QUndoCommand
{
public:
	ChangeSizeHeight(BitmapSetup *bitmapSetup, int bitmapHandle, int value) :
		QUndoCommand(),
		m_BitmapSetup(bitmapSetup),
		m_BitmapHandle(bitmapHandle),
		m_OldValue(bitmapSetup->m_BitmapInfo[bitmapHandle].SizeHeight),
		m_NewValue(value)
	{
		setText(tr("Bitmap Handle: Size height"));
	}

	virtual ~ChangeSizeHeight()
	{

	}

	virtual void undo()
	{
		m_BitmapSetup->m_BitmapInfo[m_BitmapHandle].SizeHeight = m_OldValue;
		++m_BitmapSetup->m_ModificationNb;
		m_BitmapSetup->refreshGUIInternal(m_BitmapHandle);
	}

	virtual void redo()
	{
		m_BitmapSetup->m_BitmapInfo[m_BitmapHandle].SizeHeight = m_NewValue;
		++m_BitmapSetup->m_ModificationNb;
		m_BitmapSetup->refreshGUIInternal(m_BitmapHandle);
	}

	virtual int id() const
	{
		return 9065508;
	}

	virtual bool mergeWith(const QUndoCommand *command)
	{
		const ChangeSizeHeight *c = static_cast<const ChangeSizeHeight *>(command);

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
	int m_OldValue;
	int m_NewValue;
};

void BitmapSetup::changeSizeHeight(int bitmapHandle, int value)
{
	printf("BitmapSetup::changeSizeHeight(bitmapHandle, value)\n");

	// Create undo/redo
	ChangeSizeHeight *changeSizeHeight = new ChangeSizeHeight(this, bitmapHandle, value);
	m_MainWindow->undoStack()->push(changeSizeHeight);
}

void BitmapSetup::propSizeHeightChanged(int value)
{
	if (m_Selected >= 0 && m_BitmapInfo[m_Selected].SizeHeight != value)
		changeSizeHeight(m_Selected, value);
}

////////////////////////////////////////////////////////////////////////

} /* namespace FT800EMUQT */

/* end of file */
