/**
 * content_manager.h
 * $Id$
 * \file content_manager.h
 * \brief content_manager.h
 * \date 2014-01-31 16:56GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2014  Future Technology Contents International Ltd
 */

#ifndef FT800EMUQT_CONTENT_MANAGER_H
#define FT800EMUQT_CONTENT_MANAGER_H

// STL includes

// Qt includes
#include <QWidget>
#include <QString>

// Emulator includes
#include <ft800emu_inttypes.h>

// Project includes

class QTreeWidget;

namespace FT800EMUQT {

class MainWindow;

struct ContentInfo
{
	enum ConverterType
	{
		Raw, // Raw copy to ram
		Image, // Process image
		Jpeg, // Load jpeg on coprocessor
	};

	QString SourcePath; // Relative source path
	ConverterType Converter;

	bool MemoryLoaded; //Present; // Whether this is loaded in ram
	int MemoryAddress; // Target memory address

	int DataCompressed; // Use compressed data source when embedding - only relevant for code generator scripts
	int DataEmbedded; // Whether to use embedded header or file - only relevant for code generator scripts

	int RawStart; // Raw start of memory
	int RawLength; // Raw length of memory

	int ImageFormat;
	QString ImageName;
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

private:
	MainWindow *m_MainWindow;
	QTreeWidget *m_ContentList;

private slots:
	void add();
	void remove();

private:
	ContentManager(const ContentManager &);
	ContentManager &operator=(const ContentManager &);

}; /* class ContentManager */

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_CONTENT_MANAGER_H */

/* end of file */
