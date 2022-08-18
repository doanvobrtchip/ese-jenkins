/*
Copyright (C) 2013-2016  Future Technology Devices International Ltd
Copyright (C) 2016-2022  Bridgetek Pte Lte
Author: Jan Boon <jan.boon@kaetemi.be>
*/

#pragma warning(disable : 26812) // Unscoped enum
#pragma warning(disable : 26495) // Uninitialized member
#pragma warning(disable : 26444) // Unnamed objects

#include <bt8xxemu.h>
#include <bt8xxemu_diag.h>
#include <stdio.h>
#include <stdlib.h>
#include <QString>
#include <QPixmap>
#include <QFile>
#include <QTextStream>

#include "../../fteditor/constant_mapping.h"
#include "../../fteditor/constant_common.h"
#include "../../fteditor/dl_parser.h"

void coprocdiff(BT8XXEMU_Emulator *sender, void *context);

const char *g_Case = NULL;
const char *g_Device = NULL;
static int s_Frame = 0;

int graphics(BT8XXEMU_Emulator *sender, void *context, int output, const argb8888 *buffer, uint32_t hsize, uint32_t vsize, BT8XXEMU_FrameFlags flags)
{
	if (output && (flags & BT8XXEMU_FrameSwap))
	{
		if (s_Frame % 30 == 0) // Save every 30 frames
		{
			QString path = QString("cases/coprocdiff/") + g_Device + "/" + QString(g_Case).toLower() + "/" + QString(g_Case).toLower() + "_" + QString::number(s_Frame);
			QImage image(hsize, vsize, QImage::Format_RGB32);
			for (unsigned int y = 0; y < vsize; ++y)
				memcpy(image.scanLine(y), &buffer[y * hsize], sizeof(argb8888) * hsize);
			image.save(path + ".png");
			QByteArray pathOut = path.toLocal8Bit();
			printf("Render %s\n", pathOut.data());
			
			QFile file(path + ".txt");
			file.open(QIODevice::WriteOnly);
			QTextStream out(&file);
			const uint32_t *displayList = BT8XXEMU_getDisplayList(sender);
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
	BT8XXEMU_EmulatorMode mode = BT8XXEMU_EmulatorFT801;
	// Parse command line
	for (int i = 1; i < argc - 1; ++i)
	{
		switch (argv[i][0])
		{
		case '-':
			switch (argv[i][1])
			{
			case 'b':
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
								mode = BT8XXEMU_EmulatorFT800;
								g_Device = "ft800";
								break;
							case '1':
								mode = BT8XXEMU_EmulatorFT801;
								g_Device = "ft801";
								break;
							}
							break;
						case '1':
							switch (argv[i][5])
							{
							case '0':
								mode = BT8XXEMU_EmulatorFT810;
								g_Device = "ft810";
								break;
							case '1':
								mode = BT8XXEMU_EmulatorFT811;
								g_Device = "ft811";
								break;
							case '2':
								mode = BT8XXEMU_EmulatorFT812;
								g_Device = "ft812";
								break;
							case '3':
								mode = BT8XXEMU_EmulatorFT813;
								g_Device = "ft813";
								break;
							case '5':
								mode = BT8XXEMU_EmulatorBT815;
								g_Device = "bt815";
								break;
							case '6':
								mode = BT8XXEMU_EmulatorBT816;
								g_Device = "bt816";
								break;
							case '7':
								mode = BT8XXEMU_EmulatorBT817;
								g_Device = "bt817";
								break;
							case '8':
								mode = BT8XXEMU_EmulatorBT818;
								g_Device = "bt818";
								break;
							}
							break;
						case '8':
							switch (argv[i][5])
							{
							case '0':
								mode = BT8XXEMU_EmulatorBT880;
								g_Device = "bt880";
								break;
							case '1':
								mode = BT8XXEMU_EmulatorBT881;
								g_Device = "bt881";
								break;
							case '2':
								mode = BT8XXEMU_EmulatorBT882;
								g_Device = "bt882";
								break;
							case '3':
								mode = BT8XXEMU_EmulatorBT883;
								g_Device = "bt883";
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

	BT8XXEMU_EmulatorParameters params;
	BT8XXEMU_defaults(BT8XXEMU_VERSION_API, &params, mode);
	params.Flags &= ~BT8XXEMU_EmulatorEnableDynamicDegrade;
	params.Main = coprocdiff;
	params.Graphics = graphics;
	BT8XXEMU_Emulator *emulator;
	BT8XXEMU_run(BT8XXEMU_VERSION_API, &emulator, &params);
	return 0;
}

/* end of file */
