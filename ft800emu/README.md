ft800emu
========


Emulator usage:

For an automatic emulator, call FT800EMU::Emulator.run(...) with the necessary initialization parameters.
Make sure the platform pins are correctly wired up using the FT800EMU::Emulator interface before calling run.


Manual usage:

Write to the memory directly through the FT800EMU::Memory or FT800EMU::SPII2C interfaces.
Create a display buffer in ARGB8888 format with the correct resolution as in REG_HSIZE and REG_VSIZE.
Call FT800EMU::GraphicsProcessor.process(buffer, hsize, vsize) to build the display image.
