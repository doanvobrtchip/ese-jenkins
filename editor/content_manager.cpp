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

// Emulator includes
#include <vc.h>

// Project includes
#include "main_window.h"
#include "properties_editor.h"
#include "undo_stack_disabler.h"
#include "asset_converter.h"

using namespace std;

namespace FT800EMUQT {

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

	m_PropertiesImagePreview = new QGroupBox(this);
	QVBoxLayout *imagePreviewLayout = new QVBoxLayout(this);
	m_PropertiesImagePreview->setHidden(true);
	m_PropertiesImagePreview->setTitle(tr("Image Preview"));
	m_PropertiesImageLabel = new QLabel(this);
	imagePreviewLayout->addWidget(m_PropertiesImageLabel);
	m_PropertiesImagePreview->setLayout(imagePreviewLayout);
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
		rebuildGUIInternal((ContentInfo *)current->data(0, Qt::UserRole).value<void *>());

		// Be helpful
		m_MainWindow->focusProperties();
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
		widgets.push_back(m_PropertiesImagePreview);

		props->setEditWidgets(widgets, false, this);
	}

	// Set common widget values
	m_PropertiesCommonSourceFile->setText(contentInfo->SourcePath);
	m_PropertiesCommonName->setText(contentInfo->DestName);
	m_PropertiesCommonConverter->setCurrentIndex((int)contentInfo->Converter);

	// Set user help, wizard format
	if (contentInfo->Converter == ContentInfo::Invalid)
	{
		m_PropertiesImagePreview->setHidden(true);
		props->setInfo(tr("Select a <b>Converter</b> to be used for this file. Converted files will be stored in the folder where the project is saved.<br><br><b>Image</b>: Converts an image to one of the supported formats.<br><b>Raw</b>: Does a direct binary copy.<br><b>Raw JPEG</b>: Does a raw binary copy and decodes the JPEG on the coprocessor."));
	}
	else if (!contentInfo->BuildError.isEmpty())
	{
		props->setInfo(tr("<b>Error</b>: ") + contentInfo->BuildError);
	}
	else
	{
		props->setInfo(tr("Ready."));
		switch (contentInfo->Converter)
		{
			case ContentInfo::Image:
			{
				QPixmap pixmap;
				bool loadSuccess = pixmap.load(contentInfo->DestName + "_converted.png");
				m_PropertiesImagePreview->setHidden(!loadSuccess);
				m_PropertiesImageLabel->setPixmap(pixmap.scaled(m_PropertiesImageLabel->width() - 32, m_PropertiesImageLabel->width() - 32, Qt::KeepAspectRatio));
				if (loadSuccess) m_PropertiesImageLabel->repaint();
				else props->setInfo(tr("Failed to load image preview."));
				break;
			}
			case ContentInfo::Raw:
			{
				m_PropertiesImagePreview->setHidden(true);
				break;
			}
			case ContentInfo::Jpeg:
			{
				m_PropertiesImagePreview->setHidden(true);
				break;
			}
		}
	}
}

void ContentManager::reprocessInternal(ContentInfo *contentInfo)
{
	printf("ContentManager::reprocessInternal()\n");

	// Reprocess this content
	// TODO: Check for changes and datestamps and so on...
	contentInfo->BuildError = "";
	switch (contentInfo->Converter)
	{
	case ContentInfo::Image:
		AssetConverter::convertImage(contentInfo->BuildError, contentInfo->SourcePath, contentInfo->DestName, contentInfo->ImageFormat);
		break;
	case ContentInfo::Raw:
		AssetConverter::convertRaw(contentInfo->BuildError, contentInfo->SourcePath, contentInfo->DestName, contentInfo->RawStart, contentInfo->RawLength);
		break;
	case ContentInfo::Jpeg:
		AssetConverter::convertRaw(contentInfo->BuildError, contentInfo->SourcePath, contentInfo->DestName, 0, 0);
		break;
	default:
		contentInfo->BuildError = "No converter selected";
		break;
	}

	// Update GUI if it exists
	if (m_CurrentPropertiesContent == contentInfo)
		rebuildGUIInternal(contentInfo);
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

	virtual void undo()
	{
		m_ContentInfo->DestName = m_OldValue;
		m_ContentManager->reprocessInternal(m_ContentInfo);
	}

	virtual void redo()
	{
		m_ContentInfo->DestName = m_NewValue;
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
	ChangeDestName *changeDestName = new ChangeDestName(this, contentInfo, value);
	m_MainWindow->undoStack()->push(changeDestName);
}

void ContentManager::propertiesCommonDestNameChanged()
{
	printf("ContentManager::propertiesCommonDestNameChanged(value)\n");

	if (current() && current()->DestName != m_PropertiesCommonName->text())
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

} /* namespace FT800EMUQT */

/* end of file */
