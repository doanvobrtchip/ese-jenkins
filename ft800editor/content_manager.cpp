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

// Emulator includes
#include <ft800emu_graphics_processor.h>
#include <ft800emu_vc.h>

// Project includes
#include "main_window.h"
#include "properties_editor.h"
#include "undo_stack_disabler.h"
#include "asset_converter.h"
#include "dl_editor.h"
#include "dl_parser.h"
#include "constant_mapping.h"

#define FT810EMU_BITMAP_ALWAYS_HIGH 1

using namespace std;

namespace FT800EMUQT {

namespace /* anonymous */ {

///////////////////////////////////////////////////////////////////////

#ifdef FT810EMU_MODE
static const int s_ImageFormatToUI[] = {
	0, // 0: ARGB1555
	1, // 1: L1
	3, // 2: L4
	4, // 3: L8
	5, // 4: RGB332
	6, // 5: ARGB2
	7, // 6: ARGB4
	8, // 7: RGB565
	11, // 8: PALETTED
	0, // 9
	0, // 10
	0, // 11
	0, // 12
	0, // 13
	9, // 14: PALETTED565
	10, // 15: PALETTED4444
	11, // 16: PALETTED8
	2, // 17: L2
};
#else
static const int s_ImageFormatToUI[] = {
	0, // ARGB1555
	1, // L1
	2, // L4
	3, // L8
	4, // RGB332
	5, // ARGB2
	6, // ARGB4
	7, // RGB565
	8, // PALETTED
};
#endif

static const int s_ImageFormatFromUI[] = {
	0, // ARGB1555
	1, // L1
#ifdef FT810EMU_MODE
	17, // L2
#endif
	2, // L4
	3, // L8
	4, // RGB332
	5, // ARGB2
	6, // ARGB4
	7, // RGB565
#ifdef FT810EMU_MODE
	14, // PALETTED565
	15, // PALETTED4444
	16, // PALETTED8
#else
	8, // PALETTED
#endif
};

///////////////////////////////////////////////////////////////////////

static const char *s_BitmapFormatNames[] = {
	"ARGB1555",
	"L1",
	"L4",
	"L8",
	"RGB332",
	"ARGB2",
	"ARGB4",
	"RGB565",
	"PALETTED",
#ifdef FT810EMU_MODE
	"9",
	"10",
	"11",
	"12",
	"13",
	"PALETTED565",
	"PALETTED4444",
	"PALETTED8",
	"L2",
#endif
};

///////////////////////////////////////////////////////////////////////

#ifdef FT810EMU_MODE
static const int s_FontFormatToUI[] = {
	1, // 0: ARGB1555
	0, // 1: L1
	2, // 2: L4
	3, // 3: L8
	1, // 4: RGB332
	1, // 5: ARGB2
	1, // 6: ARGB4
	1, // 7: RGB565
	1, // 8: PALETTED
	1, // 9
	1, // 10
	1, // 11
	1, // 12
	1, // 13
	1, // 14: PALETTED565
	1, // 15: PALETTED4444
	1, // 16: PALETTED8
	1, // 17: L2
};
#else
static const int s_FontFormatToUI[] = {
	1, // ARGB1555
	0, // L1
	1, // L4
	2, // L8
	1, // RGB332
	1, // ARGB2
	1, // ARGB4
	1, // RGB565
	1, // PALETTED
};
#endif

static const int s_FontFormatFromUI[] = {
	1, // L1
#ifdef FT810EMU_MODE
	17, // L2
#endif
	2, // L4
	3, // L8
};

///////////////////////////////////////////////////////////////////////

} /* anonymous namespace */

int g_RamGlobalUsage = 0;

std::vector<QString> ContentManager::s_FileExtensions;
QMutex ContentManager::s_Mutex;

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
	FontSize = 12;
	FontCharSet = "                                 !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~";
	UploadDirty = true;
	ExternalDirty = false;
	CachedImage = false;
	CachedSize = 0;
	OverlapFlag = false;
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
	case Font:
		j["converter"] = QString("Font");
		j["imageFormat"] = ImageFormat;
		j["fontSize"] = FontSize;
		j["fontCharSet"] = FontCharSet;
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
	else if (converter == "Font")
	{
		Converter = Font;
		ImageFormat = ((QJsonValue)j["imageFormat"]).toVariant().toInt();
		FontSize = ((QJsonValue)j["fontSize"]).toVariant().toInt();
		FontCharSet = j["fontCharSet"].toString();
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
	case Font:
		if (ImageFormat != other->ImageFormat)
			return false;
		if (FontSize != other->FontSize)
			return false;
		if (FontCharSet != other->FontCharSet)
			return false;
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
	// propCommonConverter->addItem(tr("Raw JPEG")); // TODO
	addLabeledWidget(this, propCommonLayout, tr("Converter: "), propCommonConverter);
	connect(m_PropertiesCommonConverter, SIGNAL(currentIndexChanged(int)), this, SLOT(propertiesCommonConverterChanged(int)));
	propCommon->setLayout(propCommonLayout);

	// Image
	m_PropertiesImage = new QGroupBox(this);
	m_PropertiesImage->setHidden(true);
	m_PropertiesImage->setTitle(tr("Image Settings"));
	QVBoxLayout *imagePropsLayout = new QVBoxLayout();
	m_PropertiesImageFormat = new QComboBox(this);
	for (int i = 0; i < sizeof(s_ImageFormatFromUI) / sizeof(s_ImageFormatFromUI[0]); ++i)
		m_PropertiesImageFormat->addItem(s_BitmapFormatNames[s_ImageFormatFromUI[i]]);
	/*m_PropertiesImageFormat->addItem("ARGB1555");
	m_PropertiesImageFormat->addItem("L1");
	m_PropertiesImageFormat->addItem("L4");
	m_PropertiesImageFormat->addItem("L8");
	m_PropertiesImageFormat->addItem("RGB332");
	m_PropertiesImageFormat->addItem("ARGB2");
	m_PropertiesImageFormat->addItem("ARGB4");
	m_PropertiesImageFormat->addItem("RGB565");
	m_PropertiesImageFormat->addItem("PALETTED");*/
	addLabeledWidget(this, imagePropsLayout, tr("Format: "), m_PropertiesImageFormat);
	connect(m_PropertiesImageFormat, SIGNAL(currentIndexChanged(int)), this, SLOT(propertiesImageFormatChanged(int)));
	m_PropertiesImage->setLayout(imagePropsLayout);

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
	for (int i = 0; i < sizeof(s_FontFormatFromUI) / sizeof(s_FontFormatFromUI[0]); ++i)
		m_PropertiesFontFormat->addItem(s_BitmapFormatNames[s_FontFormatFromUI[i]]);
	/*
	m_PropertiesFontFormat->addItem("L1");
	m_PropertiesFontFormat->addItem("L4");
	m_PropertiesFontFormat->addItem("L8");
	*/
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
	m_PropertiesRawLength->setMaximum(RAM_G_MAX);
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
	m_PropertiesMemoryAddress->setMaximum(RAM_G_MAX - 4);
	m_PropertiesMemoryAddress->setSingleStep(4);
	m_PropertiesMemoryAddress->setKeyboardTracking(false);
	addLabeledWidget(this, propMemLayout, tr("Address: "), m_PropertiesMemoryAddress);
	connect(m_PropertiesMemoryAddress, SIGNAL(valueChanged(int)), this, SLOT(propertiesMemoryAddressChanged(int)));
	m_PropertiesMemoryLoaded = new QCheckBox(this);
	addLabeledWidget(this, propMemLayout, tr("Loaded: "), m_PropertiesMemoryLoaded);
	connect(m_PropertiesMemoryLoaded, SIGNAL(stateChanged(int)), this, SLOT(propertiesMemoryLoadedChanged(int)));
	QFrame* line = new QFrame();
	line->setFrameShape(QFrame::HLine);
	line->setFrameShadow(QFrame::Sunken);
	propMemLayout->addWidget(line);
	m_PropertiesDataCompressed = new QCheckBox(this);
	addLabeledWidget(this, propMemLayout, tr("Compressed: "), m_PropertiesDataCompressed);
	connect(m_PropertiesDataCompressed, SIGNAL(stateChanged(int)), this, SLOT(propertiesDataCompressedChanged(int)));
	m_PropertiesDataEmbedded = new QCheckBox(this);
	addLabeledWidget(this, propMemLayout, tr("Embedded: "), m_PropertiesDataEmbedded);
	connect(m_PropertiesDataEmbedded, SIGNAL(stateChanged(int)), this, SLOT(propertiesDataEmbeddedChanged(int)));
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
		ContentInfo *info = (ContentInfo *)(void *)(*it)->data(0, Qt::UserRole).value<quintptr>();
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
		contentInfo->ImageFormat = L4;
	}

	if (contentInfo->Converter == ContentInfo::Invalid)
	{
		contentInfo->WantAutoLoad = true;
	}
	else
	{
		int freeAddress = getFreeAddress();
		if (freeAddress >= 0)
		{
			contentInfo->MemoryLoaded = true;
			contentInfo->MemoryAddress = freeAddress;
		}
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

int ContentManager::getFreeAddress()
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
	if (freeAddress > RAM_G_MAX)
		return -1;
	return freeAddress;
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
	contentInfo->UploadDirty = true;

	// Make sure not in content
	lockContent();
	if (m_ContentUploadDirty.find(contentInfo) != m_ContentUploadDirty.end())
		m_ContentUploadDirty.erase(contentInfo);
	unlockContent();

	// Recalculate overlap
	if (m_ContentOverlap.find(contentInfo) != m_ContentOverlap.end())
		m_ContentOverlap.erase(contentInfo);
	contentInfo->OverlapFlag = false;
	recalculateOverlapInternal();

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

	if (fileName.isNull())
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

int ContentManager::getContentSize(ContentInfo *contentInfo)
{
	QString fileName = contentInfo->DestName + ".raw";
	QFileInfo binFile(fileName);
	if (!binFile.exists()) return -1;
	return binFile.size();
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
	// contentInfo->View->setText(0, contentInfo->SourcePath);
	contentInfo->View->setText(0, contentInfo->BuildError.isEmpty() ? (contentInfo->MemoryLoaded ? (contentInfo->OverlapFlag ? "Overlap" : "Loaded") : "") : "Error");
	contentInfo->View->setIcon(0, contentInfo->BuildError.isEmpty() ? (contentInfo->MemoryLoaded ? (contentInfo->OverlapFlag ? QIcon(":/icons/exclamation-red.png") : QIcon(":/icons/tick")) : QIcon()) : QIcon(":/icons/exclamation-red.png"));
	contentInfo->View->setText(1, contentInfo->DestName);
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
		widgets.push_back(m_PropertiesRaw);
		widgets.push_back(m_PropertiesFont);
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
		m_PropertiesImageFormat->setCurrentIndex(s_ImageFormatToUI[contentInfo->ImageFormat % (sizeof(s_ImageFormatToUI) / sizeof(s_ImageFormatToUI[0]))]);
		break;
	case ContentInfo::Raw:
		m_PropertiesRawStart->setValue(contentInfo->RawStart);
		m_PropertiesRawLength->setValue(contentInfo->RawLength);
		break;
	case ContentInfo::Font:
		m_PropertiesFontFormat->setCurrentIndex(s_FontFormatToUI[contentInfo->ImageFormat % (sizeof(s_FontFormatToUI) / sizeof(s_FontFormatToUI[0]))]);
		m_PropertiesFontSize->setValue(contentInfo->FontSize);
		m_PropertiesFontCharSet->setText(contentInfo->FontCharSet); // NOTE: Number of characters up to 128; export depends on number of characters in charset also!
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
		m_PropertiesRaw->setHidden(true);
		m_PropertiesFont->setHidden(true);
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
				bool loadSuccess = pixmap.load(contentInfo->DestName + "_converted-fs8.png") ||
					pixmap.load(contentInfo->DestName + "_converted.png");
				m_PropertiesImagePreview->setHidden(!loadSuccess);
				m_PropertiesImageLabel->setPixmap(pixmap.scaled(m_PropertiesImageLabel->width() - 32, m_PropertiesImageLabel->width() - 32, Qt::KeepAspectRatio));
				if (loadSuccess) m_PropertiesImageLabel->repaint();
				else { if (!propInfo.isEmpty()) propInfo += "<br>"; propInfo += tr("<b>Error</b>: Failed to load image preview."); }
				m_PropertiesRaw->setHidden(true);
				m_PropertiesFont->setHidden(true);
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
				m_PropertiesRaw->setHidden(false);
				m_PropertiesFont->setHidden(true);
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
				m_PropertiesRaw->setHidden(false);
				m_PropertiesFont->setHidden(true);
				m_PropertiesMemory->setHidden(false);
				if (!propInfo.isEmpty()) propInfo += "<br>";
				propInfo += tr("<b>Not yet implemented</b>");
				// This will show JPG size, uncompressed size, and necessary info to load the image into the handles
				break;
			}
			case ContentInfo::Font:
			{
				m_PropertiesImage->setHidden(true);
				m_PropertiesImagePreview->setHidden(true);
				m_PropertiesRaw->setHidden(true);
				m_PropertiesFont->setHidden(false);
				m_PropertiesMemory->setHidden(false);
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
				contentInfo->CachedSize = 0;
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
					case ContentInfo::Font:
						AssetConverter::convertFont(contentInfo->BuildError, contentInfo->SourcePath, contentInfo->DestName, contentInfo->ImageFormat, contentInfo->FontSize, contentInfo->FontCharSet);
						break;
					default:
						contentInfo->BuildError = "<i>(Critical Error)</i><br>Unknown converter selected.";
						break;
					}
				}
				if (contentInfo->BuildError.isEmpty())
				{
					// Cache size
					contentInfo->CachedSize = getContentSize(contentInfo);

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
		recalculateOverlapInternal();
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
		if (contentInfo->Converter == ContentInfo::Image
			|| contentInfo->Converter == ContentInfo::Font)
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

void ContentManager::reuploadAll()
{
	printf("ContentManager::reuploadAll()\n");

	for (QTreeWidgetItemIterator it(m_ContentList); *it; ++it)
	{
		ContentInfo *info = (ContentInfo *)(void *)(*it)->data(0, Qt::UserRole).value<quintptr>();
		reuploadInternal(info);
	}
}

void ContentManager::recalculateOverlapInternal()
{
	printf("ContentManager::recalculateOverlapInternal()\n");

	std::set<ContentInfo *> contentOverlap;
	m_ContentOverlap.swap(contentOverlap);

	int ramGlobalUsage = 0;

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
				if (leftAddr + leftSize > RAM_G_MAX)
				{
					printf("CM: Content '%s' oversize\n", leftInfo->DestName.toLocal8Bit().data());
					if (m_ContentOverlap.find(leftInfo) == m_ContentOverlap.end())
						m_ContentOverlap.insert(leftInfo);
					++left;
				}
				else
				{
					ramGlobalUsage += leftSize;
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
										if (m_ContentOverlap.find(leftInfo) == m_ContentOverlap.end())
											m_ContentOverlap.insert(leftInfo);
										if (m_ContentOverlap.find(rightInfo) == m_ContentOverlap.end())
											m_ContentOverlap.insert(rightInfo);
										ramGlobalUsage -= (leftAddr + leftSize - rightAddr);
									}
								}
								else if (rightAddr < leftAddr)
								{
									if (rightAddr + rightSize > leftAddr)
									{
										// overlap
										printf("CM: Content '%s' overlap with '%s'\n", leftInfo->DestName.toLocal8Bit().data(), rightInfo->DestName.toLocal8Bit().data());
										if (m_ContentOverlap.find(leftInfo) == m_ContentOverlap.end())
											m_ContentOverlap.insert(leftInfo);
										if (m_ContentOverlap.find(rightInfo) == m_ContentOverlap.end())
											m_ContentOverlap.insert(rightInfo);
										ramGlobalUsage -= (rightAddr + rightSize - leftAddr);
									}
								}
								else
								{
									// overlap
									printf("CM: Content '%s' overlap with '%s'\n", leftInfo->DestName.toLocal8Bit().data(), rightInfo->DestName.toLocal8Bit().data());
									if (m_ContentOverlap.find(leftInfo) == m_ContentOverlap.end())
										m_ContentOverlap.insert(leftInfo);
									if (m_ContentOverlap.find(rightInfo) == m_ContentOverlap.end())
										m_ContentOverlap.insert(rightInfo);
									if (leftSize > rightSize)
										ramGlobalUsage -= rightSize;
									else ramGlobalUsage -= leftSize;
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
		if (m_ContentOverlap.find(*it) == m_ContentOverlap.end())
		{
			// Content no longer overlaps
			printf("CM: Content '%s' no longer overlapping\n", (*it)->DestName.toLocal8Bit().data());

			// Update overlap GUI
			(*it)->OverlapFlag = false;
			rebuildViewInternal(*it);

			// Repuload
			reuploadInternal(*it);
		}
	}

	for (std::set<ContentInfo *>::iterator it(m_ContentOverlap.begin()), end(m_ContentOverlap.end()); it != end; ++it)
	{
		if (contentOverlap.find(*it) == contentOverlap.end())
		{
			// Newly overlapping content
			printf("CM: Content '%s' is overlapping\n", (*it)->DestName.toLocal8Bit().data());

			// Update overlap GUI
			(*it)->OverlapFlag = true;
			rebuildViewInternal(*it);
		}
	}

	g_RamGlobalUsage = ramGlobalUsage;
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

int ContentManager::editorFindHandle(ContentInfo *contentInfo, DlEditor *dlEditor)
{
	int handle = -1;
	for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
	{
		const DlParsed &parsed = dlEditor->getLine(i);
		if (parsed.ValidId)
		{
#ifdef FT810EMU_MODE
			if (parsed.IdLeft == 0xFFFFFF00)
			{
				switch (parsed.IdRight)
				{
					case CMD_SETBITMAP & 0xFF:
						if (parsed.Parameter[0].U == (contentInfo->Converter == ContentInfo::Font ? contentInfo->MemoryAddress + 148 : contentInfo->MemoryAddress) && handle != -1)
							return handle;
						break;
					// TODO: CMD_SETFONT2: Address is in RAM (can be calculated too)
				}
			}
#endif
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
					if (parsed.Parameter[0].U == (contentInfo->Converter == ContentInfo::Font ? contentInfo->MemoryAddress + 148 : contentInfo->MemoryAddress) && handle != -1)
						return handle;
					break;
				case FTEDITOR_DL_BITMAP_LAYOUT:
				case FTEDITOR_DL_BITMAP_SIZE:
#ifdef FT810EMU_MODE
				case FTEDITOR_DL_BITMAP_LAYOUT_H:
				case FTEDITOR_DL_BITMAP_SIZE_H:
#endif
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
#ifdef FT810EMU_MODE
			if (parsed.IdLeft == 0xFFFFFF00)
			{
				switch (parsed.IdRight)
				{
					case CMD_SETBITMAP & 0xFF:
						if (parsed.Parameter[0].U == (contentInfo->Converter == ContentInfo::Font ? contentInfo->MemoryAddress + 148 : contentInfo->MemoryAddress) && handle != -1)
							return handle;
						break;
					// TODO: CMD_SETFONT2: Address is in RAM (can be calculated too)
				}
			}
#endif
			if (parsed.IdLeft != 0)
				continue;
			switch (parsed.IdRight)
			{
				case FTEDITOR_DL_BITMAP_HANDLE:
					line = i;
					handle = parsed.Parameter[0].I;
					break;
				case FTEDITOR_DL_BITMAP_SOURCE:
					if (parsed.Parameter[0].U == (contentInfo->Converter == ContentInfo::Font ? contentInfo->MemoryAddress + 148 : contentInfo->MemoryAddress) && handle != -1)
						return handle;
					break;
				case FTEDITOR_DL_BITMAP_LAYOUT:
				case FTEDITOR_DL_BITMAP_SIZE:
#ifdef FT810EMU_MODE
				case FTEDITOR_DL_BITMAP_LAYOUT_H:
				case FTEDITOR_DL_BITMAP_SIZE_H:
#endif
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
		// TODO: CMD_SETFONT2, CMD_ROMFONT, CMD_SETSCRATCH
		const DlParsed &parsed = dlEditor->getLine(i);
		if (parsed.ValidId && parsed.IdLeft == 0 && parsed.IdRight == FTEDITOR_DL_BITMAP_HANDLE && parsed.Parameter[0].U < BITMAP_SETUP_HANDLES_NB)
		{
			handles[parsed.Parameter[0].U] = true;
		}
#ifdef FT810EMU_MODE
		else if (parsed.ValidId && parsed.IdIndex == 0xFFFFFF00 && parsed.IdRight == (CMD_SETFONT2 & 0xFF) && parsed.Parameter[0].U < BITMAP_SETUP_HANDLES_NB)
		{
			handles[parsed.Parameter[0].U] = true;
		}
		else if (parsed.ValidId && parsed.IdIndex == 0xFFFFFF00 && parsed.IdRight == (CMD_SETSCRATCH & 0xFF) && parsed.Parameter[0].U < BITMAP_SETUP_HANDLES_NB)
		{
			handles[parsed.Parameter[0].U] = true;
		}
		else if (parsed.ValidId && parsed.IdIndex == 0xFFFFFF00 && parsed.IdRight == (CMD_ROMFONT & 0xFF) && parsed.Parameter[0].U < BITMAP_SETUP_HANDLES_NB)
		{
			handles[parsed.Parameter[0].U] = true;
		}
#endif
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
#ifdef FT810EMU_MODE
				case FTEDITOR_DL_BITMAP_LAYOUT_H:
				case FTEDITOR_DL_BITMAP_SIZE_H:
#endif
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
#ifdef FT810EMU_MODE
			case CMD_SETFONT2 & 0xFF:
			case CMD_SETBITMAP & 0xFF:
#endif
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

void ContentManager::editorUpdateHandle(ContentInfo *contentInfo, DlEditor *dlEditor, bool updateSize)
{
	int handleLine = -1;
	bool addressOk = false;
	int layoutLine = -1;
	int sizeLine = -1;
#ifdef FT810EMU_MODE
	int layoutHLine = -1;
	int sizeHLine = -1;
	bool cmdSetBitmap = false;
#endif
	for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
	{
		const DlParsed &parsed = dlEditor->getLine(i);
		if (parsed.ValidId)
		{
#ifdef FT810EMU_MODE
			if (parsed.IdLeft == 0xFFFFFF00)
			{
				switch (parsed.IdRight)
				{
					case CMD_SETBITMAP & 0xFF:
					{
						bool isAddressSame = (contentInfo->Converter == ContentInfo::Font)
							? (parsed.Parameter[0].U == (contentInfo->MemoryAddress + 148)) // Font bitmap offset at 148
							: (parsed.Parameter[0].U == contentInfo->MemoryAddress);
						if (addressOk)
						{
							DlParsed pa = parsed;
							pa.Parameter[1].U = contentInfo->ImageFormat;
							pa.Parameter[2].U = contentInfo->CachedImageWidth & 0x7FF;
							pa.Parameter[3].U = contentInfo->CachedImageHeight & 0x7FF;
							dlEditor->replaceLine(i, pa);
							cmdSetBitmap = true;
						}
						if (isAddressSame && handleLine != -1 && !addressOk)
						{
							i = handleLine;
							addressOk = true;
						}
						continue;
					}
					// TODO: CMD_SETFONT2, addr is from font info ram... tricky
				}
			}
#endif
			if (parsed.IdLeft != 0)
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
					bool isAddressSame = (contentInfo->Converter == ContentInfo::Font)
						? (parsed.Parameter[0].U == (contentInfo->MemoryAddress + 148)) // Font bitmap offset at 148
						: (parsed.Parameter[0].U == contentInfo->MemoryAddress);
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
#ifdef FT810EMU_MODE
				case FTEDITOR_DL_BITMAP_LAYOUT_H:
					if (addressOk)
					{
#if !FT810EMU_BITMAP_ALWAYS_HIGH
						if (!((contentInfo->CachedImageStride >> 10)
							|| (contentInfo->CachedImageHeight >> 9)))
						{
							// Remove _H if not necessary
							dlEditor->removeLine(i);
							--i;
						}
						else
#endif
						{
							// Update _H
							DlParsed pa = parsed;
							pa.Parameter[0].U = contentInfo->CachedImageStride >> 10;
							pa.Parameter[1].U = contentInfo->CachedImageHeight >> 9;
							dlEditor->replaceLine(i, pa);
							layoutHLine = i;
						}
					}
					break;
				case FTEDITOR_DL_BITMAP_SIZE_H:
					if (addressOk)
					{
#if !FT810EMU_BITMAP_ALWAYS_HIGH
						if (!((contentInfo->CachedImageWidth >> 9)
							|| (contentInfo->CachedImageHeight >> 9)))
						{
							// Remove _H if not necessary
							dlEditor->removeLine(i);
							--i;
						}
						else
#endif
						{
							// Update _H
							DlParsed pa = parsed;
							pa.Parameter[0].U = contentInfo->CachedImageWidth >> 9;
							pa.Parameter[1].U = contentInfo->CachedImageHeight >> 9;
							dlEditor->replaceLine(i, pa);
							sizeHLine = i;
						}
					}
					break;
#endif
				default:
					handleLine = -1;
					addressOk = false;
					break;
			}
		}
	}
#ifdef FT810EMU_MODE
	if (!cmdSetBitmap) // User friendly fixup for _H, does not work with bitmap handle reuse, but not a critical use case
	{
		if (layoutLine >= 0 && layoutHLine < 0)
		{
#if !FT810EMU_BITMAP_ALWAYS_HIGH
			if ((contentInfo->CachedImageStride >> 10)
				|| (contentInfo->CachedImageHeight >> 9))
#endif
			{
				// Add _H if doesn't exist and necessary
				DlParsed pa;
				pa.ValidId = true;
				pa.IdLeft = 0;
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
		}
		if (sizeLine >= 0 && sizeHLine < 0)
		{
#if !FT810EMU_BITMAP_ALWAYS_HIGH
			if ((contentInfo->CachedImageWidth >> 9)
				|| (contentInfo->CachedImageHeight >> 9))
#endif
			{
				// Add _H if doesn't exist and necessary
				DlParsed pa;
				pa.ValidId = true;
				pa.IdLeft = 0;
				pa.IdRight = FTEDITOR_DL_BITMAP_SIZE_H;
				pa.ExpectedStringParameter = false;
				pa.Parameter[0].U = contentInfo->CachedImageWidth >> 9;
				pa.Parameter[1].U = contentInfo->CachedImageHeight >> 9;
				pa.ExpectedParameterCount = 2;
				sizeHLine = sizeLine + 1;
				dlEditor->insertLine(sizeHLine, pa);
			}
		}
	}
#endif
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
#ifdef FT810EMU_MODE
		else if (parsed.ValidId && parsed.IdLeft == 0xFFFFFF00 && parsed.IdRight == (CMD_SETBITMAP & 0xFF) && parsed.Parameter[0].I == oldAddr)
		{
			DlParsed pa = parsed;
			pa.Parameter[0].I = newAddr;
			dlEditor->replaceLine(i, pa);
		}
#endif
	}
}

// Update font adress
void ContentManager::editorUpdateFontAddress(int newAddr, int oldAddr, DlEditor *dlEditor)
{
	for (int i = 0; i < FTEDITOR_DL_SIZE; ++i)
	{
		const DlParsed &parsed = dlEditor->getLine(i);
		if (parsed.ValidId && parsed.IdLeft == 0xFFFFFF00 && parsed.IdRight == (CMD_SETFONT & 0xFF) && parsed.Parameter[1].I == oldAddr)
		{
			DlParsed pa = parsed;
			pa.Parameter[1].I = newAddr;
			dlEditor->replaceLine(i, pa);
		}
		// TODO: CMD_SETFONT2
	}
	editorUpdateHandleAddress(newAddr + 148, oldAddr + 148, dlEditor);
}

void ContentManager::editorRemoveContent(ContentInfo *contentInfo, DlEditor *dlEditor)
{
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
				if (parsed.IdLeft == 0xFFFFFF00) // CMD
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
#if FT810EMU_MODE
					if (parsed.IdRight == (CMD_SETFONT2 & 0xFF))
					{
						handleActive = (parsed.Parameter[0].I == bitmapHandle); // This switches handle (TODO: VERIFY)
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
#endif
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
#ifdef FT810EMU_MODE
					case FTEDITOR_DL_BITMAP_LAYOUT_H:
					case FTEDITOR_DL_BITMAP_SIZE_H:
#endif
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
	m_MainWindow->undoStack()->push(changeSourcePath);
	m_MainWindow->propertiesEditor()->surpressSet(true);
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
		int freeAddress = getFreeAddress();
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
	m_MainWindow->undoStack()->push(changeImageFormat);
	m_MainWindow->propertiesEditor()->surpressSet(true);
	editorUpdateHandle(contentInfo, m_MainWindow->dlEditor(), false);
	editorUpdateHandle(contentInfo, m_MainWindow->cmdEditor(), false);
	m_ContentList->setCurrentItem(contentInfo->View);
	m_MainWindow->propertiesEditor()->surpressSet(false);
	m_MainWindow->undoStack()->endMacro();
}

void ContentManager::propertiesImageFormatChanged(int value)
{
	printf("ContentManager::propertiesImageFormatChanged(value)\n");

	value = s_ImageFormatFromUI[value % (sizeof(s_ImageFormatFromUI) / sizeof(s_ImageFormatFromUI[0]))];

	if (current() && current()->ImageFormat != value)
		changeImageFormat(current(), value);
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
		m_ContentManager->reuploadInternal(m_ContentInfo);
		m_ContentManager->recalculateOverlapInternal();
		m_ContentManager->reloadExternal(m_ContentInfo);
		if (m_ContentManager->m_CurrentPropertiesContent == m_ContentInfo)
			m_ContentManager->rebuildGUIInternal(m_ContentInfo);
	}

	virtual void redo()
	{
		m_ContentInfo->MemoryLoaded = m_NewValue;
		m_ContentManager->reuploadInternal(m_ContentInfo);
		m_ContentManager->recalculateOverlapInternal();
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
		m_ContentManager->reuploadInternal(m_ContentInfo);
		m_ContentManager->recalculateOverlapInternal();
		if (m_ContentManager->m_CurrentPropertiesContent == m_ContentInfo)
			m_ContentManager->rebuildGUIInternal(m_ContentInfo);
	}

	virtual void redo()
	{
		m_ContentInfo->MemoryAddress = m_NewValue;
		m_ContentManager->reuploadInternal(m_ContentInfo);
		m_ContentManager->recalculateOverlapInternal();
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
	ChangeMemoryAddress *changeMemoryAddress = new ChangeMemoryAddress(this, contentInfo, value & 0x7FFFFFFC); // Force round to 4
	if (!internal)
	{
		m_MainWindow->undoStack()->beginMacro(tr("Change memory address"));
	}
	int oldAddr = contentInfo->MemoryAddress;
	m_MainWindow->undoStack()->push(changeMemoryAddress);
	if (!internal)
	{
		m_MainWindow->propertiesEditor()->surpressSet(true);
		if (contentInfo->Converter == ContentInfo::Font)
		{
			editorUpdateFontAddress(value, oldAddr, m_MainWindow->dlEditor());
			editorUpdateFontAddress(value, oldAddr, m_MainWindow->cmdEditor());
		}
		else
		{
			editorUpdateHandleAddress(value, oldAddr, m_MainWindow->dlEditor());
			editorUpdateHandleAddress(value, oldAddr, m_MainWindow->cmdEditor());
		}
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

void ContentManager::propertiesDataCompressedChanged(int value)
{
	printf("ContentManager::propertiesDataCompressedChanged(value)\n");

	if (current() && current()->DataCompressed != (value == (int)Qt::Checked))
		changeDataCompressed(current(), (value == (int)Qt::Checked));
}

////////////////////////////////////////////////////////////////////////

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

void ContentManager::propertiesDataEmbeddedChanged(int value)
{
	printf("ContentManager::propertiesDataEmbeddedChanged(value)\n");

	if (current() && current()->DataEmbedded != (value == (int)Qt::Checked))
		changeDataEmbedded(current(), (value == (int)Qt::Checked));
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
		setText(tr("Change memory address"));
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
		setText(tr("Change memory address"));
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
	m_MainWindow->undoStack()->push(changeFontFormat);
	m_MainWindow->propertiesEditor()->surpressSet(true);
	editorUpdateHandle(contentInfo, m_MainWindow->dlEditor(), true);
	editorUpdateHandle(contentInfo, m_MainWindow->cmdEditor(), true);
	m_ContentList->setCurrentItem(contentInfo->View);
	m_MainWindow->propertiesEditor()->surpressSet(false);
	m_MainWindow->undoStack()->endMacro();
}

void ContentManager::propertiesFontFormatChanged(int value)
{
	printf("ContentManager::propertiesFontFormatChanged(value)\n");

	value = s_FontFormatFromUI[value % (sizeof(s_FontFormatFromUI) / sizeof(s_FontFormatFromUI[0]))];

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
	m_MainWindow->undoStack()->push(changeFontSize);
	m_MainWindow->propertiesEditor()->surpressSet(true);
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
		m_MainWindow->undoStack()->push(changeFontCharSet);
		m_MainWindow->propertiesEditor()->surpressSet(true);
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

} /* namespace FT800EMUQT */

/* end of file */
