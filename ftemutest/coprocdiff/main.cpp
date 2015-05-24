
/*
 * Copyright (C) 2015  Future Technology Devices International Ltd
 * Author: Jan Boon <jan.boon@kaetemi.be>
 */

#include <ft8xxemu.h>
#include <ft8xxemu_diag.h>
#include <stdio.h>
#include <stdlib.h>
#include <QString>
#include <QPixmap>
#include <QFile>
#include <QTextStream>

#include <../../fteditor/constant_mapping.h>
#include <../../fteditor/constant_common.h>
#include <../../fteditor/dl_parser.h>

void setup();
void loop();

const char *g_Case = NULL;
const char *g_Device = NULL;
static int s_Frame = 0;

int graphics(int output, const argb8888 *buffer, uint32_t hsize, uint32_t vsize, FT8XXEMU_FrameFlags flags)
{
	if (output && (flags & FT8XXEMU_FrameSwap))
	{
		if (s_Frame % 30 == 0) // Save every 30 frames
		{
			QString path = QString("cases/coprocdiff/") + g_Device + "/" + QString(g_Case).toLower() + "/" + QString(g_Case).toLower() + "_" + QString::number(s_Frame);
			QImage image(hsize, vsize, QImage::Format_RGB32);
			for (int y = 0; y < vsize; ++y)
				memcpy(image.scanLine(y), &buffer[y * hsize], sizeof(argb8888) * hsize);
			image.save(path + ".png");
			QByteArray pathOut = path.toLocal8Bit();
			printf("Render %s\n", pathOut.data());
			
			QFile file(path + ".txt");
			file.open(QIODevice::WriteOnly);
			QTextStream out(&file);
			const uint32_t *displayList = FT8XXEMU_getDisplayList();
			for (int i = 0; i < FTEDITOR::displayListSize(FTEDITOR_CURRENT_DEVICE); ++i)
				out << FTEDITOR::DlParser::toString(FTEDITOR_CURRENT_DEVICE, displayList[i]) << "\r\n";
		}
		++s_Frame;
	}
	if (s_Frame > 600)
		return 0;
	return 1;
}

int main(int argc, char *argv[])
{
	FT8XXEMU_EmulatorMode mode = FT8XXEMU_EmulatorFT801;
	// Parse command line
	for (int i = 1; i < argc - 1; ++i)
	{
		switch (argv[i][0])
		{
		case '-':
			switch (argv[i][1])
			{
			case 'f':
				switch (argv[i][2])
				{
				case 't':
					switch (argv[i][3])
					{
					case '8':
						switch (argv[i][4])
						{
						case '0':
							switch (argv[i][5])
							{
							case '0':
								mode = FT8XXEMU_EmulatorFT800;
								g_Device = "ft800";
								break;
							case '1':
								mode = FT8XXEMU_EmulatorFT801;
								g_Device = "ft801";
								break;
							}
							break;
						case '1':
							switch (argv[i][5])
							{
							case '0':
								mode = FT8XXEMU_EmulatorFT810;
								g_Device = "ft810";
								break;
							case '1':
								mode = FT8XXEMU_EmulatorFT811;
								g_Device = "ft811";
								break;
							}
							break;
						}
						break;
					}
					break;
				}
				break;
			}
			break;
		}
	}
	g_Case = argv[argc - 1];

	FTEDITOR::g_CurrentDevice = FTEDITOR::deviceToIntf(mode);

	FT8XXEMU_EmulatorParameters params;
	FT8XXEMU_defaults(FT8XXEMU_VERSION_API, &params, mode);
	params.Flags &= ~FT8XXEMU_EmulatorEnableDynamicDegrade;
	params.Setup = setup;
	params.Loop = loop;
	params.Graphics = graphics;
	FT8XXEMU_run(FT8XXEMU_VERSION_API, &params);
	return 0;
}

/* end of file */
