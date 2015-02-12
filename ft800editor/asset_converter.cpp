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
#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QCoreApplication>

// Emulator includes
#include <ft800emu_graphics_processor.h>
#include <ft800emu_vc.h>

// Project includes

namespace FT800EMUQT {

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
#endif /* FT800EMU_PYTHON */

#ifdef FT800EMU_FREETYPE
FT_Library a_FreetypeLibrary = NULL;
#endif /* FT800EMU_FREETYPE */

bool initPythonScript(PyObject *&module, PyObject *&object, PyObject *&run, QString &error, const char *scriptName, const char *className, const char *funcName)
{
#ifdef FT800EMU_PYTHON
	PyObject *pyScript = PyString_FromString(scriptName);
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

	printf("---\nPython ERROR: \n");
	PyObject *ptype, *pvalue, *ptraceback;
	PyErr_Fetch(&ptype, &pvalue, &ptraceback);
	char *pStrErrorMessage = PyString_AsString(pvalue);
	a_ImageConvError = QString::fromLocal8Bit(pStrErrorMessage);
	printf("%s\n", pStrErrorMessage);
	printf("---\n");
	return false;
#endif /* FT800EMU_PYTHON */
}

bool initPythonScript(PyObject *&module, PyObject *&run, QString &error, const char *scriptName, const char *funcName)
{
#ifdef FT800EMU_PYTHON
	PyObject *pyScript = PyString_FromString(scriptName);
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

	printf("---\nPython ERROR: \n");
	PyObject *ptype, *pvalue, *ptraceback;
	PyErr_Fetch(&ptype, &pvalue, &ptraceback);
	char *pStrErrorMessage = PyString_AsString(pvalue);
	a_ImageConvError = QString::fromLocal8Bit(pStrErrorMessage);
	printf("%s\n", pStrErrorMessage);
	printf("---\n");
	return false;
#endif /* FT800EMU_PYTHON */
}

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
}

void AssetConverter::release()
{
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

void AssetConverter::convertImage(QString &buildError, const QString &inFile, const QString &outName, int format)
{
	QString quantFile = outName + "_converted-fs8.png";
	if (QFile::exists(quantFile))
		QFile::remove(quantFile);

	if (format == PALETTED)
	{
		convertImagePaletted(buildError, inFile, outName);
		if (buildError.isEmpty())
			return;
		buildError.clear();
		printf("Failed to export using pngquant, try PIL\n");
	}
#ifdef FT800EMU_PYTHON
	if (a_ImageConvRun)
	{
		bool error = true;

		QByteArray inFileUtf8 = inFile.toUtf8();
		QByteArray outNameUtf8 = outName.toUtf8();
		std::stringstream sFormat;
		sFormat << format;

		PyObject *pyValue;
		PyObject *pyArgs = PyTuple_New(1);
		PyObject *pyTuple = PyTuple_New(6);
		pyValue = PyString_FromString("-i");
		PyTuple_SetItem(pyTuple, 0, pyValue);
		pyValue = PyUnicode_FromString(inFileUtf8.data());
		PyTuple_SetItem(pyTuple, 1, pyValue);
		pyValue = PyString_FromString("-o");
		PyTuple_SetItem(pyTuple, 2, pyValue);
		pyValue = PyUnicode_FromString(outNameUtf8.data());
		PyTuple_SetItem(pyTuple, 3, pyValue);
		pyValue = PyString_FromString("-f");
		PyTuple_SetItem(pyTuple, 4, pyValue);
		pyValue = PyString_FromString(sFormat.str().c_str());
		PyTuple_SetItem(pyTuple, 5, pyValue);
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
			printf("---\nPython ERROR: \n");
			PyObject *ptype, *pvalue, *ptraceback;
			PyErr_Fetch(&ptype, &pvalue, &ptraceback);
			PyObject *errStr = PyObject_Repr(pvalue);
			char *pStrErrorMessage = PyString_AsString(errStr);
			QString error = QString::fromLocal8Bit(pStrErrorMessage);
			if (pStrErrorMessage)
			{
				buildError = QString::fromLocal8Bit(pStrErrorMessage);
			}
			else
			{
				buildError = "<i>(Python)</i> Unknown Error";
			}
			QByteArray er = buildError.toLocal8Bit();
			printf("%s\n", er.data());
			Py_DECREF(errStr);
			printf("---\n");

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

void AssetConverter::convertImagePaletted(QString &buildError, const QString &inFile, const QString &outName)
{
#ifdef FT800EMU_PYTHON
	if (a_PalettedConvRun)
	{
		bool error = true;

		QString fileExt = QFileInfo(inFile).suffix().toLower();
		QString convertedInFile = outName + "_converted.png";
		if (QFile::exists(convertedInFile))
			QFile::remove(convertedInFile);

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
		PyObject *pyTuple = PyTuple_New(6);
		pyValue = PyString_FromString("-i");
		PyTuple_SetItem(pyTuple, 0, pyValue);
		pyValue = PyUnicode_FromString(inFileUtf8.data());
		PyTuple_SetItem(pyTuple, 1, pyValue);
		pyValue = PyString_FromString("-o");
		PyTuple_SetItem(pyTuple, 2, pyValue);
		pyValue = PyUnicode_FromString(outNameUtf8.data());
		PyTuple_SetItem(pyTuple, 3, pyValue);
		pyValue = PyString_FromString("-q");
		PyTuple_SetItem(pyTuple, 4, pyValue);
		pyValue = PyUnicode_FromString(quantPath);
		PyTuple_SetItem(pyTuple, 5, pyValue);
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
			printf("---\nPython ERROR: \n");
			PyObject *ptype, *pvalue, *ptraceback;
			PyErr_Fetch(&ptype, &pvalue, &ptraceback);
			PyObject *errStr = PyObject_Repr(pvalue);
			char *pStrErrorMessage = PyString_AsString(errStr);
			QString error = QString::fromLocal8Bit(pStrErrorMessage);
			if (pStrErrorMessage)
			{
				buildError = QString::fromLocal8Bit(pStrErrorMessage);
			}
			else
			{
				buildError = "<i>(Python)</i> Unknown Error";
			}
			QByteArray er = buildError.toLocal8Bit();
			printf("%s\n", er.data());
			Py_DECREF(errStr);
			printf("---\n");

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

bool AssetConverter::getImageInfo(FT800EMU::BitmapInfo &bitmapInfo, const QString &name)
{
	// Returns image layout
	// /*('file properties: ', 'resolution ', 360, 'x', 238, 'format ', 'L1', 'stride ', 45, ' total size ', 10710)*/
	QString fileName = name + ".rawh";
	QByteArray fileNameCStr = fileName.toLocal8Bit(); // VERIFY: UNICODE
	FILE *f = fopen(fileNameCStr.data(), "r");
	if (!f)
	{
		printf("Failed to open RAWH file\n");
		return false;
	}
	int width, height;
	char format[32];
	format[0] = 0x00;
	int stride;
	int result = fscanf(f, "/*('file properties: ', 'resolution ', %i, 'x', %i, 'format ', '%31[A-Z0-9]', 'stride ', %i", &width, &height, format, &stride);
	fclose(f);
	if (result != 4)
	{
		printf("Invalid scanf on RAWH: %i (%i, %i, %s, %i)\n", result, width, height, format, stride);
		return false;
	}
	// NOTE: LayoutWidth is set incorrectly here on purpose. The graphics processor rewrites this.
	bitmapInfo.LayoutWidth = width;
	bitmapInfo.LayoutHeight = height;
	if (!strcmp(format, "ARGB1555")) bitmapInfo.LayoutFormat = ARGB1555;
	else if (!strcmp(format, "L1")) bitmapInfo.LayoutFormat = L1;
	else if (!strcmp(format, "L4")) bitmapInfo.LayoutFormat = L4;
	else if (!strcmp(format, "L8")) bitmapInfo.LayoutFormat = L8;
	else if (!strcmp(format, "RGB332")) bitmapInfo.LayoutFormat = RGB332;
	else if (!strcmp(format, "ARGB2")) bitmapInfo.LayoutFormat = ARGB2;
	else if (!strcmp(format, "ARGB4")) bitmapInfo.LayoutFormat = ARGB4;
	else if (!strcmp(format, "RGB565")) bitmapInfo.LayoutFormat = RGB565;
	else if (!strcmp(format, "PALETTED")) bitmapInfo.LayoutFormat = PALETTED;
	else if (!strcmp(format, "TEXT8X8")) bitmapInfo.LayoutFormat = TEXT8X8;
	else if (!strcmp(format, "TEXTVGA")) bitmapInfo.LayoutFormat = TEXTVGA;
	else if (!strcmp(format, "BARGRAPH")) bitmapInfo.LayoutFormat = BARGRAPH;
	else
	{
		printf("Invalid format in RAWH: '%s'\n", format);
		return false;
	}
	bitmapInfo.LayoutStride = stride;
	return true;
}

void AssetConverter::convertRaw(QString &buildError, const QString &inFile, const QString &outName, int begin, int length)
{
#ifdef FT800EMU_PYTHON
	if (a_RawConvRun)
	{
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
		pyValue = PyString_FromString("-i");
		PyTuple_SetItem(pyTuple, 0, pyValue);
		pyValue = PyUnicode_FromString(inFileUtf8.data());
		PyTuple_SetItem(pyTuple, 1, pyValue);
		pyValue = PyString_FromString("-o");
		PyTuple_SetItem(pyTuple, 2, pyValue);
		pyValue = PyUnicode_FromString(outNameUtf8.data());
		PyTuple_SetItem(pyTuple, 3, pyValue);
		pyValue = PyString_FromString("-s");
		PyTuple_SetItem(pyTuple, 4, pyValue);
		pyValue = PyString_FromString(sBegin.str().c_str());
		PyTuple_SetItem(pyTuple, 5, pyValue);
		pyValue = PyString_FromString("-l");
		PyTuple_SetItem(pyTuple, 6, pyValue);
		pyValue = PyString_FromString(sLength.str().c_str());
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
			printf("---\nPython ERROR: \n");
			PyObject *ptype, *pvalue, *ptraceback;
			PyErr_Fetch(&ptype, &pvalue, &ptraceback);
			PyObject *errStr = PyObject_Repr(pvalue);
			char *pStrErrorMessage = PyString_AsString(errStr);
			QString error = QString::fromLocal8Bit(pStrErrorMessage);
			if (pStrErrorMessage)
			{
				buildError = QString::fromLocal8Bit(pStrErrorMessage);
			}
			else
			{
				buildError = "<i>(Python)</i> Unknown Error";
			}
			QByteArray er = buildError.toLocal8Bit();
			printf("%s\n", er.data());
			Py_DECREF(errStr);
			printf("---\n");

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

void AssetConverter::convertFont(QString &buildError, const QString &inFile, const QString &outName, int format, int size, const QString &charSet)
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
	FontMetricBlock fmb;
	memset(fmb.Data, 0, 148);
	fmb.Value.Format = format;
	fmb.Value.LineStride = (format == L1) ? (maxw / 8) : ((format == L4) ? (maxw / 2) : (maxw));
	fmb.Value.Width = maxw;
	fmb.Value.Height = maxh;
	std::vector<uint8_t> bitmapBuffer;
	bitmapBuffer.resize(((format == L1) ? fmb.Value.LineStride : maxw) * maxh * (charSet.size() + 1));
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
		for (int by = 0; by < slot->bitmap.rows; ++by)
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
					bitmapBuffer[ti + 1] |= rightvalue;
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
		fmb.Value.Advance[i] = adv;
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
			uint8_t left = bitmapBuffer[i * 2];
			uint8_t right = bitmapBuffer[i * 2 + 1];
			left &= 0xF0;
			right >>= 4;
			bitmapBuffer[i] = left | right;
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
			<< (format == L1 ? "L1" : (format == L4 ? "L4" : "L8"))
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

} /* namespace FT800EMUQT */

/* end of file */