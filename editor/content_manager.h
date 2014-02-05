/**
 * content_manager.h
 * $Id$
 * \file content_manager.h
 * \brief content_manager.h
 * \date 2014-01-31 16:56GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#ifndef FT800EMUQT_CONTENT_MANAGER_H
#define FT800EMUQT_CONTENT_MANAGER_H

// STL includes
#include <vector>

// Qt includes
#include <QWidget>
#include <QString>

// Emulator includes
#include <ft800emu_inttypes.h>

// Project includes

class QTreeWidget;
class QTreeWidgetItem;
class QPushButton;
class QComboBox;
class QLineEdit;

namespace FT800EMUQT {

class MainWindow;

// Content information. Read-only. Writable through ContentManager only.
struct ContentInfo
{
	ContentInfo(const QString &filePath);

	QTreeWidgetItem *View;

	enum ConverterType
	{
		Invalid,
		Image, // Process image
		Raw, // Raw copy to ram
		Jpeg, // Load jpeg on coprocessor
	};

	QString SourcePath; // Relative source path
	QString DestName; // Local destination name
	ConverterType Converter;

	bool MemoryLoaded; //Present; // Whether this is loaded in ram
	int MemoryAddress; // Target memory address

	bool DataCompressed; // Use compressed data source when embedding - only relevant for code generator scripts
	bool DataEmbedded; // Whether to use embedded header or file - only relevant for code generator scripts

	int RawStart; // Raw start of memory
	int RawLength; // Raw length of memory

	int ImageFormat;
};

/**
 * ContentManager
 * \brief ContentManager
 * \date 2014-01-31 16:56GMT
 * \author Jan Boon (Kaetemi)
 */
class ContentManager : public QWidget
{
	Q_OBJECT

public:
	ContentManager(MainWindow *parent);
	virtual ~ContentManager();

	// Add the file to the content (this creates the undo/redo)
	ContentInfo *add(const QString &filePath);
	// Add the content (this creates the undo/redo)
	void add(ContentInfo *contentInfo);
	// Remove the content
	void remove(ContentInfo *remove);
	// Clear all content
	void clear();
	// Get all content
	void getContentInfos(std::vector<ContentInfo *> &contentInfos);

	// Get the currently selected content, may be NULL
	ContentInfo *current();

	// Changes
	void changeSourcePath(ContentInfo *contentInfo, const QString &value);

private:
	class Add;
	class Remove;

	class ChangeSourcePath;

	void addInternal(ContentInfo *contentInfo);
	void removeInternal(ContentInfo *contentInfo);
	void reprocessInternal(ContentInfo *contentInfo);
	void rebuildViewInternal(ContentInfo *contentInfo);
	void rebuildGUIInternal(ContentInfo *contentInfo);

	MainWindow *m_MainWindow;
	QTreeWidget *m_ContentList;
	QPushButton *m_RemoveButton;
	ContentInfo *m_CurrentPropertiesContent; // This pointer may be invalid. Only use to compare with current.

	QWidget *m_PropertiesCommon;
	QLineEdit *m_PropertiesCommonSourceFile;
	QLineEdit *m_PropertiesCommonName;
	QComboBox *m_PropertiesCommonConverter;

private slots:
	void add();
	void remove();
	void selectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
	void propertiesSetterChanged(QWidget *setter);

	void propertiesCommonSourcePathChanged(const QString &value);
	void propertiesCommonSourcePathBrowse();

private:
	ContentManager(const ContentManager &);
	ContentManager &operator=(const ContentManager &);

}; /* class ContentManager */

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_CONTENT_MANAGER_H */

/* end of file */
