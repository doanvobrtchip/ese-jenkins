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

// Emulator includes
#include <vc.h>

// Project includes
#include "main_window.h"
#include "properties_editor.h"
#include "undo_stack_disabler.h"
#include "asset_converter.h"

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
	QHBoxLayout *hbox = new QHBoxLayout(parent);
	QLabel *l = new QLabel(parent);
	l->setText(label);
	hbox->addWidget(l);
	hbox->addWidget(widget);
	layout->addLayout(hbox);
}

void addLabeledWidget(QWidget *parent, QVBoxLayout *layout, const QString &label, QWidget *widget0, QWidget *widget1)
{
	QHBoxLayout *hbox = new QHBoxLayout(parent);
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
		s_FileExtensions.push_back(".ft800meta");
	}

	QVBoxLayout *layout = new QVBoxLayout(this);

	m_ContentList = new QTreeWidget(this);
	m_ContentList->header()->close();
	layout->addWidget(m_ContentList);
	connect(m_ContentList, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(selectionChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

	QHBoxLayout *buttonsLayout = new QHBoxLayout(this);

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

	setLayout(layout);

	// Attach selection to properties
	connect(m_MainWindow->propertiesEditor(), SIGNAL(setterChanged(QWidget *)), this, SLOT(propertiesSetterChanged(QWidget *)));

	// Build properties widgets
	QGroupBox *propCommon = new QGroupBox(this);
	propCommon->setHidden(true);
	m_PropertiesCommon = propCommon;
	propCommon->setTitle(tr("Content"));
	QVBoxLayout *propCommonLayout = new QVBoxLayout(this);
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
	propCommonConverter->addItem(tr("Raw JPEG"));
	addLabeledWidget(this, propCommonLayout, tr("Converter: "), propCommonConverter);
	connect(m_PropertiesCommonConverter, SIGNAL(currentIndexChanged(int)), this, SLOT(propertiesCommonConverterChanged(int)));
	propCommon->setLayout(propCommonLayout);

	m_PropertiesImage = new QGroupBox(this);
	m_PropertiesImage->setHidden(true);
	m_PropertiesImage->setTitle(tr("Image Settings"));
	QVBoxLayout *imagePropsLayout = new QVBoxLayout(this);
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
	QVBoxLayout *imagePreviewLayout = new QVBoxLayout(this);
	m_PropertiesImagePreview->setHidden(true);
	m_PropertiesImagePreview->setTitle(tr("Image Preview"));
	m_PropertiesImageLabel = new QLabel(this);
	imagePreviewLayout->addWidget(m_PropertiesImageLabel);
	m_PropertiesImagePreview->setLayout(imagePreviewLayout);

	m_PropertiesMemory = new QGroupBox(this);
	m_PropertiesMemory->setHidden(true);
	m_PropertiesMemory->setTitle(tr("Memory Options"));
	QVBoxLayout *propMemLayout = new QVBoxLayout(this);
	m_PropertiesMemoryAddress = new QSpinBox(this);
	m_PropertiesMemoryAddress->setMinimum(0);
	m_PropertiesMemoryAddress->setMaximum(RAM_DL - 4);
	m_PropertiesMemoryAddress->setSingleStep(4);
	addLabeledWidget(this, propMemLayout, tr("Address: "), m_PropertiesMemoryAddress);
	m_PropertiesMemoryLoaded = new QCheckBox(this);
	addLabeledWidget(this, propMemLayout, tr("Loaded: "), m_PropertiesMemoryLoaded);
	QFrame* line = new QFrame();
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);
	propMemLayout->addWidget(line);
	m_PropertiesDataCompressed = new QCheckBox(this);
	addLabeledWidget(this, propMemLayout, tr("Compressed: "), m_PropertiesDataCompressed);
	m_PropertiesDataEmbedded = new QCheckBox(this);
	addLabeledWidget(this, propMemLayout, tr("Embedded: "), m_PropertiesDataEmbedded);
	m_PropertiesMemory->setLayout(propMemLayout);
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
	reprocessInternal(contentInfo);

	// Be helpful
	m_ContentList->setCurrentItem(view);
}

void ContentManager::removeInternal(ContentInfo *contentInfo)
{
	printf("ContentManager::removeInternal(contentInfo)\n");

	// Remove from the content list
	delete contentInfo->View;
	contentInfo->View = NULL;
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
	QString destName = name;
	int renumber = 2;
	while (nameExists(destName))
	{
		destName = name + "_" + QString::number(renumber);
		++renumber;
	}
	return destName;
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

void ContentManager::getContentInfos(std::vector<ContentInfo *> &contentInfos)
{
	// Iterate through the list and copy the pointer to the data
	for (QTreeWidgetItemIterator it(m_ContentList); *it; ++it)
	{
		ContentInfo *info = (ContentInfo *)(*it)->data(0, Qt::UserRole).value<void *>();
		contentInfos.push_back(info);
	}
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
		ContentInfo *info = contentInfo;
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
		if (!contentInfo->BuildError.isEmpty())
		{
			props->setInfo(tr("<b>Error</b>: ") + contentInfo->BuildError);
		}
		else
		{
			props->setInfo(tr("Ready."));
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
				else if (contentInfo->BuildError.isEmpty()) props->setInfo(tr("Failed to load image preview."));
				m_PropertiesMemory->setHidden(false);
				break;
			}
			case ContentInfo::Raw:
			{
				m_PropertiesImage->setHidden(true);
				m_PropertiesImagePreview->setHidden(true);
				m_PropertiesMemory->setHidden(false);
				break;
			}
			case ContentInfo::RawJpeg:
			{
				m_PropertiesImage->setHidden(true);
				m_PropertiesImagePreview->setHidden(true);
				m_PropertiesMemory->setHidden(false);
				break;
			}
		}
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
		QString metaFile = contentInfo->DestName + ".ft800meta";
		bool metaExists = QFile::exists(metaFile);
		bool equalMeta = false;
		if (metaExists)
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
		if (!equalMeta)
		{
			if (metaExists)
			{
				QFile::remove(metaFile); // *** FILE REMOVE ***
			}
			contentInfo->BuildError = "";
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
				contentInfo->BuildError = "<b>Critical Error</b><br>Unknown converter selected.";
				break;
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
			}
		}
		else
		{
			printf("Equal meta, skip convert\n");
		}
		unlockContent();
	}

	// Reupload the content to emulator ram
	reuploadInternal(contentInfo);

	// Update GUI if it exists
	if (m_CurrentPropertiesContent == contentInfo)
		rebuildGUIInternal(contentInfo);
}

void ContentManager::reuploadInternal(ContentInfo *contentInfo)
{
	// Reupload the content to emulator RAM
	// This happens in the emulator main loop
	// Emulator main loop will lock the content mutex
	// TODO
}

void ContentManager::propertiesSetterChanged(QWidget *setter)
{
	printf("ContentManager::propertiesSetterChanged()\n");

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
		QFile::rename(from + ext, to + ext);
	}

protected:
	void renameFiles(const QString &from, const QString &to) const
	{
		m_ContentManager->lockContent();
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

} /* namespace FT800EMUQT */

/* end of file */
