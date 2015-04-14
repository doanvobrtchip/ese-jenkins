#ifndef _SAMPLEAPP_H_
#define _SAMPLEAPP_H_


#define SAMAPP_Lenaface40_SIZE (2769)
#define SAMAPP_Mandrill256_SIZE (14368)
#define SAMAPP_Roboto_BoldCondensed_12_SIZE (19348)

#define SAMAPP_ChineseFont_Metric_SIZE   (148)
#define SAMAPP_ChineseFont_BitmapData_SIZE   (20944)



#if defined(MSVC_PLATFORM) || defined(MSVC_FT800EMU)
/* Compile time switch for enabling sample app api sets - please cross check the same in SampleApp_RawData.cpp file as well */


#define SAMAPP_ENABLE_APIS_SET0
#define SAMAPP_ENABLE_APIS_SET1
#define SAMAPP_ENABLE_APIS_SET2
#define SAMAPP_ENABLE_APIS_SET3
#define SAMAPP_ENABLE_APIS_SET4 
#define SAMAPP_ENABLE_APIS_SET5
#define SAMAPP_ENABLE_APIS_SET6
#define SAMAPP_ENABLE_APIS_SET7
#define SAMAPP_ENABLE_APIS_SET8
#define SAMAPP_ENABLE_APIS_SET9

#endif

#ifdef ARDUINO_PLATFORM
/* Compile time switch for enabling sample app api sets - please cross check the same in SampleApp_RawData.cpp file as well */

#define SAMAPP_ENABLE_APIS_SET0
//#define SAMAPP_ENABLE_APIS_SET1
//#define SAMAPP_ENABLE_APIS_SET2
//#define SAMAPP_ENABLE_APIS_SET3
//#define SAMAPP_ENABLE_APIS_SET4
//#define SAMAPP_ENABLE_APIS_SET5
//#define SAMAPP_ENABLE_APIS_SET6
//#define SAMAPP_ENABLE_APIS_SET7


#endif

#ifdef FT900_PLATFORM
/* Compile time switch for enabling sample app api sets - please cross check the same in SampleApp_RawData.cpp file as well */

#define SAMAPP_ENABLE_APIS_SET0
#define SAMAPP_ENABLE_APIS_SET1
#define SAMAPP_ENABLE_APIS_SET2
#define SAMAPP_ENABLE_APIS_SET3
#define SAMAPP_ENABLE_APIS_SET4
#define SAMAPP_ENABLE_APIS_SET5
#define SAMAPP_ENABLE_APIS_SET6
#define SAMAPP_ENABLE_APIS_SET7
#define SAMAPP_ENABLE_APIS_SET8
#define SAMAPP_ENABLE_APIS_SET9
#endif

//Bouncing Circle macros
#define NO_OF_CIRCLE (5)

//Bouncing Points macros
#define NBLOBS (64)
#define OFFSCREEN   (-16384)
#define FT_APP_BLOBS_COUNT  (0x01)
#define FT_APP_BLOBS_NUMTOUCH  (5)

/* sample app structure definitions */
typedef struct SAMAPP_Bitmap_header
{
	ft_uint8_t Format;
	ft_int16_t Width;
	ft_int16_t Height;
	ft_int16_t Stride;
	ft_int32_t Arrayoffset;
}SAMAPP_Bitmap_header_t;


//bouncing squares
#define NO_OF_RECTS (5)
typedef struct BouncingSquares_t{
	ft_int16_t BRy[5],BRx[5],My[5];
	ft_uint8_t E[5];
	ft_uint8_t RectNo[5];
	ft_int16_t Count;
}BouncingSquares_Context;


//bouncing circles structures
typedef  struct 
{
	ft_uint8_t F[NO_OF_CIRCLE];
}TouchNoAndCircleNo;

typedef struct BouncingCircles_t{
	float Tsq1[NO_OF_CIRCLE];
	float C1X[NO_OF_CIRCLE];
	float C1Y[NO_OF_CIRCLE];
	float TouchX[NO_OF_CIRCLE];
	float TouchY[NO_OF_CIRCLE];
	TouchNoAndCircleNo TN[NO_OF_CIRCLE];
}BouncingCircles_Context;

//bouncing pints structures
typedef struct Blobs 
{
  ft_int16_t x;
  ft_int16_t y;
}Blobs;

typedef struct BlobsInst
{
	Blobs blobs[NBLOBS];
	ft_uint8_t CurrIdx;
}BlobsInst;

//moving points structures
#define NO_OF_POINTS (64)
typedef struct MovingPoints_t
{
	ft_uint8_t Prevtouch;
	ft_int16_t SmallX[6], SmallY;
	ft_uint8_t Flag;
	ft_int32_t val[5];
	ft_int16_t X[(NO_OF_POINTS)*4],Y[(NO_OF_POINTS)*4];
	ft_uint8_t t[((NO_OF_POINTS)*4)];
}MovingPoints_Context;

//main windows 
#define ImW (66)
#define ImH (66)
#define NO_OF_TOUCH (5)

 typedef struct logoim
{
  ft_char8_t name[40];
  ft_uint16_t image_height;
  ft_uint8_t image_format;
  ft_uint8_t filter;
  ft_uint16_t sizex;
  ft_uint16_t sizey;
  ft_uint16_t linestride;
  ft_uint16_t gram_address;
}t_imageprp;


typedef struct Squares 
{
  ft_uint16_t x, y;
}Squares;

#define APPBUFFERSIZE					(65536L)
#define APPBUFFERSIZEMINUSONE		(APPBUFFERSIZE - 1)

extern SAMAPP_Bitmap_header_t  SAMAPP_Bitmap_RawData_Header[];

extern FT_PROGMEM ft_prog_uchar8_t SAMAPP_Bitmap_RawData[];
extern FT_PROGMEM ft_prog_uchar8_t Lenaface40[];
extern FT_PROGMEM ft_prog_uchar8_t Font16[];
extern FT_PROGMEM ft_prog_uchar8_t Mandrill256[];
extern FT_PROGMEM ft_prog_uchar8_t Roboto_BoldCondensed_12[];

/*The font data contains 22 chinese characters, which is generated by fnt_cvt utility from SimFang true type font file of Windows*/
extern FT_PROGMEM ft_prog_uchar8_t SAMApp_ChineseFont_MetricBlock[];
extern FT_PROGMEM ft_prog_uchar8_t  SAMApp_ChineseFont_FontBmpData[];
extern FT_PROGMEM  ft_prog_uchar8_t SAMAPP_Tomato_Bitmap_Data_Bin[];
extern FT_PROGMEM  ft_prog_uchar8_t SAMAPP_Tomato_Palette_Table_Bin[];
extern FT_PROGMEM  ft_prog_uchar8_t SAMAPP_Tomato_Palette_Indexes_Bin[];

extern FT_PROGMEM  ft_prog_uchar8_t SAMAPP_Lowercase_Alphabet_Data[];

extern FT_PROGMEM  ft_prog_uchar8_t SAMAPP_Tomato_DXT1_C0_Data_Raw[];
extern FT_PROGMEM  ft_prog_uchar8_t SAMAPP_Tomato_DXT1_C1_Data_Raw[];

extern FT_PROGMEM  ft_prog_uchar8_t SAMAPP_Tomato_DXT1_B0_Data_Raw[];
extern FT_PROGMEM  ft_prog_uchar8_t SAMAPP_Tomato_DXT1_B1_Data_Raw[];
extern FT_PROGMEM  ft_prog_uchar8_t SAMAPP_Tomato_128_128_PNG_Hex_Dump[];
extern FT_PROGMEM  ft_prog_uchar8_t SAMAPP_Lenaface40_Paletted_L8_PNG_Hex_Dump[];
extern FT_PROGMEM  ft_prog_uchar8_t SAMAPP_LENAFACE40_PALETTED_RGB565_PNG_HEX_DUMP[];
extern FT_PROGMEM  ft_prog_uchar8_t SAMAPP_LENAFACE40_PALETTED_ARGB4_PNG_HEX_DUMP[];

#define SAMAPP_LENAFACE40_PALETTED_L8_PNG_HEX_DUMP_SIZE 2418
#define SAMAPP_LENAFACE40_PALETTED_RGB565_PNG_HEX_DUMP_SIZE 2418
#define SAMAPP_LENAFACE40_PALETTED_ARGB4_PNG_HEX_DUMP_SIZE 2414


ft_void_t SAMAPP_fadeout();
ft_void_t SAMAPP_fadein();
ft_int16_t SAMAPP_qsin(ft_uint16_t a);
ft_int16_t SAMAPP_qcos(ft_uint16_t a);
/* Sample app APIs for graphics primitives */

ft_void_t	SAMAPP_GPU_Points();
ft_void_t	SAMAPP_GPU_Lines();
ft_void_t	SAMAPP_GPU_Rectangles();
ft_void_t	SAMAPP_GPU_Bitmap();
ft_void_t	SAMAPP_GPU_Fonts();
ft_void_t	SAMAPP_GPU_Text8x8();
ft_void_t	SAMAPP_GPU_TextVGA();
ft_void_t	SAMAPP_GPU_Bargraph();
ft_void_t	SAMAPP_GPU_LineStrips();
ft_void_t	SAMAPP_GPU_EdgeStrips();
ft_void_t	SAMAPP_GPU_Scissor();
ft_void_t	SAMAPP_GPU_FtdiString();
ft_void_t	SAMAPP_GPU_StreetMap();
ft_void_t	SAMAPP_GPU_AdditiveBlendText();
ft_void_t	SAMAPP_GPU_MacroUsage();
ft_void_t	SAMAPP_GPU_AdditiveBlendPoints();

/* Sample app APIs for widgets */
ft_void_t SAMAPP_CoPro_Widget_Logo();
ft_void_t SAMAPP_CoPro_Widget_Calibrate();
ft_void_t SAMAPP_CoPro_AppendCmds();
ft_void_t SAMAPP_CoPro_Inflate();
ft_void_t SAMAPP_CoPro_Loadimage();
ft_void_t SAMAPP_CoPro_Widget_Button();
ft_void_t SAMAPP_CoPro_Widget_Clock();
ft_void_t SAMAPP_CoPro_Widget_Guage();
ft_void_t SAMAPP_CoPro_Widget_Gradient();
ft_void_t SAMAPP_CoPro_Widget_Keys();
ft_void_t SAMAPP_CoPro_Widget_Progressbar();
ft_void_t SAMAPP_CoPro_Widget_Scroll();
ft_void_t SAMAPP_CoPro_Widget_Slider();
ft_void_t SAMAPP_CoPro_Widget_Dial();
ft_void_t SAMAPP_CoPro_Widget_Toggle();
ft_void_t SAMAPP_CoPro_Widget_Text();
ft_void_t SAMAPP_CoPro_Widget_Number();
ft_void_t SAMAPP_CoPro_Widget_Spinner();
ft_void_t SAMAPP_CoPro_Screensaver();
ft_void_t SAMAPP_CoPro_Snapshot();
ft_void_t SAMAPP_CoPro_Sketch();
ft_void_t SAMAPP_CoPro_Matrix();
ft_void_t SAMAPP_CoPro_Setfont();
ft_void_t SAMAPP_CoPro_Track();

ft_void_t SAMAPP_PowerMode();
ft_void_t SAMAPP_BootupConfig();

//bouncing squares
ft_void_t RectangleCalc(BouncingSquares_Context *context,ft_uint8_t Arrayno);
ft_void_t CheckTouch(BouncingSquares_Context *context,ft_int16_t Tx1,ft_int32_t val1);
ft_void_t BouncingSquaresCall(ft_int16_t BRx,ft_int16_t BRy,ft_int16_t MovingRy,ft_uint8_t SqNumber);
ft_int16_t MovingRect(ft_int16_t BRy,ft_int16_t MovingRy,ft_uint8_t EndPtReach);
ft_void_t BouncingSquares();

//bouncing circles
ft_uint16_t ConcentricCircles(float C1,ft_uint16_t R,ft_uint16_t G,ft_uint16_t B);
ft_void_t TouchPoints(ft_int16_t C1X,ft_int16_t C1Y,ft_uint8_t i);
ft_void_t PlotXY();
ft_uint8_t CheckCircleTouchCood(BouncingCircles_Context *context, ft_int32_t val,ft_uint8_t TouchNum,ft_uint8_t i);
ft_uint16_t CirclePlot(BouncingCircles_Context *context, ft_uint16_t X,ft_uint16_t Y,ft_uint8_t Val);
ft_void_t StoreTouch(BouncingCircles_Context *context, ft_int32_t Touchval,ft_uint8_t TouchNo);
ft_void_t BouncingCircles();

//bouncing points
ft_void_t BlobColor(BlobsInst *pBInst,ft_int32_t TouchXY);
ft_void_t BouncingPoints();

//moving points
ft_void_t ColorSelection(ft_int16_t k,ft_int16_t i);
ft_int16_t linear(float p1,float p2,ft_uint16_t t,ft_uint16_t rate);
ft_void_t PointsCalc(MovingPoints_Context *context, ft_uint8_t TouchNo, ft_int16_t *X, ft_int16_t *Y, ft_uint8_t *t);
ft_void_t MovingPoints();

//main windows
  void load_inflate_image(ft_uint32_t address, const char *filename);
  void Logo_Intial_setup(struct logoim sptr[],ft_uint8_t num);
#ifdef MSVC_PLATFORM
  void Load_file2gram(ft_uint32_t add,ft_uint8_t sectors, FILE *afile);
#endif
  ft_void_t CheckTouch_tile(Squares *Sq, ft_int32_t TouchXY,ft_uint8_t TouchNo);
  ft_void_t MainWindow();

#endif /* _SAMPLEAPP_H_ */

/* Nothing beyond this */









