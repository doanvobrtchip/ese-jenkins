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

#ifndef FT800EMUQT_ASSET_CONVERTER_H
#define FT800EMUQT_ASSET_CONVERTER_H

// STL includes

// Qt includes
#include <QString>

// Emulator includes

// Project includes

namespace FT800EMUQT {

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
	static void convertRaw(QString &buildError, const QString &inFile, const QString &outName, int begin, int length);
	static void release();

}; /* class AssetConverter */

} /* namespace FT800EMUQT */

#endif /* #ifndef FT800EMUQT_ASSET_CONVERTER_H */

/* end of file */
