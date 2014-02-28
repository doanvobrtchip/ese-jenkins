/**
 * content_manager.cpp
 * $Id$
 * \file content_manager.cpp
 * \brief content_manager.cpp
 * \date 2014-01-31 16:56GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#include "content_manager.h"

// STL includes
#include <stdio.h>

// Qt includes
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QPushButton>
#include <QHeaderView>
#include <QUndoCommand>
#include <QFileDialog>
#include <QFileInfo>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QFile>
#include <QJsonDocument>
#include <QCheckBox>
#include <QSpinBox>
#include <QDateTime>

// Emulator includes
#include <ft800emu_graphics_processor.h>
#include <vc.h>

// Project includes
#include "main_window.h"
#include "properties_editor.h"
#include "undo_stack_disabler.h"
#include "asset_converter.h"
#include "bitmap_setup.h"

using namespace std;

namespace FT800EMUQT {

std::vector<QString> ContentManager::s_FileExtensions;

ContentInfo::ContentInfo(const QString &filePath)
{
	SourcePath = filePath;
	DestName = QFileInfo(filePath).baseName();
	View = NULL;
	Converter = ContentInfo::Invalid;
	MemoryLoaded = false;
	MemoryAddress = 0;
	DataCompressed = true;
	DataEmbedded = true;
	RawStart = 0;
	RawLength = 0;
	ImageFormat = 0;
	UploadDirty = true;
	ExternalDirty = false;
	CachedImage = false;
}

QJsonObject ContentInfo::toJson(bool meta) const
{
	QJsonObject j;
	j["sourcePath"] = SourcePath;
	if (!meta)
	{
		j["destName"] = DestName;
		j["memoryLoaded"] = MemoryLoaded;
		j["memoryAddress"] = MemoryAddress;
		j["dataCompressed"] = DataCompressed;
		j["dataEmbedded"] = DataEmbedded;
	}
	switch (Converter)
	{
	case Raw:
		j["converter"] = QString("Raw");
		j["rawStart"] = RawStart;
		j["rawLength"] = RawLength;
		break;
	case Image:
		j["converter"] = QString("Image");
		j["imageFormat"] = ImageFormat;
		break;
	case RawJpeg:
		j["converter"] = QString("RawJpeg");
		break;
	}
	return j;
}

void ContentInfo::fromJson(QJsonObject &j, bool meta)
{
	SourcePath = j["sourcePath"].toString();
	if (!meta)
	{
		DestName = j["destName"].toString();
		MemoryLoaded = ((QJsonValue)j["memoryLoaded"]).toVariant().toBool();
		MemoryAddress = ((QJsonValue)j["memoryAddress"]).toVariant().toInt();
		DataCompressed = ((QJsonValue)j["dataCompressed"]).toVariant().toBool();
		DataEmbedded = ((QJsonValue)j["dataEmbedded"]).toVariant().toBool();
	}
	QString converter = j["converter"].toString();
	if (converter == "Raw")
	{
		Converter = Raw;
		RawStart = ((QJsonValue)j["rawStart"]).toVariant().toInt();
		RawLength = ((QJsonValue)j["rawLength"]).toVariant().toInt();
	}
	else if (converter == "Image")
	{
		Converter = Image;
		ImageFormat = ((QJsonValue)j["imageFormat"]).toVariant().toInt();
	}
	else if (converter == "RawJpeg")
	{
		Converter = RawJpeg;
	}
	else
	{
		Converter = Invalid;
	}
}

bool ContentInfo::equalsMeta(const ContentInfo *other) const
{
	if (SourcePath != other->SourcePath)
	{
		return false;
	}
	if (Converter != other->Converter)
	{
		return false;
	}
	switch (Converter)
	{
	case Raw:
		if (RawStart != other->RawStart)
		{
			return false;
		}
		if (RawLength != other->RawLength)
		{
			return false;
		}
		break;
	case Image:
		if (ImageFormat != other->ImageFormat)
		{
			return false;
		}
		break;
	}
	return true;
}

void addLabeledWidget(QWidget *parent, QVBoxLayout *layout, const QString &label, QWidget *widget)
{
	QHBoxLayout *hbox = new QHBoxLayout();
	QLabel *l = new QLabel(parent);
	l->setText(label);
	hbox->addWidget(l);
	hbox->addWidget(widget);
	layout->addLayout(hbox);
}

void addLabeledWidget(QWidget *parent, QVBoxLayout *layout, const QString &label, QWidget *widget0, QWidget *widget1)
{
	QHBoxLayout *hbox = new QHBoxLayout();
	QLabel *l = new QLabel(parent);
	l->setText(label);
	hbox->addWidget(l);
	hbox->addWidget(widget0);
	hbox->addWidget(widget1);
	layout->addLayout(hbox);
}

ContentManager::ContentManager(MainWindow *parent) : QWidget(parent), m_MainWindow(parent), m_CurrentPropertiesContent(NULL)
{
	if (s_FileExtensions.empty())
	{
		s_FileExtensions.push_back(".bin");
		s_FileExtensions.push_back(".binh");
		s_FileExtensions.push_back(".raw");
		s_FileExtensions.push_back(".rawh");
		s_FileExtensions.push_back(".lut.bin");
		s_FileExtensions.push_back(".lut.binh");
		s_FileExtensions.push_back(".lut.raw");
		s_FileExtensions.push_back(".lut.rawh");
		s_FileExtensions.push_back("_converted.png");
		s_FileExtensions.push_back(".meta");
	}

	QVBoxLayout *layout = new QVBoxLayout();

	m_ContentList = new QTreeWidget(this);
	m_ContentList->setDragEnabled(true);
	m_ContentList->header()->close();
	layout->addWidget(m_ContentList);
	connect(m_ContentList, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(selectionChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

	QHBoxLayout *buttonsLayout = new QHBoxLayout();

	uint plusSign[2] = { 0x2795, 0 };
	QPushButton *addButton = new QPushButton();
	addButton->setText(QString::fromUcs4(plusSign) + " " + tr("Add"));
	connect(addButton, SIGNAL(clicked()), this, SLOT(add()));
	buttonsLayout->addWidget(addButton);

	uint minusSign[2] = { 0x2796, 0 };
	QPushButton *removeButton = new QPushButton();
	m_RemoveButton = removeButton;
	removeButton->setText(QString::fromUcs4(minusSign) + " " + tr("Remove"));
	connect(removeButton, SIGNAL(clicked()), this, SLOT(remove()));
	removeButton->setEnabled(false);
	buttonsLayout->addWidget(removeButton);

	layout->addLayout(buttonsLayout);

	QHBoxLayout *secondLayout = new QHBoxLayout();

	uint refreshIcon[2] = { 0x27F2, 0 };
	QPushButton *rebuildButton = new QPushButton();
	rebuildButton->setText(QString::fromUcs4(refreshIcon) + " " + tr("Rebuild All"));
	rebuildButton->setStatusTip(tr("Rebuilds all content that is out of date"));
	connect(rebuildButton, SIGNAL(clicked()), this, SLOT(rebuildAll()));
	secondLayout->addWidget(rebuildButton);

	layout->addLayout(secondLayout);

	setLayout(layout);

	// Attach selection to properties
	connect(m_MainWindow->propertiesEditor(), SIGNAL(setterChanged(QWidget *)), this, SLOT(propertiesSetterChanged(QWidget *)));

	// Build properties widgets
	QGroupBox *propCommon = new QGroupBox(this);
	propCommon->setHidden(true);
	m_PropertiesCommon = propCommon;
	propCommon->setTitle(tr("Content"));
	QVBoxLayout *propCommonLayout = new QVBoxLayout();
	m_PropertiesCommonSourceFile = new UndoStackDisabler<QLineEdit>(this);
	m_PropertiesCommonSourceFile->setUndoStack(m_MainWindow->undoStack());
	QPushButton *browseSourceFile = new QPushButton(this);
	browseSourceFile->setMaximumWidth(browseSourceFile->height());
	addLabeledWidget(this, propCommonLayout, tr("Source file: "), m_PropertiesCommonSourceFile, browseSourceFile);
	connect(m_PropertiesCommonSourceFile, SIGNAL(editingFinished()), this, SLOT(propertiesCommonSourcePathChanged()));
	connect(browseSourceFile, SIGNAL(clicked()), this, SLOT(propertiesCommonSourcePathBrowse()));
	m_PropertiesCommonName = new UndoStackDisabler<QLineEdit>(this);
	m_PropertiesCommonName->setUndoStack(m_MainWindow->undoStack());
	addLabeledWidget(this, propCommonLayout, tr("Name: "), m_PropertiesCommonName);
	connect(m_PropertiesCommonName, SIGNAL(editingFinished()), this, SLOT(propertiesCommonDestNameChanged()));
	QComboBox *propCommonConverter = new QComboBox(this);
	m_PropertiesCommonConverter = propCommonConverter;
	propCommonConverter->addItem("");
	propCommonConverter->addItem(tr("Image"));
	propCommonConverter->addItem(tr("Raw"));
	// propCommonConverter->addItem(tr("Raw JPEG")); // TODO
	addLabeledWidget(this, propCommonLayout, tr("Converter: "), propCommonConverter);
	connect(m_PropertiesCommonConverter, SIGNAL(currentIndexChanged(int)), this, SLOT(propertiesCommonConverterChanged(int)));
	propCommon->setLayout(propCommonLayout);

	m_PropertiesImage = new QGroupBox(this);
	m_PropertiesImage->setHidden(true);
	m_PropertiesImage->setTitle(tr("Image Settings"));
	QVBoxLayout *imagePropsLayout = new QVBoxLayout();
	m_PropertiesImageFormat = new QComboBox(this);
	m_PropertiesImageFormat->addItem("ARGB1555");
	m_PropertiesImageFormat->addItem("L1");
	m_PropertiesImageFormat->addItem("L4");
	m_PropertiesImageFormat->addItem("L8");
	m_PropertiesImageFormat->addItem("RGB332");
	m_PropertiesImageFormat->addItem("ARGB2");
	m_PropertiesImageFormat->addItem("ARGB4");
	m_PropertiesImageFormat->addItem("RGB565");
	m_PropertiesImageFormat->addItem("PALETTED");
	addLabeledWidget(this, imagePropsLayout, tr("Format: "), m_PropertiesImageFormat);
	connect(m_PropertiesImageFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(propertiesCommonImageFormatChanged(int)));
	m_PropertiesImage->setLayout(imagePropsLayout);

	m_PropertiesImagePreview = new QGroupBox(this);
	QVBoxLayout *imagePreviewLayout = new QVBoxLayout();
	m_PropertiesImagePreview->setHidden(true);
	m_PropertiesImagePreview->setTitle(tr("Image Preview"));
	m_PropertiesImageLabel = new QLabel(this);
	imagePreviewLayout->addWidget(m_PropertiesImageLabel);
	m_PropertiesImagePreview->setLayout(imagePreviewLayout);

	m_PropertiesMemory = new QGroupBox(this);
	m_PropertiesMemory->setHidden(true);
	m_PropertiesMemory->setTitle(tr("Memory Options"));
	QVBoxLayout *propMemLayout = new QVBoxLayout();
	m_PropertiesMemoryAddress = new QSpinBox(this);
	m_PropertiesMemoryAddress->setMinimum(0);
	m_PropertiesMemoryAddress->setMaximum(RAM_DL - 4);
	m_PropertiesMemoryAddress->setSingleStep(4);
	m_PropertiesMemoryAddress->setKeyboardTracking(false);
	addLabeledWidget(this, propMemLayout, tr("Address: "), m_PropertiesMemoryAddress);
	connect(m_PropertiesMemoryAddress, SIGNAL(valueChanged(int)), this, SLOT(propertiesCommonMemoryAddressChanged(int)));
	m_PropertiesMemoryLoaded = new QCheckBox(this);
	addLabeledWidget(this, propMemLayout, tr("Loaded: "), m_PropertiesMemoryLoaded);
	connect(m_PropertiesMemoryLoaded, SIGNAL(stateChanged(int)), this, SLOT(propertiesCommonMemoryLoadedChanged(int)));
	QFrame* line = new QFrame();
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);
	propMemLayout->addWidget(line);
	m_PropertiesDataCompressed = new QCheckBox(this);
	addLabeledWidget(this, propMemLayout, tr("Compressed: "), m_PropertiesDataCompressed);
	connect(m_PropertiesDataCompressed, SIGNAL(stateChanged(int)), this, SLOT(propertiesCommonDataCompressedChanged(int)));
	m_PropertiesDataEmbedded = new QCheckBox(this);
	addLabeledWidget(this, propMemLayout, tr("Embedded: "), m_PropertiesDataEmbedded);
	connect(m_PropertiesDataEmbedded, SIGNAL(stateChanged(int)), this, SLOT(propertiesCommonDataEmbeddedChanged(int)));
	m_PropertiesMemory->setLayout(propMemLayout);

	QVBoxLayout *helpLayout = new QVBoxLayout();
	m_HelpfulLabel = new QLabel(m_ContentList);
	m_HelpfulLabel->setWordWrap(true);
	m_HelpfulLabel->setText(tr("<i>No content has been added to the project yet.<br><br>Add new content to this project to automatically convert it to a hardware compatible format.</i>"));
	helpLayout->addWidget(m_HelpfulLabel);
	m_ContentList->setLayout(helpLayout);
}

ContentManager::~ContentManager()
{
	// Cleanup
	for (QTreeWidgetItemIterator it(m_ContentList); *it; ++it)
	{
		ContentInfo *info = (ContentInfo *)(*it)->data(0, Qt::UserRole).value<void *>();
		delete info;
	}
}

class ContentManager::Add : public QUndoCommand
{
public:
	Add(ContentManager *contentManager, ContentInfo *contentInfo) :
		QUndoCommand(),
		m_ContentManager(contentManager),
		m_ContentInfo(contentInfo),
		m_Owner(true)
	{
		setText(tr("Add content"));
	}

	virtual ~Add()
	{
		if (m_Owner)
		{
			delete m_ContentInfo;
		}
	}

	virtual void undo()
	{
		printf("Add::undo()\n");

		// Remove from the content manager
		m_ContentManager->removeInternal(m_ContentInfo);

		m_Owner = true;
	}

	virtual void redo()
	{
		printf("Add::redo()\n");

		m_Owner = false;

		// Add to the content manager
		m_ContentManager->addInternal(m_ContentInfo);
	}

private:
	ContentManager *m_ContentManager;
	ContentInfo *m_ContentInfo;
	bool m_Owner;
};

ContentInfo *ContentManager::add(const QString &filePath)
{
	printf("ContentManager::add(filePath)\n");

	ContentInfo *contentInfo = new ContentInfo(filePath);

	add(contentInfo);

	return contentInfo;
}

void ContentManager::add(ContentInfo *contentInfo)
{
	printf("ContentManager::add(contentInfo)\n");

	Add *add = new Add(this, contentInfo);
	m_MainWindow->undoStack()->push(add);
}

class ContentManager::Remove : public QUndoCommand
{
public:
	Remove(ContentManager *contentManager, ContentInfo *contentInfo) :
		QUndoCommand(),
		m_ContentManager(contentManager),
		m_ContentInfo(contentInfo),
		m_Owner(false)
	{
		setText(tr("Remove content"));
	}

	virtual ~Remove()
	{
		if (m_Owner)
		{
			delete m_ContentInfo;
		}
	}

	virtual void undo()
	{
		printf("Remove::undo()\n");

		m_Owner = false;

		// Add to the content manager
		m_ContentManager->addInternal(m_ContentInfo);
	}

	virtual void redo()
	{
		printf("Remove::redo()\n");

		// Remove from the content manager
		m_ContentManager->removeInternal(m_ContentInfo);

		m_Owner = true;
	}

private:
	ContentManager *m_ContentManager;
	// int m_Index; // FIXME: When undo-ing, the item will appear at the end rather than it's original index.
	ContentInfo *m_ContentInfo;
	bool m_Owner;
};

void ContentManager::remove(ContentInfo *contentInfo)
{
	printf("ContentManager::remove(contentInfo)\n");

	Remove *remove = new Remove(this, contentInfo);
	m_MainWindow->undoStack()->push(remove);
}

void ContentManager::addInternal(ContentInfo *contentInfo)
{
	printf("ContentManager::addInternal(contentInfo)\n");

	// Ensure no duplicate names are used
	contentInfo->DestName = createName(contentInfo->DestName);

	// Add to the content list
	QTreeWidgetItem *view = new QTreeWidgetItem(m_ContentList);
	contentInfo->View = view;
	view->setData(0, Qt::UserRole, qVariantFromValue((void *)contentInfo));
	rebuildViewInternal(contentInfo);

	// Reprocess to RAM
	contentInfo->ExternalDirty = true;
	reprocessInternal(contentInfo);

	// Be helpful
	m_ContentList->setCurrentItem(view);
	m_HelpfulLabel->setVisible(false);
}

void ContentManager::removeInternal(ContentInfo *contentInfo)
{
	printf("ContentManager::removeInternal(contentInfo)\n");

	// Remove from the content list
	delete contentInfo->View;
	contentInfo->View = NULL;

	// Mark upload as dirty
	contentInfo->UploadDirty = true;

	// Reload external dependencies
	reloadExternal(contentInfo);

	// Be helpful
	m_HelpfulLabel->setVisible(getContentCount() == 0);
}

bool ContentManager::nameExists(const QString &name)
{
	for (QTreeWidgetItemIterator it(m_ContentList); *it; ++it)
	{
		ContentInfo *info = (ContentInfo *)(*it)->data(0, Qt::UserRole).value<void *>();
		if (info->DestName == name)
		{
			return true;
		}
	}
	return false;
}

QString ContentManager::createName(const QString &name)
{
	// Strip invalid characters
	QString destName;
	bool lastIsSlash = true;
	for (QString::const_iterator it(name.begin()), end (name.end()); it != end; ++it)
	{
		QChar c = *it;
		if (c == '.'
			|| c == ' ')
		{
			if (!lastIsSlash)
			{
				destName += c;
			}
		}
		else if (c.isLetterOrNumber()
			|| c == '_'
			|| c == '-'
			|| c == '('
			|| c == ')'
			|| c == '['
			|| c == ']'
			|| c == '+')
		{
			destName += c;
			lastIsSlash = false;
		}
		else if (c == '\\' || c == '/')
		{
			if (!lastIsSlash)
			{
				destName += '/';
				lastIsSlash = true;
			}
		}
	}
	destName = destName.simplified();
	// Cannot have empty name
	if (destName.isEmpty())
		destName = "untitled";
	// Renumber in case of duplicate
	QString resultDestName = destName;
	int renumber = 2;
	while (nameExists(resultDestName))
	{
		resultDestName = destName + "_" + QString::number(renumber);
		++renumber;
	}
	// printf("%s\n", resultDestName.toLocal8Bit().data());
	return resultDestName;
}

void ContentManager::add()
{
	printf("ContentManager::add()\n");

	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Load Content"),
		m_MainWindow->getFileDialogPath(),
		tr("All files (*.*)"));

	if (fileName.isNull())
		return;

	ContentInfo *info = add(fileName);
}

void ContentManager::remove()
{
	printf("ContentManager::remove()\n");

	if (!m_ContentList->currentItem())
		return;

	ContentInfo *info = (ContentInfo *)m_ContentList->currentItem()->data(0, Qt::UserRole).value<void *>();
	remove(info);
}

void ContentManager::rebuildAll()
{
	// Reprocess all content if necessary
	for (QTreeWidgetItemIterator it(m_ContentList); *it; ++it)
	{
		ContentInfo *info = (ContentInfo *)(*it)->data(0, Qt::UserRole).value<void *>();
		reprocessInternal(info);
	}
}

void ContentManager::getContentInfos(std::vector<ContentInfo *> &contentInfos)
{
	// Iterate through the list and copy the pointer to the data
	for (QTreeWidgetItemIterator it(m_ContentList); *it; ++it)
	{
		ContentInfo *info = (ContentInfo *)(*it)->data(0, Qt::UserRole).value<void *>();
		contentInfos.push_back(info);
	}
}

int ContentManager::getContentCount() const
{
	return m_ContentList->topLevelItemCount();
}

bool ContentManager::isValidContent(ContentInfo *info)
{
	// Null is always invalid
	if (info == NULL) return false;

	// Iterate through the list and copy the pointer to the data
	for (QTreeWidgetItemIterator it(m_ContentList); *it; ++it)
	{
		if (info == (ContentInfo *)(*it)->data(0, Qt::UserRole).value<void *>())
		{
			// Found the content
			return true;
		}
	}

	// Does not exist here (may still exist in the undo stack)
	return false;
}

ContentInfo *ContentManager::find(const QString &destName)
{
	if (destName.isEmpty())
		return NULL;

	for (QTreeWidgetItemIterator it(m_ContentList); *it; ++it)
	{
		ContentInfo *info = (ContentInfo *)(*it)->data(0, Qt::UserRole).value<void *>();
		if (info->DestName == destName)
			return info;
	}

	printf("Content '%s' not found\n", destName.toLocal8Bit().data());
	return NULL;
}

bool ContentManager::cacheImageInfo(ContentInfo *info)
{
	if (info->CachedImage)
		return true;
	FT800EMU::BitmapInfo bitmapInfo;
	if (!AssetConverter::getImageInfo(bitmapInfo, info->DestName))
		return false;
	info->CachedImageWidth = bitmapInfo.LayoutWidth;
	info->CachedImageHeight = bitmapInfo.LayoutHeight;
	info->CachedImageStride = bitmapInfo.LayoutStride;
	info->CachedImage = true;
	return true;
}

void ContentManager::clear()
{
	printf("ContentManager::clear()\n");

	std::vector<ContentInfo *> contentInfos;
	// Iterate through the list and copy, because we shouldn't delete while iterating
	getContentInfos(contentInfos);
	// Delete all
	m_MainWindow->undoStack()->beginMacro(tr("Clear content"));
	for (std::vector<ContentInfo *>::iterator it = contentInfos.begin(), end = contentInfos.end(); it != end; ++it)
	{
		remove(*it);
	}
	m_MainWindow->undoStack()->endMacro();
}

void ContentManager::selectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous)
{
	printf("ContentManager::selectionChanged\n");

	if (current)
	{
		m_RemoveButton->setEnabled(true);

		// Be helpful
		m_MainWindow->focusProperties();

		// Rebuild GUI
		rebuildGUIInternal((ContentInfo *)current->data(0, Qt::UserRole).value<void *>());
	}
	else
	{
		m_RemoveButton->setEnabled(false);
		PropertiesEditor *props = m_MainWindow->propertiesEditor();
		if (props->getEditWidgetSetter() == this)
		{
			props->setInfo("");
			props->setEditWidget(NULL, false, NULL);
		}
		m_CurrentPropertiesContent = NULL;
	}
}

void ContentManager::rebuildViewInternal(ContentInfo *contentInfo)
{
	contentInfo->View->setText(0, contentInfo->SourcePath);
}

void ContentManager::rebuildGUIInternal(ContentInfo *contentInfo)
{
	printf("ContentManager::rebuildGUIInternal()\n");

	// Build GUI for this content
	m_CurrentPropertiesContent = contentInfo;
	rebuildViewInternal(contentInfo);

	// Display widgets in properties tab
	PropertiesEditor *props = m_MainWindow->propertiesEditor();
	if (m_CurrentPropertiesContent != contentInfo
		|| props->getEditWidgetSetter() != this)
	{
		std::vector<QWidget *> widgets;

		widgets.push_back(m_PropertiesCommon);
		widgets.push_back(m_PropertiesImage);
		widgets.push_back(m_PropertiesImagePreview);
		widgets.push_back(m_PropertiesMemory);

		props->setEditWidgets(widgets, false, this);
	}

	// Set common widget values
	m_PropertiesCommonSourceFile->setText(contentInfo->SourcePath);
	m_PropertiesCommonName->setText(contentInfo->DestName);
	m_PropertiesCommonConverter->setCurrentIndex((int)contentInfo->Converter);
	switch (contentInfo->Converter)
	{
	case ContentInfo::Image:
		m_PropertiesImageFormat->setCurrentIndex(contentInfo->ImageFormat);
		break;
	}
	if (contentInfo->Converter != ContentInfo::Invalid)
	{
		m_PropertiesMemoryLoaded->setCheckState(contentInfo->MemoryLoaded ? Qt::Checked : Qt::Unchecked);
		m_PropertiesMemoryAddress->setValue(contentInfo->MemoryAddress);
		m_PropertiesDataCompressed->setCheckState(contentInfo->DataCompressed ? Qt::Checked : Qt::Unchecked);
		m_PropertiesDataEmbedded->setCheckState(contentInfo->DataEmbedded ? Qt::Checked : Qt::Unchecked);
	}

	// Set user help, wizard format
	if (contentInfo->Converter == ContentInfo::Invalid)
	{
		m_PropertiesImage->setHidden(true);
		m_PropertiesImagePreview->setHidden(true);
		m_PropertiesMemory->setHidden(true);
		props->setInfo(tr("Select a <b>Converter</b> to be used for this file. Converted files will be stored in the folder where the project is saved.<br><br><b>Image</b>: Converts an image to one of the supported formats.<br><b>Raw</b>: Does a direct binary copy.<br><b>Raw JPEG</b>: Does a raw binary copy and decodes the JPEG on the coprocessor."));
	}
	else
	{
		QString propInfo;
		if (!contentInfo->BuildError.isEmpty())
		{
			propInfo = tr("<b>Error</b>: ") + contentInfo->BuildError;
		}
		switch (contentInfo->Converter)
		{
			case ContentInfo::Image:
			{
				m_PropertiesImage->setHidden(false);
				QPixmap pixmap;
				bool loadSuccess = pixmap.load(contentInfo->DestName + "_converted.png");
				m_PropertiesImagePreview->setHidden(!loadSuccess);
				m_PropertiesImageLabel->setPixmap(pixmap.scaled(m_PropertiesImageLabel->width() - 32, m_PropertiesImageLabel->width() - 32, Qt::KeepAspectRatio));
				if (loadSuccess) m_PropertiesImageLabel->repaint();
				else { if (!propInfo.isEmpty()) propInfo += "<br>"; propInfo += tr("<b>Error</b>: Failed to load image preview."); }
				m_PropertiesMemory->setHidden(false);
				if (contentInfo->BuildError.isEmpty())
				{
					if (!propInfo.isEmpty()) propInfo += "<br>";
					QFileInfo rawInfo(contentInfo->DestName + ".raw");
					QFileInfo binInfo(contentInfo->DestName + ".bin");
					propInfo += tr("<b>Size: </b> ") + QString::number(rawInfo.size()) + " bytes";
					propInfo += tr("<br><b>Compressed: </b> ") + QString::number(binInfo.size()) + " bytes";
					/*if (loadSuccess)
					{
						// stride etc
						FT800EMU::BitmapInfo bitmapInfo;
						if (AssetConverter::getImageInfo(bitmapInfo, contentInfo->DestName))
						{
							propInfo += tr("<br><b>Width: </b> ") + QString::number(bitmapInfo.LayoutWidth);
							propInfo += tr("<br><b>Height: </b> ") + QString::number(bitmapInfo.LayoutHeight);
							propInfo += tr("<br><b>Format: </b> ");
							switch (bitmapInfo.LayoutFormat)
							{
								case ARGB1555: propInfo += "ARGB1555"; break;
								case L1: propInfo += "L1"; break;
								case L4: propInfo += "L4"; break;
								case L8: propInfo += "L8"; break;
								case RGB332: propInfo += "RGB332"; break;
								case ARGB2: propInfo += "ARGB2"; break;
								case ARGB4: propInfo += "ARGB4"; break;
								case RGB565: propInfo += "RGB565"; break;
								case PALETTED: propInfo += "PALETTED"; break;
								case TEXT8X8: propInfo += "TEXT8X8"; break;
								case TEXTVGA: propInfo += "TEXTVGA"; break;
								case BARGRAPH: propInfo += "BARGRAPH"; break;
							}
							propInfo += tr("<br><b>Stride: </b> ") + QString::number(bitmapInfo.LayoutStride);
						}
						else
						{
							propInfo += tr("<br><b>Error: </b>Unable to load raw header.");
						}
					}*/
					if (cacheImageInfo(contentInfo))
					{
						propInfo += tr("<br><b>Width: </b> ") + QString::number(contentInfo->CachedImageWidth);
						propInfo += tr("<br><b>Height: </b> ") + QString::number(contentInfo->CachedImageHeight);
						propInfo += tr("<br><b>Stride: </b> ") + QString::number(contentInfo->CachedImageStride);
					}
				}
				break;
			}
			case ContentInfo::Raw:
			{
				m_PropertiesImage->setHidden(true);
				m_PropertiesImagePreview->setHidden(true);
				m_PropertiesMemory->setHidden(false);
				if (contentInfo->BuildError.isEmpty())
				{
					if (!propInfo.isEmpty()) propInfo += "<br>";
					QFileInfo rawInfo(contentInfo->DestName + ".raw");
					QFileInfo binInfo(contentInfo->DestName + ".bin");
					propInfo += tr("<b>Size: </b> ") + QString::number(rawInfo.size()) + " bytes";
					propInfo += tr("<br><b>Compressed: </b> ") + QString::number(binInfo.size()) + " bytes";
				}
				break;
			}
			case ContentInfo::RawJpeg:
			{
				m_PropertiesImage->setHidden(true);
				m_PropertiesImagePreview->setHidden(true);
				m_PropertiesMemory->setHidden(false);
				if (!propInfo.isEmpty()) propInfo += "<br>";
				propInfo += tr("<b>Not yet implemented</b>");
				// This will show JPG size, uncompressed size, and necessary info to load the image into the handles
				break;
			}
		}
		props->setInfo(propInfo);
	}
}

void ContentManager::reprocessInternal(ContentInfo *contentInfo)
{
	printf("ContentManager::reprocessInternal()\n");

	// Reprocess this content
	if (contentInfo->Converter != ContentInfo::Invalid)
	{
		lockContent();
		// TODO: Compare datestamp of source file with meta file
		QString srcFile = contentInfo->SourcePath;
		bool srcExists = QFile::exists(srcFile);
		if (!srcExists)
		{
			contentInfo->BuildError = "<i>(Filesystem)</i><br>Source file does not exist.";
			contentInfo->ExternalDirty = true;
		}
		else
		{
			// Only able to skip rebuild if no build error occured last time
			bool equalMeta = false;
			QString metaFile = contentInfo->DestName + ".meta";
			bool metaExists = QFile::exists(metaFile);
			if (contentInfo->BuildError.isEmpty())
			{
				if (metaExists)
				{
					QFileInfo srcInfo(srcFile);
					QFileInfo metaInfo(metaFile);
					if (srcInfo.lastModified() > metaInfo.lastModified())
					{
						// FIXME: Perhaps better to store the srcInfo.lastModified() in the meta and compare if equal in case of swapping with older files
						printf("Source file has been modified, ignore meta, rebuild\n");
					}
					else
					{
						QFile file(metaFile);
						file.open(QIODevice::ReadOnly);
						QByteArray data = file.readAll();
						file.close();
						QJsonParseError parseError;
						QJsonDocument doc = QJsonDocument::fromJson(data, &parseError);
						if (parseError.error == QJsonParseError::NoError)
						{
							QJsonObject root = doc.object();
							ContentInfo ci("");
							ci.fromJson(root, true);
							equalMeta = contentInfo->equalsMeta(&ci);
						}
					}
				}
			}
			if (equalMeta)
			{
				printf("Equal meta, skip convert\n");
			}
			else
			{
				if (metaExists)
				{
					QFile::remove(metaFile); // *** FILE REMOVE ***
				}
				contentInfo->CachedImage = false;
				contentInfo->ExternalDirty = true;
				contentInfo->BuildError = "";
				// Create directory if necessary
				if (contentInfo->DestName.contains('/'))
				{
					QString destDir = contentInfo->DestName.left(contentInfo->DestName.lastIndexOf('/'));
					if (!QDir(QDir::currentPath()).mkpath(destDir))
					{
						contentInfo->BuildError = "<i>(Filesystem)</i><br>Unable to create destination path.";
					}
				}
				if (contentInfo->BuildError.isEmpty())
				{
					// Run convertor
					switch (contentInfo->Converter)
					{
					case ContentInfo::Image:
						AssetConverter::convertImage(contentInfo->BuildError, contentInfo->SourcePath, contentInfo->DestName, contentInfo->ImageFormat);
						break;
					case ContentInfo::Raw:
						AssetConverter::convertRaw(contentInfo->BuildError, contentInfo->SourcePath, contentInfo->DestName, contentInfo->RawStart, contentInfo->RawLength);
						break;
					case ContentInfo::RawJpeg:
						AssetConverter::convertRaw(contentInfo->BuildError, contentInfo->SourcePath, contentInfo->DestName, 0, 0);
						break;
					default:
						contentInfo->BuildError = "<i>(Critical Error)</i><br>Unknown converter selected.";
						break;
					}
				}
				if (contentInfo->BuildError.isEmpty())
				{
					// Write meta file
					QFile file(metaFile);
					file.open(QIODevice::WriteOnly);
					QDataStream out(&file);
					QJsonDocument doc(contentInfo->toJson(true));
					QByteArray data = doc.toJson();
					out.writeRawData(data, data.size());
					contentInfo->UploadDirty = true;
				}
			}
		}
		unlockContent();
	}
	else
	{
		// Always refresh external if converter deselected...
		contentInfo->ExternalDirty = true;
	}

	// Reupload the content to emulator RAM if dirty
	if (contentInfo->UploadDirty)
	{
		contentInfo->UploadDirty = false;
		reuploadInternal(contentInfo);
	}

	// Reload external if dirty
	if (contentInfo->ExternalDirty)
	{
		if (contentInfo->Converter != ContentInfo::Invalid)
			contentInfo->ExternalDirty = false;

		reloadExternal(contentInfo);
	}

	// Update GUI if it exists
	if (m_CurrentPropertiesContent == contentInfo)
		rebuildGUIInternal(contentInfo);
}

void ContentManager::reuploadInternal(ContentInfo *contentInfo)
{
	printf("ContentManager::reuploadInternal(contentInfo)\n");

	// Reupload the content to emulator RAM
	// This happens in the emulator main loop
	// Emulator main loop will lock the content mutex
	if (contentInfo->Converter != ContentInfo::Invalid && contentInfo->MemoryLoaded)
	{
		if (contentInfo->Converter == ContentInfo::Image)
		{
			// Bitmap setup is always updated after upload and requires cached image info
			cacheImageInfo(contentInfo);
		}

		lockContent();
		if (m_ContentUploadDirty.find(contentInfo) == m_ContentUploadDirty.end())
			m_ContentUploadDirty.insert(contentInfo);
		unlockContent();
	}
}

void ContentManager::reloadExternal(ContentInfo *contentInfo)
{
	printf("ContentManager::reloadExternal(contentInfo)\n");

	m_MainWindow->bitmapSetup()->reloadContent(contentInfo);
}

void ContentManager::propertiesSetterChanged(QWidget *setter)
{
	printf("ContentManager::propertiesSetterChanged(setter)\n");

	if (setter != this)
	{
		m_ContentList->setCurrentItem(NULL);
	}
}

ContentInfo *ContentManager::current()
{
	if (!m_ContentList->currentItem())
		return NULL;
	return (ContentInfo *)m_ContentList->currentItem()->data(0, Qt::UserRole).value<void *>();
}

class ContentManager::ChangeSourcePath : public QUndoCommand
{
public:
	ChangeSourcePath(ContentManager *contentManager, ContentInfo *contentInfo, const QString &value) :
		QUndoCommand(),
		m_ContentManager(contentManager),
		m_ContentInfo(contentInfo),
		m_OldValue(contentInfo->SourcePath),
		m_NewValue(value)
	{
		setText(tr("Change content source path"));
	}

	virtual ~ChangeSourcePath()
	{

	}

	virtual void undo()
	{
		m_ContentInfo->SourcePath = m_OldValue;
		m_ContentManager->reprocessInternal(m_ContentInfo);
	}

	virtual void redo()
	{
		m_ContentInfo->SourcePath = m_NewValue;
		m_ContentManager->reprocessInternal(m_ContentInfo);
	}

	virtual int id() const
	{
		return 9064400;
	}

	virtual bool mergeWith(const QUndoCommand *command)
	{
		const ChangeSourcePath *c = static_cast<const ChangeSourcePath *>(command);

		if (c->m_ContentInfo != m_ContentInfo)
			return false;

		if (c->m_NewValue == m_OldValue)
			return false;

		m_NewValue = c->m_NewValue;
		return true;
	}

private:
	ContentManager *m_ContentManager;
	ContentInfo *m_ContentInfo;
	QString m_OldValue;
	QString m_NewValue;
};

void ContentManager::changeSourcePath(ContentInfo *contentInfo, const QString &value)
{
	// Create undo/redo
	ChangeSourcePath *changeSourcePath = new ChangeSourcePath(this, contentInfo, value);
	m_MainWindow->undoStack()->push(changeSourcePath);
}

void ContentManager::propertiesCommonSourcePathChanged()
{
	printf("ContentManager::propertiesCommonSourcePathChanged(value)\n");

	if (current() && current()->SourcePath != m_PropertiesCommonSourceFile->text())
		changeSourcePath(current(), m_PropertiesCommonSourceFile->text());
}

void ContentManager::propertiesCommonSourcePathBrowse()
{
	if (!current())
		return;

	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Load Content"),
		m_MainWindow->getFileDialogPath(),
		tr("All files (*.*)"));

	if (fileName.isNull())
		return;

	changeSourcePath(current(), fileName);
}

class ContentManager::ChangeDestName : public QUndoCommand
{
public:
	ChangeDestName(ContentManager *contentManager, ContentInfo *contentInfo, const QString &value) :
		QUndoCommand(),
		m_ContentManager(contentManager),
		m_ContentInfo(contentInfo),
		m_OldValue(contentInfo->DestName),
		m_NewValue(value)
	{
		setText(tr("Change content destination name"));
	}

	virtual ~ChangeDestName()
	{

	}

private:
	void renameFileExt(const QString &from, const QString &to, const QString &ext) const
	{
		if (!QFile::rename(from + ext, to + ext))
		{
			// printf("cannot rename '%s' to '%s'\n", (from + ext).toLocal8Bit().data(), (to + ext).toLocal8Bit().data());
			if (QFile::copy(from + ext, to + ext))
			{
				QFile::remove(from + ext);
			}
			else
			{
				// printf("cannot copy\n");
			}
		}
	}

protected:
	void renameFiles(const QString &from, const QString &to) const
	{
		m_ContentManager->lockContent();
		// Create destination directory
		if (to.contains('/'))
		{
			QString destDir = to.left(to.lastIndexOf('/'));
			if (!QDir(QDir::currentPath()).mkpath(destDir))
			{
				// Will fail at copy, and a rebuild will happen which will also fail with mkpath failure
				printf("Unable to create destination path\n");
			}
		}
		// Rename files to their new destination name
		const std::vector<QString> &fileExt = ContentManager::getFileExtensions();
		for (std::vector<QString>::const_iterator it(fileExt.begin()), end(fileExt.end()); it != end; ++it)
		{
			renameFileExt(from, to, (*it));
		}
		m_ContentManager->unlockContent();
	}

public:
	virtual void undo()
	{
		m_ContentInfo->DestName = m_OldValue;
		renameFiles(m_NewValue, m_OldValue);
		m_ContentManager->reprocessInternal(m_ContentInfo);
	}

	virtual void redo()
	{
		m_ContentInfo->DestName = m_NewValue;
		renameFiles(m_OldValue, m_NewValue);
		m_ContentManager->reprocessInternal(m_ContentInfo);
	}

	virtual int id() const
	{
		return 9064401;
	}

	virtual bool mergeWith(const QUndoCommand *command)
	{
		const ChangeDestName *c = static_cast<const ChangeDestName *>(command);

		if (c->m_ContentInfo != m_ContentInfo)
			return false;

		if (c->m_NewValue == m_OldValue)
			return false;

		m_NewValue = c->m_NewValue;
		return true;
	}

private:
	ContentManager *m_ContentManager;
	ContentInfo *m_ContentInfo;
	QString m_OldValue;
	QString m_NewValue;
};

void ContentManager::changeDestName(ContentInfo *contentInfo, const QString &value)
{
	// Create undo/redo
	if (contentInfo->DestName != value)
	{
		ChangeDestName *changeDestName = new ChangeDestName(this, contentInfo, createName(value));
		m_MainWindow->undoStack()->push(changeDestName);
	}
}

void ContentManager::propertiesCommonDestNameChanged()
{
	printf("ContentManager::propertiesCommonDestNameChanged(value)\n");

	if (current())
		changeDestName(current(), m_PropertiesCommonName->text());
}

class ContentManager::ChangeConverter : public QUndoCommand
{
public:
	ChangeConverter(ContentManager *contentManager, ContentInfo *contentInfo, ContentInfo::ConverterType value) :
		QUndoCommand(),
		m_ContentManager(contentManager),
		m_ContentInfo(contentInfo),
		m_OldValue(contentInfo->Converter),
		m_NewValue(value)
	{
		setText(tr("Set content converter"));
	}

	virtual ~ChangeConverter()
	{

	}

	virtual void undo()
	{
		m_ContentInfo->Converter = m_OldValue;
		m_ContentManager->reprocessInternal(m_ContentInfo);
	}

	virtual void redo()
	{
		m_ContentInfo->Converter = m_NewValue;
		m_ContentManager->reprocessInternal(m_ContentInfo);
	}

	virtual int id() const
	{
		return 9064402;
	}

	virtual bool mergeWith(const QUndoCommand *command)
	{
		const ChangeConverter *c = static_cast<const ChangeConverter *>(command);

		if (c->m_ContentInfo != m_ContentInfo)
			return false;

		if (c->m_NewValue == m_OldValue)
			return false;

		m_NewValue = c->m_NewValue;
		return true;
	}

private:
	ContentManager *m_ContentManager;
	ContentInfo *m_ContentInfo;
	ContentInfo::ConverterType m_OldValue;
	ContentInfo::ConverterType m_NewValue;
};

void ContentManager::changeConverter(ContentInfo *contentInfo, ContentInfo::ConverterType value)
{
	// Create undo/redo
	ChangeConverter *changeConverter = new ChangeConverter(this, contentInfo, value);
	m_MainWindow->undoStack()->push(changeConverter);
}

void ContentManager::propertiesCommonConverterChanged(int value)
{
	printf("ContentManager::propertiesCommonConverterChanged(value)\n");

	if (current() && current()->Converter != (ContentInfo::ConverterType)value)
		changeConverter(current(), (ContentInfo::ConverterType)value);
}

class ContentManager::ChangeImageFormat : public QUndoCommand
{
public:
	ChangeImageFormat(ContentManager *contentManager, ContentInfo *contentInfo, int value) :
		QUndoCommand(),
		m_ContentManager(contentManager),
		m_ContentInfo(contentInfo),
		m_OldValue(contentInfo->ImageFormat),
		m_NewValue(value)
	{
		setText(tr("Change image format"));
	}

	virtual ~ChangeImageFormat()
	{

	}

	virtual void undo()
	{
		m_ContentInfo->ImageFormat = m_OldValue;
		m_ContentManager->reprocessInternal(m_ContentInfo);
	}

	virtual void redo()
	{
		m_ContentInfo->ImageFormat = m_NewValue;
		m_ContentManager->reprocessInternal(m_ContentInfo);
	}

	virtual int id() const
	{
		return 9064403;
	}

	virtual bool mergeWith(const QUndoCommand *command)
	{
		const ChangeImageFormat *c = static_cast<const ChangeImageFormat *>(command);

		if (c->m_ContentInfo != m_ContentInfo)
			return false;

		if (c->m_NewValue == m_OldValue)
			return false;

		m_NewValue = c->m_NewValue;
		return true;
	}

private:
	ContentManager *m_ContentManager;
	ContentInfo *m_ContentInfo;
	int m_OldValue;
	int m_NewValue;
};

void ContentManager::changeImageFormat(ContentInfo *contentInfo, int value)
{
	// Create undo/redo
	ChangeImageFormat *changeImageFormat = new ChangeImageFormat(this, contentInfo, value);
	m_MainWindow->undoStack()->push(changeImageFormat);
}

void ContentManager::propertiesCommonImageFormatChanged(int value)
{
	printf("ContentManager::propertiesCommonImageFormatChanged(value)\n");

	if (current() && current()->ImageFormat != value)
		changeImageFormat(current(), value);
}

class ContentManager::ChangeMemoryLoaded : public QUndoCommand
{
public:
	ChangeMemoryLoaded(ContentManager *contentManager, ContentInfo *contentInfo, int value) :
		QUndoCommand(),
		m_ContentManager(contentManager),
		m_ContentInfo(contentInfo),
		m_OldValue(contentInfo->MemoryLoaded),
		m_NewValue(value)
	{
		setText(value ? tr("Load content to memory") : tr("Unload content from memory"));
	}

	virtual ~ChangeMemoryLoaded()
	{

	}

	virtual void undo()
	{
		m_ContentInfo->MemoryLoaded = m_OldValue;
		m_ContentManager->reuploadInternal(m_ContentInfo);
		m_ContentManager->reloadExternal(m_ContentInfo);
		if (m_ContentManager->m_CurrentPropertiesContent == m_ContentInfo)
			m_ContentManager->rebuildGUIInternal(m_ContentInfo);
	}

	virtual void redo()
	{
		m_ContentInfo->MemoryLoaded = m_NewValue;
		m_ContentManager->reuploadInternal(m_ContentInfo);
		m_ContentManager->reloadExternal(m_ContentInfo);
		if (m_ContentManager->m_CurrentPropertiesContent == m_ContentInfo)
			m_ContentManager->rebuildGUIInternal(m_ContentInfo);
	}

private:
	ContentManager *m_ContentManager;
	ContentInfo *m_ContentInfo;
	bool m_OldValue;
	bool m_NewValue;
};

void ContentManager::changeMemoryLoaded(ContentInfo *contentInfo, bool value)
{
	// Create undo/redo
	ChangeMemoryLoaded *changeMemoryLoaded = new ChangeMemoryLoaded(this, contentInfo, value);
	m_MainWindow->undoStack()->push(changeMemoryLoaded);
}

void ContentManager::propertiesCommonMemoryLoadedChanged(int value)
{
	printf("ContentManager::propertiesCommonMemoryLoadedChanged(value)\n");

	if (current() && current()->MemoryLoaded != (value == (int)Qt::Checked))
		changeMemoryLoaded(current(), (value == (int)Qt::Checked));
}

class ContentManager::ChangeMemoryAddress : public QUndoCommand
{
public:
	ChangeMemoryAddress(ContentManager *contentManager, ContentInfo *contentInfo, int value) :
		QUndoCommand(),
		m_ContentManager(contentManager),
		m_ContentInfo(contentInfo),
		m_OldValue(contentInfo->MemoryAddress),
		m_NewValue(value)
	{
		setText(tr("Change memory address"));
	}

	virtual ~ChangeMemoryAddress()
	{

	}

	virtual void undo()
	{
		m_ContentInfo->MemoryAddress = m_OldValue;
		m_ContentManager->reuploadInternal(m_ContentInfo);
		if (m_ContentManager->m_CurrentPropertiesContent == m_ContentInfo)
			m_ContentManager->rebuildGUIInternal(m_ContentInfo);
	}

	virtual void redo()
	{
		m_ContentInfo->MemoryAddress = m_NewValue;
		m_ContentManager->reuploadInternal(m_ContentInfo);
		if (m_ContentManager->m_CurrentPropertiesContent == m_ContentInfo)
			m_ContentManager->rebuildGUIInternal(m_ContentInfo);
	}

	virtual int id() const
	{
		return 9064404;
	}

	virtual bool mergeWith(const QUndoCommand *command)
	{
		const ChangeMemoryAddress *c = static_cast<const ChangeMemoryAddress *>(command);

		if (c->m_ContentInfo != m_ContentInfo)
			return false;

		if (c->m_NewValue == m_OldValue)
			return false;

		m_NewValue = c->m_NewValue;
		return true;
	}

private:
	ContentManager *m_ContentManager;
	ContentInfo *m_ContentInfo;
	int m_OldValue;
	int m_NewValue;
};

void ContentManager::changeMemoryAddress(ContentInfo *contentInfo, int value)
{
	// Create undo/redo
	ChangeMemoryAddress *changeMemoryAddress = new ChangeMemoryAddress(this, contentInfo, value & 0x7FFFFFFC); // Force round to 4
	m_MainWindow->undoStack()->push(changeMemoryAddress);
}

void ContentManager::propertiesCommonMemoryAddressChanged(int value)
{
	printf("ContentManager::propertiesCommonMemoryAddressChanged(value)\n");

	if (current() && current()->MemoryAddress != (value & 0x7FFFFFFC))
		changeMemoryAddress(current(), value);
	else if (current() && value != (value & 0x7FFFFFFC))
		rebuildGUIInternal(current());
}

class ContentManager::ChangeDataCompressed : public QUndoCommand
{
public:
	ChangeDataCompressed(ContentManager *contentManager, ContentInfo *contentInfo, int value) :
		QUndoCommand(),
		m_ContentManager(contentManager),
		m_ContentInfo(contentInfo),
		m_OldValue(contentInfo->DataCompressed),
		m_NewValue(value)
	{
		setText(value ? tr("Store data compressed") : tr("Store data uncompressed"));
	}

	virtual ~ChangeDataCompressed()
	{

	}

	virtual void undo()
	{
		m_ContentInfo->DataCompressed = m_OldValue;
		if (m_ContentManager->m_CurrentPropertiesContent == m_ContentInfo)
			m_ContentManager->rebuildGUIInternal(m_ContentInfo);
	}

	virtual void redo()
	{
		m_ContentInfo->DataCompressed = m_NewValue;
		if (m_ContentManager->m_CurrentPropertiesContent == m_ContentInfo)
			m_ContentManager->rebuildGUIInternal(m_ContentInfo);
	}

private:
	ContentManager *m_ContentManager;
	ContentInfo *m_ContentInfo;
	bool m_OldValue;
	bool m_NewValue;
};

void ContentManager::changeDataCompressed(ContentInfo *contentInfo, bool value)
{
	// Create undo/redo
	ChangeDataCompressed *changeDataCompressed = new ChangeDataCompressed(this, contentInfo, value);
	m_MainWindow->undoStack()->push(changeDataCompressed);
}

void ContentManager::propertiesCommonDataCompressedChanged(int value)
{
	printf("ContentManager::propertiesCommonDataCompressedChanged(value)\n");

	if (current() && current()->DataCompressed != (value == (int)Qt::Checked))
		changeDataCompressed(current(), (value == (int)Qt::Checked));
}

class ContentManager::ChangeDataEmbedded : public QUndoCommand
{
public:
	ChangeDataEmbedded(ContentManager *contentManager, ContentInfo *contentInfo, int value) :
		QUndoCommand(),
		m_ContentManager(contentManager),
		m_ContentInfo(contentInfo),
		m_OldValue(contentInfo->DataEmbedded),
		m_NewValue(value)
	{
		setText(value ? tr("Store data in header") : tr("Store data as file"));
	}

	virtual ~ChangeDataEmbedded()
	{

	}

	virtual void undo()
	{
		m_ContentInfo->DataEmbedded = m_OldValue;
		if (m_ContentManager->m_CurrentPropertiesContent == m_ContentInfo)
			m_ContentManager->rebuildGUIInternal(m_ContentInfo);
	}

	virtual void redo()
	{
		m_ContentInfo->DataEmbedded = m_NewValue;
		if (m_ContentManager->m_CurrentPropertiesContent == m_ContentInfo)
			m_ContentManager->rebuildGUIInternal(m_ContentInfo);
	}

private:
	ContentManager *m_ContentManager;
	ContentInfo *m_ContentInfo;
	bool m_OldValue;
	bool m_NewValue;
};

void ContentManager::changeDataEmbedded(ContentInfo *contentInfo, bool value)
{
	// Create undo/redo
	ChangeDataEmbedded *changeDataEmbedded = new ChangeDataEmbedded(this, contentInfo, value);
	m_MainWindow->undoStack()->push(changeDataEmbedded);
}

void ContentManager::propertiesCommonDataEmbeddedChanged(int value)
{
	printf("ContentManager::propertiesCommonDataEmbeddedChanged(value)\n");

	if (current() && current()->DataEmbedded != (value == (int)Qt::Checked))
		changeDataEmbedded(current(), (value == (int)Qt::Checked));
}

} /* namespace FT800EMUQT */

/* end of file */
