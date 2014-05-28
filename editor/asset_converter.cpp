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

// Emulator includes
#include <ft800emu_graphics_processor.h>
#include <vc.h>

// Project includes

namespace FT800EMUQT {

namespace {

#ifdef FT800EMU_PYTHON
PyObject *a_ImageConvModule = NULL;
PyObject *a_ImageConvObject = NULL;
PyObject *a_ImageConvRun = NULL;
QString a_ImageConvError;
PyObject *a_RawConvModule = NULL;
PyObject *a_RawConvObject = NULL;
PyObject *a_RawConvRun = NULL;
QString a_RawConvError;
#endif /* FT800EMU_PYTHON */

#ifdef FT800EMU_FREETYPE
FT_Library a_FreetypeLibrary = NULL;
#endif

}

void AssetConverter::init()
{
#ifdef FT800EMU_PYTHON
	bool error = true;

	PyObject *pyImageConvScript = PyString_FromString("img_cvt");
	a_ImageConvModule = PyImport_Import(pyImageConvScript);
	Py_DECREF(pyImageConvScript); pyImageConvScript = NULL;

	if (a_ImageConvModule)
	{
		PyObject *pyImageConvClass = PyObject_GetAttrString(a_ImageConvModule, "Image_Conv");
		if (pyImageConvClass)
		{
			PyObject *pyArgs = PyTuple_New(0);
			a_ImageConvObject = PyObject_CallObject(pyImageConvClass, pyArgs);
			Py_DECREF(pyImageConvClass); pyImageConvClass = NULL;
			Py_DECREF(pyArgs); pyArgs = NULL;

			if (a_ImageConvObject)
			{
				a_ImageConvRun = PyObject_GetAttrString(a_ImageConvObject, "run");
				if (a_ImageConvRun)
				{
					printf("Image Converter available\n");
					error = false;
				}
			}
		}
	}

	if (error)
	{
		printf("---\nPython ERROR: \n");
		PyObject *ptype, *pvalue, *ptraceback;
		PyErr_Fetch(&ptype, &pvalue, &ptraceback);
		char *pStrErrorMessage = PyString_AsString(pvalue);
		a_ImageConvError = QString::fromLocal8Bit(pStrErrorMessage);
		printf("%s\n", pStrErrorMessage);
		printf("---\n");
	}

	error = true;

	PyObject *pyRawConvScript = PyString_FromString("raw_cvt");
	a_RawConvModule = PyImport_Import(pyRawConvScript);
	Py_DECREF(pyRawConvScript); pyRawConvScript = NULL;

	if (a_RawConvModule)
	{
		PyObject *pyRawConvClass = PyObject_GetAttrString(a_RawConvModule, "Raw_Conv");
		if (pyRawConvClass)
		{
			PyObject *pyArgs = PyTuple_New(0);
			a_RawConvObject = PyObject_CallObject(pyRawConvClass, pyArgs);
			Py_DECREF(pyRawConvClass); pyRawConvClass = NULL;
			Py_DECREF(pyArgs); pyArgs = NULL;

			if (a_RawConvObject)
			{
				a_RawConvRun = PyObject_GetAttrString(a_RawConvObject, "run");
				if (a_RawConvRun)
				{
					printf("Raw Converter available\n");
					error = false;
				}
			}
		}
	}

	if (error)
	{
		printf("---\nPython ERROR: \n");
		PyObject *ptype, *pvalue, *ptraceback;
		PyErr_Fetch(&ptype, &pvalue, &ptraceback);
		char *pStrErrorMessage = PyString_AsString(pvalue);
		a_RawConvError = QString::fromLocal8Bit(pStrErrorMessage);
		printf("%s\n", pStrErrorMessage);
		printf("---\n");
	}

	// error = true;

	// ...
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
	Py_XDECREF(a_RawConvRun); a_RawConvRun = NULL;
	Py_XDECREF(a_RawConvObject); a_RawConvObject = NULL;
	Py_XDECREF(a_RawConvModule); a_RawConvModule = NULL;
#endif /* FT800EMU_PYTHON */
}

void AssetConverter::convertImage(QString &buildError, const QString &inFile, const QString &outName, int format)
{
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

void AssetConverter::convertFont(QString &buildError, const QString &inFile, const QString &outName, int format, int size, const QString &charSet)
{
#ifdef FT800EMU_FREETYPE
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
	if (error == FT_Err_Unknown_File_Format) // TODO?: Replace 0 with the correct face index to support multi-face fonts
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
		int my = slot->bitmap_top + slot->bitmap.rows;
		if (mx > maxx)
			maxx = mx;
		if (my > maxy)
			maxy = my;
		if (slot->bitmap_left < minx)
			minx = slot->bitmap_left;
		if (slot->bitmap_top < miny)
			miny = slot->bitmap_top;
	}
	printf("Glyph max x=%i, y=%i; min x=%i, y=%i\n", maxx, maxy, minx, miny);
	int maxw = maxx - minx;
	int maxh = maxy - miny;
	printf("Glyph max w=%i, h=%i\n", maxw, maxh);
	if (format == L1)
	{
		maxw = (maxw + 7) & (~7); // Round up per byte of 8 bits
	}
	else
	{
		maxw = (maxw + 3) & (~3); // Rount up per 4 bytes
	}
	// width = slot->advance.x.. bitmap_left -> always substract (add) minx (negative) // pen_x += slot->advance.x >> 6
	// bitmap = slot->bitmap.buffer, bitmap.pitch
	std::vector<uint8_t> bitmapBuffer;
	bitmapBuffer.resize((format == L1) ? (maxw / 8 * maxh * (charSet.size() + 1)) : (maxw * maxh * (charSet.size() + 1)));
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
			continue;
		error = FT_Render_Glyph(slot, (format == L1) ? FT_RENDER_MODE_MONO : FT_RENDER_MODE_NORMAL);
		if (error)
			continue;
		int x = slot->bitmap_left - minx;
		int y = slot->bitmap_top - miny;
		int adv = (slot->advance.x >> 6) + minx;
		// printf("Pixel mode: %i\n", (int)slot->bitmap.pixel_mode);
		if (format == L1 && slot->bitmap.pixel_mode != FT_PIXEL_MODE_MONO) // Ensure proper format
			continue;
		else if (slot->bitmap.pixel_mode != FT_PIXEL_MODE_GRAY)
			continue;
		uint8_t *buffer = slot->bitmap.buffer;
		printf("Stride: %i\n", slot->bitmap.pitch);
		if (slot->bitmap.pitch < 0)
		{
			buildError = "Negative bitmap pitch in font not supported"; // TODO
			return;
		}
		int idx = 0;
		int ci = (format == L1) ? i * maxw / 8 * maxh : i * maxw * maxh;
		for (int by = 0; by < slot->bitmap.rows; ++by)
		{
			int ty = y + by;
			for (int bx = 0; bx < slot->bitmap.pitch; ++bx) // NOTE: Stride may be longer than target, hence the + 1 on charSet.size() in the target buffer
			{
				int bi = idx + bx; // Current byte in source buffer
				int tx = x + bx; // Right shift of data
				if (format == L1)
				{
					int txi = tx / 8; // Right shift in bytes
					int txb = tx % 8; // Right shift remaining bits
					int ti = ci + (ty * maxw / 8) + txi;
					uint8_t leftvalue = slot->bitmap.buffer[bi] >> txb;
					uint8_t rightvalue = slot->bitmap.buffer[bi] << (8 - txb);
					bitmapBuffer[ti] |= leftvalue;
					bitmapBuffer[ti + 1] |= rightvalue;
					// TODO: Test L1 generation
				}
				else
				{
					int ti = ci + (ty * maxw) + tx;
					bitmapBuffer[ti] = slot->bitmap.buffer[bi];
					printf("TI %i\n", ti);
				}
			}
			idx += slot->bitmap.pitch;
		}
		// TODO: Store adv in buffer...
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
#else
	buildError = "Freetype not available";
#endif
}

} /* namespace FT800EMUQT */

/* end of file */
