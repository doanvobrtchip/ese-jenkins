/**
 * asset_converter.h
 * $Id$
 * \file asset_converter.h
 * \brief asset_converter.h
 * \date 2014-01-30 14:26GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#ifndef FTEDITOR_ASSET_CONVERTER_H
#define FTEDITOR_ASSET_CONVERTER_H

// STL includes
#include <map>

// Qt includes
#include <QString>

// Emulator includes

// Project includes

namespace FTEDITOR {

struct ImageInfo
{
	int LayoutFormat;
	int LayoutWidth;
	int LayoutStride;
	int LayoutHeight;
};

struct FlashMapEntry
{
	QString Name;
	int Index;
	int Size;
};

typedef std::map<QString, FlashMapEntry> FlashMapInfo;

/**
 * AssetConverter
 * \brief AssetConverter
 * \date 2014-01-30 14:26GMT
 * \author Jan Boon (Kaetemi)
 */
class AssetConverter
{
public:
	static void init();
	static void convertImage(QString &buildError, const QString &inFile, const QString &outName, int format);
	static void convertImagePaletted(QString &buildError, const QString &inFile, const QString &outName, int format);
	static bool getImageInfo(ImageInfo &bitmapInfo, const QString &name);
	static void convertRaw(QString &buildError, const QString &inFile, const QString &outName, int begin, int length);
	static void convertFont(QString &buildError, const QString &inFile, const QString &outName, int format, int size, const QString &charSet, int offset);
	static void convertImageCoprocessor(QString &buildError, const QString &inFile, const QString &outName, bool mono, bool supportJpeg, bool supportPNG);
	static void lockFlashMap(bool lock);
	static const FlashMapInfo &parseFlashMap(const QString &flashMapPath);
	static void convertFlashMap(QString &buildError, const QString &inFile, const QString &outName, const QString &mappedName);
	static void release();

}; /* class AssetConverter */

} /* namespace FTEDITOR */

#endif /* #ifndef FTEDITOR_ASSET_CONVERTER_H */

/* end of file */
