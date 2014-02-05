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
	m_PropertiesCommonSourceFile = new QLineEdit(this);
	QPushButton *browseSourceFile = new QPushButton(this);
	browseSourceFile->setMaximumWidth(browseSourceFile->height());
	addLabeledWidget(this, propCommonLayout, tr("Source file: "), m_PropertiesCommonSourceFile, browseSourceFile);
	m_PropertiesCommonName = new QLineEdit(this);
	addLabeledWidget(this, propCommonLayout, tr("Name: "), m_PropertiesCommonName);
	QComboBox *propCommonConverter = new QComboBox(this);
	m_PropertiesCommonConverter = propCommonConverter;
	propCommonConverter->addItem("");
	propCommonConverter->addItem(tr("Raw"));
	propCommonConverter->addItem(tr("Image"));
	propCommonConverter->addItem(tr("Raw Jpeg"));
	addLabeledWidget(this, propCommonLayout, tr("Converter: "), propCommonConverter);
	propCommon->setLayout(propCommonLayout);
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
		m_Owner(true) { }

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
		m_Owner(false) { }

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
	view->setData(0, Qt::UserRole, qVariantFromValue((void *)contentInfo));
	view->setText(0, contentInfo->SourcePath);
	contentInfo->View = view;

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
	}
}

void ContentManager::rebuildGUIInternal(ContentInfo *contentInfo)
{
	printf("ContentManager::rebuildGUIInternal()\n");

	// Build GUI for this content
	// Set widget values
	m_PropertiesCommonSourceFile->setText(contentInfo->SourcePath);
	m_PropertiesCommonName->setText(contentInfo->DestName);
	m_PropertiesCommonConverter->setCurrentIndex((int)contentInfo->Converter);

	// Display widgets in properties tab
	PropertiesEditor *props = m_MainWindow->propertiesEditor();
	if (m_CurrentPropertiesContent != contentInfo
		|| props->getEditWidgetSetter() != this)
	{
		ContentInfo *info = contentInfo;
		std::vector<QWidget *> widgets;

		widgets.push_back(m_PropertiesCommon);
		// ...

		props->setInfo(tr("<b>Source</b>: ") + info->SourcePath);
		props->setEditWidgets(widgets, false, this);
	}
}

void ContentManager::reprocessInternal(ContentInfo *contentInfo)
{
	printf("ContentManager::reprocessInternal()\n");

	// Reprocess this content

}

void ContentManager::propertiesSetterChanged(QWidget *setter)
{
	printf("ContentManager::propertiesSetterChanged()\n");

	if (setter != this)
	{
		m_ContentList->setCurrentItem(NULL);
	}
}

} /* namespace FT800EMUQT */

/* end of file */
