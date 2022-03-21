using System;
using System.IO;

namespace hexrom_conv
{
	internal class Program
	{
		static void ConvertRomMain16(string fileName, string fileOut)
		{
			FileStream fso = new FileStream(fileOut, FileMode.Create, FileAccess.Write, FileShare.Read);
			StreamWriter swo = new StreamWriter(fso);
			int chars = 0;
			FileStream[] fsi = new FileStream[16];
			StreamReader[] sri = new StreamReader[16];
			for (int i = 0; i < 16; ++i)
			{
				string fileIn = fileName + i.ToString().PadLeft(2, '0');
				fsi[i] = new FileStream(fileIn, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);
				sri[i] = new StreamReader(fsi[i]);
			}
			while (!sri[0].EndOfStream)
			{
				for (int i = 0; i < 16; ++i)
				{
					if (!sri[i].EndOfStream)
					{
						string line = sri[i].ReadLine().Trim();
						if (line.Length == 0) break;
						if (line.Length != 4) throw new Exception("not a valid line");
						swo.Write("0x");
						swo.Write(line.Substring(2, 2));
						swo.Write(", ");
						swo.Write("0x");
						swo.Write(line.Substring(0, 2));
						swo.Write(", ");
						++chars;
						if (chars >= 8)
						{
							// swo.WriteLine();
							chars = 0;
						}
					}
				}
				swo.WriteLine();
			}
			for (int i = 0; i < 16; ++i)
			{
				sri[i].Close();
				fsi[i].Close();
				sri[i].Dispose();
				fsi[i].Dispose();
			}
			swo.Close();
			fso.Close();
			swo.Dispose();
			fso.Dispose();
		}

		static void Main(string[] args)
		{
			// FT81X
			ConvertRomMain16(
				@"..\..\..\..\vc2roms\rom_main",
				@"..\..\..\..\..\ft810emu\resources\rom_ft810.h");

			// BT817/8
			ConvertRomMain16(
				@"..\..\..\..\vc4roms\rom_main",
				@"..\..\..\..\..\bt817emu\resources\rom_bt817.h");

			// BT88X
			ConvertRomMain16(
				@"..\..\..\..\vc2roms880\rom_main",
				@"..\..\..\..\..\bt880emu\resources\rom_bt880.h");
		}
	}
}
