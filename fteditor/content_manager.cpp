/*
Copyright (C) 2014-2015  Future Technology Devices International Ltd
Author: Jan Boon <jan.boon@kaetemi.be>
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
#include <QMessageBox>
#include <QMessageBox>

// Emulator includes
#include "bt8xxemu_diag.h"

// Project includes
#include "main_window.h"
#include "properties_editor.h"
#include "undo_stack_disabler.h"
#include "asset_converter.h"
#include "dl_editor.h"
#include "dl_parser.h"
#include "constant_mapping.h"
#include "constant_common.h"
#include "code_editor.h"

namespace FTEDITOR {

int g_RamGlobalUsage = 0;
int g_FlashGlobalUsage = 0;

extern BT8XXEMU_Flash *g_Flash;

std::vector<QString> ContentManager::s_FileExtensions;
QMutex ContentManager::s_Mutex;

ContentInfo::ContentInfo(const QString &filePath)
{
	SourcePath = filePath;
	DestName = QFileInfo(filePath).completeBaseName();
	View = NULL;
	Converter = ContentInfo::Invalid;
	MemoryLoaded = false;
	MemoryAddress = 0;
	DataStorage = Embedded;
	DataCompressed = true;
	FlashAddress = FTEDITOR_FLASH_FIRMWARE_SIZE;
	RawStart = 0;
	RawLength = 0;
	ImageFormat = 0;
	ImageMono = false;
	FontSize = 12;
	FontCharSet = " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
	FontOffset = 32;
	MappedName = QString::null;
	UploadMemoryDirty = true;
	UploadFlashDirty = true;
	ExternalDirty = false;
	CachedImage = false;
	CachedMemorySize = 0;
	OverlapMemoryFlag = false;
	OverlapFlashFlag = false;
	WantAutoLoad = false;
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
		switch (DataStorage)
		{
		case File:
			j["dataStorage"] = QString("File");
			break;
		case Embedded:
			j["dataStorage"] = QString("Embedded");
			break;
		case Flash:
			j["dataStorage"] = QString("Flash");
			break;
		}
		if (Converter != FlashMap)
		{
			j["dataCompressed"] = DataCompressed;
		}
		if (DataStorage == Flash)
		{
			j["flashAddress"] = FlashAddress;
		}
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
	case ImageCoprocessor:
		j["converter"] = QString("ImageCoprocessor");
		j["imageMono"] = ImageMono;
		break;
	case Font:
		j["converter"] = QString("Font");
		j["imageFormat"] = ImageFormat;
		j["fontSize"] = FontSize;
		j["fontCharSet"] = FontCharSet;
		j["fontOffset"] = FontOffset;
		break;
	case FlashMap:
		j["converter"] = QString("FlashMap");
		j["mappedName"] = MappedName;
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
		if (j.contains("dataEmbedded"))
		{
			DataStorage = ((QJsonValue)j["dataEmbedded"]).toVariant().toBool()
				? Embedded : File;
		}
		if (j.contains("flashLoaded"))
		{
			// Development temporary
			DataStorage = ((QJsonValue)j["flashLoaded"]).toVariant().toBool()
				? Flash : DataStorage;
		}
		if (j.contains("dataStorage"))
		{
			QString dataStorage = j["dataStorage"].toString();
			if (dataStorage == "File") DataStorage = File;
			else if (dataStorage == "Embedded") DataStorage = Embedded;
			else if (dataStorage == "Flash") DataStorage = Flash;
		}
		if (j.contains("dataCompressed"))
		{
			DataCompressed = ((QJsonValue)j["dataCompressed"]).toVariant().toBool();
		}
		else
		{
			DataCompressed = false;
		}
		if (DataStorage == Flash)
		{
			FlashAddress = ((QJsonValue)j["flashAddress"]).toVariant().toInt();
		}
		else
		{
			FlashAddress = FTEDITOR_FLASH_FIRMWARE_SIZE;
		}
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
	else if (converter == "ImageCoprocessor")
	{
		Converter = ImageCoprocessor;
		ImageMono = ((QJsonValue)j["imageMono"]).toVariant().toBool();
	}
	else if (converter == "Font")
	{
		Converter = Font;
		ImageFormat = ((QJsonValue)j["imageFormat"]).toVariant().toInt();
		FontSize = ((QJsonValue)j["fontSize"]).toVariant().toInt();
		FontCharSet = j["fontCharSet"].toString();
		if (j.contains("fontOffset")) FontOffset = ((QJsonValue)j["fontOffset"]).toVariant().toInt();
		else FontOffset = 0;
	}
	else if (converter == "FlashMap")
	{
		Converter = FlashMap;
		MappedName = j["mappedName"].toString();
	}
	else
	{
		Converter = Invalid;
	}
}

bool ContentInfo::equalsMeta(const ContentInfo *other) const
{
	if (SourcePath != other->SourcePath)
		return false;
	if (Converter != other->Converter)
		return false;
	switch (Converter)
	{
	case Raw:
		if (RawStart != other->RawStart)
			return false;
		if (RawLength != other->RawLength)
			return false;
		break;
	case Image:
		if (ImageFormat != other->ImageFormat)
			return false;
		break;
	case ImageCoprocessor:
		if (ImageMono != other->ImageMono)
			return false;
		break;
	case Font:
		if (ImageFormat != other->ImageFormat)
			return false;
		if (FontSize != other->FontSize)
			return false;
		if (FontCharSet != other->FontCharSet)
			return false;
		if (FontOffset != other->FontOffset)
			return false;
		break;
	case FlashMap:
		if (MappedName != other->MappedName)
			return false;
		break;
	}
	return true;
}

int ContentInfo::bitmapAddress(int deviceIntf) const
{
	if (Converter == ContentInfo::Image)
	{
		switch (ImageFormat)
		{
			// Add palette into content
			case PALETTED8:
				return MemoryAddress + (256 * 4);
			case PALETTED565:
			case PALETTED4444:
				return MemoryAddress + (256 * 2);
			default:
				return MemoryAddress;
		}
	}
	if (Converter == ContentInfo::Font)
	{
		// Calculate bitmap address for font with offset
		int res = MemoryAddress + 148;
		int offset = FontOffset * CachedImageStride * CachedImageHeight;
		res -= offset;
		/*if (res < 0)
			res += addressMask(deviceIntf) + 1;*/
		return res;
	}
	return MemoryAddress;
}

QLabel *addLabeledWidget(QWidget *parent, QVBoxLayout *layout, const QString &label, QWidget *widget)
{
	QHBoxLayout *hbox = new QHBoxLayout();
	QLabel *l = new QLabel(parent);
	l->setText(label);
	hbox->addWidget(l);
	hbox->addWidget(widget);
	layout->addLayout(hbox);
	return l;
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

ContentManager::ContentManager(MainWindow *parent) : QWidget(parent), m_MainWindow(parent), m_CurrentPropertiesContent(NULL), m_OverlapSuppressed(0), m_OverlapMemorySuppressed(false),  m_OverlapFlashSuppressed(false)
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
		s_FileExtensions.push_back("_converted-fs8.png");
		s_FileExtensions.push_back(".meta");
	}

	QVBoxLayout *layout = new QVBoxLayout();

	m_ContentList = new QTreeWidget(this);
	m_ContentList->setDragEnabled(true);
	//m_ContentList->header()->close();
	m_ContentList->setColumnCount(2);
	QStringList headers;
	headers.push_back(tr("Status"));
	headers.push_back(tr("Name"));
	m_ContentList->setHeaderLabels(headers);
	layout->addWidget(m_ContentList);
	//m_ContentList->resizeColumnToContents(0);
	connect(m_ContentList, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem *)), this, SLOT(selectionChanged(QTreeWidgetItem *, QTreeWidgetItem *)));

	QHBoxLayout *buttonsLayout = new QHBoxLayout();

	//uint plusSign[2] = { 0x2795, 0 };
	QPushButton *addButton = new QPushButton();
	addButton->setText(/*QString::fromUcs4(plusSign) + " " +*/ tr("Add"));
	//addButton->setIcon(QIcon(":/icons/plus.png"));
	connect(addButton, SIGNAL(clicked()), this, SLOT(add()));
	buttonsLayout->addWidget(addButton);

	//uint minusSign[2] = { 0x2796, 0 };
	QPushButton *removeButton = new QPushButton();
	m_RemoveButton = removeButton;
	removeButton->setText(/*QString::fromUcs4(minusSign) + " " +*/ tr("Remove"));
	//removeButton->setIcon(QIcon(":/icons/minus.png"));
	connect(removeButton, SIGNAL(clicked()), this, SLOT(remove()));
	removeButton->setEnabled(false);
	buttonsLayout->addWidget(removeButton);

	layout->addLayout(buttonsLayout);

	QHBoxLayout *secondLayout = new QHBoxLayout();

	//uint refreshIcon[2] = { 0x27F2, 0 };
	QPushButton *rebuildButton = new QPushButton();
	rebuildButton->setText(/*QString::fromUcs4(refreshIcon) + " " +*/ tr("Rebuild All"));
	//rebuildButton->setIcon(QIcon(":/icons/arrow-circle-225-left.png"));
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
	propCommonConverter->addItem(tr("Font"));
	propCommonConverter->addItem(tr("Image Coprocessor"));
	m_PropertiesCommonConverterLabel = addLabeledWidget(this, propCommonLayout, tr("Converter: "), propCommonConverter);
	connect(m_PropertiesCommonConverter, SIGNAL(currentIndexChanged(int)), this, SLOT(propertiesCommonConverterChanged(int)));
	propCommon->setLayout(propCommonLayout);

	// Image
	m_PropertiesImage = new QGroupBox(this);
	m_PropertiesImage->setHidden(true);
	m_PropertiesImage->setTitle(tr("Image Settings"));
	QVBoxLayout *imagePropsLayout = new QVBoxLayout();
	m_PropertiesImageFormat = new QComboBox(this);
	
	addLabeledWidget(this, imagePropsLayout, tr("Format: "), m_PropertiesImageFormat);
	connect(m_PropertiesImageFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(propertiesImageFormatChanged(int)));
	m_PropertiesImage->setLayout(imagePropsLayout);

	// Image Coprocessor
	m_PropertiesImageCoprocessor = new QGroupBox(this);
	m_PropertiesImageCoprocessor->setHidden(true);
	m_PropertiesImageCoprocessor->setTitle(tr("Image Settings"));
	QVBoxLayout *imageCoprocessorPropsLayout = new QVBoxLayout();
	m_PropertiesImageMono = new QCheckBox(this);

	addLabeledWidget(this, imageCoprocessorPropsLayout, tr("Mono (Jpeg only): "), m_PropertiesImageMono);
	connect(m_PropertiesImageMono, SIGNAL(stateChanged(int)), this, SLOT(propertiesImageMonoChanged(int)));
	m_PropertiesImageCoprocessor->setLayout(imageCoprocessorPropsLayout);

	// Image Preview
	m_PropertiesImagePreview = new QGroupBox(this);
	QVBoxLayout *imagePreviewLayout = new QVBoxLayout();
	m_PropertiesImagePreview->setHidden(true);
	m_PropertiesImagePreview->setTitle(tr("Image Preview"));
	m_PropertiesImageLabel = new QLabel(this);
	imagePreviewLayout->addWidget(m_PropertiesImageLabel);
	m_PropertiesImagePreview->setLayout(imagePreviewLayout);

	// Font
	m_PropertiesFont = new QGroupBox(this);
	m_PropertiesFont->setHidden(true);
	m_PropertiesFont->setTitle(tr("Font"));
	QVBoxLayout *fontPropsLayout = new QVBoxLayout();
	m_PropertiesFontFormat = new QComboBox(this);
	
	addLabeledWidget(this, fontPropsLayout, tr("Format: "), m_PropertiesFontFormat);
	connect(m_PropertiesFontFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(propertiesFontFormatChanged(int)));
	m_PropertiesFontSize = new UndoStackDisabler<QSpinBox>(this);
	m_PropertiesFontSize->setMinimum(0);
	m_PropertiesFontSize->setMaximum(512);
	m_PropertiesFontSize->setSingleStep(1);
	addLabeledWidget(this, fontPropsLayout, tr("Size: "), m_PropertiesFontSize);
	connect(m_PropertiesFontSize, SIGNAL(valueChanged(int)), this, SLOT(propertiesFontSizeChanged(int)));
	m_PropertiesFontCharSet = new UndoStackDisabler<QLineEdit>(this);
	m_PropertiesFontCharSet->setUndoStack(m_MainWindow->undoStack());
	addLabeledWidget(this, fontPropsLayout, tr("Charset: "), m_PropertiesFontCharSet);
	connect(m_PropertiesFontCharSet, SIGNAL(editingFinished()), this, SLOT(propertiesFontCharSetChanged()));
	m_PropertiesFontOffset = new UndoStackDisabler<QSpinBox>(this);
	m_PropertiesFontOffset->setMinimum(0);
	m_PropertiesFontOffset->setMaximum(255);
	m_PropertiesFontOffset->setSingleStep(1);
	addLabeledWidget(this, fontPropsLayout, tr("First character: "), m_PropertiesFontOffset);
	connect(m_PropertiesFontOffset, SIGNAL(valueChanged(int)), this, SLOT(propertiesFontOffsetChanged(int)));
	m_PropertiesFont->setLayout(fontPropsLayout);

	// Raw
	m_PropertiesRaw = new QGroupBox(this);
	QVBoxLayout *rawLayout = new QVBoxLayout();
	m_PropertiesRaw->setHidden(true);
	m_PropertiesRaw->setTitle(tr("Raw"));
	m_PropertiesRawStart = new UndoStackDisabler<QSpinBox>(this);
	m_PropertiesRawStart->setMinimum(0);
	m_PropertiesRawStart->setMaximum(0x7FFFFFFF);
	m_PropertiesRawStart->setSingleStep(4);
	m_PropertiesRawStart->setKeyboardTracking(false);
	addLabeledWidget(this, rawLayout, tr("Start: "), m_PropertiesRawStart);
	connect(m_PropertiesRawStart, SIGNAL(valueChanged(int)), this, SLOT(propertiesRawStartChanged(int)));
	m_PropertiesRawLength = new UndoStackDisabler<QSpinBox>(this);
	m_PropertiesRawLength->setMinimum(0);
	m_PropertiesRawLength->setMaximum(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G_END));
	m_PropertiesRawLength->setSingleStep(4);
	m_PropertiesRawLength->setKeyboardTracking(false);
	addLabeledWidget(this, rawLayout, tr("Length: "), m_PropertiesRawLength);
	connect(m_PropertiesRawLength, SIGNAL(valueChanged(int)), this, SLOT(propertiesRawLengthChanged(int)));
	m_PropertiesRaw->setLayout(rawLayout);

	// Common/Memory
	m_PropertiesMemory = new QGroupBox(this);
	m_PropertiesMemory->setHidden(true);
	m_PropertiesMemory->setTitle(tr("Memory Options"));
	QVBoxLayout *propMemLayout = new QVBoxLayout();
	m_PropertiesMemoryAddress = new UndoStackDisabler<QSpinBox>(this);
	m_PropertiesMemoryAddress->setMinimum(0);
	m_PropertiesMemoryAddress->setMaximum(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G_END) - 4);
	m_PropertiesMemoryAddress->setSingleStep(4);
	m_PropertiesMemoryAddress->setKeyboardTracking(false);
	addLabeledWidget(this, propMemLayout, tr("Address: "), m_PropertiesMemoryAddress);
	connect(m_PropertiesMemoryAddress, SIGNAL(valueChanged(int)), this, SLOT(propertiesMemoryAddressChanged(int)));
	m_PropertiesMemoryLoaded = new QCheckBox(this);
	addLabeledWidget(this, propMemLayout, tr("Loaded: "), m_PropertiesMemoryLoaded);
	connect(m_PropertiesMemoryLoaded, SIGNAL(stateChanged(int)), this, SLOT(propertiesMemoryLoadedChanged(int)));
	// QFrame* line = new QFrame();
	// line->setFrameShape(QFrame::HLine);
	// line->setFrameShadow(QFrame::Sunken);
	// propMemLayout->addWidget(line);
	// TODO: New groupbox Storage Options: Compressed:, Embedded:, Flash:, Address:
	// m_PropertiesDataEmbedded = new QCheckBox(this);
	// addLabeledWidget(this, propMemLayout, tr("Embedded: "), m_PropertiesDataEmbedded);
	// connect(m_PropertiesDataEmbedded, SIGNAL(stateChanged(int)), this, SLOT(propertiesDataEmbeddedChanged(int)));
	m_PropertiesMemory->setLayout(propMemLayout);

	// Storage
	m_PropertiesData = new QGroupBox(this);
	m_PropertiesData->setHidden(true);
	m_PropertiesData->setTitle(tr("Storage Options"));
	QVBoxLayout *propDataLayout = new QVBoxLayout();
	QComboBox *propDataStorage = new QComboBox(this);
	m_PropertiesDataStorage = propDataStorage;
	propDataStorage->addItem(tr("File"));
	propDataStorage->addItem(tr("Embedded"));
	propDataStorage->addItem(tr("Flash"));
	addLabeledWidget(this, propDataLayout, tr("Storage: "), propDataStorage);
	connect(m_PropertiesDataStorage, SIGNAL(currentIndexChanged(int)), this, SLOT(propertiesDataStorageChanged(int)));
	m_PropertiesDataCompressed = new QCheckBox(this);
	addLabeledWidget(this, propDataLayout, tr("Compressed: "), m_PropertiesDataCompressed);
	connect(m_PropertiesDataCompressed, SIGNAL(stateChanged(int)), this, SLOT(propertiesDataCompressedChanged(int)));
	m_PropertiesFlashAddress = new UndoStackDisabler<QSpinBox>(this);
	m_PropertiesFlashAddress->setMinimum(FTEDITOR_FLASH_FIRMWARE_SIZE);
	m_PropertiesFlashAddress->setSingleStep(32);
	m_PropertiesFlashAddress->setKeyboardTracking(false);
	m_PropertiesFlashAddressLabel = addLabeledWidget(this, propDataLayout, tr("Flash Address: "), m_PropertiesFlashAddress);
	connect(m_PropertiesFlashAddress, SIGNAL(valueChanged(int)), this, SLOT(propertiesFlashAddressChanged(int)));
	m_PropertiesData->setLayout(propDataLayout);

	QVBoxLayout *helpLayout = new QVBoxLayout();
	m_HelpfulLabel = new QLabel(m_ContentList);
	m_HelpfulLabel->setWordWrap(true);
	m_HelpfulLabel->setText(tr("<i>No content has been added to the project yet.<br><br>Add new content to this project to automatically convert it to a hardware compatible format.</i>"));
	helpLayout->addWidget(m_HelpfulLabel);
	m_ContentList->setLayout(helpLayout);
	
	// bindCurrentDevice();
}

ContentManager::~ContentManager()
{
	// Cleanup
	for (QTreeWidgetItemIterator it(m_ContentList); *it; ++it)
	{
		ContentInfo *info = (ContentInfo *)(void *)(*it)->data(0, Qt::UserRole).value<quintptr>();
		delete info;
	}
}

void ContentManager::bindCurrentDevice()
{
	ContentInfo *info = current();
	m_ContentList->setCurrentItem(NULL);
	
	m_PropertiesImageFormat->clear();
	for (int i = 0; i < g_ImageFormatIntfNb[FTEDITOR_CURRENT_DEVICE]; ++i)
		m_PropertiesImageFormat->addItem(g_BitmapFormatToString[FTEDITOR_CURRENT_DEVICE][g_ImageFormatFromIntf[FTEDITOR_CURRENT_DEVICE][i]]);

	m_PropertiesFontFormat->clear();
	for (int i = 0; i < g_FontFormatIntfNb[FTEDITOR_CURRENT_DEVICE]; ++i)
		m_PropertiesFontFormat->addItem(g_BitmapFormatToString[FTEDITOR_CURRENT_DEVICE][g_FontFormatFromIntf[FTEDITOR_CURRENT_DEVICE][i]]);
	
	m_PropertiesRawLength->setMaximum(addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G_END));

	int memSize = addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G_END);
	if (m_PropertiesMemoryAddress->value() > memSize - 4)
		m_PropertiesMemoryAddress->setMaximum(m_PropertiesMemoryAddress->value());
	else
		m_PropertiesMemoryAddress->setMaximum(memSize - 4);

	if (g_Flash)
	{
		int size = (int)BT8XXEMU_Flash_size(g_Flash);
		if (m_PropertiesFlashAddress->value() < size - 32)
			m_PropertiesFlashAddress->setMaximum(size - 32);
		else
			m_PropertiesFlashAddress->setMaximum(m_PropertiesFlashAddress->value());
	}
	
	if (info)
		m_ContentList->setCurrentItem(info->View);

	recalculateOverlapMemoryInternal();
	recalculateOverlapFlashInternal();
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

	QString fileExt = QFileInfo(filePath).suffix().toLower();
	if (fileExt == "jpg") contentInfo->Converter = ContentInfo::Image;
	else if (fileExt == "png") contentInfo->Converter = ContentInfo::Image;
	else if (fileExt == "ttf") contentInfo->Converter = ContentInfo::Font;
	else if (fileExt == "otf") contentInfo->Converter = ContentInfo::Font;
	else if (fileExt == "pfa") contentInfo->Converter = ContentInfo::Font;
	else if (fileExt == "pfb") contentInfo->Converter = ContentInfo::Font;
	else if (fileExt == "cff") contentInfo->Converter = ContentInfo::Font;
	else if (fileExt == "pcf") contentInfo->Converter = ContentInfo::Font;
	else if (fileExt == "fnt") contentInfo->Converter = ContentInfo::Font;
	else if (fileExt == "bdf") contentInfo->Converter = ContentInfo::Font;
	else if (fileExt == "pfr") contentInfo->Converter = ContentInfo::Font;

	if (contentInfo->Converter == ContentInfo::Font)
	{
		if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810)
			contentInfo->ImageFormat = L2;
		else
			contentInfo->ImageFormat = L4;
	}

	if (contentInfo->Converter == ContentInfo::Invalid)
	{
		contentInfo->WantAutoLoad = true;
	}
	else
	{
		int freeAddress = getFreeMemoryAddress();
		if (freeAddress >= 0)
		{
			contentInfo->MemoryLoaded = true;
			contentInfo->MemoryAddress = freeAddress;
		}
	}

	switch (contentInfo->Converter)
	{
	case ContentInfo::ImageCoprocessor:
		contentInfo->DestName = "images/" + contentInfo->DestName;
		break;
	case ContentInfo::Image:
		contentInfo->DestName = "images/" + contentInfo->DestName;
		break;
	case ContentInfo::Font:
		contentInfo->DestName = "fonts/" + contentInfo->DestName;
		break;
	default:
		contentInfo->DestName = "content/" + contentInfo->DestName;
		break;
	}

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

	m_MainWindow->undoStack()->beginMacro(tr("Remove content"));

	Remove *remove = new Remove(this, contentInfo);
	m_MainWindow->undoStack()->push(remove);

	// ISSUE#113: Remove all related commands
	m_MainWindow->propertiesEditor()->surpressSet(true);
	editorRemoveContent(contentInfo, m_MainWindow->dlEditor());
	editorRemoveContent(contentInfo, m_MainWindow->cmdEditor());
	m_MainWindow->propertiesEditor()->surpressSet(false);
	
	m_MainWindow->undoStack()->endMacro();
}

int ContentManager::getFreeMemoryAddress()
{
	int freeAddress = 0;
	for (QTreeWidgetItemIterator it(m_ContentList); *it; ++it)
	{
		ContentInfo *info = (ContentInfo *)(void *)(*it)->data(0, Qt::UserRole).value<quintptr>();
		if (info->MemoryLoaded)
		{
			int size = getContentSize(info);
			if (size > 0)
			{
				int nextAddress = info->MemoryAddress + size;
				if (nextAddress > freeAddress)
					freeAddress = nextAddress;
			}
		}
	}
	if (freeAddress % 4)
	{
		freeAddress &= 0x7FFFFFFC;
		freeAddress += 4;
	}
	if (freeAddress > addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G_END))
		return -1;
	return freeAddress;
}

bool ContentManager::loadFlashMap(QString flashMapPath)
{
	printf("ContentManager::loadFlashMap(\"%s\")\n", flashMapPath.toLocal8Bit().data());

	bool success = false;
	m_MainWindow->undoStack()->beginMacro(tr("Load flash map"));
	suppressOverlapCheck();

	if (flashMapPath.isEmpty())
	{
		flashMapPath = findFlashMapPath();
	}

	if (!flashMapPath.isEmpty())
	{
		QFileInfo flashMapFileInfo = QFileInfo(flashMapPath);
		QString filePath = flashMapFileInfo.absolutePath() + "/" + flashMapFileInfo.completeBaseName() + ".bin";

		// Load flash map 
		// C:\sync_projects_work\ft800emu\bt815\paul\FULL.map
		const FlashMapInfo &flashMapInfo = AssetConverter::parseFlashMap(flashMapPath);
		AssetConverter::lockFlashMap(true);

		// Create list of existing content entries to be updated and removed
		std::vector<ContentInfo *> removeEntries;
		std::map<QString, ContentInfo *> updateEntries;
		QString absoluteMapPath = flashMapFileInfo.absoluteFilePath();
		for (QTreeWidgetItemIterator it(m_ContentList); *it; ++it)
		{
			ContentInfo *info = (ContentInfo *)(void *)(*it)->data(0, Qt::UserRole).value<quintptr>();
			if (info->Converter == ContentInfo::FlashMap)
			{
				QString otherAbsoluteMapPath = QFileInfo(info->SourcePath).absoluteFilePath();
				if ((otherAbsoluteMapPath != absoluteMapPath)
					|| (flashMapInfo.find(info->MappedName) == flashMapInfo.end()))
				{
					removeEntries.push_back(info);
				}
				else
				{
					updateEntries[info->MappedName] = info;
				}
			}
		}

		// Add or update from the flash map
		for (FlashMapInfo::const_iterator it(flashMapInfo.begin()), end(flashMapInfo.end()); it != end; ++it)
		{
			std::map<QString, ContentInfo *>::iterator update = updateEntries.find(it->second.Name);
			if (update != updateEntries.end())
			{
				// Reprocess existing content info
				ContentInfo *contentInfo = update->second;
				if (contentInfo->FlashAddress != it->second.Index)
					changeFlashAddress(contentInfo, it->second.Index);
				reprocessInternal(contentInfo);
				success = true;
			}
			else
			{
				// Add new content info
				ContentInfo *contentInfo = new ContentInfo(flashMapPath);
				contentInfo->DestName = it->second.Name;
				contentInfo->Converter = ContentInfo::FlashMap;
				contentInfo->MappedName = it->second.Name;
				contentInfo->DataCompressed = false;
				contentInfo->FlashAddress = it->second.Index;
				contentInfo->DataStorage = ContentInfo::Flash;
				add(contentInfo);
				success = true;
			}
		}

		// Remove previous content
		if (success)
		{
			for (std::vector<ContentInfo *>::iterator it(removeEntries.begin()), end(removeEntries.end()); it != end; ++it)
			{
				// Remove it
				remove(*it);
			}
		}

		AssetConverter::lockFlashMap(false);
	}

	resumeOverlapCheck();
	m_MainWindow->undoStack()->endMacro();

	return success;
}

void ContentManager::addInternal(ContentInfo *contentInfo)
{
	printf("ContentManager::addInternal(contentInfo)\n");

	// Ensure no duplicate names are used
	contentInfo->DestName = createName(contentInfo->DestName);

	// Add to the content list
	QTreeWidgetItem *view = new QTreeWidgetItem(m_ContentList);
	contentInfo->View = view;
	view->setData(0, Qt::UserRole, qVariantFromValue((quintptr)(void *)contentInfo));
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
	contentInfo->UploadMemoryDirty = true;
	contentInfo->UploadFlashDirty = true;

	// Make sure not in content
	lockContent();
	if (m_ContentUploadMemoryDirty.find(contentInfo) != m_ContentUploadMemoryDirty.end())
		m_ContentUploadMemoryDirty.erase(contentInfo);
	if (m_ContentUploadFlashDirty.find(contentInfo) != m_ContentUploadFlashDirty.end())
		m_ContentUploadFlashDirty.erase(contentInfo);
	unlockContent();

	// Recalculate overlap in memory
	if (m_ContentOverlapMemory.find(contentInfo) != m_ContentOverlapMemory.end())
		m_ContentOverlapMemory.erase(contentInfo);
	contentInfo->OverlapMemoryFlag = false;
	recalculateOverlapMemoryInternal();

	// Recalculate overlap in flash
	if (m_ContentOverlapFlash.find(contentInfo) != m_ContentOverlapFlash.end())
		m_ContentOverlapFlash.erase(contentInfo);
	contentInfo->OverlapFlashFlag = false;
	recalculateOverlapFlashInternal();

	// Reload external dependencies
	reloadExternal(contentInfo);

	// Be helpful
	m_HelpfulLabel->setVisible(getContentCount() == 0);
}

bool ContentManager::nameExists(const QString &name)
{
	for (QTreeWidgetItemIterator it(m_ContentList); *it; ++it)
	{
		ContentInfo *info = (ContentInfo *)(void *)(*it)->data(0, Qt::UserRole).value<quintptr>();
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

	if (fileName.isEmpty())
		return;

	ContentInfo *info = add(fileName);
}

void ContentManager::remove()
{
	printf("ContentManager::remove()\n");

	if (!m_ContentList->currentItem())
		return;

	ContentInfo *info = (ContentInfo *)(void *)m_ContentList->currentItem()->data(0, Qt::UserRole).value<quintptr>();
	remove(info);
}

void ContentManager::rebuildAll()
{
	// Reprocess all content if necessary
	for (QTreeWidgetItemIterator it(m_ContentList); *it; ++it)
	{
		ContentInfo *info = (ContentInfo *)(void *)(*it)->data(0, Qt::UserRole).value<quintptr>();
		reprocessInternal(info);
	}
}

void ContentManager::copyFlashFile()
{
	if (m_MainWindow->isProjectSaved())
	{
		QDir projectFolder(m_MainWindow->getProjectSavedFolder());
		projectFolder.mkpath("flash");
		projectFolder.cd("flash");
		QString projectFlashFolder = projectFolder.absolutePath() + "/";

		// copy file .map
		QFile::copy(m_FlashFileName, projectFlashFolder + QFileInfo(m_FlashFileName).fileName());

		// copy file .bin
		QString binFileName(m_FlashFileName);
		binFileName.replace(m_FlashFileName.length() - 3, 3, "bin");
		QFile::copy(binFileName, projectFlashFolder + QFileInfo(binFileName).fileName());

		// flash file point to new folder
		m_FlashFileName = projectFlashFolder + QFileInfo(m_FlashFileName).fileName();
	}
	else
	{
		m_MainWindow->requestSave();
		copyFlashFile();
	}
}

void ContentManager::importFlashMapped()
{
	printf("ContentManager::importFlashMapped()\n");

	QString fileName = QFileDialog::getOpenFileName(this,
		tr("Import Mapped Flash Image"),
		m_MainWindow->getFileDialogPath(),
		tr("Flash image map (*.map)"));

	if (fileName.isEmpty())
		return;

	m_FlashFileName = fileName;

	// check flash size
	if (false == m_MainWindow->checkAndPromptFlashPath(m_FlashFileName))
	{
		return;
	}

	// ask if user wanna move flash files
	int answer = QMessageBox::question(this, tr("Copy flash files"), tr("Do you want to copy flash files to project folder?"), QMessageBox::Yes, QMessageBox::No);
	if (QMessageBox::Yes == answer)
	{
		copyFlashFile();
	}

	// set flash file name to GUI
	m_MainWindow->setFlashFileNameToLabel(m_FlashFileName);

	QString relativeFlashMapPath = QDir(QDir::currentPath()).relativeFilePath(m_FlashFileName);
	if (!loadFlashMap(relativeFlashMapPath))
	{
		QMessageBox::critical(this, tr("Import Mapped Flash Image"), tr("Unable to import mapped flash image"));
	}
	else if (QMessageBox::Yes == answer)
	{
		m_MainWindow->requestSave();
	}
}

void ContentManager::exportFlashMapped()
{

}

void ContentManager::getContentInfos(std::vector<ContentInfo *> &contentInfos)
{
	// Iterate through the list and copy the pointer to the data
	for (QTreeWidgetItemIterator it(m_ContentList); *it; ++it)
	{
		ContentInfo *info = (ContentInfo *)(void *)(*it)->data(0, Qt::UserRole).value<quintptr>();
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
		if (info == (ContentInfo *)(void *)(*it)->data(0, Qt::UserRole).value<quintptr>())
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
		ContentInfo *info = (ContentInfo *)(void *)(*it)->data(0, Qt::UserRole).value<quintptr>();
		if (info->DestName == destName)
			return info;
	}

	printf("Content '%s' not found\n", destName.toLocal8Bit().data());
	return NULL;
}

void ContentManager::suppressOverlapCheck()
{
	++m_OverlapSuppressed;
}

void ContentManager::resumeOverlapCheck()
{
	--m_OverlapSuppressed;
	if (!m_OverlapSuppressed)
	{
		if (m_OverlapFlashSuppressed)
		{
			recalculateOverlapFlashInternal();
			m_OverlapFlashSuppressed = false;
		}
		if (m_OverlapMemorySuppressed)
		{
			recalculateOverlapMemoryInternal();
			m_OverlapMemorySuppressed = false;
		}
	}
}

bool ContentManager::cacheImageInfo(ContentInfo *info)
{
	if (info->CachedImage)
		return true;
	ImageInfo bitmapInfo;
	if (!AssetConverter::getImageInfo(bitmapInfo, info->DestName))
		return false;
	if (info->Converter == ContentInfo::ImageCoprocessor)
		info->ImageFormat = bitmapInfo.LayoutFormat;
	info->CachedImageWidth = bitmapInfo.LayoutWidth;
	info->CachedImageHeight = bitmapInfo.LayoutHeight;
	info->CachedImageStride = bitmapInfo.LayoutStride;
	info->CachedImage = true;
	return true;
}

int ContentManager::getContentSize(ContentInfo *contentInfo)
{
	/* REMOVE
	if (contentInfo->Converter == ContentInfo::FlashMap)
	{
		// In future, if useful, may want special handling for a few file extensions that can load through the coprocessor (jpg, jpeg, png, bin vs raw)
		return getFlashSize(contentInfo);
	}*/

	if (contentInfo->Converter == ContentInfo::ImageCoprocessor)
	{
		ImageInfo bitmapInfo;
		if (!AssetConverter::getImageInfo(bitmapInfo, contentInfo->DestName))
			return -1;
		return bitmapInfo.LayoutHeight * bitmapInfo.LayoutStride;
	}

	QString fileName = contentInfo->DestName + ".raw";
	QFileInfo binFile(fileName);
	if (!binFile.exists()) return -1;
	int contentSize = binFile.size();
	if (contentInfo->Converter == ContentInfo::Image || contentInfo->Converter == ContentInfo::Font)
	{
		switch (contentInfo->ImageFormat)
		{
			// Add palette into content
		case PALETTED8:
			contentSize += 256 * 4;
			break;
		case PALETTED565:
		case PALETTED4444:
			contentSize += 256 * 2;
			break;
		}
	}
	return contentSize;
}

int ContentManager::getFlashSize(ContentInfo *contentInfo)
{
	// TODO: Maybe cache getFlashSize
	int size;
	if (contentInfo->DataCompressed)
	{
		QString fileName = contentInfo->DestName + ".bin";
		QFileInfo binFile(fileName);
		if (!binFile.exists()) return -1;
		size = binFile.size();
	}
	else
	{
		QString fileName = contentInfo->DestName + ".raw";
		QFileInfo binFile(fileName);
		if (!binFile.exists()) return -1;
		size = binFile.size();
	}
	/*
	if (contentInfo->Converter == ContentInfo::Image || contentInfo->Converter == ContentInfo::Font)
	{
		switch (contentInfo->ImageFormat)
		{
			// Add palette into content
		case PALETTED8:
			contentSize += 256 * 4;
			break;
		case PALETTED565:
		case PALETTED4444:
			contentSize += 256 * 2;
			break;
		}
	}
	*/
	return size;
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
		rebuildGUIInternal((ContentInfo *)(void *)current->data(0, Qt::UserRole).value<quintptr>());
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
	QString text;
	QIcon icon;

	if (!contentInfo->BuildError.isEmpty())
	{
		text = "Error";
		icon = QIcon(":/icons/exclamation-red.png");
	}
	else
	{
		if ((contentInfo->MemoryLoaded && contentInfo->OverlapMemoryFlag)
			|| ((contentInfo->DataStorage == ContentInfo::Flash) && contentInfo->OverlapFlashFlag))
		{
			if (contentInfo->OverlapFlashFlag)
			{
				int globalUsage = 0;
				size_t globalSize = g_Flash ? BT8XXEMU_Flash_size(g_Flash) : 0;
				if (contentInfo->FlashAddress + getFlashSize(contentInfo) > globalSize) // TODO: Maybe cache getFlashSize
				{
					text = "No Space";
				}
				else
				{
					text = "Overlap";
				}
			}
			else
			{
				text = "Overlap";
			}
			icon = QIcon(":/icons/exclamation-red.png");
		}
		else if (contentInfo->MemoryLoaded)
		{
			text = "Loaded";
			icon = QIcon(":/icons/tick");
		}
		else if ((contentInfo->DataStorage == ContentInfo::Flash))
		{
			text = "Flash";
			icon = QIcon(":/icons/tick");
		}
	}

	// contentInfo->View->setText(0, contentInfo->SourcePath);
	contentInfo->View->setText(0, text);
	contentInfo->View->setIcon(0, icon);
	contentInfo->View->setText(1, contentInfo->DestName);

	m_ContentList->resizeColumnToContents(1);
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
		widgets.push_back(m_PropertiesImageCoprocessor);
		widgets.push_back(m_PropertiesImagePreview);
		widgets.push_back(m_PropertiesRaw);
		widgets.push_back(m_PropertiesFont);
		widgets.push_back(m_PropertiesMemory);
		widgets.push_back(m_PropertiesData);

		props->setEditWidgets(widgets, false, this);
	}

	// Set common widget values
	m_PropertiesCommonSourceFile->setText(contentInfo->SourcePath);
	m_PropertiesCommonName->setText(contentInfo->DestName);
	if (contentInfo->Converter < ContentInfo::FlashMap)
	{
		m_PropertiesCommonConverter->setCurrentIndex((int)contentInfo->Converter);
	}
	switch (contentInfo->Converter)
	{
	case ContentInfo::Image:
		m_PropertiesImageFormat->setCurrentIndex(g_ImageFormatToIntf[FTEDITOR_CURRENT_DEVICE][contentInfo->ImageFormat % g_BitmapFormatEnumNb[FTEDITOR_CURRENT_DEVICE]]);
		break;
	case ContentInfo::Raw:
		m_PropertiesRawStart->setValue(contentInfo->RawStart);
		m_PropertiesRawLength->setValue(contentInfo->RawLength);
		break;
	case ContentInfo::Font:
		m_PropertiesFontFormat->setCurrentIndex(g_FontFormatToIntf[FTEDITOR_CURRENT_DEVICE][contentInfo->ImageFormat % g_BitmapFormatEnumNb[FTEDITOR_CURRENT_DEVICE]]);
		m_PropertiesFontSize->setValue(contentInfo->FontSize);
		m_PropertiesFontCharSet->setText(contentInfo->FontCharSet); // NOTE: Number of characters up to 128; export depends on number of characters in charset also!
		m_PropertiesFontOffset->setValue(contentInfo->FontOffset);
		break;
	case ContentInfo::ImageCoprocessor:
		m_PropertiesImageMono->setCheckState(contentInfo->ImageMono ? Qt::Checked : Qt::Unchecked);
		break;
	}
	if (contentInfo->Converter != ContentInfo::Invalid)
	{
		m_PropertiesMemoryLoaded->setCheckState(contentInfo->MemoryLoaded ? Qt::Checked : Qt::Unchecked);
		int memSize = addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G_END);
		bool mb = m_PropertiesMemoryAddress->blockSignals(true);
		if (contentInfo->MemoryAddress > memSize - 4)
			m_PropertiesMemoryAddress->setMaximum(contentInfo->MemoryAddress);
		else
			m_PropertiesMemoryAddress->setMaximum(memSize - 4);
		m_PropertiesMemoryAddress->blockSignals(mb);
		m_PropertiesMemoryAddress->setValue(contentInfo->MemoryAddress);
		m_PropertiesDataStorage->setCurrentIndex((int)contentInfo->DataStorage);
		m_PropertiesDataCompressed->setCheckState(contentInfo->DataCompressed ? Qt::Checked : Qt::Unchecked);
		if (g_Flash)
		{
			int flashSize = (int)BT8XXEMU_Flash_size(g_Flash);
			bool b = m_PropertiesFlashAddress->blockSignals(true);
			if (contentInfo->FlashAddress < FTEDITOR_FLASH_FIRMWARE_SIZE)
				m_PropertiesFlashAddress->setMinimum(contentInfo->FlashAddress);
			else
				m_PropertiesFlashAddress->setMinimum(FTEDITOR_FLASH_FIRMWARE_SIZE);
			if (contentInfo->FlashAddress > flashSize - 32)
				m_PropertiesFlashAddress->setMaximum(contentInfo->FlashAddress);
			else
				m_PropertiesFlashAddress->setMaximum(flashSize - 32);
			m_PropertiesFlashAddress->blockSignals(b);
			m_PropertiesFlashAddress->setValue(contentInfo->FlashAddress);
			m_PropertiesFlashAddress->setEnabled(true);
		}
		else
		{
			m_PropertiesFlashAddress->setEnabled(false);
		}
	}

	bool flashMapOnly = (contentInfo->Converter == ContentInfo::FlashMap);
	m_PropertiesCommonConverter->setHidden(flashMapOnly);
	m_PropertiesCommonConverterLabel->setHidden(flashMapOnly);

	// Set user help, wizard format
	if (contentInfo->Converter == ContentInfo::Invalid)
	{
		m_PropertiesImage->setHidden(true);
		m_PropertiesImageCoprocessor->setHidden(true);
		m_PropertiesImagePreview->setHidden(true);
		m_PropertiesRaw->setHidden(true);
		m_PropertiesFont->setHidden(true);
		m_PropertiesMemory->setHidden(true);
		m_PropertiesData->setHidden(true);
		props->setInfo(tr("Select a <b>Converter</b> to be used for this file. Converted files will be stored in the folder where the project is saved.<br><br><b>Image</b>: Converts an image to one of the supported formats.<br><b>Raw</b>: Does a direct binary copy.<br><b>Raw JPEG</b>: Does a raw binary copy and decodes the JPEG on the coprocessor."));
	}
	else
	{
		QString propInfo;
		if (!contentInfo->BuildError.isEmpty())
		{
			propInfo = tr("<b>Error</b>: ") + contentInfo->BuildError;
		}
		m_PropertiesDataStorage->setEnabled(!flashMapOnly);
		m_PropertiesDataCompressed->setDisabled((contentInfo->Converter == ContentInfo::ImageCoprocessor) || flashMapOnly);
		bool flashStorage = (contentInfo->DataStorage == ContentInfo::Flash);
		m_PropertiesFlashAddress->setVisible(flashStorage);
		m_PropertiesFlashAddressLabel->setVisible(flashStorage);
		switch (contentInfo->Converter)
		{
			case ContentInfo::Image:
			{
				m_PropertiesImage->setHidden(false);
				QPixmap pixmap;
				bool loadSuccess = pixmap.load(contentInfo->DestName + "_converted-fs8.png") ||
					pixmap.load(contentInfo->DestName + "_converted.png");
				m_PropertiesImagePreview->setHidden(!loadSuccess);
				m_PropertiesImageLabel->setPixmap(pixmap.scaled(m_PropertiesImageLabel->width() - 32, m_PropertiesImageLabel->width() - 32, Qt::KeepAspectRatio));
				if (loadSuccess) m_PropertiesImageLabel->repaint();
				else { if (!propInfo.isEmpty()) propInfo += "<br>"; propInfo += tr("<b>Error</b>: Failed to load image preview."); }
				m_PropertiesImageCoprocessor->setHidden(true);
				m_PropertiesRaw->setHidden(true);
				m_PropertiesFont->setHidden(true);
				m_PropertiesMemory->setHidden(false);
				m_PropertiesData->setHidden(false);
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
				m_PropertiesImageCoprocessor->setHidden(true);
				m_PropertiesImagePreview->setHidden(true);
				m_PropertiesRaw->setHidden(false);
				m_PropertiesFont->setHidden(true);
				m_PropertiesMemory->setHidden(false);
				m_PropertiesData->setHidden(false);
				if (contentInfo->BuildError.isEmpty())
				{
					if (!propInfo.isEmpty()) propInfo += "<br>";
					QFileInfo rawInfo(contentInfo->DestName + ".raw");
					QFileInfo binInfo(contentInfo->DestName + ".bin");
					propInfo += tr("<b>Size: </b> ") + QString::number(rawInfo.size()) + " bytes";
					propInfo += tr("<br><b>Compressed: </b> ") + QString::number(binInfo.size()) + " bytes";
					goto RawDetect;
				}
				break;
			}
			RawDetect:
			{
				QFile rawFile(contentInfo->DestName + ".raw");
				if (rawFile.open(QIODevice::ReadOnly))
				{
					QDataStream in(&rawFile);
					in.setByteOrder(QDataStream::LittleEndian);
					quint32 signature;
					in >> signature;
					if (signature == 0xAAAA0100)
					{
						if (!propInfo.isEmpty()) propInfo += "<br>";
						propInfo += tr("<b>Signature: </b> ANIM_SIGNATURE");
						quint32 num_frames;
						in >> num_frames;
						propInfo += tr("<br><b>Number of Frames: </b> ") + QString::number(num_frames);
						for (quint32 i = 0; i < num_frames && i < 256; ++i)
						{
							quint32 nbytes, ptr;
							in >> nbytes;
							in >> ptr;
							propInfo += QString("<br><b>") + QString::number(i) + " </b> (0x" + QString::number(ptr, 16).rightJustified(8, '0').toLower() + "): " + QString::number(nbytes) + " bytes";
							if ((nbytes & (4 - 1)) || ptr & (64 - 1)) propInfo += tr(" (invalid)");
						}
						if (num_frames >= 256)
						{
							propInfo += tr("<br>...");
						}
					}
					rawFile.close();
				}
				break;
			}
			case ContentInfo::ImageCoprocessor:
			{
				m_PropertiesImage->setHidden(true);
				m_PropertiesImageCoprocessor->setHidden(false);
				QPixmap pixmap;
				bool loadSuccess = pixmap.load(contentInfo->DestName + ".raw");
				m_PropertiesImagePreview->setHidden(!loadSuccess);
				m_PropertiesImageLabel->setPixmap(pixmap.scaled(m_PropertiesImageLabel->width() - 32, m_PropertiesImageLabel->width() - 32, Qt::KeepAspectRatio));
				if (loadSuccess) m_PropertiesImageLabel->repaint();
				else { if (!propInfo.isEmpty()) propInfo += "<br>"; propInfo += tr("<b>Error</b>: Failed to load image preview."); }
				m_PropertiesRaw->setHidden(true);
				m_PropertiesFont->setHidden(true);
				m_PropertiesMemory->setHidden(false);
				m_PropertiesData->setHidden(false);
				if (contentInfo->BuildError.isEmpty())
				{
					if (cacheImageInfo(contentInfo))
					{
						propInfo += tr("<br><b>Format: </b> ") + bitmapFormatToString(FTEDITOR_CURRENT_DEVICE, contentInfo->ImageFormat);
						propInfo += tr("<br><b>Width: </b> ") + QString::number(contentInfo->CachedImageWidth);
						propInfo += tr("<br><b>Height: </b> ") + QString::number(contentInfo->CachedImageHeight);
						propInfo += tr("<br><b>Stride: </b> ") + QString::number(contentInfo->CachedImageStride);
					}
				}
				break;
			}
			case ContentInfo::Font:
			{
				m_PropertiesImage->setHidden(true);
				m_PropertiesImagePreview->setHidden(true);
				m_PropertiesImageCoprocessor->setHidden(true);
				m_PropertiesRaw->setHidden(true);
				m_PropertiesFont->setHidden(false);
				m_PropertiesMemory->setHidden(false);
				m_PropertiesData->setHidden(false);
				if (contentInfo->BuildError.isEmpty())
				{
					if (!propInfo.isEmpty()) propInfo += "<br>";
					QFileInfo rawInfo(contentInfo->DestName + ".raw");
					QFileInfo binInfo(contentInfo->DestName + ".bin");
					propInfo += tr("<b>Size: </b> ") + QString::number(rawInfo.size()) + " bytes";
					propInfo += tr("<br><b>Compressed: </b> ") + QString::number(binInfo.size()) + " bytes";
					if (cacheImageInfo(contentInfo))
					{
						propInfo += tr("<br><b>Width: </b> ") + QString::number(contentInfo->CachedImageWidth);
						propInfo += tr("<br><b>Height: </b> ") + QString::number(contentInfo->CachedImageHeight);
						propInfo += tr("<br><b>Stride: </b> ") + QString::number(contentInfo->CachedImageStride);
					}
				}
				break;
			}
			case ContentInfo::FlashMap:
			{
				m_PropertiesImage->setHidden(true);
				m_PropertiesImagePreview->setHidden(true);
				m_PropertiesImageCoprocessor->setHidden(true);
				m_PropertiesRaw->setHidden(true);
				m_PropertiesFont->setHidden(true);
				m_PropertiesMemory->setHidden(false);
				m_PropertiesData->setHidden(false);
				if (!propInfo.isEmpty()) propInfo += "<br>";
				propInfo += tr("<b>Mapped Name: </b> ") + contentInfo->MappedName;
				if (contentInfo->BuildError.isEmpty())
				{
					QFileInfo rawInfo(contentInfo->DestName + ".raw");
					if (rawInfo.exists()) propInfo += tr("<br><b>Size: </b> ") + QString::number(rawInfo.size()) + " bytes";
					goto RawDetect;
				}
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
			// goto SkipRebuild; // Implicit // Error, skip rebuild
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
						goto IgnoreMeta; // Ignore meta comparison and rebuild
					}
					else if (contentInfo->Converter == ContentInfo::FlashMap)
					{
						QString srcBinFile = srcInfo.absolutePath() + "/" + srcInfo.completeBaseName() + ".bin";
						bool srcBinExists = QFile::exists(srcBinFile);
						if (!srcBinExists)
						{
							contentInfo->BuildError = "<i>(Filesystem)</i><br>Flash binary file does not exist.";
							contentInfo->ExternalDirty = true;
							goto SkipRebuild; // Error, skip rebuild
						}
						else
						{
							QFileInfo srcBinInfo(srcBinFile);
							if (srcBinInfo.lastModified() > metaInfo.lastModified())
							{
								// FIXME: Perhaps better to store the srcInfo.lastModified() in the meta and compare if equal in case of swapping with older files
								printf("Flash binary file has been modified, ignore meta, rebuild\n");
								goto IgnoreMeta; // Ignore meta comparison and rebuild
							}
						}
					}
					// else goto CompareMeta; // Implicit // Ignore meta comparison and rebuild
				// CompareMeta:
					; {
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
				IgnoreMeta:;
				}
			}
			if (equalMeta)
			{
				printf("Equal meta, skip convert\n");
				
				// Cache size
				if (!contentInfo->CachedMemorySize)
				{
					contentInfo->CachedMemorySize = getContentSize(contentInfo);
				}
				// TODO: Maybe cache flash size
			}
			else
			{
				if (metaExists)
				{
					QFile::remove(metaFile); // *** FILE REMOVE ***
				}
				contentInfo->CachedImage = false;
				contentInfo->CachedMemorySize = 0;
				// TODO: Maybe cache flash size
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
					case ContentInfo::ImageCoprocessor:
						AssetConverter::convertImageCoprocessor(contentInfo->BuildError, contentInfo->SourcePath, contentInfo->DestName, contentInfo->ImageMono, true, FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810);
						break;
					case ContentInfo::Font:
						AssetConverter::convertFont(contentInfo->BuildError, contentInfo->SourcePath, contentInfo->DestName, contentInfo->ImageFormat, contentInfo->FontSize, contentInfo->FontCharSet, contentInfo->FontOffset);
						break;
					case ContentInfo::FlashMap:
						AssetConverter::convertFlashMap(contentInfo->BuildError, contentInfo->SourcePath, contentInfo->DestName, contentInfo->MappedName);
						break;
					default:
						contentInfo->BuildError = "<i>(Critical Error)</i><br>Unknown converter selected.";
						break;
					}
				}
				if (contentInfo->BuildError.isEmpty())
				{
					// Cache size
					contentInfo->CachedMemorySize = getContentSize(contentInfo);
					// TODO: Maybe cache flash size

					// Write meta file
					QFile file(metaFile);
					file.open(QIODevice::WriteOnly);
					QDataStream out(&file);
					QJsonDocument doc(contentInfo->toJson(true));
					QByteArray data = doc.toJson();
					out.writeRawData(data, data.size());
					contentInfo->UploadMemoryDirty = true;
					contentInfo->UploadFlashDirty = true;
				}
			}
		}
	SkipRebuild:
		unlockContent();
	}
	else
	{
		// Always refresh external if converter deselected...
		contentInfo->ExternalDirty = true;
	}

	// Reupload the content to emulator RAM and/or flash if dirty
	if (contentInfo->UploadMemoryDirty || contentInfo->UploadFlashDirty)
	{
		bool memory = contentInfo->UploadMemoryDirty;
		bool flash = contentInfo->UploadFlashDirty;
		contentInfo->UploadMemoryDirty = false;
		contentInfo->UploadFlashDirty = false;
		reuploadInternal(contentInfo, memory, flash);
		if (memory) recalculateOverlapMemoryInternal();
		if (flash) recalculateOverlapFlashInternal();
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

void ContentManager::reuploadInternal(ContentInfo *contentInfo, bool memory, bool flash)
{
	printf("ContentManager::reuploadInternal(contentInfo, memory, flash)\n");

	// Reupload the content to emulator RAM or flash
	// This happens in the emulator main loop
	// Emulator main loop will lock the content mutex
	if (contentInfo->Converter != ContentInfo::Invalid && ((contentInfo->MemoryLoaded && memory) || ((contentInfo->DataStorage == ContentInfo::Flash) && flash)))
	{
		if (contentInfo->Converter == ContentInfo::Image
			|| contentInfo->Converter == ContentInfo::Font)
		{
			// Bitmap setup is always updated after upload and requires cached image info
			cacheImageInfo(contentInfo);
		}

		lockContent();
		if (memory)
		{
			if (contentInfo->MemoryLoaded)
			{
				if (m_ContentUploadMemoryDirty.find(contentInfo) == m_ContentUploadMemoryDirty.end())
					m_ContentUploadMemoryDirty.insert(contentInfo);
			}
			else
			{
				if (m_ContentUploadMemoryDirty.find(contentInfo) != m_ContentUploadMemoryDirty.end())
					m_ContentUploadMemoryDirty.erase(contentInfo);
			}
		}
		if (flash)
		{
			if ((contentInfo->DataStorage == ContentInfo::Flash))
			{
				if (m_ContentUploadFlashDirty.find(contentInfo) == m_ContentUploadFlashDirty.end())
					m_ContentUploadFlashDirty.insert(contentInfo);
			}
			else
			{
				if (m_ContentUploadFlashDirty.find(contentInfo) != m_ContentUploadFlashDirty.end())
					m_ContentUploadFlashDirty.erase(contentInfo);
			}
		}
		unlockContent();
	}
}

void ContentManager::reuploadAll()
{
	printf("ContentManager::reuploadAll()\n");

	for (QTreeWidgetItemIterator it(m_ContentList); *it; ++it)
	{
		ContentInfo *info = (ContentInfo *)(void *)(*it)->data(0, Qt::UserRole).value<quintptr>();
		reuploadInternal(info, true, true);
	}
}

void ContentManager::recalculateOverlapMemoryInternal()
{
	printf("ContentManager::recalculateOverlapMemoryInternal()\n");

	if (m_OverlapSuppressed)
	{
		m_OverlapMemorySuppressed = true;
		return;
	}

	std::set<ContentInfo *> contentOverlap;
	m_ContentOverlapMemory.swap(contentOverlap);

	int globalUsage = 0;
	int globalSize = addr(FTEDITOR_CURRENT_DEVICE, FTEDITOR_RAM_G_END);

	for (QTreeWidgetItemIterator left(m_ContentList); *left; )
	{
		ContentInfo *leftInfo = (ContentInfo *)(void *)(*left)->data(0, Qt::UserRole).value<quintptr>();
		if (leftInfo->Converter != ContentInfo::Invalid && leftInfo->MemoryLoaded)
		{
			int leftSize = getContentSize(leftInfo);
			// printf("leftSize: %i\n", leftSize);
			if (leftSize >= 0)
			{
				int leftAddr = leftInfo->MemoryAddress;
				if (leftAddr + leftSize > globalSize)
				{
					printf("CM: Content '%s' oversize\n", leftInfo->DestName.toLocal8Bit().data());
					if (m_ContentOverlapMemory.find(leftInfo) == m_ContentOverlapMemory.end())
						m_ContentOverlapMemory.insert(leftInfo);
					++left;
				}
				else
				{
					globalUsage += leftSize;
					for (QTreeWidgetItemIterator right(++left); *right; ++right)
					{
						ContentInfo *rightInfo = (ContentInfo *)(void *)(*right)->data(0, Qt::UserRole).value<quintptr>();
						if (rightInfo->Converter != ContentInfo::Invalid && rightInfo->MemoryLoaded)
						{
							int rightSize = getContentSize(rightInfo);
							if (rightSize >= 0)
							{
								int rightAddr = rightInfo->MemoryAddress;

								if  (leftAddr < rightAddr)
								{
									if (leftAddr + leftSize > rightAddr)
									{
										// overlap
										printf("CM: Content '%s' overlap with '%s'\n", leftInfo->DestName.toLocal8Bit().data(), rightInfo->DestName.toLocal8Bit().data());
										if (m_ContentOverlapMemory.find(leftInfo) == m_ContentOverlapMemory.end())
											m_ContentOverlapMemory.insert(leftInfo);
										if (m_ContentOverlapMemory.find(rightInfo) == m_ContentOverlapMemory.end())
											m_ContentOverlapMemory.insert(rightInfo);
										globalUsage -= (leftAddr + leftSize - rightAddr);
									}
								}
								else if (rightAddr < leftAddr)
								{
									if (rightAddr + rightSize > leftAddr)
									{
										// overlap
										printf("CM: Content '%s' overlap with '%s'\n", leftInfo->DestName.toLocal8Bit().data(), rightInfo->DestName.toLocal8Bit().data());
										if (m_ContentOverlapMemory.find(leftInfo) == m_ContentOverlapMemory.end())
											m_ContentOverlapMemory.insert(leftInfo);
										if (m_ContentOverlapMemory.find(rightInfo) == m_ContentOverlapMemory.end())
											m_ContentOverlapMemory.insert(rightInfo);
										globalUsage -= (rightAddr + rightSize - leftAddr);
									}
								}
								else
								{
									// overlap
									printf("CM: Content '%s' overlap with '%s'\n", leftInfo->DestName.toLocal8Bit().data(), rightInfo->DestName.toLocal8Bit().data());
									if (m_ContentOverlapMemory.find(leftInfo) == m_ContentOverlapMemory.end())
										m_ContentOverlapMemory.insert(leftInfo);
									if (m_ContentOverlapMemory.find(rightInfo) == m_ContentOverlapMemory.end())
										m_ContentOverlapMemory.insert(rightInfo);
									if (leftSize > rightSize)
										globalUsage -= rightSize;
									else globalUsage -= leftSize;
								}
							}
						}
					}
				}
			}
			else
			{
				++left;
			}
		}
		else
		{
			++left;
		}
	}

	for (std::set<ContentInfo *>::iterator it(contentOverlap.begin()), end(contentOverlap.end()); it != end; ++it)
	{
		if (m_ContentOverlapMemory.find(*it) == m_ContentOverlapMemory.end())
		{
			// Content no longer overlaps
			printf("CM: Content '%s' no longer overlapping\n", (*it)->DestName.toLocal8Bit().data());

			// Update overlap GUI
			(*it)->OverlapMemoryFlag = false;
			rebuildViewInternal(*it);

			// Reupload
			reuploadInternal(*it, true, false);
		}
	}

	for (std::set<ContentInfo *>::iterator it(m_ContentOverlapMemory.begin()), end(m_ContentOverlapMemory.end()); it != end; ++it)
	{
		if (contentOverlap.find(*it) == contentOverlap.end())
		{
			// Newly overlapping content
			printf("CM: Content '%s' is overlapping\n", (*it)->DestName.toLocal8Bit().data());

			// Update overlap GUI
			(*it)->OverlapMemoryFlag = true;
			rebuildViewInternal(*it);
		}
	}

	g_RamGlobalUsage = globalUsage;
}

void ContentManager::recalculateOverlapFlashInternal()
{
	printf("ContentManager::recalculateOverlapFlashInternal()\n");

	if (m_OverlapSuppressed)
	{
		m_OverlapFlashSuppressed = true;
		return;
	}

	std::set<ContentInfo *> contentOverlap;
	m_ContentOverlapFlash.swap(contentOverlap);

	int globalUsage = 0;
	size_t globalSize = g_Flash ? BT8XXEMU_Flash_size(g_Flash) : 0;

	for (QTreeWidgetItemIterator left(m_ContentList); *left; )
	{
		ContentInfo *leftInfo = (ContentInfo *)(void *)(*left)->data(0, Qt::UserRole).value<quintptr>();
		if (leftInfo->Converter != ContentInfo::Invalid && (leftInfo->DataStorage == ContentInfo::Flash))
		{
			int leftSize = getContentSize(leftInfo);
			// printf("leftSize: %i\n", leftSize);
			if (leftSize >= 0)
			{
				int leftAddr = leftInfo->FlashAddress;
				if (leftAddr < FTEDITOR_FLASH_FIRMWARE_SIZE)
				{
					printf("CM: Content '%s' overlaps with flash firmware\n", leftInfo->DestName.toLocal8Bit().data());
					if (m_ContentOverlapFlash.find(leftInfo) == m_ContentOverlapFlash.end())
						m_ContentOverlapFlash.insert(leftInfo);
					++left;
				}
				if (leftAddr + leftSize > globalSize)
				{
					printf("CM: Content '%s' oversize\n", leftInfo->DestName.toLocal8Bit().data());
					if (m_ContentOverlapFlash.find(leftInfo) == m_ContentOverlapFlash.end())
						m_ContentOverlapFlash.insert(leftInfo);
					++left;
				}
				else
				{
					globalUsage += leftSize;
					for (QTreeWidgetItemIterator right(++left); *right; ++right)
					{
						ContentInfo *rightInfo = (ContentInfo *)(void *)(*right)->data(0, Qt::UserRole).value<quintptr>();
						if (rightInfo->Converter != ContentInfo::Invalid && (rightInfo->DataStorage == ContentInfo::Flash))
						{
							int rightSize = getContentSize(rightInfo);
							if (rightSize >= 0)
							{
								int rightAddr = rightInfo->FlashAddress;

								if (leftAddr < rightAddr)
								{
									if (leftAddr + leftSize > rightAddr)
									{
										// overlap
										printf("CM: Content '%s' overlap with '%s'\n", leftInfo->DestName.toLocal8Bit().data(), rightInfo->DestName.toLocal8Bit().data());
										if (m_ContentOverlapFlash.find(leftInfo) == m_ContentOverlapFlash.end())
											m_ContentOverlapFlash.insert(leftInfo);
										if (m_ContentOverlapFlash.find(rightInfo) == m_ContentOverlapFlash.end())
											m_ContentOverlapFlash.insert(rightInfo);
										globalUsage -= (leftAddr + leftSize - rightAddr);
									}
								}
								else if (rightAddr < leftAddr)
								{
									if (rightAddr + rightSize > leftAddr)
									{
										// overlap
										printf("CM: Content '%s' overlap with '%s'\n", leftInfo->DestName.toLocal8Bit().data(), rightInfo->DestName.toLocal8Bit().data());
										if (m_ContentOverlapFlash.find(leftInfo) == m_ContentOverlapFlash.end())
											m_ContentOverlapFlash.insert(leftInfo);
										if (m_ContentOverlapFlash.find(rightInfo) == m_ContentOverlapFlash.end())
											m_ContentOverlapFlash.insert(rightInfo);
										globalUsage -= (rightAddr + rightSize - leftAddr);
									}
								}
								else
								{
									// overlap
									printf("CM: Content '%s' overlap with '%s'\n", leftInfo->DestName.toLocal8Bit().data(), rightInfo->DestName.toLocal8Bit().data());
									if (m_ContentOverlapFlash.find(leftInfo) == m_ContentOverlapFlash.end())
										m_ContentOverlapFlash.insert(leftInfo);
									if (m_ContentOverlapFlash.find(rightInfo) == m_ContentOverlapFlash.end())
										m_ContentOverlapFlash.insert(rightInfo);
									if (leftSize > rightSize)
										globalUsage -= rightSize;
									else globalUsage -= leftSize;
								}
							}
						}
					}
				}
			}
			else
			{
				++left;
			}
		}
		else
		{
			++left;
		}
	}

	for (std::set<ContentInfo *>::iterator it(contentOverlap.begin()), end(contentOverlap.end()); it != end; ++it)
	{
		if (m_ContentOverlapFlash.find(*it) == m_ContentOverlapFlash.end())
		{
			// Content no longer overlaps
			printf("CM: Content '%s' no longer overlapping\n", (*it)->DestName.toLocal8Bit().data());

			// Update overlap GUI
			(*it)->OverlapFlashFlag = false;
			rebuildViewInternal(*it);

			// Reupload
			reuploadInternal(*it, false, true);
		}
	}

	for (std::set<ContentInfo *>::iterator it(m_ContentOverlapFlash.begin()), end(m_ContentOverlapFlash.end()); it != end; ++it)
	{
		if (contentOverlap.find(*it) == contentOverlap.end())
		{
			// Newly overlapping content
			printf("CM: Content '%s' is overlapping\n", (*it)->DestName.toLocal8Bit().data());

			// Update overlap GUI
			(*it)->OverlapFlashFlag = true;
			rebuildViewInternal(*it);
		}
	}

	g_FlashGlobalUsage = globalUsage;
}

void ContentManager::reloadExternal(ContentInfo *contentInfo)
{
	// printf("ContentManager::reloadExternal(contentInfo)\n");

	// m_MainWindow->bitmapSetup()->reloadContent(contentInfo);
}

void ContentManager::propertiesSetterChanged(QWidget *setter)
{
	// printf("ContentManager::propertiesSetterChanged(setter)\n");

	if (setter != this)
	{
		m_ContentList->setCurrentItem(NULL);
	}
}

ContentInfo *ContentManager::current()
{
	if (!m_ContentList->currentItem())
		return NULL;
	return (ContentInfo *)(void *)m_ContentList->currentItem()->data(0, Qt::UserRole).value<quintptr>();
}

QString ContentManager::findFlashMapPath()
{
	QString flashMapPath;

	for (QTreeWidgetItemIterator it(m_ContentList); *it; ++it)
	{
		ContentInfo *info = (ContentInfo *)(void *)(*it)->data(0, Qt::UserRole).value<quintptr>();
		if (info->Converter == ContentInfo::FlashMap)
		{
			flashMapPath = info->SourcePath;
			if (!flashMapPath.isEmpty()) break;
		}
	}

	return flashMapPath;
}

int ContentManager::editorFindHandle(ContentInfo *contentInfo, DlEditor *dlEditor)
{
	int handle = -1;
	for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
	{
		const DlParsed &parsed = dlEditor->getLine(i);
		if (parsed.ValidId)
		{
			if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810)
			{
				if (parsed.IdLeft == FTEDITOR_CO_COMMAND)
				{
					switch (parsed.IdRight)
					{
					case CMD_SETBITMAP & 0xFF:
						if (parsed.Parameter[0].U == contentInfo->bitmapAddress() && handle != -1)
							return handle;
						break;
					case CMD_SETFONT2 & 0xFF:
						if (parsed.Parameter[1].I == contentInfo->MemoryAddress)
							return parsed.Parameter[0].I;
						break;
					}
				}
			}
			if (parsed.IdLeft != 0)
			{
				handle = -1;
				continue;
			}
			switch (parsed.IdRight)
			{
				case FTEDITOR_DL_BITMAP_HANDLE:
					handle = parsed.Parameter[0].I;
					break;
				case FTEDITOR_DL_BITMAP_SOURCE:
					if (parsed.Parameter[0].U == contentInfo->bitmapAddress() && handle != -1)
						return handle;
					break;
				case FTEDITOR_DL_BITMAP_LAYOUT:
				case FTEDITOR_DL_BITMAP_SIZE:
				case FTEDITOR_DL_BITMAP_LAYOUT_H:
				case FTEDITOR_DL_BITMAP_SIZE_H:
					break;
				default:
					handle = -1;
					break;
			}
		}
	}
	return -1;
}

int ContentManager::editorFindHandle(ContentInfo *contentInfo, DlEditor *dlEditor, int &line)
{
	int handle = -1;
	line = -1;
	for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
	{
		const DlParsed &parsed = dlEditor->getLine(i);
		if (parsed.ValidId)
		{
			if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810)
			{
				if (parsed.IdLeft == FTEDITOR_CO_COMMAND)
				{
					switch (parsed.IdRight)
					{
					case CMD_SETBITMAP & 0xFF:
						if (parsed.Parameter[0].U == contentInfo->bitmapAddress() && handle != -1)
							return handle;
						break;
					case CMD_SETFONT2 & 0xFF:
						if (parsed.Parameter[1].I == contentInfo->MemoryAddress)
						{
							line = i;
							return parsed.Parameter[0].I;
						}
						break;
					}
				}
			}
			if (parsed.IdLeft != 0)
				continue;
			switch (parsed.IdRight)
			{
				case FTEDITOR_DL_BITMAP_HANDLE:
					line = i;
					handle = parsed.Parameter[0].I;
					break;
				case FTEDITOR_DL_BITMAP_SOURCE:
					if (parsed.Parameter[0].U == contentInfo->bitmapAddress() && handle != -1)
						return handle;
					break;
				case FTEDITOR_DL_BITMAP_LAYOUT:
				case FTEDITOR_DL_BITMAP_SIZE:
				case FTEDITOR_DL_BITMAP_LAYOUT_H:
				case FTEDITOR_DL_BITMAP_SIZE_H:
					break;
			}
		}
	}
	return -1;
}

int ContentManager::editorFindFreeHandle(DlEditor *dlEditor)
{
	bool handles[BITMAP_SETUP_HANDLES_NB];
	for (int i = 0; i < BITMAP_SETUP_HANDLES_NB; ++i)
	{
		handles[i] = false;
	}
	for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
	{
		const DlParsed &parsed = dlEditor->getLine(i);
		if (parsed.ValidId && parsed.IdLeft == 0 && parsed.IdRight == FTEDITOR_DL_BITMAP_HANDLE && parsed.Parameter[0].U < BITMAP_SETUP_HANDLES_NB)
		{
			handles[parsed.Parameter[0].U] = true;
		}
		else if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810)
		{
			if (parsed.ValidId && parsed.IdLeft == FTEDITOR_CO_COMMAND && parsed.IdRight == (CMD_SETFONT2 & 0xFF) && parsed.Parameter[0].U < BITMAP_SETUP_HANDLES_NB)
			{
				handles[parsed.Parameter[0].U] = true;
			}
			else if (parsed.ValidId && parsed.IdLeft == FTEDITOR_CO_COMMAND && parsed.IdRight == (CMD_SETSCRATCH & 0xFF) && parsed.Parameter[0].U < BITMAP_SETUP_HANDLES_NB)
			{
				handles[parsed.Parameter[0].U] = true;
			}
			else if (parsed.ValidId && parsed.IdLeft == FTEDITOR_CO_COMMAND && parsed.IdRight == (CMD_ROMFONT & 0xFF) && parsed.Parameter[0].U < BITMAP_SETUP_HANDLES_NB)
			{
				handles[parsed.Parameter[0].U] = true;
			}
		}
	}
	for (int i = 0; i < BITMAP_SETUP_HANDLES_NB; ++i)
	{
		if (!handles[i]) return i;
	}
	return -1;
}

// Find where to start with bitmap lines in the editor
int ContentManager::editorFindNextBitmapLine(DlEditor *dlEditor)
{
	for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
	{
		const DlParsed &parsed = dlEditor->getLine(i);
		if (parsed.ValidId && parsed.IdLeft == 0)
		{
			switch (parsed.IdRight)
			{
				case FTEDITOR_DL_BITMAP_HANDLE:
				case FTEDITOR_DL_BITMAP_SOURCE:
				case FTEDITOR_DL_BITMAP_LAYOUT:
				case FTEDITOR_DL_BITMAP_SIZE:
				case FTEDITOR_DL_BITMAP_LAYOUT_H:
				case FTEDITOR_DL_BITMAP_SIZE_H:
					break; // do nothing
				default:
					return i;
			}
		}
		else if (parsed.ValidId && parsed.IdLeft == 0xFFFFFF00)
		{
			switch (parsed.IdRight)
			{
			case CMD_SETFONT & 0xFF:
			case CMD_SETFONT2 & 0xFF:
			case CMD_SETBITMAP & 0xFF:
				break; // do nothing
			default:
				return i;
			}
		}
		else
		{
			return i;
		}
	}
	return 0;
}

inline bool requirePaletteAddress(ContentInfo *contentInfo)
{
	switch (contentInfo->ImageFormat)
	{
	case PALETTED4444:
	case PALETTED565:
	case PALETTED8:
		return true;
	}
	return false;
}

void ContentManager::editorUpdateHandle(ContentInfo *contentInfo, DlEditor *dlEditor, bool updateSize)
{
	int handleLine = -1;
	bool addressOk = false;
	int layoutLine = -1;
	int sizeLine = -1;

	int insertOkLine = -1;
	int layoutHLine = -1;
	int sizeHLine = -1;
	// int paletteAddrLine = -1;
	bool cmdSetBitmap = false;

	for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
	{
		const DlParsed &parsed = dlEditor->getLine(i);
		if (parsed.ValidId)
		{
			if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810)
			{
				if (parsed.IdLeft == FTEDITOR_CO_COMMAND)
				{
					switch (parsed.IdRight)
					{
						case CMD_SETBITMAP & 0xFF:
						{
							bool isAddressSame = parsed.Parameter[0].U == contentInfo->bitmapAddress();
							if (addressOk)
							{
								DlParsed pa = parsed;
								pa.Parameter[1].U = contentInfo->ImageFormat;
								pa.Parameter[2].U = contentInfo->CachedImageWidth & 0x7FF;
								pa.Parameter[3].U = contentInfo->CachedImageHeight & 0x7FF;
								dlEditor->replaceLine(i, pa);
								cmdSetBitmap = true;
								insertOkLine = i + 1;
							}
							if (isAddressSame && handleLine != -1 && !addressOk)
							{
								i = handleLine;
								addressOk = true;
							}
							continue;
						}
						case CMD_SETFONT2 & 0xFF:
						{
							// This is really not a useful case, but it can happen, so support it...
							handleLine = i;
							addressOk = (parsed.Parameter[1].I == contentInfo->MemoryAddress);
							if (addressOk)
								printf("Unusual CMD_SETFONT2 usage detected in the editor\n");
							continue;
						}
					}
				}
			}
			if (parsed.IdLeft != FTEDITOR_DL_INSTRUCTION)
			{
				handleLine = -1;
				addressOk = false;
				continue;
			}
			switch (parsed.IdRight)
			{
				case FTEDITOR_DL_BITMAP_HANDLE:
					handleLine = i;
					addressOk = false;
					break;
				case FTEDITOR_DL_BITMAP_SOURCE:
				{
					bool isAddressSame = parsed.Parameter[0].U == contentInfo->bitmapAddress();
					if (addressOk)
					{
						insertOkLine = i + 1;
					}
					if (isAddressSame && handleLine != -1 && !addressOk)
					{
						i = handleLine;
						addressOk = true;
					}
					break;
				}
				case FTEDITOR_DL_BITMAP_LAYOUT:
					if (addressOk)
					{
						DlParsed pa = parsed;
						pa.Parameter[0].U = contentInfo->ImageFormat;
						pa.Parameter[1].U = contentInfo->CachedImageStride & 0x3FF;
						pa.Parameter[2].U = contentInfo->CachedImageHeight & 0x1FF;
						dlEditor->replaceLine(i, pa);
						layoutLine = i;
					}
					break;
				case FTEDITOR_DL_BITMAP_SIZE:
					if (addressOk && updateSize)
					{
						DlParsed pa = parsed;
						pa.Parameter[3].U = contentInfo->CachedImageWidth & 0x1FF;
						pa.Parameter[4].U = contentInfo->CachedImageHeight & 0x1FF;
						dlEditor->replaceLine(i, pa);
						sizeLine = i;
					}
					break;
				case FTEDITOR_DL_BITMAP_LAYOUT_H:
					if (addressOk)
					{
						// Update _H
						DlParsed pa = parsed;
						pa.Parameter[0].U = contentInfo->CachedImageStride >> 10;
						pa.Parameter[1].U = contentInfo->CachedImageHeight >> 9;
						dlEditor->replaceLine(i, pa);
						layoutHLine = i;
					}
					break;
				case FTEDITOR_DL_BITMAP_SIZE_H:
					if (addressOk)
					{
						// Update _H
						DlParsed pa = parsed;
						pa.Parameter[0].U = contentInfo->CachedImageWidth >> 9;
						pa.Parameter[1].U = contentInfo->CachedImageHeight >> 9;
						dlEditor->replaceLine(i, pa);
						sizeHLine = i;
					}
					break;
				/* case FTEDITOR_DL_PALETTE_SOURCE:
					if (addressOk && parsed.Parameter[0].U == contentInfo->MemoryAddress)
					{
						if (!requirePaletteAddress(contentInfo))
						{
							// Remove it, as it is no longer needed
							dlEditor->removeLine(i);
							--i;
						}
						else
						{
							// Have it, no change
							paletteAddrLine = i;
						}
					}
					break; */
				default:
					handleLine = -1;
					addressOk = false;
					break;
			}
		}
	}
	if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810)
	{
		if (!cmdSetBitmap) // User friendly fixup for _H, does not work with bitmap handle reuse, but not a critical use case
		{
			if (layoutLine >= 0 && layoutHLine < 0)
			{
				// Add _H if doesn't exist
				DlParsed pa;
				pa.ValidId = true;
				pa.IdLeft = FTEDITOR_DL_INSTRUCTION;
				pa.IdRight = FTEDITOR_DL_BITMAP_LAYOUT_H;
				pa.ExpectedStringParameter = false;
				pa.Parameter[0].U = contentInfo->CachedImageStride >> 10;
				pa.Parameter[1].U = contentInfo->CachedImageHeight >> 9;
				pa.ExpectedParameterCount = 2;
				layoutHLine = layoutLine + 1;
				dlEditor->insertLine(layoutHLine, pa);
				if (sizeLine > layoutLine)
					++sizeLine;
				if (sizeHLine > layoutLine)
					++sizeHLine;
			}
			if (sizeLine >= 0 && sizeHLine < 0)
			{
				// Add _H if doesn't exist and necessary
				DlParsed pa;
				pa.ValidId = true;
				pa.IdLeft = FTEDITOR_DL_INSTRUCTION;
				pa.IdRight = FTEDITOR_DL_BITMAP_SIZE_H;
				pa.ExpectedStringParameter = false;
				pa.Parameter[0].U = contentInfo->CachedImageWidth >> 9;
				pa.Parameter[1].U = contentInfo->CachedImageHeight >> 9;
				pa.ExpectedParameterCount = 2;
				sizeHLine = sizeLine + 1;
				dlEditor->insertLine(sizeHLine, pa);
			}
		}
		/* if (insertOkLine >= 0 && paletteAddrLine < 0 && requirePaletteAddress(contentInfo))
		{
			// Add palette source
			DlParsed pa;
			pa.ValidId = true;
			pa.IdLeft = FTEDITOR_DL_INSTRUCTION;
			pa.IdRight = FTEDITOR_DL_PALETTE_SOURCE;
			pa.ExpectedStringParameter = false;
			pa.Parameter[0].U = contentInfo->MemoryAddress;
			pa.ExpectedParameterCount = 1;
			paletteAddrLine = insertOkLine;
			++insertOkLine;
			dlEditor->insertLine(paletteAddrLine, pa);
		} */
	}
}

// Update palette adress
void ContentManager::editorUpdatePaletteAddress(int newAddr, int oldAddr, DlEditor *dlEditor)
{
	for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
	{
		const DlParsed &parsed = dlEditor->getLine(i);
		if (parsed.ValidId && parsed.IdLeft == 0 && parsed.IdRight == FTEDITOR_DL_PALETTE_SOURCE && (parsed.Parameter[0].I & ~3) == oldAddr)
		{
			DlParsed pa = parsed;
			pa.Parameter[0].I = newAddr + (parsed.Parameter[0].I % 4);
			dlEditor->replaceLine(i, pa);
		}
	}
}

// Update handle adress
void ContentManager::editorUpdateHandleAddress(int newAddr, int oldAddr, DlEditor *dlEditor)
{
	for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
	{
		const DlParsed &parsed = dlEditor->getLine(i);
		if (parsed.ValidId && parsed.IdLeft == 0 && parsed.IdRight == FTEDITOR_DL_BITMAP_SOURCE && parsed.Parameter[0].I == oldAddr)
		{
			DlParsed pa = parsed;
			pa.Parameter[0].I = newAddr;
			dlEditor->replaceLine(i, pa);
		}
		else if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810)
		{
			if (parsed.ValidId && parsed.IdLeft == FTEDITOR_CO_COMMAND && parsed.IdRight == (CMD_SETBITMAP & 0xFF) && parsed.Parameter[0].I == oldAddr)
			{
				DlParsed pa = parsed;
				pa.Parameter[0].I = newAddr;
				dlEditor->replaceLine(i, pa);
			}
		}
	}
}

// Update font adress
void ContentManager::editorUpdateFontAddress(int newAddr, int oldAddr, DlEditor *dlEditor)
{
	for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
	{
		const DlParsed &parsed = dlEditor->getLine(i);
		if (parsed.ValidId && parsed.IdLeft == FTEDITOR_CO_COMMAND && (parsed.IdRight == (CMD_SETFONT & 0xFF) || parsed.IdRight == (CMD_SETFONT2 & 0xFF)) && parsed.Parameter[1].I == oldAddr)
		{
			DlParsed pa = parsed;
			pa.Parameter[1].I = newAddr;
			dlEditor->replaceLine(i, pa);
		}
	}
}

void ContentManager::editorUpdateFontOffset(ContentInfo *contentInfo, DlEditor *dlEditor)
{
	for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
	{
		const DlParsed &parsed = dlEditor->getLine(i);
		if (parsed.ValidId && parsed.IdLeft == FTEDITOR_CO_COMMAND && parsed.IdRight == (CMD_SETFONT2 & 0xFF) && parsed.Parameter[1].I == contentInfo->MemoryAddress)
		{
			DlParsed pa = parsed;
			pa.Parameter[2].I = contentInfo->FontOffset;
			dlEditor->replaceLine(i, pa);
		}
	}

}

void editorPurgePalette8(DlEditor *dlEditor, int &line)
{
	//printf("purge p8\n");
	//printf("start line %i\n", line);
	const DlParsed pav = dlEditor->getLine(line);
	if (line > 0)
	{
		--line;
		for (; line >= 0; --line)
		{
			//printf("line %i\n", line);

			const DlParsed &parsed = dlEditor->getLine(line);
			if (!parsed.ValidId)
				break;

			if (parsed.IdLeft == FTEDITOR_DL_VERTEX2F
				|| parsed.IdLeft == FTEDITOR_DL_VERTEX2II)
			{
				if (parsed.Parameter[0].I == pav.Parameter[0].I
					&& parsed.Parameter[1].I == pav.Parameter[1].I)
				{
					dlEditor->removeLine(line);
					--line;
					continue;
				}
				else
				{
					break;
				}
			}

			if (parsed.IdLeft != FTEDITOR_DL_INSTRUCTION)
				break;

			if (parsed.IdRight == FTEDITOR_DL_COLOR_MASK
				|| parsed.IdRight == FTEDITOR_DL_PALETTE_SOURCE
				|| parsed.IdRight == FTEDITOR_DL_BLEND_FUNC
				|| parsed.IdRight == FTEDITOR_DL_BITMAP_HANDLE
				|| parsed.IdRight == FTEDITOR_DL_CELL)
				dlEditor->removeLine(line);
			else
				break;
		}
		++line;
	}
	if (line + 1 < FTEDITOR_DL_SIZE && line + 1 < dlEditor->getLineCount())
	{
		++line; // keep one vertex
		for (; line < FTEDITOR_DL_SIZE && line < dlEditor->getLineCount(); ++line)
		{
			//printf("at+ %i\n", line);
			const DlParsed &parsed = dlEditor->getLine(line);
			if (!parsed.ValidId)
				break;

			if (parsed.IdLeft == FTEDITOR_DL_VERTEX2F
				|| parsed.IdLeft == FTEDITOR_DL_VERTEX2II)
			{
				if (parsed.Parameter[0].I == pav.Parameter[0].I
					&& parsed.Parameter[1].I == pav.Parameter[1].I)
				{
					dlEditor->removeLine(line);
					--line;
					continue;
				}
				else
				{
					break;
				}
			}

			if (parsed.IdLeft != FTEDITOR_DL_INSTRUCTION)
				break;

			if (parsed.IdRight == FTEDITOR_DL_COLOR_MASK
				|| parsed.IdRight == FTEDITOR_DL_PALETTE_SOURCE
				|| parsed.IdRight == FTEDITOR_DL_BLEND_FUNC
				|| parsed.IdRight == FTEDITOR_DL_BITMAP_HANDLE
				|| parsed.IdRight == FTEDITOR_DL_CELL)
			{
				dlEditor->removeLine(line);
				--line;
			}
			else
				break;
		}
		//printf("x line %i\n", line);
		--line; // return to vertex
	}
	//printf("pre line %i\n", line);
	// strip save/restore context
	if (line > 0 && line + 1 < FTEDITOR_DL_SIZE && line + 1 < dlEditor->getLineCount())
	{
		const DlParsed &prev = dlEditor->getLine(line - 1);
		const DlParsed &next = dlEditor->getLine(line + 1);
		if (prev.ValidId && prev.IdLeft == FTEDITOR_DL_INSTRUCTION && prev.IdRight == FTEDITOR_DL_SAVE_CONTEXT
			&& next.ValidId && next.IdLeft == FTEDITOR_DL_INSTRUCTION && next.IdRight == FTEDITOR_DL_RESTORE_CONTEXT)
		{
			dlEditor->removeLine(line - 1);
			dlEditor->removeLine(line);
			--line;
		}
	}
	//printf("end line %i\n", line);
}

void tempBeginIdel(CodeEditor *dlEditor)
{
	static_cast<DlEditor *>(dlEditor->parent())->mainWindow()->undoStack()->beginMacro("Interactive Delete");
}

void tempEndIdel(CodeEditor *dlEditor)
{
	static_cast<DlEditor *>(dlEditor->parent())->mainWindow()->undoStack()->endMacro();
}

void editorPurgePalette8(CodeEditor *dlEditor, int &line)
{
	editorPurgePalette8(static_cast<DlEditor *>(dlEditor->parent()), line);
}

void editorInsertPallette8(int paletteAddress, DlEditor *dlEditor, int &line)
{
	DlParsed pav = dlEditor->getLine(line);

	DlParsed pa;
	pa.ValidId = true;
	pa.IdLeft = 0;
	pa.ExpectedStringParameter = false;
	
	pa.IdRight = FTEDITOR_DL_SAVE_CONTEXT;
	pa.ExpectedParameterCount = 0;
	dlEditor->insertLine(line, pa);
	++line;
	pa.IdRight = FTEDITOR_DL_BLEND_FUNC;
	pa.Parameter[0].U = ONE;
	pa.Parameter[1].U = ZERO;
	pa.ExpectedParameterCount = 2;
	dlEditor->insertLine(line, pa);
	++line;
	pa.IdRight = FTEDITOR_DL_COLOR_MASK;
	pa.Parameter[0].U = 0;
	pa.Parameter[1].U = 0;
	pa.Parameter[2].U = 0;
	pa.Parameter[3].U = 1;
	pa.ExpectedParameterCount = 4;
	dlEditor->insertLine(line, pa);
	++line;
	pa.IdRight = FTEDITOR_DL_PALETTE_SOURCE;
	pa.Parameter[0].U = paletteAddress + 3;
	pa.ExpectedParameterCount = 1;
	dlEditor->insertLine(line, pa);
	++line;
	dlEditor->insertLine(line, pav);
	++line;
	pa.IdRight = FTEDITOR_DL_BLEND_FUNC;
	pa.Parameter[0].U = DST_ALPHA;
	pa.Parameter[1].U = ONE_MINUS_DST_ALPHA;
	pa.ExpectedParameterCount = 2;
	dlEditor->insertLine(line, pa);
	++line;
	pa.IdRight = FTEDITOR_DL_COLOR_MASK;
	pa.Parameter[0].U = 1;
	pa.Parameter[1].U = 0;
	pa.Parameter[2].U = 0;
	pa.Parameter[3].U = 0;
	pa.ExpectedParameterCount = 4;
	dlEditor->insertLine(line, pa);
	++line;
	pa.IdRight = FTEDITOR_DL_PALETTE_SOURCE;
	pa.Parameter[0].U = paletteAddress + 2;
	pa.ExpectedParameterCount = 1;
	dlEditor->insertLine(line, pa);
	++line;
	dlEditor->insertLine(line, pav);
	++line;
	pa.IdRight = FTEDITOR_DL_COLOR_MASK;
	pa.Parameter[0].U = 0;
	pa.Parameter[1].U = 1;
	pa.Parameter[2].U = 0;
	pa.Parameter[3].U = 0;
	pa.ExpectedParameterCount = 4;
	dlEditor->insertLine(line, pa);
	++line;
	pa.IdRight = FTEDITOR_DL_PALETTE_SOURCE;
	pa.Parameter[0].U = paletteAddress + 1;
	pa.ExpectedParameterCount = 1;
	dlEditor->insertLine(line, pa);
	++line;
	dlEditor->insertLine(line, pav);
	++line;
	pa.IdRight = FTEDITOR_DL_COLOR_MASK;
	pa.Parameter[0].U = 0;
	pa.Parameter[1].U = 0;
	pa.Parameter[2].U = 1;
	pa.Parameter[3].U = 0;
	pa.ExpectedParameterCount = 4;
	dlEditor->insertLine(line, pa);
	++line;
	pa.IdRight = FTEDITOR_DL_PALETTE_SOURCE;
	pa.Parameter[0].U = paletteAddress;
	pa.ExpectedParameterCount = 1;
	dlEditor->insertLine(line, pa);
	++line;
	// Use existing line // dlEditor->insertLine(line, pav);
	++line;
	pa.IdRight = FTEDITOR_DL_RESTORE_CONTEXT;
	pa.ExpectedParameterCount = 0;
	dlEditor->insertLine(line, pa);
	// ++line;
}

#define FTEDITOR_INSERT_PALETTE_SOURCE 1
#define FTEDITOR_INSERT_PALETTE_SOURCE_8 2
#define FTEDITOR_PURGE_PALETTE_SOURCE 3
#define FTEDITOR_PURGE_PALETTE_SOURCE_8 4
template <int process>
void editorProcessPaletteSource(int bitmapAddress, int paletteAddress, DlEditor *dlEditor)
{
	//if (process == FTEDITOR_PURGE_PALETTE_SOURCE_8)
	//	printf("yes purge8\n");

	bool bitmapHandle[32];
	for (int i = 0; i < 32; ++i) bitmapHandle[0] = false;
	int curBitmapHandle = 0;
	bool drawingBitmaps = false;

	DlParsed paPaletteSource;
	paPaletteSource.ValidId = true;
	paPaletteSource.IdLeft = 0;
	paPaletteSource.IdRight = FTEDITOR_DL_PALETTE_SOURCE;
	paPaletteSource.ExpectedParameterCount = 1;
	paPaletteSource.ExpectedStringParameter = false;
	paPaletteSource.Parameter[0].I = paletteAddress;

	for (int i = 0; i < FTEDITOR_DL_SIZE && i < dlEditor->getLineCount(); ++i)
	{
		//printf("at %i\n", i);

		const DlParsed &parsed = dlEditor->getLine(i);
		if (!parsed.ValidId)
			continue;

		if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810)
		{
			if (parsed.IdLeft == FTEDITOR_CO_COMMAND)
			{
				switch (parsed.IdRight)
				{
				case CMD_SETBITMAP & 0xFF:
					bitmapHandle[curBitmapHandle]
						= parsed.Parameter[0].U == bitmapAddress;
					break;
				case CMD_SETFONT2 & 0xFF:
					if (parsed.Parameter[0].I < 32)
						bitmapHandle[parsed.Parameter[0].I]
							= parsed.Parameter[1].I == bitmapAddress;
					break;
				}
			}
		}
		if (parsed.IdLeft != 0)
		{
			if (drawingBitmaps)
			{
				if (parsed.IdLeft == FTEDITOR_DL_VERTEX2II)
				{
					if (bitmapHandle[parsed.Parameter[2].I])
					{
						if (process == FTEDITOR_INSERT_PALETTE_SOURCE)
						{
							dlEditor->insertLine(i, paPaletteSource);
							++i;
						}
						if (process == FTEDITOR_INSERT_PALETTE_SOURCE_8)
						{
							editorInsertPallette8(paletteAddress, dlEditor, i);
						}
						if (process == FTEDITOR_PURGE_PALETTE_SOURCE_8)
						{
							editorPurgePalette8(dlEditor, i);
						}
					}
				}
				else if (parsed.IdLeft == FTEDITOR_DL_VERTEX2F)
				{
					if (bitmapHandle[curBitmapHandle])
					{
						if (process == FTEDITOR_INSERT_PALETTE_SOURCE)
						{
							dlEditor->insertLine(i, paPaletteSource);
							++i;
						}
						if (process == FTEDITOR_INSERT_PALETTE_SOURCE_8)
						{
							editorInsertPallette8(paletteAddress, dlEditor, i);
						}
						if (process == FTEDITOR_PURGE_PALETTE_SOURCE_8)
						{
							editorPurgePalette8(dlEditor, i);
						}
					}
				}
			}
			continue;
		}
		switch (parsed.IdRight)
		{
		case FTEDITOR_DL_PALETTE_SOURCE:
			if ((parsed.Parameter[0].I & ~3) == paletteAddress)
			{
				if (process == FTEDITOR_PURGE_PALETTE_SOURCE) // BUG: May remove too many
				{
					dlEditor->removeLine(i);
					--i;
				}
			}
			break;
		case FTEDITOR_DL_BITMAP_HANDLE:
			if (parsed.Parameter[0].I < 32)
				curBitmapHandle = parsed.Parameter[0].I;
			break;
		case FTEDITOR_DL_BITMAP_SOURCE:
			bitmapHandle[curBitmapHandle]
				= parsed.Parameter[0].U == bitmapAddress;
			break;
		case FTEDITOR_DL_BEGIN:
			drawingBitmaps = (parsed.Parameter[0].I == BITMAPS);
			break;
		case FTEDITOR_DL_END:
			drawingBitmaps = false;
			break;
		}
	}
}

void ContentManager::editorRemoveContent(ContentInfo *contentInfo, DlEditor *dlEditor)
{
	if (requirePaletteAddress(contentInfo))
	{
		if (contentInfo->ImageFormat == PALETTED8)
				editorProcessPaletteSource<FTEDITOR_PURGE_PALETTE_SOURCE_8>(contentInfo->bitmapAddress(), contentInfo->MemoryAddress, m_MainWindow->dlEditor()),
				editorProcessPaletteSource<FTEDITOR_PURGE_PALETTE_SOURCE_8>(contentInfo->bitmapAddress(), contentInfo->MemoryAddress, m_MainWindow->cmdEditor());
		else
			editorProcessPaletteSource<FTEDITOR_PURGE_PALETTE_SOURCE>(contentInfo->bitmapAddress(), contentInfo->MemoryAddress, m_MainWindow->dlEditor()),
			editorProcessPaletteSource<FTEDITOR_PURGE_PALETTE_SOURCE>(contentInfo->bitmapAddress(), contentInfo->MemoryAddress, m_MainWindow->cmdEditor());
	}

	// ISSUE#113: Remove all entries related to content info
	int bitmapLine;
	int bitmapHandle = editorFindHandle(contentInfo, dlEditor, bitmapLine);
	bool handleActive = true;
	int bitmapSource = -1;
	bool drawingBitmaps = false;
	if (bitmapHandle >= 0)
	{
		printf("ISSUE#113: Purging bitmap handle %i\n", bitmapHandle);
		dlEditor->removeLine(bitmapLine);
		for (int i = bitmapLine; i < FTEDITOR_DL_SIZE && i < dlEditor->getLineCount(); ++i)
		{
			const DlParsed &parsed = dlEditor->getLine(i);
			if (parsed.ValidId)
			{
				if (drawingBitmaps) // ISSUE#113: NOTE: If you do not want bitmaps removed, disable this block
				{
					bool removedVertex = false;
					if (parsed.IdLeft == FTEDITOR_DL_VERTEX2II)
					{
						if (parsed.Parameter[2].I == bitmapHandle)
						{
							// Purge bitmaps drawn through VERTEX2II
							dlEditor->removeLine(i);
							--i;
							removedVertex = true;
						}
					}
					else if (parsed.IdLeft == FTEDITOR_DL_VERTEX2F)
					{
						if (handleActive)
						{
							// Purge bitmaps drawn through VERTEX2F
							dlEditor->removeLine(i);
							--i;
							removedVertex = true;
						}
					}
					if (removedVertex)
					{
						++i;
						if (i < FTEDITOR_DL_SIZE && i < dlEditor->getLineCount())
						{
							const DlParsed &beginVertex = dlEditor->getLine(i - 1);
							const DlParsed &endVertex = dlEditor->getLine(i);
							printf("%i - %i\n", beginVertex.IdRight, endVertex.IdRight);
							if (beginVertex.IdLeft == 0 && beginVertex.IdRight == FTEDITOR_DL_BEGIN
								&& endVertex.IdLeft == 0 && endVertex.IdRight == FTEDITOR_DL_END)
							{
								// Last vertex removed in BEGIN/END block, purge the block
								dlEditor->removeLine(i);
								--i;
								dlEditor->removeLine(i);
								--i;
							}
						}
						--i;
						continue;
					}
				}
				if (parsed.IdLeft == FTEDITOR_CO_COMMAND) // CMD
				{
					if (parsed.IdRight == (CMD_TEXT & 0xFF) && parsed.Parameter[2].I == bitmapHandle)
					{
						// Change text drawn with font to use default font
						DlParsed p = parsed;
						p.Parameter[2].I = 28;
						dlEditor->replaceLine(i, p);
						continue;
					}
					if (parsed.IdRight == (CMD_SETFONT & 0xFF) && parsed.Parameter[0].I == bitmapHandle)
					{
						// Purge fonts
						dlEditor->removeLine(i);
						--i;
						continue;
					}
					if (FTEDITOR_CURRENT_DEVICE >= FTEDITOR_FT810)
					{
						if (parsed.IdRight == (CMD_SETFONT2 & 0xFF))
						{
							handleActive = (parsed.Parameter[0].I == bitmapHandle); // This switches handle
							if (handleActive)
							{
								// Purge fonts
								dlEditor->removeLine(i);
								--i;
								continue;
							}
						}
						if (parsed.IdRight == (CMD_SETBITMAP & 0xFF) && handleActive)
						{
							// Purge bitmap parameters
							dlEditor->removeLine(i);
							--i;
							continue;
						}
					}
				}
				if (parsed.IdLeft != 0)
					continue;
				if (parsed.IdRight == FTEDITOR_DL_BEGIN)
				{
					drawingBitmaps = (parsed.Parameter[0].I == BITMAPS);
					continue;
				}
				if (parsed.IdRight == FTEDITOR_DL_END)
				{
					drawingBitmaps = false;
					continue;
				}
				if (parsed.IdRight == FTEDITOR_DL_BITMAP_HANDLE)
				{
					// Flag if the content handle is active
					handleActive = (parsed.Parameter[0].I == bitmapHandle);
				}
				if (parsed.IdRight == FTEDITOR_DL_BITMAP_SOURCE)
				{
					if (handleActive)
					{
						if (bitmapSource == -1)
						{
							// First occurrence, this is the current source
							bitmapSource = parsed.Parameter[0].I;
							dlEditor->removeLine(i);
							--i;
							continue;
						}
						else if (bitmapSource == parsed.Parameter[0].I)
						{
							// Re-occurrence of source, not normal, but legal syntax
							dlEditor->removeLine(i);
							--i;
							continue;
						}
						else
						{
							// If active, then source is being changed, and we need to stop
							break;
						}
					}
				}
				if (handleActive)
				{
					switch (parsed.IdRight)
					{
					case FTEDITOR_DL_BITMAP_HANDLE:
					case FTEDITOR_DL_BITMAP_LAYOUT:
					case FTEDITOR_DL_BITMAP_SIZE:
					case FTEDITOR_DL_BITMAP_LAYOUT_H:
					case FTEDITOR_DL_BITMAP_SIZE_H:
					case FTEDITOR_DL_PALETTE_SOURCE:
						dlEditor->removeLine(i);
						--i;
						break;
					}
				}
			}
		}
	}
}

////////////////////////////////////////////////////////////////////////

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
	m_MainWindow->undoStack()->beginMacro(tr("Change source path"));
	int oldBitmapAddr = contentInfo->bitmapAddress();
	m_MainWindow->undoStack()->push(changeSourcePath);
	m_MainWindow->propertiesEditor()->surpressSet(true);
	int newBitmapAddr = contentInfo->bitmapAddress();
	if (newBitmapAddr != oldBitmapAddr)
	{
		editorUpdateHandleAddress(newBitmapAddr, oldBitmapAddr, m_MainWindow->dlEditor());
		editorUpdateHandleAddress(newBitmapAddr, oldBitmapAddr, m_MainWindow->cmdEditor());
	}
	editorUpdateHandle(contentInfo, m_MainWindow->dlEditor(), true);
	editorUpdateHandle(contentInfo, m_MainWindow->cmdEditor(), true);
	m_ContentList->setCurrentItem(contentInfo->View);
	m_MainWindow->propertiesEditor()->surpressSet(false);
	m_MainWindow->undoStack()->endMacro();
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

////////////////////////////////////////////////////////////////////////

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

////////////////////////////////////////////////////////////////////////

class ContentManager::ChangeConverter : public QUndoCommand
{
public:
	ChangeConverter(ContentManager *contentManager, ContentInfo *contentInfo, ContentInfo::ConverterType value) :
		QUndoCommand(),
		m_ContentManager(contentManager),
		m_ContentInfo(contentInfo),
		m_OldValue(contentInfo->Converter),
		m_NewValue(value),
		m_OldFontFormat(contentInfo->ImageFormat)
	{
		if (m_NewValue == ContentInfo::Font)
		{
			if (m_OldFontFormat >= L1 && m_OldFontFormat <= L8)
			{
				m_NewFontFormat = m_OldFontFormat;
			}
			else
			{
				m_NewFontFormat = L4;
			}
		}
		else
		{
			m_NewFontFormat = m_OldFontFormat;
		}
		setText(tr("Set content converter"));
	}

	virtual ~ChangeConverter()
	{

	}

	virtual void undo()
	{
		m_ContentInfo->Converter = m_OldValue;
		m_ContentInfo->ImageFormat = m_OldFontFormat;
		m_ContentManager->reprocessInternal(m_ContentInfo);
	}

	virtual void redo()
	{
		m_ContentInfo->Converter = m_NewValue;
		m_ContentInfo->ImageFormat = m_NewFontFormat;
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
	int m_OldFontFormat;
	int m_NewFontFormat;
};

void ContentManager::changeConverter(ContentInfo *contentInfo, ContentInfo::ConverterType value)
{
	// Create undo/redo
	m_MainWindow->undoStack()->beginMacro(tr("Change converter"));
	ChangeConverter *changeConverter = new ChangeConverter(this, contentInfo, value);
	m_MainWindow->undoStack()->push(changeConverter);
	if (contentInfo->WantAutoLoad && contentInfo->Converter != ContentInfo::Invalid)
	{
		int freeAddress = getFreeMemoryAddress();
		if (freeAddress >= 0)
		{
			changeMemoryAddress(contentInfo, freeAddress, true);
			changeMemoryLoaded(contentInfo, true);
		}
		contentInfo->WantAutoLoad = false;
	}
	m_MainWindow->undoStack()->endMacro();
}

void ContentManager::propertiesCommonConverterChanged(int value)
{
	printf("ContentManager::propertiesCommonConverterChanged(value)\n");

	if (current() && current()->Converter != (ContentInfo::ConverterType)value)
		changeConverter(current(), (ContentInfo::ConverterType)value);
}

////////////////////////////////////////////////////////////////////////

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
	m_MainWindow->undoStack()->beginMacro(tr("Change image format"));
	int oldImageFormat = contentInfo->ImageFormat;
	bool oldPaletteSource = requirePaletteAddress(contentInfo) && oldImageFormat != PALETTED8;
	int oldBitmapAddr = contentInfo->bitmapAddress();
	m_MainWindow->undoStack()->push(changeImageFormat);
	m_MainWindow->propertiesEditor()->surpressSet(true);
	bool paletteSource = requirePaletteAddress(contentInfo) && contentInfo->ImageFormat != PALETTED8;
	if (oldImageFormat == PALETTED8)
		editorProcessPaletteSource<FTEDITOR_PURGE_PALETTE_SOURCE_8>(oldBitmapAddr, contentInfo->MemoryAddress, m_MainWindow->dlEditor()),
		editorProcessPaletteSource<FTEDITOR_PURGE_PALETTE_SOURCE_8>(oldBitmapAddr, contentInfo->MemoryAddress, m_MainWindow->cmdEditor());
	else if (oldPaletteSource != paletteSource && oldPaletteSource)
		editorProcessPaletteSource<FTEDITOR_PURGE_PALETTE_SOURCE>(oldBitmapAddr, contentInfo->MemoryAddress, m_MainWindow->dlEditor()),
		editorProcessPaletteSource<FTEDITOR_PURGE_PALETTE_SOURCE>(oldBitmapAddr, contentInfo->MemoryAddress, m_MainWindow->cmdEditor());
	int newBitmapAddr = contentInfo->bitmapAddress();
	if (newBitmapAddr != oldBitmapAddr)
		editorUpdateHandleAddress(newBitmapAddr, oldBitmapAddr, m_MainWindow->dlEditor()),
		editorUpdateHandleAddress(newBitmapAddr, oldBitmapAddr, m_MainWindow->cmdEditor());
	editorUpdateHandle(contentInfo, m_MainWindow->dlEditor(), false);
	editorUpdateHandle(contentInfo, m_MainWindow->cmdEditor(), false);
	// add new palette entries
	if (contentInfo->ImageFormat == PALETTED8)
		editorProcessPaletteSource<FTEDITOR_INSERT_PALETTE_SOURCE_8>(newBitmapAddr, contentInfo->MemoryAddress, m_MainWindow->dlEditor()),
		editorProcessPaletteSource<FTEDITOR_INSERT_PALETTE_SOURCE_8>(newBitmapAddr, contentInfo->MemoryAddress, m_MainWindow->cmdEditor());
	else if (oldPaletteSource != paletteSource && paletteSource)
		editorProcessPaletteSource<FTEDITOR_INSERT_PALETTE_SOURCE>(newBitmapAddr, contentInfo->MemoryAddress, m_MainWindow->dlEditor()),
		editorProcessPaletteSource<FTEDITOR_INSERT_PALETTE_SOURCE>(newBitmapAddr, contentInfo->MemoryAddress, m_MainWindow->cmdEditor());
	m_ContentList->setCurrentItem(contentInfo->View);
	m_MainWindow->propertiesEditor()->surpressSet(false);
	m_MainWindow->undoStack()->endMacro();
}

void ContentManager::propertiesImageFormatChanged(int value)
{
	printf("ContentManager::propertiesImageFormatChanged(value)\n");

	value = g_ImageFormatFromIntf[FTEDITOR_CURRENT_DEVICE][value % g_ImageFormatIntfNb[FTEDITOR_CURRENT_DEVICE]];

	if (current() && current()->ImageFormat != value)
		changeImageFormat(current(), value);
}

////////////////////////////////////////////////////////////////////////

class ContentManager::ChangeImageMono : public QUndoCommand
{
public:
	ChangeImageMono(ContentManager *contentManager, ContentInfo *contentInfo, int value) :
		QUndoCommand(),
		m_ContentManager(contentManager),
		m_ContentInfo(contentInfo),
		m_OldValue(contentInfo->ImageMono),
		m_NewValue(value)
	{
		setText(value ? tr("Mono image") : tr("Color image"));
	}

	virtual ~ChangeImageMono()
	{

	}

	virtual void undo()
	{
		m_ContentInfo->ImageMono = m_OldValue;
		m_ContentManager->reprocessInternal(m_ContentInfo);
	}

	virtual void redo()
	{
		m_ContentInfo->ImageMono = m_NewValue;
		m_ContentManager->reprocessInternal(m_ContentInfo);
	}

private:
	ContentManager *m_ContentManager;
	ContentInfo *m_ContentInfo;
	bool m_OldValue;
	bool m_NewValue;
};

void ContentManager::changeImageMono(ContentInfo *contentInfo, bool value)
{
	// Create undo/redo
	ChangeImageMono *changeImageMono = new ChangeImageMono(this, contentInfo, value);
	m_MainWindow->undoStack()->beginMacro(tr("Change mono"));
	m_MainWindow->undoStack()->push(changeImageMono);
	m_MainWindow->propertiesEditor()->surpressSet(true);
	editorUpdateHandle(contentInfo, m_MainWindow->dlEditor(), false);
	editorUpdateHandle(contentInfo, m_MainWindow->cmdEditor(), false);
	m_MainWindow->propertiesEditor()->surpressSet(false);
	m_MainWindow->undoStack()->endMacro();
}

void ContentManager::propertiesImageMonoChanged(int value)
{
	printf("ContentManager::propertiesImageMonoChanged(value)\n");

	if (current() && current()->ImageMono != (value == (int)Qt::Checked))
		changeImageMono(current(), (value == (int)Qt::Checked));
}

////////////////////////////////////////////////////////////////////////

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
		m_ContentManager->reuploadInternal(m_ContentInfo, true, false);
		m_ContentManager->recalculateOverlapMemoryInternal();
		m_ContentManager->reloadExternal(m_ContentInfo);
		if (m_ContentManager->m_CurrentPropertiesContent == m_ContentInfo)
			m_ContentManager->rebuildGUIInternal(m_ContentInfo);
	}

	virtual void redo()
	{
		m_ContentInfo->MemoryLoaded = m_NewValue;
		m_ContentManager->reuploadInternal(m_ContentInfo, true, false);
		m_ContentManager->recalculateOverlapMemoryInternal();
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

void ContentManager::propertiesMemoryLoadedChanged(int value)
{
	printf("ContentManager::propertiesMemoryLoadedChanged(value)\n");

	if (current() && current()->MemoryLoaded != (value == (int)Qt::Checked))
		changeMemoryLoaded(current(), (value == (int)Qt::Checked));
}

////////////////////////////////////////////////////////////////////////

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
		m_ContentManager->reuploadInternal(m_ContentInfo, true, false);
		m_ContentManager->recalculateOverlapMemoryInternal();
		if (m_ContentManager->m_CurrentPropertiesContent == m_ContentInfo)
			m_ContentManager->rebuildGUIInternal(m_ContentInfo);
	}

	virtual void redo()
	{
		m_ContentInfo->MemoryAddress = m_NewValue;
		m_ContentManager->reuploadInternal(m_ContentInfo, true, false);
		m_ContentManager->recalculateOverlapMemoryInternal();
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

void ContentManager::changeMemoryAddress(ContentInfo *contentInfo, int value, bool internal)
{
	// Create undo/redo
	value &= 0x7FFFFFFC;
	ChangeMemoryAddress *changeMemoryAddress = new ChangeMemoryAddress(this, contentInfo, value); // Force round to 4
	if (!internal)
	{
		m_MainWindow->undoStack()->beginMacro(tr("Change memory address"));
	}
	int oldMemoryAddr = contentInfo->MemoryAddress;
	int oldBitmapAddr = contentInfo->bitmapAddress();
	m_MainWindow->undoStack()->push(changeMemoryAddress);
	if (!internal)
	{
		m_MainWindow->propertiesEditor()->surpressSet(true);
		int newBitmapAddr = contentInfo->bitmapAddress();
		if (contentInfo->Converter == ContentInfo::Font)
		{
			editorUpdateFontAddress(value, oldMemoryAddr, m_MainWindow->dlEditor());
			editorUpdateFontAddress(value, oldMemoryAddr, m_MainWindow->cmdEditor());
		}
		if (requirePaletteAddress(contentInfo))
		{
			editorUpdatePaletteAddress(value, oldMemoryAddr, m_MainWindow->dlEditor());
			editorUpdatePaletteAddress(value, oldMemoryAddr, m_MainWindow->cmdEditor());
		}
		editorUpdateHandleAddress(newBitmapAddr, oldBitmapAddr, m_MainWindow->dlEditor());
		editorUpdateHandleAddress(newBitmapAddr, oldBitmapAddr, m_MainWindow->cmdEditor());
		m_ContentList->setCurrentItem(contentInfo->View);
		m_MainWindow->propertiesEditor()->surpressSet(false);
		m_MainWindow->undoStack()->endMacro();
	}
}

void ContentManager::propertiesMemoryAddressChanged(int value)
{
	printf("ContentManager::propertiesMemoryAddressChanged(value)\n");

	if (current() && current()->MemoryAddress != (value & 0x7FFFFFFC))
		changeMemoryAddress(current(), value);
	else if (current() && value != (value & 0x7FFFFFFC))
		rebuildGUIInternal(current());
}

////////////////////////////////////////////////////////////////////////

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
		if (m_ContentInfo->DataStorage == ContentInfo::Flash)
		{
			m_ContentManager->reuploadInternal(m_ContentInfo, false, true);
			m_ContentManager->recalculateOverlapFlashInternal();
		}
		if (m_ContentManager->m_CurrentPropertiesContent == m_ContentInfo)
			m_ContentManager->rebuildGUIInternal(m_ContentInfo);
	}

	virtual void redo()
	{
		m_ContentInfo->DataCompressed = m_NewValue;
		if (m_ContentInfo->DataStorage == ContentInfo::Flash)
		{
			m_ContentManager->reuploadInternal(m_ContentInfo, false, true);
			m_ContentManager->recalculateOverlapFlashInternal();
		}
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

void ContentManager::propertiesDataCompressedChanged(int value)
{
	printf("ContentManager::propertiesDataCompressedChanged(value)\n");

	if (current() && current()->DataCompressed != (value == (int)Qt::Checked))
		changeDataCompressed(current(), (value == (int)Qt::Checked));
}

////////////////////////////////////////////////////////////////////////

class ContentManager::ChangeDataStorage : public QUndoCommand
{
public:
	ChangeDataStorage(ContentManager *contentManager, ContentInfo *contentInfo, ContentInfo::StorageType value) :
		QUndoCommand(),
		m_ContentManager(contentManager),
		m_ContentInfo(contentInfo),
		m_OldValue(contentInfo->DataStorage),
		m_NewValue(value)
	{
		setText("Change data storage");
	}

	virtual ~ChangeDataStorage()
	{

	}

	virtual void undo()
	{
		m_ContentInfo->DataStorage = m_OldValue;
		if (m_ContentInfo->DataStorage == ContentInfo::Flash)
		{
			m_ContentManager->reuploadInternal(m_ContentInfo, false, true);
			m_ContentManager->recalculateOverlapFlashInternal();
		}
		if (m_ContentManager->m_CurrentPropertiesContent == m_ContentInfo)
			m_ContentManager->rebuildGUIInternal(m_ContentInfo);
	}

	virtual void redo()
	{
		m_ContentInfo->DataStorage = m_NewValue;
		if (m_ContentInfo->DataStorage == ContentInfo::Flash)
		{
			m_ContentManager->reuploadInternal(m_ContentInfo, false, true);
			m_ContentManager->recalculateOverlapFlashInternal();
		}
		if (m_ContentManager->m_CurrentPropertiesContent == m_ContentInfo)
			m_ContentManager->rebuildGUIInternal(m_ContentInfo);
	}

	virtual int id() const
	{
		return 9076474;
	}

	virtual bool mergeWith(const QUndoCommand *command)
	{
		const ChangeDataStorage *c = static_cast<const ChangeDataStorage *>(command);

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
	ContentInfo::StorageType m_OldValue;
	ContentInfo::StorageType m_NewValue;
};

void ContentManager::changeDataStorage(ContentInfo *contentInfo, ContentInfo::StorageType value)
{
	// Create undo/redo
	ChangeDataStorage *changeDataStorage = new ChangeDataStorage(this, contentInfo, value);
	m_MainWindow->undoStack()->push(changeDataStorage);
}

void ContentManager::propertiesDataStorageChanged(int value)
{
	printf("ContentManager::propertiesDataStorageChanged(value)\n");

	if (current() && current()->DataStorage != (ContentInfo::StorageType)value)
		changeDataStorage(current(), (ContentInfo::StorageType)value);
}

////////////////////////////////////////////////////////////////////////
/*

class ContentManager::ChangeFlashLoaded : public QUndoCommand
{
public:
	ChangeFlashLoaded(ContentManager *contentManager, ContentInfo *contentInfo, int value) :
		QUndoCommand(),
		m_ContentManager(contentManager),
		m_ContentInfo(contentInfo),
		m_OldValue(((contentInfo->DataStorage == ContentInfo::Flash)),
		m_NewValue(value)
	{
		setText(value ? tr("Load content to flash") : tr("Unload content from flash"));
	}

	virtual ~ChangeFlashLoaded()
	{

	}

	virtual void undo()
	{
		m_ContentInfo->FlashLoaded = m_OldValue;
		m_ContentManager->reuploadInternal(m_ContentInfo, true, false);
		m_ContentManager->recalculateOverlapFlashInternal();
		m_ContentManager->reloadExternal(m_ContentInfo);
		if (m_ContentManager->m_CurrentPropertiesContent == m_ContentInfo)
			m_ContentManager->rebuildGUIInternal(m_ContentInfo);
	}

	virtual void redo()
	{
		m_ContentInfo->FlashLoaded = m_NewValue;
		m_ContentManager->reuploadInternal(m_ContentInfo, true, false);
		m_ContentManager->recalculateOverlapFlashInternal();
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

void ContentManager::changeFlashLoaded(ContentInfo *contentInfo, bool value)
{
	// Create undo/redo
	ChangeFlashLoaded *changeFlashLoaded = new ChangeFlashLoaded(this, contentInfo, value);
	m_MainWindow->undoStack()->push(changeFlashLoaded);
}

void ContentManager::propertiesFlashLoadedChanged(int value)
{
	printf("ContentManager::propertiesFlashLoadedChanged(value)\n");

	if (current() && current()->FlashLoaded != (value == (int)Qt::Checked))
		changeFlashLoaded(current(), (value == (int)Qt::Checked));
}
*/
////////////////////////////////////////////////////////////////////////

class ContentManager::ChangeFlashAddress : public QUndoCommand
{
public:
	ChangeFlashAddress(ContentManager *contentManager, ContentInfo *contentInfo, int value) :
		QUndoCommand(),
		m_ContentManager(contentManager),
		m_ContentInfo(contentInfo),
		m_OldValue(contentInfo->FlashAddress),
		m_NewValue(value)
	{
		setText(tr("Change flash address"));
	}

	virtual ~ChangeFlashAddress()
	{

	}

	virtual void undo()
	{
		m_ContentInfo->FlashAddress = m_OldValue;
		m_ContentManager->reuploadInternal(m_ContentInfo, false, true);
		m_ContentManager->recalculateOverlapFlashInternal();
		if (m_ContentManager->m_CurrentPropertiesContent == m_ContentInfo)
			m_ContentManager->rebuildGUIInternal(m_ContentInfo);
	}

	virtual void redo()
	{
		m_ContentInfo->FlashAddress = m_NewValue;
		m_ContentManager->reuploadInternal(m_ContentInfo, false, true);
		m_ContentManager->recalculateOverlapFlashInternal();
		if (m_ContentManager->m_CurrentPropertiesContent == m_ContentInfo)
			m_ContentManager->rebuildGUIInternal(m_ContentInfo);
	}

	virtual int id() const
	{
		return 9074404;
	}

	virtual bool mergeWith(const QUndoCommand *command)
	{
		const ChangeFlashAddress *c = static_cast<const ChangeFlashAddress *>(command);

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

void ContentManager::changeFlashAddress(ContentInfo *contentInfo, int value, bool internal)
{
	// Create undo/redo
	value &= (0x7FFFFF << 5);
	int editorAddress = 0x800000 | ((value >> 5) & 0x7FFFFF);
	ChangeFlashAddress *changeFlashAddress = new ChangeFlashAddress(this, contentInfo, value); // Force round to 4
	if (!internal)
	{
		m_MainWindow->undoStack()->beginMacro(tr("Change flash address"));
	}
	int oldEditorAddr = 0x800000 | ((contentInfo->FlashAddress >> 5) & 0x7FFFFF);
	m_MainWindow->undoStack()->push(changeFlashAddress);
	if (!internal)
	{
		m_MainWindow->propertiesEditor()->surpressSet(true);
		editorUpdateHandleAddress(editorAddress, oldEditorAddr, m_MainWindow->dlEditor());
		editorUpdateHandleAddress(editorAddress, oldEditorAddr, m_MainWindow->cmdEditor());
		m_ContentList->setCurrentItem(contentInfo->View);
		m_MainWindow->propertiesEditor()->surpressSet(false);
		m_MainWindow->undoStack()->endMacro();
	}
}

void ContentManager::propertiesFlashAddressChanged(int value)
{
	printf("ContentManager::propertiesFlashAddressChanged(value)\n");

	if (current() && current()->FlashAddress != (value & (0x7FFFFF << 5)))
		changeFlashAddress(current(), value);
	else if (current() && value != (value & (0x7FFFFF << 5)))
		rebuildGUIInternal(current());
}

////////////////////////////////////////////////////////////////////////

class ContentManager::ChangeRawStart : public QUndoCommand
{
public:
	ChangeRawStart(ContentManager *contentManager, ContentInfo *contentInfo, int value) :
		QUndoCommand(),
		m_ContentManager(contentManager),
		m_ContentInfo(contentInfo),
		m_OldValue(contentInfo->RawStart),
		m_NewValue(value)
	{
		setText(tr("Change raw start"));
	}

	virtual ~ChangeRawStart()
	{

	}

	virtual void undo()
	{
		m_ContentInfo->RawStart = m_OldValue;
		m_ContentManager->reprocessInternal(m_ContentInfo);
	}

	virtual void redo()
	{
		m_ContentInfo->RawStart = m_NewValue;
		m_ContentManager->reprocessInternal(m_ContentInfo);
	}

	virtual int id() const
	{
		return 9064411;
	}

	virtual bool mergeWith(const QUndoCommand *command)
	{
		const ChangeRawStart *c = static_cast<const ChangeRawStart *>(command);

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

void ContentManager::changeRawStart(ContentInfo *contentInfo, int value)
{
	// Create undo/redo
	ChangeRawStart *changeRawStart = new ChangeRawStart(this, contentInfo, value & 0x7FFFFFFC); // Force round to 4
	m_MainWindow->undoStack()->push(changeRawStart);
}

void ContentManager::propertiesRawStartChanged(int value)
{
	printf("ContentManager::propertiesRawStartChanged(value)\n");

	if (current() && current()->RawStart != (value & 0x7FFFFFFC))
		changeRawStart(current(), value);
	else if (current() && value != (value & 0x7FFFFFFC))
		rebuildGUIInternal(current());
}

////////////////////////////////////////////////////////////////////////

class ContentManager::ChangeRawLength : public QUndoCommand
{
public:
	ChangeRawLength(ContentManager *contentManager, ContentInfo *contentInfo, int value) :
		QUndoCommand(),
		m_ContentManager(contentManager),
		m_ContentInfo(contentInfo),
		m_OldValue(contentInfo->RawLength),
		m_NewValue(value)
	{
		setText(tr("Change raw length"));
	}

	virtual ~ChangeRawLength()
	{

	}

	virtual void undo()
	{
		m_ContentInfo->RawLength = m_OldValue;
		m_ContentManager->reprocessInternal(m_ContentInfo);
	}

	virtual void redo()
	{
		m_ContentInfo->RawLength = m_NewValue;
		m_ContentManager->reprocessInternal(m_ContentInfo);
	}

	virtual int id() const
	{
		return 9064412;
	}

	virtual bool mergeWith(const QUndoCommand *command)
	{
		const ChangeRawLength *c = static_cast<const ChangeRawLength *>(command);

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

void ContentManager::changeRawLength(ContentInfo *contentInfo, int value)
{
	// Create undo/redo
	ChangeRawLength *changeRawLength = new ChangeRawLength(this, contentInfo, value & 0x7FFFFFFC); // Force round to 4
	m_MainWindow->undoStack()->push(changeRawLength);
}

void ContentManager::propertiesRawLengthChanged(int value)
{
	printf("ContentManager::propertiesRawLengthChanged(value)\n");

	if (current() && current()->RawLength != (value & 0x7FFFFFFC))
		changeRawLength(current(), value);
	else if (current() && value != (value & 0x7FFFFFFC))
		rebuildGUIInternal(current());
}

////////////////////////////////////////////////////////////////////////

class ContentManager::ChangeFontFormat : public QUndoCommand
{
public:
	ChangeFontFormat(ContentManager *contentManager, ContentInfo *contentInfo, int value) :
		QUndoCommand(),
		m_ContentManager(contentManager),
		m_ContentInfo(contentInfo),
		m_OldValue(contentInfo->ImageFormat),
		m_NewValue(value)
	{
		setText(tr("Change font format"));
	}

	virtual ~ChangeFontFormat()
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
		return 9064413;
	}

	virtual bool mergeWith(const QUndoCommand *command)
	{
		const ChangeFontFormat *c = static_cast<const ChangeFontFormat *>(command);

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

void ContentManager::changeFontFormat(ContentInfo *contentInfo, int value)
{
	// Create undo/redo
	ChangeFontFormat *changeFontFormat = new ChangeFontFormat(this, contentInfo, value);
	m_MainWindow->undoStack()->beginMacro(tr("Change font format"));
	int oldBitmapAddr = contentInfo->bitmapAddress();
	m_MainWindow->undoStack()->push(changeFontFormat);
	m_MainWindow->propertiesEditor()->surpressSet(true);
	int newBitmapAddr = contentInfo->bitmapAddress();
	if (newBitmapAddr != oldBitmapAddr)
	{
		editorUpdateHandleAddress(newBitmapAddr, oldBitmapAddr, m_MainWindow->dlEditor());
		editorUpdateHandleAddress(newBitmapAddr, oldBitmapAddr, m_MainWindow->cmdEditor());
	}
	editorUpdateHandle(contentInfo, m_MainWindow->dlEditor(), true);
	editorUpdateHandle(contentInfo, m_MainWindow->cmdEditor(), true);
	m_ContentList->setCurrentItem(contentInfo->View);
	m_MainWindow->propertiesEditor()->surpressSet(false);
	m_MainWindow->undoStack()->endMacro();
}

void ContentManager::propertiesFontFormatChanged(int value)
{
	printf("ContentManager::propertiesFontFormatChanged(value)\n");

	value = g_FontFormatFromIntf[FTEDITOR_CURRENT_DEVICE][value % g_FontFormatIntfNb[FTEDITOR_CURRENT_DEVICE]];

	if (current() && current()->ImageFormat != (value))
		changeFontFormat(current(), (value));
}

////////////////////////////////////////////////////////////////////////

class ContentManager::ChangeFontSize : public QUndoCommand
{
public:
	ChangeFontSize(ContentManager *contentManager, ContentInfo *contentInfo, int value) :
		QUndoCommand(),
		m_ContentManager(contentManager),
		m_ContentInfo(contentInfo),
		m_OldValue(contentInfo->FontSize),
		m_NewValue(value)
	{
		setText(tr("Change font size"));
	}

	virtual ~ChangeFontSize()
	{

	}

	virtual void undo()
	{
		m_ContentInfo->FontSize = m_OldValue;
		m_ContentManager->reprocessInternal(m_ContentInfo);
	}

	virtual void redo()
	{
		m_ContentInfo->FontSize = m_NewValue;
		m_ContentManager->reprocessInternal(m_ContentInfo);
	}

	virtual int id() const
	{
		return 9064414;
	}

	virtual bool mergeWith(const QUndoCommand *command)
	{
		const ChangeFontSize *c = static_cast<const ChangeFontSize *>(command);

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

void ContentManager::changeFontSize(ContentInfo *contentInfo, int value)
{
	// Create undo/redo
	ChangeFontSize *changeFontSize = new ChangeFontSize(this, contentInfo, value);
	m_MainWindow->undoStack()->beginMacro(tr("Change font size"));
	int oldBitmapAddr = contentInfo->bitmapAddress();
	m_MainWindow->undoStack()->push(changeFontSize);
	m_MainWindow->propertiesEditor()->surpressSet(true);
	int newBitmapAddr = contentInfo->bitmapAddress();
	if (newBitmapAddr != oldBitmapAddr)
	{
		editorUpdateHandleAddress(newBitmapAddr, oldBitmapAddr, m_MainWindow->dlEditor());
		editorUpdateHandleAddress(newBitmapAddr, oldBitmapAddr, m_MainWindow->cmdEditor());
	}
	editorUpdateHandle(contentInfo, m_MainWindow->dlEditor(), true);
	editorUpdateHandle(contentInfo, m_MainWindow->cmdEditor(), true);
	m_ContentList->setCurrentItem(contentInfo->View);
	m_MainWindow->propertiesEditor()->surpressSet(false);
	m_MainWindow->undoStack()->endMacro();
}

void ContentManager::propertiesFontSizeChanged(int value)
{
	printf("ContentManager::propertiesFontSizeChanged(value)\n");

	if (current() && current()->FontSize != value)
		changeFontSize(current(), value);
}

////////////////////////////////////////////////////////////////////////

class ContentManager::ChangeFontCharSet : public QUndoCommand
{
public:
	ChangeFontCharSet(ContentManager *contentManager, ContentInfo *contentInfo, const QString &value) :
		QUndoCommand(),
		m_ContentManager(contentManager),
		m_ContentInfo(contentInfo),
		m_OldValue(contentInfo->FontCharSet),
		m_NewValue(value)
	{
		setText(tr("Change font charset"));
	}

	virtual ~ChangeFontCharSet()
	{

	}

public:
	virtual void undo()
	{
		m_ContentInfo->FontCharSet = m_OldValue;
		m_ContentManager->reprocessInternal(m_ContentInfo);
	}

	virtual void redo()
	{
		m_ContentInfo->FontCharSet = m_NewValue;
		m_ContentManager->reprocessInternal(m_ContentInfo);
	}

	virtual int id() const
	{
		return 9064415;
	}

	virtual bool mergeWith(const QUndoCommand *command)
	{
		const ChangeFontCharSet *c = static_cast<const ChangeFontCharSet *>(command);

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

void ContentManager::changeFontCharSet(ContentInfo *contentInfo, const QString &value)
{
	// Create undo/redo
	if (contentInfo->FontCharSet != value)
	{
		ChangeFontCharSet *changeFontCharSet = new ChangeFontCharSet(this, contentInfo, value);
		m_MainWindow->undoStack()->beginMacro(tr("Change font charset"));
		int oldBitmapAddr = contentInfo->bitmapAddress();
		m_MainWindow->undoStack()->push(changeFontCharSet);
		m_MainWindow->propertiesEditor()->surpressSet(true);
		int newBitmapAddr = contentInfo->bitmapAddress();
		if (newBitmapAddr != oldBitmapAddr)
		{
			editorUpdateHandleAddress(newBitmapAddr, oldBitmapAddr, m_MainWindow->dlEditor());
			editorUpdateHandleAddress(newBitmapAddr, oldBitmapAddr, m_MainWindow->cmdEditor());
		}
		editorUpdateHandle(contentInfo, m_MainWindow->dlEditor(), true);
		editorUpdateHandle(contentInfo, m_MainWindow->cmdEditor(), true);
		m_ContentList->setCurrentItem(contentInfo->View);
		m_MainWindow->propertiesEditor()->surpressSet(false);
		m_MainWindow->undoStack()->endMacro();
	}
}

void ContentManager::propertiesFontCharSetChanged()
{
	printf("ContentManager::propertiesCommonFontCharSetChanged(value)\n");

	if (current())
		changeFontCharSet(current(), m_PropertiesFontCharSet->text());
}

////////////////////////////////////////////////////////////////////////

class ContentManager::ChangeFontOffset : public QUndoCommand
{
public:
	ChangeFontOffset(ContentManager *contentManager, ContentInfo *contentInfo, int value) :
		QUndoCommand(),
		m_ContentManager(contentManager),
		m_ContentInfo(contentInfo),
		m_OldValue(contentInfo->FontOffset),
		m_NewValue(value)
	{
		setText(tr("Change font size"));
	}

	virtual ~ChangeFontOffset()
	{

	}

	virtual void undo()
	{
		m_ContentInfo->FontOffset = m_OldValue;
		m_ContentManager->reprocessInternal(m_ContentInfo);
	}

	virtual void redo()
	{
		m_ContentInfo->FontOffset = m_NewValue;
		m_ContentManager->reprocessInternal(m_ContentInfo);
	}

	virtual int id() const
	{
		return 9064416;
	}

	virtual bool mergeWith(const QUndoCommand *command)
	{
		const ChangeFontOffset *c = static_cast<const ChangeFontOffset *>(command);

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

void ContentManager::changeFontOffset(ContentInfo *contentInfo, int value)
{
	// Create undo/redo
	ChangeFontOffset *changeFontOffset = new ChangeFontOffset(this, contentInfo, value);
	m_MainWindow->undoStack()->beginMacro(tr("Change font size"));
	int oldBitmapAddr = contentInfo->bitmapAddress();
	m_MainWindow->undoStack()->push(changeFontOffset);
	m_MainWindow->propertiesEditor()->surpressSet(true);
	int newBitmapAddr = contentInfo->bitmapAddress();
	if (newBitmapAddr != oldBitmapAddr)
	{
		editorUpdateHandleAddress(newBitmapAddr, oldBitmapAddr, m_MainWindow->dlEditor());
		editorUpdateHandleAddress(newBitmapAddr, oldBitmapAddr, m_MainWindow->cmdEditor());
	}
	editorUpdateHandle(contentInfo, m_MainWindow->dlEditor(), true);
	editorUpdateHandle(contentInfo, m_MainWindow->cmdEditor(), true);
	editorUpdateFontOffset(contentInfo, m_MainWindow->dlEditor());
	editorUpdateFontOffset(contentInfo, m_MainWindow->cmdEditor());
	m_ContentList->setCurrentItem(contentInfo->View);
	m_MainWindow->propertiesEditor()->surpressSet(false);
	m_MainWindow->undoStack()->endMacro();
}

void ContentManager::propertiesFontOffsetChanged(int value)
{
	printf("ContentManager::propertiesFontOffsetChanged(value)\n");

	if (current() && current()->FontOffset != value)
		changeFontOffset(current(), value);
}

////////////////////////////////////////////////////////////////////////

} /* namespace FTEDITOR */

/* end of file */
