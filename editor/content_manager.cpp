/**
 * content_manager.cpp
 * $Id$
 * \file content_manager.cpp
 * \brief content_manager.cpp
 * \date 2014-01-31 16:56GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2014  Future Technology Contents International Ltd
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

// Emulator includes
#include <vc.h>

// Project includes
#include "main_window.h"

using namespace std;

namespace FT800EMUQT {

ContentManager::ContentManager(MainWindow *parent) : QWidget(parent), m_MainWindow(parent)
{
	QVBoxLayout *layout = new QVBoxLayout(this);

	m_ContentList = new QTreeWidget(this);
	m_ContentList->header()->close();
	layout->addWidget(m_ContentList);

	QHBoxLayout *buttonsLayout = new QHBoxLayout(this);

	uint plusSign[2] = { 0x2795, 0 };
	QPushButton *addButton = new QPushButton();
	addButton->setText(QString::fromUcs4(plusSign) + " " + tr("Add"));
	connect(addButton, SIGNAL(clicked()), this, SLOT(add()));
	buttonsLayout->addWidget(addButton);

	uint minusSign[2] = { 0x2796, 0 };
	QPushButton *removeButton = new QPushButton();
	removeButton->setText(QString::fromUcs4(minusSign) + " " + tr("Remove"));
	connect(removeButton, SIGNAL(clicked()), this, SLOT(remove()));
	buttonsLayout->addWidget(removeButton);

	layout->addLayout(buttonsLayout);

	setLayout(layout);
}

ContentManager::~ContentManager()
{

}

void ContentManager::add()
{
	printf("ContentManager::add()\n");
}

void ContentManager::remove()
{
	printf("ContentManager::remove()\n");
}

} /* namespace FT800EMUQT */

/* end of file */
