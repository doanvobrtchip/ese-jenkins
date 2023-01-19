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

#ifndef FTEDITOR_CONTENT_MANAGER_H
#define FTEDITOR_CONTENT_MANAGER_H

#pragma warning(disable : 26812)
#pragma warning(disable : 26495)
#pragma warning(disable : 26444)

// STL includes
#include <vector>
#include <set>

// Qt includes
#include <QWidget>
#include <QTreeWidget>
#include <QLabel>
#include <QString>
#include <QMutex>
#include <QJsonObject>

// Emulator includes
#include <bt8xxemu_inttypes.h>

// Project includes
#include "undo_stack_disabler.h"
#include "constant_mapping.h"

class QTreeWidgetItem;
class QPushButton;
class QComboBox;
class QLineEdit;
class QGroupBox;
class QLabel;
class QSpinBox;
class QCheckBox;

namespace FTEDITOR {
struct ContentInfo;
}

class ContentTreeWidget : public QTreeWidget
{
	Q_OBJECT

signals:
	void contentDropped(QString url);

public:
	virtual void dragEnterEvent(QDragEnterEvent *event) override;
	virtual void dropEvent(QDropEvent *event) override;
	virtual QStringList mimeTypes() const override;
	void mousePressEvent(QMouseEvent *event) override;

public:
	explicit ContentTreeWidget(QWidget *parent = nullptr);
	~ContentTreeWidget();

signals:
	void addItem(FTEDITOR::ContentInfo *item);
	void removeItem(FTEDITOR::ContentInfo *item);
};

class ContentLabel : public QLabel
{
	Q_OBJECT

signals:
	void contentDropped(QString url);

public:
	virtual void dragEnterEvent(QDragEnterEvent *event) override;
	virtual void dropEvent(QDropEvent *event) override;

public:
	explicit ContentLabel(QWidget *parent = nullptr);
	~ContentLabel();
};

namespace FTEDITOR {

extern int g_RamGlobalUsage;
extern int g_FlashGlobalUsage;

#define FTEDITOR_FLASH_FIRMWARE_SIZE 4096

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

	static const QMap<QString, QString> MapInfoFileType;

	enum ConverterType // Do not change order.
	{
		Invalid,
		Image, // Process image
		Raw, // Raw copy to ram
		Font, // Font
		ImageCoprocessor, // Load jpeg on coprocessor
		FlashMap = 1025, // Loaded from flash map, can only be set using import
	};

	enum StorageType
	{
		File,
		Embedded,
		Flash
	};

	// ContentInfo::FlashMap functionality:
	// Similar to Raw, but takes a .map file as input, and renames the .map extension to .bin to get the source data
	// Links to specific file by means of name. Error in case name no longer exists in the map

	QString SourcePath; // Relative source path
	QString DestName; // Local destination name
	ConverterType Converter;

	bool MemoryLoaded; //Present; // Whether this is loaded in ram
	int MemoryAddress; // Target memory address

	StorageType DataStorage; // Whether to use embedded header or file - only relevant for code generator scripts, or whether this is loaded into flash
	bool DataCompressed; // Use compressed data source when embedding - only relevant for code generator scripts
	int FlashAddress; // Target flash address

	int RawStart; // Raw start of memory
	int RawLength; // Raw length of memory

	int ImageFormat;
	
	bool ImageMono;

	int FontSize;
	QString FontCharSet;
	int FontOffset;

	QString MappedName; // Name of this asset in the flash map

	QString BuildError;

	QJsonObject toJson(bool meta) const;
	void fromJson(QJsonObject &j, bool meta);
	bool equalsMeta(const ContentInfo *other) const;
	bool requirePaletteAddress();

	bool UploadMemoryDirty;
	bool UploadFlashDirty;
	bool ExternalDirty;

	bool CachedImage;
	int CachedImageWidth;
	int CachedImageHeight;
	int CachedImageStride;
	int CachedBitmapSource;

	int CachedMemorySize; // Memory size (for example, after JPEG decompression by coprocessor)

	bool OverlapMemoryFlag;
	bool OverlapFlashFlag;
	bool WantAutoLoad;

	int bitmapAddress(int deviceIntf = FTEDITOR_CURRENT_DEVICE) const;
	int PalettedAddress;
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
	static const QString ResourceDir;

public:
	ContentManager(MainWindow *parent);
	virtual ~ContentManager();

	void bindCurrentDevice();

	// Add the file to the content (this creates the undo/redo)
	ContentInfo *add(const QString &filePath);
	// Add the content (this creates the undo/redo)
	void add(ContentInfo *contentInfo);

	void addInternal(QStringList fileNameList);

	// Remove the content
	void remove(ContentInfo *remove, bool whenCloseProject = false);
	// Load or reload a flash map. Only one flash map will be included at a time
	bool loadFlashMap(QString flashMapPath = QString());
	// Clear all content
	void clear(bool whenCloseProject = false);
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
	// Suppress overlap check
	void suppressOverlapCheck();
	// Resume overlap check
	void resumeOverlapCheck();

	// Get the currently selected content, may be NULL
	ContentInfo *current();
	// Get all content that's selected to be loaded into RAM
	std::vector<ContentInfo *> allRam();
	// Get all content that's selected to be loaded into Flash
	std::vector<ContentInfo *> allFlash();

	// Get the current flash map path
	QString findFlashMapPath(bool forceScan = false);

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
	// Update palette adress
	void editorUpdatePaletteAddress(int newAddr, int oldAddr, DlEditor *dlEditor);
	// Update font adress
	void editorUpdateFontAddress(int newAddr, int oldAddr, DlEditor *dlEditor);
	// Update font adress
	void editorUpdateFontOffset(ContentInfo *contentInfo, DlEditor *dlEditor);
	// ISSUE#113: Remove all entries related to content info
	void editorRemoveContent(ContentInfo *contentInfo, DlEditor *dlEditor);

	// Changes
	void changeSourcePath(ContentInfo *contentInfo, const QString &value);
	void changeDestName(ContentInfo *contentInfo, const QString &value);
	void changeConverter(ContentInfo *contentInfo, ContentInfo::ConverterType value);
	void changeImageFormat(ContentInfo *contentInfo, int value);
	void changeImageMono(ContentInfo *contentInfo, bool value);
	void changeFontFormat(ContentInfo *contentInfo, int value);
	void changeFontSize(ContentInfo *contentInfo, int value);
	void changeFontCharSet(ContentInfo *contentInfo, const QString &value);
	void changeFontOffset(ContentInfo *contentInfo, int value);
	void changeRawStart(ContentInfo *contentInfo, int value);
	void changeRawLength(ContentInfo *contentInfo, int value);
	void changeMemoryLoaded(ContentInfo *contentInfo, bool value);
	void changeMemoryAddress(ContentInfo *contentInfo, int value, bool internal = false);
	void changeDataStorage(ContentInfo *contentInfo, ContentInfo::StorageType value);
	void changeDataCompressed(ContentInfo *contentInfo, bool value);
	void changeFlashAddress(ContentInfo *contentInfo, int value, bool internal = false);

	// Lock to call when editing/moving content files from qt thread, when reading content from non-qt threads
	void lockContent() { s_Mutex.lock(); }
	void unlockContent() { s_Mutex.unlock(); }

	// Get
	inline static const std::vector<QString> &getFileExtensions() { return s_FileExtensions; }
	inline void swapUploadMemoryDirty(std::set<ContentInfo *> &contentInfo) { contentInfo.clear(); m_ContentUploadMemoryDirty.swap(contentInfo); }
	inline void swapUploadFlashDirty(std::set<ContentInfo *> &contentInfo) { contentInfo.clear(); m_ContentUploadFlashDirty.swap(contentInfo); }
	void reuploadAll();

	// Utility
	inline const ContentTreeWidget * contentList() const { return m_ContentList; }

	void copyFlashFile();

	int getFreeMemoryAddress(); // Return -1 if no more space
	int getContentSize(ContentInfo *contentInfo); // Return -1 if not exist

private:
	class Add;
	class Remove;

	class ChangeSourcePath;
	class ChangeDestName;
	class ChangeConverter;
	class ChangeImageFormat;
	class ChangeImageMono;
	class ChangeFontFormat;
	class ChangeFontSize;
	class ChangeFontCharSet;
	class ChangeFontOffset;
	class ChangeRawStart;
	class ChangeRawLength;
	class ChangeMemoryLoaded;
	class ChangeMemoryAddress;
	class ChangeDataStorage;
	class ChangeDataCompressed;
	class ChangeFlashAddress;

	bool nameExists(const QString &name);
	QString createName(const QString &name); // Rename if already exists.

	int getFlashSize(ContentInfo *contentInfo); // Return -1 if not exist
	
	// int getFreeFlashAddress(int size); // Return -1 if no more space

	void addInternal(ContentInfo *contentInfo);
	void removeInternal(ContentInfo *contentInfo);
	void reprocessInternal(ContentInfo *contentInfo);
	void reuploadInternal(ContentInfo *contentInfo, bool memory, bool flash);
	void recalculateOverlapMemoryInternal();
	void recalculateOverlapFlashInternal();
	void rebuildViewInternal(ContentInfo *contentInfo);
	void rebuildGUIInternal(ContentInfo *contentInfo);

	void reloadExternal(ContentInfo *contentInfo);

	MainWindow *m_MainWindow;
	ContentTreeWidget *m_ContentList;
	QPushButton *m_RemoveButton;
	ContentInfo *m_CurrentPropertiesContent; // This pointer may be invalid. Only use to compare with current.

	ContentLabel *m_HelpfulLabel;

	QWidget *m_PropertiesCommon;
	UndoStackDisabler<QLineEdit> *m_PropertiesCommonSourceFile;
	UndoStackDisabler<QLineEdit> *m_PropertiesCommonName;
	QLabel *m_PropertiesCommonConverterLabel;
	QComboBox *m_PropertiesCommonConverter;

	QGroupBox *m_PropertiesImage;
	QComboBox *m_PropertiesImageFormat;

	QGroupBox *m_PropertiesImageCoprocessor;
	QCheckBox *m_PropertiesImageMono;

	QGroupBox *m_PropertiesImagePreview;
	QLabel *m_PropertiesImageLabel;

	QGroupBox *m_PropertiesRaw;
	UndoStackDisabler<QSpinBox> *m_PropertiesRawStart;
	UndoStackDisabler<QSpinBox> *m_PropertiesRawLength;

	QGroupBox *m_PropertiesFont;
	QComboBox *m_PropertiesFontFormat;
	UndoStackDisabler<QSpinBox> *m_PropertiesFontSize;
	UndoStackDisabler<QLineEdit> *m_PropertiesFontCharSet;
	UndoStackDisabler<QSpinBox> *m_PropertiesFontOffset;

	QGroupBox *m_PropertiesMemory;
	QCheckBox *m_PropertiesMemoryLoaded;
	UndoStackDisabler<QSpinBox> *m_PropertiesMemoryAddress;

	QGroupBox *m_PropertiesData; // Storage
	QComboBox *m_PropertiesDataStorage;
	QCheckBox *m_PropertiesDataCompressed;
	QLabel *m_PropertiesFlashAddressLabel;
	UndoStackDisabler<QSpinBox> *m_PropertiesFlashAddress;

	static std::vector<QString> s_FileExtensions;

	std::set<ContentInfo *> m_ContentUploadMemoryDirty;
	std::set<ContentInfo *> m_ContentOverlapMemory;

	std::set<ContentInfo *> m_ContentUploadFlashDirty;
	std::set<ContentInfo *> m_ContentOverlapFlash;

	int m_OverlapSuppressed;
	bool m_OverlapMemorySuppressed;
	bool m_OverlapFlashSuppressed;

	static QMutex s_Mutex;

	QString m_FlashFileName;

private slots:
	void add();
	void remove();

public slots:
	void rebuildAll();

	void importFlashMapped();
	void exportFlashMapped();
	void setup(QObject *obj = nullptr);
	void handleUpdateCurrentInfo(FTEDITOR::ContentInfo *contentInfo);

private slots:
	void selectionChanged(QTreeWidgetItem *current, QTreeWidgetItem *previous);
	void propertiesSetterChanged(QWidget *setter);

	void propertiesCommonSourcePathChanged();
	void propertiesCommonSourcePathBrowse();
	void propertiesCommonDestNameChanged();
	void propertiesCommonConverterChanged(int value);
	void propertiesImageFormatChanged(int value);
	void propertiesImageMonoChanged(int value);
	void propertiesFontFormatChanged(int value);
	void propertiesFontSizeChanged(int value);
	void propertiesFontCharSetChanged();
	void propertiesFontOffsetChanged(int value);
	void propertiesRawStartChanged(int value);
	void propertiesRawLengthChanged(int value);
	void propertiesMemoryLoadedChanged(int value);
	void propertiesMemoryAddressChanged(int value);
	void propertiesDataStorageChanged(int value);
	void propertiesDataCompressedChanged(int value);
	void propertiesFlashAddressChanged(int value);

private:
	ContentManager(const ContentManager &);
	ContentManager &operator=(const ContentManager &);

signals:
	void ramGlobalUsageChanged(int value);
	void busyNow(QObject *obj);
	void freeNow(QObject *obj);

}; /* class ContentManager */

} /* namespace FTEDITOR */

#endif /* #ifndef FTEDITOR_CONTENT_MANAGER_H */

/* end of file */
