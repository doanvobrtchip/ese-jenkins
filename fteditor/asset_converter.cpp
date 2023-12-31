/**
 * asset_converter.cpp
 * $Id$
 * \file asset_converter.cpp
 * \brief asset_converter.cpp
 * \date 2014-01-30 14:26GMT
 * \author Jan Boon (Kaetemi)
 */

/*
 * Copyright (C) 2014  Future Technology Devices International Ltd
 */

#pragma warning(disable : 26812) // Unscoped enum
#pragma warning(disable : 26495) // Uninitialized member
#pragma warning(disable : 26444) // Unnamed objects

#ifdef FT800EMU_PYTHON
#include <Python.h>
#endif /* FT800EMU_PYTHON */
#include "asset_converter.h"

// STL includes
#include <stdio.h>
#include <sstream>

// Freetype includs
#ifdef FT800EMU_FREETYPE
#include <ft2build.h>
#include FT_FREETYPE_H
#endif /* FT800EMU_FREETYPE */

// Qt includes
#include <QImage>
#include <QByteArray>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QCoreApplication>
#include <QDateTime>

// Emulator includes
#include <bt8xxemu_inttypes.h>

// Project includes
#include "constant_common.h"

namespace FTEDITOR {

extern QString pythonError();

namespace {

#ifdef FT800EMU_PYTHON
PyObject *a_ImageConvModule = NULL;
PyObject *a_ImageConvObject = NULL;
PyObject *a_ImageConvRun = NULL;
QString a_ImageConvError;
PyObject *a_PalettedConvModule = NULL;
PyObject *a_PalettedConvRun = NULL;
QString a_PalettedConvError;
PyObject *a_RawConvModule = NULL;
PyObject *a_RawConvObject = NULL;
PyObject *a_RawConvRun = NULL;
QString a_RawConvError;
std::map<std::string, int> a_ImageFormatMap;
#endif /* FT800EMU_PYTHON */

#ifdef FT800EMU_FREETYPE
FT_Library a_FreetypeLibrary = NULL;
#endif /* FT800EMU_FREETYPE */

#ifdef FT800EMU_PYTHON
bool initPythonScript(PyObject *&module, PyObject *&object, PyObject *&run, QString &error, const char *scriptName, const char *className, const char *funcName)
{
	PyErr_Clear();

	PyObject *pyScript = PyUnicode_FromString(scriptName);
	module = PyImport_Import(pyScript);
	Py_DECREF(pyScript); pyScript = NULL;

	if (module)
	{
		PyObject *pyClass = PyObject_GetAttrString(module, className);
		if (pyClass)
		{
			PyObject *pyArgs = PyTuple_New(0);
			object = PyObject_CallObject(pyClass, pyArgs);
			Py_DECREF(pyClass); pyClass = NULL;
			Py_DECREF(pyArgs); pyArgs = NULL;

			if (object)
			{
				run = PyObject_GetAttrString(object, "run");
				if (run)
				{
					return true;
				}
			}
		}
	}

	a_ImageConvError = pythonError();
	return false;
}
#endif /* FT800EMU_PYTHON */

#ifdef FT800EMU_PYTHON
bool initPythonScript(PyObject *&module, PyObject *&run, QString &error, const char *scriptName, const char *funcName)
{
	PyErr_Clear();

	PyObject *pyScript = PyUnicode_FromString(scriptName);
	module = PyImport_Import(pyScript);
	Py_DECREF(pyScript); pyScript = NULL;

	if (module)
	{
		run = PyObject_GetAttrString(module, "run");
		if (run)
		{
			return true;
		}
	}

	a_ImageConvError = pythonError();
	return false;
}
#endif /* FT800EMU_PYTHON */

}

void AssetConverter::init()
{
#ifdef FT800EMU_PYTHON
	if (initPythonScript(
		a_ImageConvModule, a_ImageConvObject, a_ImageConvRun, a_ImageConvError,
		"img_cvt", "Image_Conv", "run"))
		printf("Image Converter available\n");
	if (initPythonScript(
		a_PalettedConvModule, a_PalettedConvRun, a_PalettedConvError,
		"pngp2pa", "run"))
		printf("Paletted Converter available\n");
	if (initPythonScript(
		a_RawConvModule, a_RawConvObject, a_RawConvRun, a_RawConvError,
		"raw_cvt", "Raw_Conv", "run"))
		printf("Raw Converter available\n");
#endif /* FT800EMU_PYTHON */
#ifdef FT800EMU_FREETYPE
	int fterr;
	fterr = FT_Init_FreeType(&a_FreetypeLibrary);
	if (fterr)
	{
		printf("Freetype could not initialize\n");
	}
	else
	{
		printf("Font Converter available (freetype)\n");
	}
#endif /* FT800EMU_FREETYPE */
	
	a_ImageFormatMap.clear();
	a_ImageFormatMap["ARGB1555"] = ARGB1555;
	a_ImageFormatMap["L1"] = L1;
	a_ImageFormatMap["L2"] = L2;
	a_ImageFormatMap["L4"] = L4;
	a_ImageFormatMap["L8"] = L8;
	a_ImageFormatMap["RGB332"] = RGB332;
	a_ImageFormatMap["ARGB2"] = ARGB2;
	a_ImageFormatMap["ARGB4"] = ARGB4;
	a_ImageFormatMap["RGB565"] = RGB565;
	a_ImageFormatMap["PALETTED"] = PALETTED;
	a_ImageFormatMap["TEXT8X8"] = TEXT8X8;
	a_ImageFormatMap["TEXTVGA"] = TEXTVGA;
	a_ImageFormatMap["BARGRAPH"] = BARGRAPH;
	a_ImageFormatMap["PALETTED8"] = PALETTED8;
	a_ImageFormatMap["PALETTED565"] = PALETTED565;
	a_ImageFormatMap["PALETTED4444"] = PALETTED4444;
	a_ImageFormatMap["COMPRESSED_RGBA_ASTC_4x4_KHR"] = COMPRESSED_RGBA_ASTC_4x4_KHR;
	a_ImageFormatMap["COMPRESSED_RGBA_ASTC_5x4_KHR"] = COMPRESSED_RGBA_ASTC_5x4_KHR;
	a_ImageFormatMap["COMPRESSED_RGBA_ASTC_5x5_KHR"] = COMPRESSED_RGBA_ASTC_5x5_KHR;
	a_ImageFormatMap["COMPRESSED_RGBA_ASTC_6x5_KHR"] = COMPRESSED_RGBA_ASTC_6x5_KHR;
	a_ImageFormatMap["COMPRESSED_RGBA_ASTC_6x6_KHR"] = COMPRESSED_RGBA_ASTC_6x6_KHR;
	a_ImageFormatMap["COMPRESSED_RGBA_ASTC_8x5_KHR"] = COMPRESSED_RGBA_ASTC_8x5_KHR;
	a_ImageFormatMap["COMPRESSED_RGBA_ASTC_8x6_KHR"] = COMPRESSED_RGBA_ASTC_8x6_KHR;
	a_ImageFormatMap["COMPRESSED_RGBA_ASTC_8x8_KHR"] = COMPRESSED_RGBA_ASTC_8x8_KHR;
	a_ImageFormatMap["COMPRESSED_RGBA_ASTC_10x5_KHR"] = COMPRESSED_RGBA_ASTC_10x5_KHR;
	a_ImageFormatMap["COMPRESSED_RGBA_ASTC_10x6_KHR"] = COMPRESSED_RGBA_ASTC_10x6_KHR;
	a_ImageFormatMap["COMPRESSED_RGBA_ASTC_10x8_KHR"] = COMPRESSED_RGBA_ASTC_10x8_KHR;
	a_ImageFormatMap["COMPRESSED_RGBA_ASTC_10x10_KHR"] = COMPRESSED_RGBA_ASTC_10x10_KHR;
	a_ImageFormatMap["COMPRESSED_RGBA_ASTC_12x10_KHR"] = COMPRESSED_RGBA_ASTC_12x10_KHR;
	a_ImageFormatMap["COMPRESSED_RGBA_ASTC_12x12_KHR"] = COMPRESSED_RGBA_ASTC_12x12_KHR;
}

void AssetConverter::release()
{
	a_ImageFormatMap.clear();
	
#ifdef FT800EMU_FREETYPE
	FT_Done_FreeType(a_FreetypeLibrary);
	a_FreetypeLibrary = NULL;
#endif /* FT800EMU_FREETYPE */
#ifdef FT800EMU_PYTHON
	Py_XDECREF(a_ImageConvRun); a_ImageConvRun = NULL;
	Py_XDECREF(a_ImageConvObject); a_ImageConvObject = NULL;
	Py_XDECREF(a_ImageConvModule); a_ImageConvModule = NULL;
	Py_XDECREF(a_PalettedConvRun); a_PalettedConvRun = NULL;
	Py_XDECREF(a_PalettedConvModule); a_PalettedConvModule = NULL;
	Py_XDECREF(a_RawConvRun); a_RawConvRun = NULL;
	Py_XDECREF(a_RawConvObject); a_RawConvObject = NULL;
	Py_XDECREF(a_RawConvModule); a_RawConvModule = NULL;
#endif /* FT800EMU_PYTHON */
}

int AssetConverter::imageStringToEnum(char *imageString)
{
	std::map<std::string, int>::iterator it = a_ImageFormatMap.find(imageString);
	if (it != a_ImageFormatMap.end())
		return it->second;
	return -1;
}

void AssetConverter::convertImage(QString &buildError, const QString &inFile, const QString &outName, int format)
{
	QString quantFile = outName + "_converted-fs8.png";
	if (QFile::exists(quantFile))
		QFile::remove(quantFile);

#ifdef FT800EMU_PYTHON
    if (a_ImageConvRun)
    {
        PyErr_Clear();
        bool error = true;

        QByteArray inFileUtf8 = inFile.toUtf8();
        QByteArray outNameUtf8 = outName.toUtf8();

        // convert fteditor's format to python script's format
        switch (format)
        {
        case ARGB1555:                          format = 0;  break;
        case L1:                                format = 1;  break;
        case L4:                                format = 2;  break;
        case L8:                                format = 3;  break;
        case RGB332:                            format = 4;  break;
        case ARGB2:                             format = 5;  break;
        case ARGB4:                             format = 6;  break;
        case RGB565:                            format = 7;  break;
        case PALETTED:                          format = 8;  break;
        case L2:                                format = 9;  break;
        case PALETTED565:                       format = 10; break;
        case PALETTED4444:                      format = 11; break;
        case PALETTED8:                         format = 12; break;
        case COMPRESSED_RGBA_ASTC_4x4_KHR:		format = 13; break;
        case COMPRESSED_RGBA_ASTC_5x4_KHR:		format = 14; break;
        case COMPRESSED_RGBA_ASTC_5x5_KHR:	    format = 15; break;
        case COMPRESSED_RGBA_ASTC_6x5_KHR:		format = 16; break;
        case COMPRESSED_RGBA_ASTC_6x6_KHR:		format = 17; break;
        case COMPRESSED_RGBA_ASTC_8x5_KHR:		format = 18; break;
        case COMPRESSED_RGBA_ASTC_8x6_KHR:		format = 19; break;
        case COMPRESSED_RGBA_ASTC_8x8_KHR:		format = 20; break;
        case COMPRESSED_RGBA_ASTC_10x5_KHR:		format = 21; break;
        case COMPRESSED_RGBA_ASTC_10x6_KHR:		format = 22; break;
        case COMPRESSED_RGBA_ASTC_10x8_KHR:		format = 23; break;
        case COMPRESSED_RGBA_ASTC_10x10_KHR:	format = 24; break;
        case COMPRESSED_RGBA_ASTC_12x10_KHR:	format = 25; break;
        case COMPRESSED_RGBA_ASTC_12x12_KHR:	format = 26; break;
        default: break;
        }

        QString sFormat = QString::number(format);

		PyObject *pyValue;
		PyObject *pyArgs = PyTuple_New(1);
		PyObject *pyTuple = PyTuple_New(8);
		pyValue = PyUnicode_FromString("-i");
		PyTuple_SetItem(pyTuple, 0, pyValue);
		pyValue = PyUnicode_FromString(inFileUtf8.data());
		PyTuple_SetItem(pyTuple, 1, pyValue);
		pyValue = PyUnicode_FromString("-o");
		PyTuple_SetItem(pyTuple, 2, pyValue);
		pyValue = PyUnicode_FromString(outNameUtf8.data());
		PyTuple_SetItem(pyTuple, 3, pyValue);
		pyValue = PyUnicode_FromString("-f");
		PyTuple_SetItem(pyTuple, 4, pyValue);
		pyValue = PyUnicode_FromString(sFormat.toUtf8().data());
		PyTuple_SetItem(pyTuple, 5, pyValue);

        pyValue = PyUnicode_FromString("-e");
        PyTuple_SetItem(pyTuple, 6, pyValue);
        pyValue = PyUnicode_FromString("thorough");
        PyTuple_SetItem(pyTuple, 7, pyValue);

		PyTuple_SetItem(pyArgs, 0, pyTuple);
		PyObject *pyResult = PyObject_CallObject(a_ImageConvRun, pyArgs);
		Py_DECREF(pyArgs); pyArgs = NULL;
		if (pyResult)
		{
			printf("Image converted\n");
			error = false;
		}
		if (error)
		{
			buildError = pythonError();
			if (buildError.isEmpty())
				buildError = "<i>(Python)</i> Unknown Error";

			// Reinitialize Python converters
			release();
			init();
		}
	}
	else
	{
		buildError = "<i>(Python)</i> " + a_ImageConvError;
	}
#else
	buildError = "Python not available";
#endif /* FT800EMU_PYTHON */
}

void AssetConverter::convertImagePaletted(QString &buildError, const QString &inFile, const QString &outName, int format)
{
#ifdef FT800EMU_PYTHON
	if (a_PalettedConvRun)
	{
		PyErr_Clear();
		bool error = true;

		QString fileExt = QFileInfo(inFile).suffix().toLower();
		QString convertedInFile = outName + "_converted.png";
		if (QFile::exists(convertedInFile))
			QFile::remove(convertedInFile);

		int nformat;
		switch (format)
		{
		case PALETTED8:
			nformat = 1;
			break;
		case PALETTED565:
			nformat = 2;
			break;
		case PALETTED4444:
			nformat = 3;
			break;
		default:
			nformat = 0;
			break;
		}
		std::stringstream sFormat;
		sFormat << nformat;

		if (fileExt == "png")
		{
			// Copy png to output
			QFile::copy(inFile, convertedInFile);
		}
		else
		{
			// Convert image to png
			QImage image(inFile);
			image.save(convertedInFile, "PNG");
		}

		QByteArray inFileUtf8 = convertedInFile.toUtf8();
		QByteArray outNameUtf8 = outName.toUtf8();

#ifdef WIN32
		QString quantPathStr = QCoreApplication::applicationDirPath() + "\\pngquant.exe"; // With application distribution
		QByteArray quantPathArr = quantPathStr.toUtf8();
		const char *quantPath = quantPathArr.data();
#else
		const char *quantPath = "pngquant"; // System installed
#endif

		PyObject *pyValue;
		PyObject *pyArgs = PyTuple_New(1);
		PyObject *pyTuple = PyTuple_New(8);
		pyValue = PyUnicode_FromString("-i");
		PyTuple_SetItem(pyTuple, 0, pyValue);
		pyValue = PyUnicode_FromString(inFileUtf8.data());
		PyTuple_SetItem(pyTuple, 1, pyValue);
		pyValue = PyUnicode_FromString("-o");
		PyTuple_SetItem(pyTuple, 2, pyValue);
		pyValue = PyUnicode_FromString(outNameUtf8.data());
		PyTuple_SetItem(pyTuple, 3, pyValue);
		pyValue = PyUnicode_FromString("-q");
		PyTuple_SetItem(pyTuple, 4, pyValue);
		pyValue = PyUnicode_FromString(quantPath);
		PyTuple_SetItem(pyTuple, 5, pyValue);
		pyValue = PyUnicode_FromString("-f");
		PyTuple_SetItem(pyTuple, 6, pyValue);
		pyValue = PyUnicode_FromString(sFormat.str().c_str());
		PyTuple_SetItem(pyTuple, 7, pyValue);
		PyTuple_SetItem(pyArgs, 0, pyTuple);
		PyObject *pyResult = PyObject_CallObject(a_PalettedConvRun, pyArgs);
		Py_DECREF(pyArgs); pyArgs = NULL;
		if (pyResult)
		{
			printf("Paletted image converted\n");
			error = false;
		}

		if (error)
		{
			buildError = pythonError();
			if (buildError.isEmpty())
				buildError = "<i>(Python)</i> Unknown Error";

			// Reinitialize Python converters
			release();
			init();
		}
	}
	else
	{
		buildError = "<i>(Python)</i> " + a_PalettedConvError;
	}
#else
	buildError = "Python not available";
#endif /* FT800EMU_PYTHON */
}

bool AssetConverter::getImageInfo(ImageInfo &bitmapInfo, const QString &name)
{
	// Returns image layout
	// /*('file properties: ', 'resolution ', 360, 'x', 238, 'format ', 'L1', 'stride ', 45, ' total size ', 10710)*/
	QString fileName = name + ".rawh";
#ifdef WIN32
	FILE *f = _wfopen((wchar_t *)fileName.data(), L"r");
#else
	QByteArray fileNameCStr = fileName.toLocal8Bit();
	FILE *f = fopen(fileNameCStr.data(), "r");
#endif
	if (!f)
	{
		printf("Failed to open RAWH file\n");
		return false;
	}
	int width, height;
	char format[32];
	format[0] = 0x00;
	int stride;
	int result = fscanf(f, "/*('file properties: ', 'resolution ', %i, 'x', %i, 'format ', '%31[_A-Za-z0-9]', 'stride ', %i", &width, &height, format, &stride);
	fclose(f);
	if (result != 4)
	{
		printf("Invalid scanf on RAWH: %i (%i, %i, %s, %i)\n", result, width, height, format, stride);
		return false;
	}
	// NOTE: LayoutWidth is set incorrectly here on purpose. The graphics processor rewrites this.
	bitmapInfo.LayoutWidth = width;
	bitmapInfo.LayoutHeight = height;
	int imgEnum = imageStringToEnum(format);
	if (imgEnum != -1) {
		bitmapInfo.LayoutFormat = imgEnum;
	}
	else
	{
		printf("Invalid format in RAWH: '%s'\n", format);
		return false;
	}
	bitmapInfo.LayoutStride = stride;
	return true;
}

static void copyCompress(QString &buildError, const QString &inFile, const QString &outName, int begin, int length, bool compress, bool headers, const QString &comment)
{
	QFile fi(inFile);
	if (!fi.open(QIODevice::ReadOnly))
	{
		buildError = "Could not read file";
		return;
	}
	fi.seek(begin);
	QByteArray ba = fi.read(length);
	QByteArray zba;
	; {
		// Compress
		zba = qCompress(ba, 9);
		zba = zba.right(zba.size() - 4);
	}
	; {
		// Write raw
		QFile fo(outName + ".raw");
		if (!fo.open(QIODevice::WriteOnly | QIODevice::Truncate))
		{
			buildError = "Could not open file for writing";
			return;
		}
		if (fo.write(ba) != ba.length())
		{
			buildError = "Write error";
			return;
		}
		fo.close();
	}
	if (compress) 
	{
		// Write compressed
		QFile fo(outName + ".bin");
		if (!fo.open(QIODevice::WriteOnly | QIODevice::Truncate))
		{
			buildError = "Could not open file for writing";
			return;
		}
		if (fo.write(zba) != zba.length())
		{
			buildError = "Write error";
			return;
		}
		fo.close();
	}
	if (headers)
	{
		; {
			QFile fo(outName + ".rawh");
			fo.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
			QTextStream out(&fo);
			out << comment;
			out.setIntegerBase(16);
			for (int i = 0; i < ba.size(); ++i)
			{
				out << "0x" << ((unsigned int)ba[i] & 0xFF) << ", ";
				if (i % 32 == 31)
					out << "\n";
			}
			fo.close();
		}
		if (compress)
		{
			QFile fo(outName + ".binh");
			fo.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
			QTextStream out(&fo);
			// out << comment;
			out.setIntegerBase(16);
			for (int i = 0; i < zba.size(); ++i)
			{
				out << "0x" << ((unsigned int)zba[i] & 0xFF) << ", ";
				if (i % 32 == 31)
					out << "\n";
			}
			fo.close();
		}
	}
}

void AssetConverter::convertRaw(QString &buildError, const QString &inFile, const QString &outName, int begin, int length)
{
#if 1
	copyCompress(buildError, inFile, outName, begin, (length > 0) ? (length) : (QFileInfo(inFile).size() - begin + length), true, true, 
		QString("/*('file properties: ', 'total size ', ") + QString::number(length) + ")*/\n");
#elif defined(FT800EMU_PYTHON)
	if (a_RawConvRun)
	{
		PyErr_Clear();
		bool error = true;

		QByteArray inFileUtf8 = inFile.toUtf8();
		QByteArray outNameUtf8 = outName.toUtf8();
		std::stringstream sBegin;
		sBegin << begin;
		std::stringstream sLength;
		sLength << length;

		PyObject *pyValue;
		PyObject *pyArgs = PyTuple_New(1);
		PyObject *pyTuple = PyTuple_New(8);
		pyValue = PyUnicode_FromString("-i");
		PyTuple_SetItem(pyTuple, 0, pyValue);
		pyValue = PyUnicode_FromString(inFileUtf8.data());
		PyTuple_SetItem(pyTuple, 1, pyValue);
		pyValue = PyUnicode_FromString("-o");
		PyTuple_SetItem(pyTuple, 2, pyValue);
		pyValue = PyUnicode_FromString(outNameUtf8.data());
		PyTuple_SetItem(pyTuple, 3, pyValue);
		pyValue = PyUnicode_FromString("-s");
		PyTuple_SetItem(pyTuple, 4, pyValue);
		pyValue = PyUnicode_FromString(sBegin.str().c_str());
		PyTuple_SetItem(pyTuple, 5, pyValue);
		pyValue = PyUnicode_FromString("-l");
		PyTuple_SetItem(pyTuple, 6, pyValue);
		pyValue = PyUnicode_FromString(sLength.str().c_str());
		PyTuple_SetItem(pyTuple, 7, pyValue);
		PyTuple_SetItem(pyArgs, 0, pyTuple);
		PyObject *pyResult = PyObject_CallObject(a_RawConvRun, pyArgs);
		Py_DECREF(pyArgs); pyArgs = NULL;
		if (pyResult)
		{
			printf("Raw converted\n");
			error = false;
		}
		if (error)
		{
			buildError = pythonError();
			if (buildError.isEmpty())
				buildError = "<i>(Python)</i> Unknown Error";

			// Reinitialize Python converters
			release();
			init();
		}
	}
	else
	{
		buildError = "<i>(Python)</i> " + a_RawConvError;
	}
#else
	buildError = "Python not available";
#endif /* FT800EMU_PYTHON */
}

struct FontMetricBlock
{
	union
	{
		struct
		{
			uint8_t Advance[128];
			uint32_t Format;
			uint32_t LineStride;
			uint32_t Width;
			uint32_t Height;
			uint32_t Pointer;
		} Value;
		uint8_t Data[148];
	};
};

void AssetConverter::convertFont(QString &buildError, const QString &inFile, const QString &outName, int format, int size, const QString &charSet, int offset)
{
#ifdef FT800EMU_FREETYPE
	if (charSet.size() > 128)
	{
		buildError = "Charset too large";
		return;
	}
	FT_Library &library = a_FreetypeLibrary;
	if (!library)
	{
		buildError = "Freetype failed to initialize";
		return;
	}
	FT_Face face;
	int error;
	
	QByteArray inFileLocal8 = inFile.toLocal8Bit(); // If this does not work for unicode, we can load the font file to ram first
	error = FT_New_Face(library, inFileLocal8.data(), 0, &face); // and use FT_New_Memory_Face instead
	if (error == FT_Err_Unknown_File_Format) // TODO?: Replace 0 in FT_New_Face call with the correct face index to support multi-face fonts
	{
		buildError = "Font format unsuppported";
		return;
	}
	else if (error)
	{
		buildError = "Font could not be opened";
		return;
	}
	printf("Glyphs in font: %i\n", (int)face->num_glyphs);
	error = FT_Select_Charmap(face, ft_encoding_unicode);
	if (error)
	{
		buildError = "Unicode charmap not available in this font";
		return;
	}
	error = FT_Set_Pixel_Sizes(face, 0, size);
	if (error)
	{
		buildError = "Selected font size not available for this font";
		return;
	}
	FT_GlyphSlot &slot = face->glyph;
	int xheight = -1;
	{
		// to find x height
		QChar c = QChar('X');
		uint32_t charcode = c.unicode();
		int glyph_index = FT_Get_Char_Index(face, charcode);
		if (glyph_index)
		{
			error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
			if (!error)
			{
				xheight = slot->metrics.height;
			}
		}
	}
	xheight >>= 6;
	printf("Glyph xheight=%i\n", xheight);
	int minx = 0, miny = 0;
	int maxx = 0, maxy = 0;
	for (int i = 0; i < charSet.size(); ++i)
	{
		QChar c = charSet[i];
		uint32_t charcode = c.unicode();
		int glyph_index = FT_Get_Char_Index(face, charcode);
		if (!glyph_index)
			continue; // blank
		error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
		if (error)
			continue;
		error = FT_Render_Glyph(slot, (format == L1) ? FT_RENDER_MODE_MONO : FT_RENDER_MODE_NORMAL);
		if (error)
			continue;
		int mx = slot->bitmap_left + slot->bitmap.width;
		int my = (-slot->bitmap_top) + slot->bitmap.rows;
		if (mx > maxx)
			maxx = mx;
		if (my > maxy)
			maxy = my;
		if (slot->bitmap_left < minx)
			minx = slot->bitmap_left;
		if ((-slot->bitmap_top) < miny)
			miny = (-slot->bitmap_top);
	}
	printf("Glyph max x=%i, y=%i; min x=%i, y=%i\n", maxx, maxy, minx, miny);
	if (xheight >= 0)
	{
		int difup = -miny - xheight;
		int difdn = maxy;
		int dify = difdn - difup;
		if (dify > 0) // more is down
		{
			miny -= dify; // add more up
		}
		else
		{
			maxy -= dify; // add more down
		}
	}
	printf("Glyph max x=%i, y=%i; min x=%i, y=%i\n", maxx, maxy, minx, miny);
	int maxw = maxx - minx;
	int maxh = maxy - miny;
	printf("Glyph max w=%i, h=%i\n", maxw, maxh);
	if (format == L1)
	{
		// maxw = (maxw + 7) & (~7); // Round up per byte of 8 bits
		maxw = (maxw + 31) & (~31); // Round up per 32 bits
	}
	else if (format == L2)
	{
		maxw = (maxw + 15) & (~15); // Round up per 16 bytes (downgraded to L2 afterwards)
	}
	else if (format == L4)
	{
		maxw = (maxw + 7) & (~7); // Round up per 8 bytes (downgraded to L4 afterwards)
	}
	else
	{
		maxw = (maxw + 3) & (~3); // Round up per 4 bytes
	}
	// width = slot->advance.x.. bitmap_left -> always substract (add) minx (negative) // pen_x += slot->advance.x >> 6
	// bitmap = slot->bitmap.buffer, bitmap.pitch
	FontMetricBlock fmb = { 0 };
	fmb.Value.Format = format;
	fmb.Value.LineStride = (format == L1) ? (maxw / 8) : ((format == L2) ? (maxw / 4) : ((format == L4) ? (maxw / 2) : (maxw)));
	fmb.Value.Width = maxw;
	fmb.Value.Height = maxh;
	std::vector<uint8_t> bitmapBuffer;
	bitmapBuffer.resize((size_t)((format == L1) ? fmb.Value.LineStride : maxw) * maxh * ((size_t)charSet.size() + 1));
	std::fill(bitmapBuffer.begin(), bitmapBuffer.end(), 0);
	for (int i = 0; i < charSet.size(); ++i)
	{
		QChar c = charSet[i];
		uint32_t charcode = c.unicode();
		int glyph_index = FT_Get_Char_Index(face, charcode);
		if (!glyph_index)
			continue; // blank
		error = FT_Load_Glyph(face, glyph_index, FT_LOAD_DEFAULT);
		if (error)
		{
			printf("Error at FT_Load_Glyph, %i\n", i);
			continue;
		}
		error = FT_Render_Glyph(slot, (format == L1) ? FT_RENDER_MODE_MONO : FT_RENDER_MODE_NORMAL);
		if (error)
		{
			printf("Error at FT_Render_Glyph, %i\n", i);
			continue;
		}
		const int x = slot->bitmap_left - minx;
		const int y = (-slot->bitmap_top) - miny;
		int adv = (slot->advance.x >> 6);
		adv = adv > 0 ? adv : 0;
		// printf("Advance X: %#x, H: %#x, adv: %u\n", slot->advance.x, slot->linearHoriAdvance, adv);
		// printf("Pixel mode: %i\n", (int)slot->bitmap.pixel_mode);
		if (format == L1 && slot->bitmap.pixel_mode != FT_PIXEL_MODE_MONO) // Ensure proper format
		{
			printf("Invalid mono format, %i\n", i);
			continue;
		}
		else if (format != L1 && slot->bitmap.pixel_mode != FT_PIXEL_MODE_GRAY)
		{
			printf("Invalid gray format, %i\n", i);
			continue;
		}
		const uint8_t *buffer = slot->bitmap.buffer;
		// printf("Stride: %i\n", slot->bitmap.pitch);
		if (slot->bitmap.pitch < 0)
		{
			buildError = "Negative bitmap pitch in font not supported"; // TODO?
			return;
		}
		int idx = 0;
		const int ci = (format == L1) ? (i * fmb.Value.LineStride * maxh) : (i * maxw * maxh);
		//printf("rows: %i\n", slot->bitmap.rows);
		//printf("pitch: %i\n", slot->bitmap.pitch);
		for (unsigned int by = 0; by < slot->bitmap.rows; ++by)
		{
			//printf("by%i\n", by);
			const int ty = y + by;
			for (int bx = 0; bx < slot->bitmap.pitch; ++bx) // NOTE: Stride may be longer than target, hence the + 1 on charSet.size() in the target buffer
			{
				//printf("bx%i\n", bx);
				const int bi = idx + bx; // Current byte in source buffer
				if (format == L1)
				{
					const int tx = x + (bx * 8); // Right shift of data in pixels
					int txi = tx / 8; // Right shift in bytes
					int txb = tx % 8; // Right shift remaining bits
					int ti = ci + (ty * fmb.Value.LineStride) + txi;
					uint8_t leftvalue = slot->bitmap.buffer[bi] >> txb;
					uint8_t rightvalue = slot->bitmap.buffer[bi] << (8 - txb);
					bitmapBuffer[ti] |= leftvalue;
					bitmapBuffer[(size_t)ti + 1] |= rightvalue;
					//printf("CI %i\n", ci);
				}
				else
				{
					const int tx = x + bx; // Right shift of data in pixels
					const int ti = ci + (ty * maxw) + tx;
					bitmapBuffer[ti] = slot->bitmap.buffer[bi];
					// printf("TI %i\n", ti);
				}
			}
			idx += slot->bitmap.pitch;
		}
		fmb.Value.Advance[(i + offset) & 0x7F] = adv;
	}
	// TEST ->
	QImage qimage(&bitmapBuffer[0], maxw, maxh * charSet.size(), (format == L1) ? QImage::Format_Mono : QImage::Format_Indexed8);
	if (format != L1)
	{
		QVector<QRgb> grayTable;
		for (int i = 0; i < 256; ++i)
			grayTable.push_back(qRgb(i, i, i));
		qimage.setColorTable(grayTable);
	}
	else
	{
		qimage = qimage.convertToFormat(QImage::Format_RGB32);
	}
	qimage.save(outName + "_converted.png", "PNG");
	// <- TEST
	int nbbytes = fmb.Value.LineStride * fmb.Value.Height * charSet.size();
	if (format == L4) // Downgrade to L4
	{
		for (int i = 0; i < nbbytes; ++i)
		{
			uint8_t left = bitmapBuffer[(size_t)i * 2];
			uint8_t right = bitmapBuffer[(size_t)i * 2 + 1];
			left &= 0xF0;
			right >>= 4;
			bitmapBuffer[i] = left | right;
		}
	}
	else if (format == L2)
	{
		for (int i = 0; i < nbbytes; ++i)
		{
			uint8_t a = bitmapBuffer[(size_t)i * 4];
			uint8_t b = bitmapBuffer[(size_t)i * 4 + 1];
			uint8_t c = bitmapBuffer[(size_t)i * 4 + 2];
			uint8_t d = bitmapBuffer[(size_t)i * 4 + 3];
			a >>= 6;
			b >>= 6;
			c >>= 6;
			d >>= 6;
			uint8_t res = (a << 6) | (b << 4) | (c << 2) | d;
			bitmapBuffer[i] = res;
		}
	}
	QByteArray ba;
	ba.resize(148 + nbbytes);
	for (int i = 0; i < 148; ++i)
		ba[i] = fmb.Data[i];
	for (int i = 0; i < nbbytes; ++i)
		ba[148 + i] = bitmapBuffer[i];
	QByteArray zba = qCompress(ba, 9);
	zba = zba.right(zba.size() - 4);
	{
		QFile file(outName + ".raw");
		file.open(QIODevice::WriteOnly);
		file.write(ba);
		file.close();
	}
	{
		QFile file(outName + ".rawh");
		file.open(QIODevice::WriteOnly | QIODevice::Text);
		QTextStream out(&file);
		out << "/*('file properties: ', 'resolution ', "
			<< (int)fmb.Value.Width << ", 'x', "
			<< (int)fmb.Value.Height << ", 'format ', '"
			<< (format == L1 ? "L1" : (format == L2 ? "L2" : (format == L4 ? "L4" : "L8")))
			<< "', 'stride ', " << (int)fmb.Value.LineStride
			<< ")*/\n";
		for (int i = 0; i < ba.size(); ++i)
		{
			out << ((unsigned int)ba[i] & 0xFF) << ", ";
			if (i % 32 == 31)
				out << "\n";
		}
		file.close();
	}
	{
		QFile file(outName + ".bin");
		file.open(QIODevice::WriteOnly);
		file.write(zba);
		file.close();
	}
	{
		QFile file(outName + ".binh");
		file.open(QIODevice::WriteOnly | QIODevice::Text);
		QTextStream out(&file);
		for (int i = 0; i < zba.size(); ++i)
		{
			out << ((unsigned int)zba[i] & 0xFF) << ", ";
			if (i % 32 == 31)
				out << "\n";
		}
		file.close();
	}
#else
	buildError = "Freetype not available";
#endif
}

// Doesn't actually do anything in terms of conversion.
// Exports only the .raw and .rawh unmodified.
// Places calculated meta information about image size in the header.
void AssetConverter::convertImageCoprocessor(QString &buildError, const QString &inFile, const QString &outName, bool mono, bool supportJpeg, bool supportPNG, int &palettedAddress)
{
	// Remove old output
	QString outRawName = outName + ".raw";
	if (QFile::exists(outRawName))
		QFile::remove(outRawName);
	QString outBinName = outName + ".bin";
	if (QFile::exists(outBinName))
		QFile::remove(outBinName);

	QFile file(inFile);
	if (!file.open(QIODevice::ReadOnly))
	{
		buildError = "Could not read file";
		return;
	}

	bool isPNG;
	// Validate input format by file header
	; {
		QDataStream in(&file);
		char header[4];
		memset(header, 0, 4);
		in.readRawData(header, 4);
		if (supportJpeg && (header[0] == '\xFF') && (header[1] == '\xD8'))
		{ 
			isPNG = false;
		}
		else if (supportPNG && (header[0] == '\x89') && (header[1] == '\x50'))
		{ 
			isPNG = true; 
		}
		else
		{
			buildError = "Unsupported input file format";
			return;
		}
	}
	file.close();
	
	int w, h;
	bool a;
	bool png_quant = false;
	// Have to load entire image to get Width and Height for now...
	; {
		QImage image;
		if (!image.load(inFile))
		{
			buildError = "Could not load image";
			return;
		}
		w = image.width();
		h = image.height();
		a = image.hasAlphaChannel();
		palettedAddress = image.colorCount() * 2;
		png_quant = (image.colorCount() > 0);
	}

	int format;
	// JPG: RGB565 or L8 when mono
	// PNG: RGB565 or ARGB4 when alpha, no mono support
	if (isPNG) {
		if (png_quant) {
			format = a ? PALETTED4444 : PALETTED565;
		}
		else {		
			format = a ? ARGB4 : RGB565;
		}
	}
	else format = mono ? L8 : RGB565;

	int bpp = (format == L8) ? 1 : 2;
	int stride = w * bpp;

	QString str_format;
	switch (format)
	{
	case L8:			str_format = "L8";				break;
	case ARGB4:			str_format = "ARGB4";			break;
	case RGB565:		str_format = "RGB565";			break;
	case PALETTED4444:	str_format = "PALETTED4444";	break;
	case PALETTED565:	str_format = "PALETTED565";		break;
	default:			str_format = "L8";				break;
	}

	// Output the raw binary
	file.copy(outRawName);

	// Output the raw header
	; {
		if (!file.open(QIODevice::ReadOnly))
		{
			buildError = "Could not read file";
			return;
		}
		QDataStream in(&file);
		QFile fo(outName + ".rawh");
		fo.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate);
		QTextStream out(&fo);
		out << "/*('file properties: ', 'resolution ', "
		    << w << ", 'x', "
		    << h << ", 'format ', '"
		    << str_format
			<< "', 'stride ', " << stride
			<< ")*/\n";
		int i = 0;
		char c[4096];
		int jm;
		while ((jm = in.readRawData(c, 4096)) > 0)
		{
			for (int j = 0; j < jm; ++j)
			{
				out << ((unsigned int)c[j] & 0xFF) << ", ";
				if (i % 32 == 31)
					out << "\n";
				++i;
			}
		}
		fo.close();
		file.close();
	}

	// Compression not supported because it makes no sense
	// Need files for meta check, though
	; {
		QFile fo(outName + ".bin");
		fo.open(QIODevice::WriteOnly | QIODevice::Truncate);
	}
	; {
		QFile fo(outName + ".binh");
		fo.open(QIODevice::WriteOnly | QIODevice::Truncate);
	}
}

static QString s_CachedFlashMapPath;
static QDateTime s_CachedFlashMapModified;
static FlashMapInfo s_CachedFlashMapInfo;
static FlashMapInfo s_BlankFlashMapInfo;
static bool s_LockedFlashMapInfo = false;

void AssetConverter::lockFlashMap(bool lock)
{
	s_LockedFlashMapInfo = lock;
}

const FlashMapInfo &AssetConverter::parseFlashMap(const QString &flashMapPath)
{
	FlashMapInfo &flashMapInfo = s_CachedFlashMapInfo;

	QDateTime lastModified = QFileInfo(flashMapPath).lastModified();
	if (s_CachedFlashMapPath == flashMapPath && s_CachedFlashMapModified == lastModified)
		return flashMapInfo;

	if (s_LockedFlashMapInfo)
	{
		printf("Flash map cannot be updated while loading from it, return blank");
		return s_BlankFlashMapInfo;
	}

	flashMapInfo.clear();
	s_CachedFlashMapPath = flashMapPath;
	s_CachedFlashMapModified = lastModified;

	QFile file(flashMapPath);
	if (!file.open(QIODevice::ReadOnly))
		return flashMapInfo;

	QTextStream in(&file);
	FlashMapEntry lastEntry;
	while (!in.atEnd())
	{
		QString line = in.readLine();
		QStringList list = line.split(':');
		if (list.size() >= 2)
		{
			// flashMapInfo[list[0]]
			const QString &name = list[0];
			bool ok;
			int index = list[1].toInt(&ok);
			if (!ok) continue;
			if (!lastEntry.Name.isEmpty())
			{
				int lastSize = index - lastEntry.Index;
				lastEntry.Size = lastEntry.Size >= 0 ? std::min(lastEntry.Size, lastSize) : lastSize;
				flashMapInfo[lastEntry.Name] = lastEntry;
			}
			lastEntry.Name = list[0].trimmed().replace('\\', '/');
			lastEntry.Index = index;
			int sz = (list.size() >= 3) ? (list[2].toInt(&ok)) : (ok = false, 0);
			lastEntry.Size = ok ? sz : -1;
		}
	}
	if (!lastEntry.Name.isEmpty())
	{
		QFileInfo flashMapPathInfo = QFileInfo(flashMapPath);
		QString flashBinPath = flashMapPathInfo.absolutePath() + "/" + flashMapPathInfo.completeBaseName() + ".bin";
		QFileInfo flashBinInfo(flashBinPath);
		if (flashBinInfo.exists())
		{
			int lastSize = flashBinInfo.size() - lastEntry.Index;
			lastEntry.Size = lastEntry.Size >= 0 ? std::min(lastEntry.Size, lastSize) : lastSize;
			flashMapInfo[lastEntry.Name] = lastEntry;
		}
	}
	file.close();

	return flashMapInfo;
}

void AssetConverter::convertFlashMap(QString &buildError, const QString &inFile, const QString &outName, const QString &mappedName)
{
	// Remove old output
	QString outRawName = outName + ".raw";
	if (QFile::exists(outRawName))
		QFile::remove(outRawName);
	QString outBinName = outName + ".bin";
	if (QFile::exists(outBinName))
		QFile::remove(outBinName);

	QFileInfo flashMapFileInfo = QFileInfo(inFile);
	if (!flashMapFileInfo.exists())
	{
		buildError = "Flash map does not exist";
		return;
	}

	QString flashBinPath = flashMapFileInfo.absolutePath() + "/" + flashMapFileInfo.completeBaseName() + ".bin";
	QFileInfo flashBinInfo(flashBinPath);
	if (!flashBinInfo.exists())
	{
		buildError = "Flash binary does not exist";
		return;
	}

	const FlashMapInfo &flashMapInfo = parseFlashMap(inFile);

	FlashMapInfo::const_iterator entryIt = flashMapInfo.find(mappedName);
	if (entryIt == flashMapInfo.end())
	{
		buildError = "Mapped name does not exist in flash map";
		return;
	}
	const FlashMapEntry &entry = entryIt->second;

	// Copy raw and compressed data
	copyCompress(buildError, flashBinPath, outName, entry.Index, entry.Size, false, false, QString());

	; {
		QFile fo(outName + ".bin");
		fo.open(QIODevice::WriteOnly | QIODevice::Truncate);
	}
	; {
		QFile fo(outName + ".binh");
		fo.open(QIODevice::WriteOnly | QIODevice::Truncate);
	}
	; {
		QFile fo(outName + ".rawh");
		fo.open(QIODevice::WriteOnly | QIODevice::Truncate);
	}
}

} /* namespace FTEDITOR */

/* end of file */
