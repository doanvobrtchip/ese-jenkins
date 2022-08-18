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

		static void ConvertRomJ1(string fileName, string fileOut)
		{
			FileStream fso = new FileStream(fileOut, FileMode.Create, FileAccess.Write, FileShare.Read);
			StreamWriter swo = new StreamWriter(fso);
			int chars = 0;
			string fileIn = fileName;
			FileStream fsi = new FileStream(fileIn, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);
			StreamReader sri = new StreamReader(fsi);
			while (!sri.EndOfStream)
			{
				string line = sri.ReadLine().Trim();
				if (line.Length == 0) break;
				if (line.Length != 4) throw new Exception("not a valid line");
				swo.Write("0x");
				swo.Write(line);
				swo.Write(", ");
				++chars;
				if (chars >= 8)
				{
					swo.WriteLine();
					chars = 0;
				}
			}
			sri.Close();
			fsi.Close();
			sri.Dispose();
			fsi.Dispose();
			swo.Close();
			fso.Close();
			swo.Dispose();
			fso.Dispose();
		}

		static void ConverOtpHex(string fileName, string fileOut)
		{
			FileStream fso = new FileStream(fileOut, FileMode.Create, FileAccess.Write, FileShare.Read);
			StreamWriter swo = new StreamWriter(fso);
			int chars = 0;
			string fileIn = fileName;
			FileStream fsi = new FileStream(fileIn, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);
			StreamReader sri = new StreamReader(fsi);
			while (!sri.EndOfStream)
			{
				string line = sri.ReadLine().Trim();
				if (line.Length == 0) break;
				if (line.Length != 2) throw new Exception("not a valid line");
				swo.Write("0x");
				swo.Write(line);
				swo.Write(", ");
				++chars;
				if (chars >= 16)
				{
					swo.WriteLine();
					chars = 0;
				}
			}
			sri.Close();
			fsi.Close();
			sri.Dispose();
			fsi.Dispose();
			swo.Close();
			fso.Close();
			swo.Dispose();
			fso.Dispose();
		}

		static void ConvertRomBin(string fileName, string fileOut)
		{
			FileStream fso = new FileStream(fileOut, FileMode.Create, FileAccess.Write, FileShare.Read);
			StreamWriter swo = new StreamWriter(fso);
			int chars = 0;
			string fileIn = fileName;
			FileStream fsi = new FileStream(fileIn, FileMode.Open, FileAccess.Read, FileShare.ReadWrite);
			while (fsi.Position < fsi.Length)
			{
				// string line = // sri.ReadLine().Trim();
				// if (line.Length == 0) break;
				// if (line.Length != 4) throw new Exception("not a valid line");
				char c = (char)fsi.ReadByte();
				string hex = Convert.ToString(c, 16).PadLeft(2, '0');
				swo.Write("0x");
				swo.Write(hex);
				swo.Write(", ");
				++chars;
				if (chars >= 16)
				{
					swo.WriteLine();
					chars = 0;
				}
			}
			fsi.Close();
			fsi.Dispose();
			swo.Close();
			fso.Close();
			swo.Dispose();
			fso.Dispose();
		}

		static void Main(string[] args)
		{
			// ---------------------------------------------------------
			// FT81X ---------------------------------------------------
			// ---------------------------------------------------------
			ConvertRomMain16(
				@"..\..\..\..\vc2roms\rom_main",
				@"..\..\..\..\..\ft810emu\resources\rom_ft810.h");
			ConvertRomJ1(
				@"..\..\..\..\vc2roms\rom_j1boot",
				@"..\..\..\..\..\ft810emu\resources\crom_ft810.h");
			ConverOtpHex(
				@"..\..\..\..\vc2roms\otp_810.hex",
				@"..\..\..\..\..\ft810emu\resources\otp_810.h");
			ConverOtpHex(
				@"..\..\..\..\vc2roms\otp_811.hex",
				@"..\..\..\..\..\ft810emu\resources\otp_811.h");
			ConverOtpHex(
				@"..\..\..\..\vc2roms\otp_812.hex",
				@"..\..\..\..\..\ft810emu\resources\otp_812.h");
			ConverOtpHex(
				@"..\..\..\..\vc2roms\otp_813.hex",
				@"..\..\..\..\..\ft810emu\resources\otp_813.h");
			// ---------------------------------------------------------
			// ---------------------------------------------------------
			// ---------------------------------------------------------

			// ---------------------------------------------------------
			// BT815 ---------------------------------------------------
			// ---------------------------------------------------------
			ConvertRomBin(
				@"..\..\..\..\vc3roms\mainrom.bin",
				@"..\..\..\..\..\bt815emu\resources\rom_bt815.h");
			ConvertRomJ1(
				@"..\..\..\..\vc3roms\rom_j1boot",
				@"..\..\..\..\..\bt815emu\resources\crom_bt815.h");
			ConverOtpHex(
				@"..\..\..\..\vc3roms\otp_815.hex",
				@"..\..\..\..\..\bt815emu\resources\otp_815.h");
			ConverOtpHex(
				@"..\..\..\..\vc3roms\otp_816.hex",
				@"..\..\..\..\..\bt815emu\resources\otp_816.h");
			// ---------------------------------------------------------
			// ---------------------------------------------------------
			// ---------------------------------------------------------

			// ---------------------------------------------------------
			// BT817/8 -------------------------------------------------
			// ---------------------------------------------------------
			ConvertRomMain16(
				@"..\..\..\..\vc4roms\rom_main",
				@"..\..\..\..\..\bt817emu\resources\rom_bt817.h");
			ConvertRomJ1(
				@"..\..\..\..\vc4roms\rom_j1boot",
				@"..\..\..\..\..\bt817emu\resources\crom_bt817.h");
			ConverOtpHex(
				@"..\..\..\..\vc4roms\otp_817.hex",
				@"..\..\..\..\..\bt817emu\resources\otp_817.h");
			ConverOtpHex(
				@"..\..\..\..\vc4roms\otp_818.hex",
				@"..\..\..\..\..\bt817emu\resources\otp_818.h");
			// ---------------------------------------------------------
			// ---------------------------------------------------------
			// ---------------------------------------------------------

			// ---------------------------------------------------------
			// BT88X ---------------------------------------------------
			// ---------------------------------------------------------
			ConvertRomMain16(
				@"..\..\..\..\vc2roms880\rom_main",
				@"..\..\..\..\..\bt880emu\resources\rom_bt880.h");
			ConvertRomJ1(
				@"..\..\..\..\vc2roms880\rom_j1boot",
				@"..\..\..\..\..\bt880emu\resources\crom_bt880.h");
			ConverOtpHex(
				@"..\..\..\..\vc2roms880\otp_880.hex",
				@"..\..\..\..\..\bt880emu\resources\otp_880.h");
			ConverOtpHex(
				@"..\..\..\..\vc2roms880\otp_881.hex",
				@"..\..\..\..\..\bt880emu\resources\otp_881.h");
			ConverOtpHex(
				@"..\..\..\..\vc2roms880\otp_882.hex",
				@"..\..\..\..\..\bt880emu\resources\otp_882.h");
			ConverOtpHex(
				@"..\..\..\..\vc2roms880\otp_883.hex",
				@"..\..\..\..\..\bt880emu\resources\otp_883.h");
			// ---------------------------------------------------------
			// ---------------------------------------------------------
			// ---------------------------------------------------------
		}
	}
}
