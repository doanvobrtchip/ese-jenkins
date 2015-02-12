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
#include <set>

// Qt includes
#include <QWidget>
#include <QString>
#include <QMutex>
#include <QJsonObject>

// Emulator includes
#include <ft8xxemu_inttypes.h>

// Project includes
#include "undo_stack_disabler.h"

class QTreeWidget;
class QTreeWidgetItem;
class QPushButton;
class QComboBox;
class QLineEdit;
class QGroupBox;
class QLabel;
class QSpinBox;
class QCheckBox;

namespace FT800EMUQT {

extern int g_RamGlobalUsage;

#define BITMAP_SETUP_HANDLES_NB 16

class MainWindow;
class DlEditor;

template<class T>
class UndoStackDisabler;

// Content information. Read-only. Writable through ContentManager only.
struct ContentInfo
{
	ContentInfo(const QString &filePath);

	QTreeWidgetItem *View;

	enum ConverterType // Do not change order.
	{
		Invalid,
		Image, // Process image
		Raw, // Raw copy to ram
		Font, // Font
		RawJpeg, // Load jpeg on coprocessor // TODO
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

	int FontSize;
	QString FontCharSet;

	QString BuildError;

	QJsonObject toJson(bool meta) const;
	void fromJson(QJsonObject &j, bool meta);
	bool equalsMeta(const ContentInfo *other) const;

	bool UploadDirty;
	bool ExternalDirty;

	bool CachedImage;
	int CachedImageWidth;
	int CachedImageHeight;
	int CachedImageStride;

	int CachedSize; // Memory size

	bool OverlapFlag;
	bool WantAutoLoad;
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
	// Get the number of content
	int getContentCount() const;
	// Returns if content info is part of the manager currently
	bool isValidContent(ContentInfo *info);
	// Cache image build info (Only call if info is valid!)
	bool cacheImageInfo(ContentInfo *info);
	// Find content info
	ContentInfo *find(const QString &destName);

	// Get the currently selected content, may be NULL
	ContentInfo *current();

	// Editor utilities
	// Find handle related to content address, return -1 on failure // TODO: depend on current editor line
	int editorFindHandle(ContentInfo *contentInfo, DlEditor *dlEditor);
	int editorFindHandle(ContentInfo *contentInfo, DlEditor *dlEditor, int &line);
	// Find a free handle, return -1 when no free handle available // TODO: depend on current editor line as fallback
	int editorFindFreeHandle(DlEditor *dlEditor);
	// Find where to start with bitmap lines in the editor
	int editorFindNextBitmapLine(DlEditor *dlEditor);
	// Update handle layout related to content info
	void editorUpdateHandle(ContentInfo *contentInfo, DlEditor *dlEditor, bool updateSize);
	// Update handle adress
	void editorUpdateHandleAddress(int newAddr, int oldAddr, DlEditor *dlEditor);
	// Update font adress
	void editorUpdateFontAddress(int newAddr, int oldAddr, DlEditor *dlEditor);
	// ISSUE#113: Remove all entries related to content info
	void editorRemoveContent(ContentInfo *contentInfo, DlEditor *dlEditor);

	// Changes
	void changeSourcePath(ContentInfo *contentInfo, const QString &value);
	void changeDestName(ContentInfo *contentInfo, const QString &value);
	void changeConverter(ContentInfo *contentInfo, ContentInfo::ConverterType value);
	void changeImageFormat(ContentInfo *contentInfo, int value);
	void changeFontFormat(ContentInfo *contentInfo, int value);
	void changeFontSize(ContentInfo *contentInfo, int value);
	void changeFontCharSet(ContentInfo *contentInfo, const QString &value);
	void changeRawStart(ContentInfo *contentInfo, int value);
	void changeRawLength(ContentInfo *contentInfo, int value);
	void changeMemoryLoaded(ContentInfo *contentInfo, bool value);
	void changeMemoryAddress(ContentInfo *contentInfo, int value, bool internal = false);
	void changeDataCompressed(ContentInfo *contentInfo, bool value);
	void changeDataEmbedded(ContentInfo *contentInfo, bool value);

	// Lock to call when editing/moving content files from qt thread, when reading content from non-qt threads
	void lockContent() { m_Mutex.lock(); }
	void unlockContent() { m_Mutex.unlock(); }

	// Get
	inline static const std::vector<QString> &getFileExtensions() { return s_FileExtensions; }
	inline void swapUploadDirty(std::set<ContentInfo *> &contentInfo) { contentInfo.clear(); m_ContentUploadDirty.swap(contentInfo); }

	// Utility
	inline const QTreeWidget *contentList() const { return m_ContentList; }

private:
	class Add;
	class Remove;

	class ChangeSourcePath;
	class ChangeDestName;
	class ChangeConverter;
	class ChangeImageFormat;
	class ChangeFontFormat;
	class ChangeFontSize;
	class ChangeFontCharSet;
	class ChangeRawStart;
	class ChangeRawLength;
	class ChangeMemoryLoaded;
	class ChangeMemoryAddress;
	class ChangeDataCompressed;
	class ChangeDataEmbedded;

	bool nameExists(const QString &name);
	QString createName(const QString &name); // Rename if already exists.

	int getContentSize(ContentInfo *contentInfo); // Return -1 if not exist
	int getFreeAddress(); // Return -1 if no more space

	void addInternal(ContentInfo *contentInfo);
	void removeInternal(ContentInfo *contentInfo);
	void reprocessInternal(ContentInfo *contentInfo);
	void reuploadInternal(ContentInfo *contentInfo);
	void recalculateOverlapInternal();
	void rebuildViewInternal(ContentInfo *contentInfo);
	void rebuildGUIInternal(ContentInfo *contentInfo);

	void reloadExternal(ContentInfo *contentInfo);

	MainWindow *m_MainWindow;
	QTreeWidget *m_ContentList;
	QPushButton *m_RemoveButton;
	ContentInfo *m_CurrentPropertiesContent; // This pointer may be invalid. Only use to compare with current.

	QLabel *m_HelpfulLabel;

	QWidget *m_PropertiesCommon;
	UndoStackDisabler<QLineEdit> *m_PropertiesCommonSourceFile;
	UndoStackDisabler<QLineEdit> *m_PropertiesCommonName;
	QComboBox *m_PropertiesCommonConverter;

	QGroupBox *m_PropertiesImage;
	QComboBox *m_PropertiesImageFormat;

	QGroupBox *m_PropertiesImagePreview;
	QLabel *m_PropertiesImageLabel;

	QGroupBox *m_PropertiesRaw;
	UndoStackDisabler<QSpinBox> *m_PropertiesRawStart;
	UndoStackDisabler<QSpinBox> *m_PropertiesRawLength;

	QGroupBox *m_PropertiesFont;
	QComboBox *m_PropertiesFontFormat;
	UndoStackDisabler<QSpinBox> *m_PropertiesFontSize;
	UndoStackDisabler<QLineEdit> *m_PropertiesFontCharSet;

	QGroupBox *m_PropertiesMemory;
	QCheckBox *m_PropertiesMemoryLoaded;
	UndoStackDisabler<QSpinBox> *m_PropertiesMemoryAddress;
	QCheckBox *m_PropertiesDataCompressed;
	QCheckBox *m_PropertiesDataEmbedded;

	static std::vector<QString> s_FileExtensions;

	std::set<ContentInfo *> m_ContentUploadDirty;
	std::set<ContentInfo *> m_ContentOverlap;

	QMutex m_Mutex;

private slots:
	void add();
	void remove();

public slots:
	void rebuildAll();

private slots:
	void selectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
	void propertiesSetterChanged(QWidget *setter);

	void propertiesCommonSourcePathChanged();
	void propertiesCommonSourcePathBrowse();
	void propertiesCommonDestNameChanged();
	void propertiesCommonConverterChanged(int value);
	void propertiesImageFormatChanged(int value);
	void propertiesFontFormatChanged(int value);
	void propertiesFontSizeChanged(int value);
	void propertiesFontCharSetChanged();
	void propertiesRawStartChanged(int value);
	void propertiesRawLengthChanged(int value);
	void propertiesMemoryLoadedChanged(int value);
	void propertiesMemoryAddressChanged(int value);
	void propertiesDataCompressedChanged(int value);
	void propertiesDataEmbeddedChanged(int value);

private:
	ContentManager(const ContentManager &);
	ContentManager &operator=(const ContentManager &);

}; /* class ContentManager */

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_CONTENT_MANAGER_H */

/* end of file */
