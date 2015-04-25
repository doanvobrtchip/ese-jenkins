<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="en_US">
<context>
    <name>CodeEditor</name>
    <message>
        <location filename="code_editor.cpp" line="146"/>
        <source>Edit code</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>FT800EMUQT</name>
    <message>
        <source>Start typing in the &lt;b&gt;Coprocessor&lt;/b&gt; editor, or drag and drop items from the &lt;b&gt;Toolbox&lt;/b&gt; onto the display viewport.</source>
        <translation type="vanished">Start typing in the &lt;b&gt;Coprocessor&lt;/b&gt; editor, or drag and drop items from the &lt;b&gt;Toolbox&lt;/b&gt; onto the display viewport.</translation>
    </message>
</context>
<context>
    <name>FT800EMUQT::InteractiveProperties</name>
    <message>
        <source>Unknown command &apos;&lt;i&gt;%s&lt;/i&gt;&apos;</source>
        <translation type="vanished">Unknown command &apos;&lt;i&gt;%s&lt;/i&gt;&apos;</translation>
    </message>
    <message>
        <source>DESCRIPTION_VERTEX2F.</source>
        <oldsource>DESCRIPTION_VERTEX2F</oldsource>
        <translation type="vanished">&lt;b&gt;VERTEX2F&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: Signed x-coordinate in 1/16 pixel precision&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: Signed x-coordinate in 1/16 pixel precision&lt;br&gt;&lt;br&gt;Start the operation of graphics primitives at the specified screen coordinate, in 1/16th pixel precision.&lt;br&gt;&lt;br&gt;The range of coordinates can be from -16384 to +16383 in terms of 1/16 th pixel units. Please note the negative x coordinate value means the coordinate in the left virtual screen from (0, 0), while the negative y coordinate value means the coordinate in the upper virtual screen from (0, 0). If drawing on the negative coordinate position, the drawing operation will not be visible.</translation>
    </message>
    <message>
        <source>DESCRIPTION_VERTEX2II.</source>
        <oldsource>DESCRIPTION_VERTEX2II</oldsource>
        <translation type="vanished">&lt;b&gt;VERTEX2II&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;handle&lt;/i&gt;, &lt;/i&gt;cell&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: x-coordinate in pixels&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: y-coordinate in pixels&lt;br&gt;&lt;b&gt;handle&lt;/b&gt;: Bitmap handle. The valid range is from 0 to 31. From 16 to 31, the bitmap handle is dedicated to the FT800 built-in font.&lt;br&gt;&lt;b&gt;cell&lt;/b&gt;: Cell number. Cell number is the index of bitmap with same bitmap layout and format. For example, for handle 31, the cell 65 means the character &quot;A&quot; in the largest built in font.&lt;br&gt;&lt;br&gt;Start the operation of graphics primitive at the specified coordinates. The handle and cell parameters will be ignored unless the graphics primitive is specified as bitmap by command BEGIN, prior to this command.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_DLSTART.</source>
        <oldsource>DESCRIPTION_CMD_DLSTART</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_DLSTART&lt;/b&gt;()&lt;br&gt;&lt;br&gt;When the co-processor engine executes this command, it waits until the display list is ready for writing, then sets REG_CMD_DL to zero.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_SWAP.</source>
        <oldsource>DESCRIPTION_CMD_SWAP</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_SWAP&lt;/b&gt;()&lt;br&gt;&lt;br&gt;When the co-processor engine executes this command, it requests a display list swap by writing to REG_DLSWAP.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_INTERRUPT.</source>
        <oldsource>DESCRIPTION_CMD_INTERRUPT</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_INTERRUPT&lt;/b&gt;(&lt;i&gt;ms&lt;/i&gt;)&lt;br&gt;&lt;b&gt;ms&lt;/b&gt;: Delay before interrupt triggers, in milliseconds. The interrupt is guaranteed not to fire before this delay. If ms is zero, the interrupt fires immediately.&lt;br&gt;&lt;br&gt;When the co-processor engine executes this command, it triggers interrupt INT_CMDFLAG.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_BGCOLOR.</source>
        <oldsource>DESCRIPTION_CMD_BGCOLOR</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_BGCOLOR&lt;/b&gt;(&lt;i&gt;r&lt;/i&gt;, &lt;i&gt;g&lt;/i&gt;, &lt;i&gt;b&lt;/i&gt;)&lt;br&gt;&lt;b&gt;rgb&lt;/b&gt;: New background color, as a 24-bit RGB number. Red is the most significant 8 bits, blue is the least. So 0xff0000 is bright red.&lt;br&gt;Background color is applicable for things that the user cannot move. Example behind gauges and sliders etc.&lt;br&gt;&lt;br&gt;Set the background color.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_FGCOLOR.</source>
        <oldsource>DESCRIPTION_CMD_FGCOLOR</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_FGCOLOR&lt;/b&gt;(&lt;i&gt;r&lt;/i&gt;, &lt;i&gt;g&lt;/i&gt;, &lt;i&gt;b&lt;/i&gt;)&lt;br&gt;&lt;b&gt;rgb&lt;/b&gt;: New foreground color, as a 24-bit RGB number. Red is the most significant 8 bits, blue is the least. So 0xff0000 is bright red.&lt;br&gt;Foreground color is applicable for things that the user can move such as handles and buttons (&quot;affordances&quot;).&lt;br&gt;&lt;br&gt;Set the foreground color.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_GRADIENT.</source>
        <oldsource>DESCRIPTION_CMD_GRADIENT</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_GRADIENT&lt;/b&gt;(&lt;i&gt;x0&lt;/i&gt;, &lt;i&gt;y0&lt;/i&gt;, &lt;i&gt;a0&lt;/i&gt;, &lt;i&gt;r0&lt;/i&gt;, &lt;i&gt;g0&lt;/i&gt;, &lt;i&gt;b0&lt;/i&gt;, &lt;i&gt;x1&lt;/i&gt;, &lt;i&gt;y1&lt;/i&gt;, &lt;i&gt;a1&lt;/i&gt;, &lt;i&gt;r1&lt;/i&gt;, &lt;i&gt;g1&lt;/i&gt;, &lt;i&gt;b1&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x0&lt;/b&gt;: x-coordinate of point 0, in pixels&lt;br&gt;&lt;b&gt;y0&lt;/b&gt;: y-coordinate of point 0, in pixels&lt;br&gt;&lt;b&gt;argb0&lt;/b&gt;: Color of point 0, as a 24-bit RGB number. R is the most significant 8 bits, B is the least. So 0xff0000 is bright red.&lt;br&gt;&lt;b&gt;x1&lt;/b&gt;: x-coordinate of point 1, in pixels&lt;br&gt;&lt;b&gt;y1&lt;/b&gt;: y-coordinate of point 1, in pixels&lt;br&gt;&lt;b&gt;argb1&lt;/b&gt;: Color of point 1.&lt;br&gt;&lt;br&gt;Draw a smooth color gradient.&lt;br&gt;&lt;br&gt;All the colours step values are calculated based on smooth curve interpolated from the RGB0 to RGB1 parameter. The smooth curve equation is independently calculated for all three colors and the equation used is R0 + t * (R1 - R0), where t is interpolated between 0 and 1. Gradient must be used with Scissor function to get the intended gradient display.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_TEXT.</source>
        <oldsource>DESCRIPTION_CMD_TEXT</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_TEXT&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;font&lt;/i&gt;, &lt;i&gt;options&lt;/i&gt;, &lt;i&gt;s&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: x-coordinate of text base, in pixels&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: y-coordinate of text base, in pixels&lt;br&gt;&lt;b&gt;font&lt;/b&gt;: Font to use for text, 0-31. See ROM and RAM Fonts&lt;br&gt;&lt;b&gt;options&lt;/b&gt;: By default (x; y) is the top-left pixel of the text. OPT_CENTERX centers the text horizontally, OPT_CENTERY centers it vertically. OPT_CENTER centers the text in both directions. OPT_RIGHTX right-justifies the text, so that the x is the rightmost pixel.&lt;br&gt;&lt;b&gt;s&lt;/b&gt;: text&lt;br&gt;&lt;br&gt;Draw text.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_BUTTON.</source>
        <oldsource>DESCRIPTION_CMD_BUTTON</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_BUTTON&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;w&lt;/i&gt;, &lt;i&gt;h&lt;/i&gt;, &lt;i&gt;font&lt;/i&gt;, &lt;i&gt;options&lt;/i&gt;, &lt;i&gt;s&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: x-coordinate of button top-left, in pixels&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: y-coordinate of button top-left, in pixels&lt;br&gt;&lt;b&gt;w&lt;/b&gt;: width of button, in pixels&lt;br&gt;&lt;b&gt;h&lt;/b&gt;: height of button, in pixels&lt;br&gt;&lt;b&gt;font&lt;/b&gt;: Font to use for text, 0-31. See ROM and RAM Fonts&lt;br&gt;&lt;b&gt;options&lt;/b&gt;: By default the button is drawn with a 3D effect. OPT_FLAT removes the 3D effect.&lt;br&gt;&lt;b&gt;s&lt;/b&gt;: button label&lt;br&gt;&lt;br&gt;Draw a button.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_KEYS.</source>
        <oldsource>DESCRIPTION_CMD_KEYS</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_KEYS&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;w&lt;/i&gt;, &lt;i&gt;h&lt;/i&gt;, &lt;i&gt;font&lt;/i&gt;, &lt;i&gt;options&lt;/i&gt;, &lt;i&gt;s&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: x-coordinate of keys top-left, in pixels&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: y-coordinate of keys top-left, in pixels&lt;br&gt;&lt;b&gt;w&lt;/b&gt;: width of keys, in pixels&lt;br&gt;&lt;b&gt;h&lt;/b&gt;: height of keys, in pixels&lt;br&gt;&lt;b&gt;font&lt;/b&gt;: Font to use for keys, 0-31. See ROM and RAM Fonts&lt;br&gt;&lt;b&gt;options&lt;/b&gt;: By default the keys are drawn with a 3D effect. OPT_FLAT removes the 3D effect. If OPT_CENTER is given the keys are drawn at minimum size centered within the w x h rectangle. Otherwise the keys are expanded so that they completely fill the available space. If an ASCII code is specified, that key is drawn &apos;pressed&apos; - i.e. in background color with any 3D effect removed..&lt;br&gt;&lt;b&gt;s&lt;/b&gt;: key labels, one character per key. The TAG value is set to the ASCII value of each key, so that key presses can be detected using the REG_TOUCH_TAG register.&lt;br&gt;&lt;br&gt;Draw a row of keys.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_PROGRESS.</source>
        <oldsource>DESCRIPTION_CMD_PROGRESS</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_PROGRESS&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;w&lt;/i&gt;, &lt;i&gt;h&lt;/i&gt;, &lt;i&gt;options&lt;/i&gt;, &lt;i&gt;val&lt;/i&gt;, &lt;i&gt;range&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: x-coordinate of progress bar top-left, in pixels&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: y-coordinate of progress bar top-left, in pixels&lt;br&gt;&lt;b&gt;w&lt;/b&gt;: width of progress bar, in pixels&lt;br&gt;&lt;b&gt;h&lt;/b&gt;: height of progress bar, in pixels&lt;br&gt;&lt;b&gt;options&lt;/b&gt;: By default the progress bar is drawn with a 3D effect. OPT_FLAT removes the 3D effect&lt;br&gt;&lt;b&gt;val&lt;/b&gt;: Displayed value of progress bar, between 0 and range inclusive&lt;br&gt;&lt;b&gt;range&lt;/b&gt;: Maximum value&lt;br&gt;&lt;br&gt;Draw a progress bar.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_SLIDER.</source>
        <oldsource>DESCRIPTION_CMD_SLIDER</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_SLIDER&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;w&lt;/i&gt;, &lt;i&gt;h&lt;/i&gt;, &lt;i&gt;options&lt;/i&gt;, &lt;i&gt;val&lt;/i&gt;, &lt;i&gt;range&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: x-coordinate of slider top-left, in pixels&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: y-coordinate of slider top-left, in pixels&lt;br&gt;&lt;b&gt;w&lt;/b&gt;: width of slider, in pixels&lt;br&gt;&lt;b&gt;h&lt;/b&gt;: height of slider, in pixels&lt;br&gt;&lt;b&gt;options&lt;/b&gt;: By default the slider is drawn with a 3D effect. OPT_FLAT removes the 3D effect&lt;br&gt;&lt;b&gt;val&lt;/b&gt;: Displayed value of slider, between 0 and range inclusive&lt;br&gt;&lt;b&gt;range&lt;/b&gt;: Maximum value&lt;br&gt;&lt;br&gt;Draw a slider.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_SCROLLBAR.</source>
        <oldsource>DESCRIPTION_CMD_SCROLLBAR</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_SCROLLBAR&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;w&lt;/i&gt;, &lt;i&gt;h&lt;/i&gt;, &lt;i&gt;options&lt;/i&gt;, &lt;i&gt;val&lt;/i&gt;, &lt;i&gt;size&lt;/i&gt;, &lt;i&gt;range&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: x-coordinate of scroll bar top-left, in pixels&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: y-coordinate of scroll bar top-left, in pixels&lt;br&gt;&lt;b&gt;w&lt;/b&gt;: width of scroll bar, in pixels. If width is greater, the scroll bar is drawn horizontally&lt;br&gt;&lt;b&gt;h&lt;/b&gt;: height of scroll bar, in pixels. If height is greater, the scroll bar is drawn vertically&lt;br&gt;&lt;b&gt;options&lt;/b&gt;: By default the scroll bar is drawn with a 3D effect. OPT_FLAT removes the 3D effect&lt;br&gt;&lt;b&gt;val&lt;/b&gt;: Displayed value of scroll bar, between 0 and range inclusive&lt;br&gt;&lt;b&gt;size&lt;/b&gt;: Size&lt;br&gt;&lt;b&gt;range&lt;/b&gt;: Maximum value&lt;br&gt;&lt;br&gt;Draw a scroll bar.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_TOGGLE.</source>
        <oldsource>DESCRIPTION_CMD_TOGGLE</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_TOGGLE&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;w&lt;/i&gt;, &lt;i&gt;f&lt;/i&gt;, &lt;i&gt;options&lt;/i&gt;, &lt;i&gt;state&lt;/i&gt;, &lt;i&gt;s&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: x-coordinate of top-left of toggle, in pixels&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: y-coordinate of top-left of toggle, in pixels&lt;br&gt;&lt;b&gt;w&lt;/b&gt;: width of toggle, in pixels&lt;br&gt;&lt;b&gt;f&lt;/b&gt;: font to use for text, 0-31. See ROM and RAM Fonts&lt;br&gt;&lt;b&gt;options&lt;/b&gt;: By default the toggle bar is drawn with a 3D effect. OPT_FLAT removes the 3D effect&lt;br&gt;&lt;b&gt;state&lt;/b&gt;: state of the toggle: 0 is off, 65535 is on.&lt;br&gt;&lt;b&gt;s&lt;/b&gt;: String label for toggle. A character value of 255 (in C it can be written as �) separates the two labels.&lt;br&gt;&lt;br&gt;Draw a toggle switch.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_GAUGE</source>
        <translation type="vanished">&lt;b&gt;CMD_GAUGE&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;r&lt;/i&gt;, &lt;i&gt;options&lt;/i&gt;, &lt;i&gt;major&lt;/i&gt;, &lt;i&gt;minor&lt;/i&gt;, &lt;i&gt;val&lt;/i&gt;, &lt;i&gt;range&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: X-coordinate of gauge center, in pixels&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: Y-coordinate of gauge center, in pixels&lt;br&gt;&lt;b&gt;r&lt;/b&gt;: Radius of the gauge, in pixels&lt;br&gt;&lt;b&gt;options&lt;/b&gt;: By default the gauge dial is drawn with a 3D effect. OPT_FLAT removes the 3D effect. With option OPT_NOBACK, the background is not drawn. With option OPT_NOTICKS, the tick marks are not drawn. With option OPT_NOPOINTER, the pointer is not drawn.&lt;br&gt;&lt;b&gt;major&lt;/b&gt;: Number of major subdivisions on the dial, 1-10&lt;br&gt;&lt;b&gt;minor&lt;/b&gt;: Number of minor subdivisions on the dial, 1-10&lt;br&gt;&lt;b&gt;val&lt;/b&gt;: Gauge indicated value, between 0 and range, inclusive&lt;br&gt;&lt;b&gt;range&lt;/b&gt;: Maximum value&lt;br&gt;&lt;br&gt;Draw a gauge.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_CLOCK.</source>
        <oldsource>DESCRIPTION_CMD_CLOCK</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_CLOCK&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;r&lt;/i&gt;, &lt;i&gt;options&lt;/i&gt;, &lt;i&gt;h&lt;/i&gt;, &lt;i&gt;m&lt;/i&gt;, &lt;i&gt;s&lt;/i&gt;, &lt;i&gt;ms&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: X-coordinate of clock center, in pixels&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: Y-coordinate of clock center, in pixels&lt;br&gt;&lt;b&gt;r&lt;/b&gt;: Radius of the clock, in pixels&lt;br&gt;&lt;b&gt;options&lt;/b&gt;: By default the clock dial is drawn with a 3D effect. OPT_FLAT removes the 3D effect. With option OPT_NOBACK, the background is not drawn. With option OPT_NOTICKS, the twelve hour ticks are not drawn. With option OPT_NOSECS, the seconds hand is not drawn. With option OPT_NOHANDS, no hands are drawn. With option OPT_NOHM, no hour and minutes hands are drawn.&lt;br&gt;&lt;b&gt;h&lt;/b&gt;: hours&lt;br&gt;&lt;b&gt;m&lt;/b&gt;: minutes&lt;br&gt;&lt;b&gt;s&lt;/b&gt;: seconds&lt;br&gt;&lt;b&gt;ms&lt;/b&gt;: milliseconds&lt;br&gt;&lt;br&gt;Draw a clock.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_CALIBRATE.</source>
        <oldsource>DESCRIPTION_CMD_CALIBRATE</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_CALIBRATE&lt;/b&gt;(&lt;i&gt;result&lt;/i&gt;)&lt;br&gt;&lt;b&gt;result&lt;/b&gt;: output parameter; written with 0 on failure&lt;br&gt;&lt;br&gt;The calibration procedure collects three touches from the touch screen, then computes and loads an appropriate matrix into REG_TOUCH_TRANSFORM_A-F. To use it, create a display list and then use CMD_CALIBRATE. The co-processor engine overlays the touch targets on the current display list, gathers the calibration input and updates REG_TOUCH_TRANSFORM_A-F.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_SPINNER.</source>
        <oldsource>DESCRIPTION_CMD_SPINNER</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_SPINNER&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;style&lt;/i&gt;, &lt;i&gt;scale&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: x&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: y&lt;br&gt;&lt;b&gt;style&lt;/b&gt;: style&lt;br&gt;&lt;b&gt;scale&lt;/b&gt;: scale&lt;br&gt;&lt;br&gt;The spinner is an animated overlay that shows the user that some task is continuing. To trigger the spinner, create a display list and then use CMD_SPINNER. The co-processor engine overlays the spinner on the current display list, swaps the display list to make it visible, then continuously animates until it receives CMD_STOP. REG_MACRO_0 and REG_MACRO_1 registers are utilized to perform the animation kind of effect. The frequency of points movement is wrt display frame rate configured.&lt;br&gt;Typically for 480x272 display panels the display rate is ~60fps. For style 0 and 60fps, the point repeats the sequence within 2 seconds. For style 1 and 60fps, the point repeats the sequence within 1.25 seconds. For style 2 and 60fps, the clock hand repeats the sequence within 2 seconds. For style 3 and 60fps, the moving dots repeat the sequence within 1 second.&lt;br&gt;Note that only one of CMD_SKETCH, CMD_CSKETCH, CMD_SCREENSAVER, or CMD_SPINNER can be active at one time.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_STOP.</source>
        <oldsource>DESCRIPTION_CMD_STOP</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_STOP&lt;/b&gt;()&lt;br&gt;&lt;br&gt;Stop any spinner, screensaver or sketch.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_MEMSET.</source>
        <oldsource>DESCRIPTION_CMD_MEMSET</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_MEMSET&lt;/b&gt;(&lt;i&gt;ptr&lt;/i&gt;, &lt;i&gt;value&lt;/i&gt;, &lt;i&gt;num&lt;/i&gt;)&lt;br&gt;&lt;b&gt;ptr&lt;/b&gt;: Starting address of the memory block&lt;br&gt;&lt;b&gt;value&lt;/b&gt;: Value to be written to memory&lt;br&gt;&lt;b&gt;num&lt;/b&gt;: Number of bytes in the memory block&lt;br&gt;&lt;br&gt;Fill memory with a byte value.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_MEMZERO.</source>
        <oldsource>DESCRIPTION_CMD_MEMZERO</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_MEMZERO&lt;/b&gt;(&lt;i&gt;ptr&lt;/i&gt;, &lt;i&gt;num&lt;/i&gt;)&lt;br&gt;&lt;b&gt;ptr&lt;/b&gt;: Starting address of the memory block&lt;br&gt;&lt;b&gt;num&lt;/b&gt;: Number of bytes in the memory block&lt;br&gt;&lt;br&gt;Write zero to a block of memory.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_MEMCPY.</source>
        <oldsource>DESCRIPTION_CMD_MEMCPY</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_MEMCPY&lt;/b&gt;(&lt;i&gt;dest&lt;/i&gt;, &lt;i&gt;src&lt;/i&gt;, &lt;i&gt;num&lt;/i&gt;)&lt;br&gt;&lt;b&gt;dest&lt;/b&gt;: address of the destination memory block&lt;br&gt;&lt;b&gt;src&lt;/b&gt;: address of the source memory block&lt;br&gt;&lt;b&gt;num&lt;/b&gt;: Number of bytes in the memory block&lt;br&gt;&lt;br&gt;Copy a block of memory.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_APPEND.</source>
        <oldsource>DESCRIPTION_CMD_APPEND</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_APPEND&lt;/b&gt;(&lt;i&gt;ptr&lt;/i&gt;, &lt;i&gt;num&lt;/i&gt;)&lt;br&gt;&lt;b&gt;ptr&lt;/b&gt;: Start of source commands in main memory&lt;br&gt;&lt;b&gt;num&lt;/b&gt;: Number of bytes to copy. This must be a multiple of 4&lt;br&gt;&lt;br&gt;Append memory to display list.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_SNAPSHOT.</source>
        <oldsource>DESCRIPTION_CMD_SNAPSHOT</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_SNAPSHOT&lt;/b&gt;(&lt;i&gt;ptr&lt;/i&gt;)&lt;br&gt;&lt;b&gt;ptr&lt;/b&gt;: Snapshot destination address, in main memory&lt;br&gt;&lt;br&gt;Take a snapshot of the current screen.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_LOADIDENTITY.</source>
        <oldsource>DESCRIPTION_CMD_LOADIDENTITY</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_LOADIDENTITY&lt;/b&gt;()&lt;br&gt;&lt;br&gt;Set the current matrix to identity.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_TRANSLATE.</source>
        <oldsource>DESCRIPTION_CMD_TRANSLATE</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_TRANSLATE&lt;/b&gt;(&lt;i&gt;tx&lt;/i&gt;, &lt;i&gt;ty&lt;/i&gt;)&lt;br&gt;&lt;b&gt;tx&lt;/b&gt;: x translate factor, in signed 16.16 bit fixed-point form.&lt;br&gt;&lt;b&gt;ty&lt;/b&gt;: y translate factor, in signed 16.16 bit fixed-point form.&lt;br&gt;&lt;br&gt;Apply a translation to the current matrix.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_SCALE.</source>
        <oldsource>DESCRIPTION_CMD_SCALE</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_SCALE&lt;/b&gt;(&lt;i&gt;sx&lt;/i&gt;, &lt;i&gt;sy&lt;/i&gt;)&lt;br&gt;&lt;b&gt;sx&lt;/b&gt;: x scale factor, in signed 16.16 bit fixed-point form.&lt;br&gt;&lt;b&gt;sy&lt;/b&gt;: y scale factor, in signed 16.16 bit fixed-point form.&lt;br&gt;&lt;br&gt;Apply a scale to the current matrix.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_ROTATE.</source>
        <oldsource>DESCRIPTION_CMD_ROTATE</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_ROTATE&lt;/b&gt;(&lt;i&gt;a&lt;/i&gt;)&lt;br&gt;&lt;b&gt;a&lt;/b&gt;: Clockwise rotation angle, in units of 1/65536 of a circle.&lt;br&gt;&lt;br&gt;Apply a rotation to the current matrix.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_SETMATRIX.</source>
        <oldsource>DESCRIPTION_CMD_SETMATRIX</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_SETMATRIX&lt;/b&gt;()&lt;br&gt;&lt;br&gt;Write the current matrix as a bitmap transform.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_SETFONT.</source>
        <oldsource>DESCRIPTION_CMD_SETFONT</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_SETFONT&lt;/b&gt;(&lt;i&gt;font&lt;/i&gt;, &lt;i&gt;ptr&lt;/i&gt;)&lt;br&gt;&lt;b&gt;font&lt;/b&gt;: font&lt;br&gt;&lt;b&gt;ptr&lt;/b&gt;: ptr&lt;br&gt;&lt;br&gt;To use a custom font with the co-processor engine objects, create the font definition data in FT800 RAM and issue CMD_SETFONT, as described in ROM and RAM Fonts.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_TRACK.</source>
        <oldsource>DESCRIPTION_CMD_TRACK</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_TRACK&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;w&lt;/i&gt;, &lt;i&gt;h&lt;/i&gt;, &lt;i&gt;tag&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: x-coordinate of track area top-left, in pixels&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: y-coordinate of track area top-left, in pixels&lt;br&gt;&lt;b&gt;w&lt;/b&gt;: Width of track area, in pixels&lt;br&gt;&lt;b&gt;h&lt;/b&gt;: Height of track area, in pixels.&lt;br&gt;A w and h of (1,1) means that the tracker is rotary, and reports an angle value in REG_TRACKER. (0,0) disables the tracker. Other values mean that the tracker is linear, and reports values along its length from 0 to 65535 in REG_TRACKER.&lt;br&gt;&lt;b&gt;tag&lt;/b&gt;: tag for this track, 1-255&lt;br&gt;&lt;br&gt;The co-processor engine can assist the MCU in tracking touches on graphical objects. For example touches on dial objects can be reported as angles, saving MCU computation. To do this the MCU draws the object using a chosen tag value, and registers a track area for that tag.&lt;br&gt;From then on any touch on that object is reported in REG_TRACKER. The MCU can detect any touch on the object by reading the 32-bit value in REG_TRACKER. The low 8 bits give the current tag, or zero if there is no touch. The high sixteen bits give the tracked value.&lt;br&gt;For a rotary tracker - used for clocks, gauges and dials - this value is the angle of the touch point relative to the object center, in units of 1=65536 of a circle. 0 means that the angle is straight down, 0x4000 left, 0x8000 up, and 0xc000 right.&lt;br&gt;For a linear tracker - used for sliders and scrollbars - this value is the distance along the tracked object, from 0 to 65535.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_DIAL.</source>
        <oldsource>DESCRIPTION_CMD_DIAL</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_DIAL&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;r&lt;/i&gt;, &lt;i&gt;options&lt;/i&gt;, &lt;i&gt;val&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: x-coordinate of dial center, in pixels&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: y-coordinate of dial center, in pixels&lt;br&gt;&lt;b&gt;r&lt;/b&gt;: radius of dial, in pixels&lt;br&gt;&lt;b&gt;options&lt;/b&gt;: By default the dial is drawn with a 3D effect. OPT_FLAT removes the 3D effect&lt;br&gt;&lt;b&gt;val&lt;/b&gt;: Displayed value of slider, between 0 and 65535 inclusive. 0 means that the dial points straight down, 0x4000 left, 0x8000 up, and 0xc000 right.&lt;br&gt;&lt;br&gt;Draw a rotary dial control.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_NUMBER.</source>
        <oldsource>DESCRIPTION_CMD_NUMBER</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_NUMBER&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;font&lt;/i&gt;, &lt;i&gt;options&lt;/i&gt;, &lt;i&gt;n&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: x-coordinate of text base, in pixels&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: y-coordinate of text base, in pixels&lt;br&gt;&lt;b&gt;font&lt;/b&gt;: Font to use for text, 0-31. See ROM and RAM Fonts&lt;br&gt;&lt;b&gt;options&lt;/b&gt;: By default (x; y) is the top-left pixel of the text. OPT_CENTERX centers the text horizontally, OPT_CENTERY centers it vertically. OPT_CENTER centers the text in both directions. OPT_RIGHTX right-justifies the text, so that the x is the rightmost pixel. By default the number is displayed with no leading zeroes, but if a width 1-9 is specified in the options, then the number is padded if necessary with leading zeroes so that it has the given width. If OPT_SIGNED is given, the number is treated as signed, and prefixed by a minus sign if negative.&lt;br&gt;&lt;b&gt;n&lt;/b&gt;: The number to display, either unsigned or signed 32-bit&lt;br&gt;&lt;br&gt;Draw text.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_SCREENSAVER.</source>
        <oldsource>DESCRIPTION_CMD_SCREENSAVER</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_SCREENSAVER&lt;/b&gt;()&lt;br&gt;&lt;br&gt;After the screensaver command, the co-processor engine continuously updates REG_MACRO_0 with VERTEX2F with varying (x; y) coordinates. With an appropriate display list, this causes a bitmap to move around the screen without any MCU work.&lt;br&gt;Command CMD_STOP stops the update process.&lt;br&gt;Note that only one of CMD_SKETCH, CMD_CSKETCH, CMD_SCREENSAVER, or CMD_SPINNER can be active at one time.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_SKETCH.</source>
        <oldsource>DESCRIPTION_CMD_SKETCH</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_SKETCH&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;w&lt;/i&gt;, &lt;i&gt;h&lt;/i&gt;, &lt;i&gt;ptr&lt;/i&gt;, &lt;i&gt;format&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: x-coordinate of sketch area, in pixels&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: y-coordinate of sketch area, in pixels&lt;br&gt;&lt;b&gt;w&lt;/b&gt;: Width of sketch area, in pixels&lt;br&gt;&lt;b&gt;h&lt;/b&gt;: Height of sketch area, in pixels&lt;br&gt;&lt;b&gt;ptr&lt;/b&gt;: Base address of sketch bitmap&lt;br&gt;&lt;b&gt;format&lt;/b&gt;: Format of sketch bitmap, either L1 or L8&lt;br&gt;&lt;br&gt;After the sketch command, the co-processor engine continuously samples the touch inputs and paints pixels into a bitmap, according to the touch (x; y). This means that the user touch inputs are drawn into the bitmap without any need for MCU work.&lt;br&gt;Command CMD_STOP stops the update process.&lt;br&gt;Note that only one of CMD_SKETCH, CMD_CSKETCH, CMD_SCREENSAVER, or CMD_SPINNER can be active at one time.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_CSKETCH.</source>
        <oldsource>DESCRIPTION_CMD_CSKETCH</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_CSKETCH&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;w&lt;/i&gt;, &lt;i&gt;h&lt;/i&gt;, &lt;i&gt;ptr&lt;/i&gt;, &lt;i&gt;format&lt;/i&gt;, &lt;i&gt;freq&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: x-coordinate of sketch area, in pixels&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: y-coordinate of sketch area, in pixels&lt;br&gt;&lt;b&gt;w&lt;/b&gt;: Width of sketch area, in pixels&lt;br&gt;&lt;b&gt;h&lt;/b&gt;: Height of sketch area, in pixels&lt;br&gt;&lt;b&gt;ptr&lt;/b&gt;: Base address of sketch bitmap&lt;br&gt;&lt;b&gt;format&lt;/b&gt;: Format of sketch bitmap, either L1 or L8&lt;br&gt;&lt;b&gt;freq&lt;/b&gt;: The oversampling frequency. The typical value is 1500 to make sure the lines are connected smoothly. The value zero means no oversampling operation&lt;br&gt;&lt;br&gt;This command is only valid for FT801 silicon. FT801 co-processor will oversample the coordinates reported by the capacitive touch panel in the frequency of �freq� and forms the lines with a smoother effect.&lt;br&gt;&lt;br&gt;After the sketch command, the co-processor engine continuously samples the touch inputs and paints pixels into a bitmap, according to the touch (x; y). This means that the user touch inputs are drawn into the bitmap without any need for MCU work.&lt;br&gt;Command CMD_STOP stops the update process.&lt;br&gt;Note that only one of CMD_SKETCH, CMD_CSKETCH, CMD_SCREENSAVER, or CMD_SPINNER can be active at one time.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_LOGO.</source>
        <oldsource>DESCRIPTION_CMD_LOGO</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_LOGO&lt;/b&gt;()&lt;br&gt;&lt;br&gt;Play device logo animation.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_COLDSTART.</source>
        <oldsource>DESCRIPTION_CMD_COLDSTART</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_COLDSTART&lt;/b&gt;()&lt;br&gt;&lt;br&gt;Set co-processor engine state to default values.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CMD_GRADCOLOR.</source>
        <oldsource>DESCRIPTION_CMD_GRADCOLOR</oldsource>
        <translation type="vanished">&lt;b&gt;CMD_GRADCOLOR&lt;/b&gt;(&lt;i&gt;r&lt;/i&gt;, &lt;i&gt;g&lt;/i&gt;, &lt;i&gt;b&lt;/i&gt;)&lt;br&gt;&lt;b&gt;rgb&lt;/b&gt;: New highlight gradient color, as a 24-bit RGB number. Red is the most significant 8 bits, blue is the least. So 0xff0000 is bright red.&lt;br&gt;&lt;br&gt;Set the 3D button highlight color.</translation>
    </message>
    <message>
        <source>DESCRIPTION_DISPLAY.</source>
        <oldsource>DESCRIPTION_DISPLAY</oldsource>
        <translation type="vanished">&lt;b&gt;DISPLAY&lt;/b&gt;()&lt;br&gt;&lt;br&gt;End the display list. FT800 will ignore all the commands following this command.</translation>
    </message>
    <message>
        <source>DESCRIPTION_BITMAP_SOURCE.</source>
        <oldsource>DESCRIPTION_BITMAP_SOURCE</oldsource>
        <translation type="vanished">&lt;b&gt;BITMAP_SOURCE&lt;/b&gt;(&lt;i&gt;addr&lt;/i&gt;)&lt;br&gt;&lt;b&gt;addr&lt;/b&gt;: Bitmap address in graphics SRAM FT800, aligned with respect to the bitmap format.&lt;br&gt;For example, if the bitmap format is RGB565/ARGB4/ARGB1555, the bitmap source shall be aligned to 2 bytes.&lt;br&gt;&lt;br&gt;Specify the source address of bitmap data in FT800 graphics memory RAM_G.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CLEAR_COLOR_RGB.</source>
        <oldsource>DESCRIPTION_CLEAR_COLOR_RGB</oldsource>
        <translation type="vanished">&lt;b&gt;CLEAR_COLOR_RGB&lt;/b&gt;(&lt;i&gt;red&lt;/i&gt;, &lt;i&gt;green&lt;/i&gt;, &lt;i&gt;blue&lt;/i&gt;)&lt;br&gt;&lt;b&gt;red&lt;/b&gt;: Red value used when the color buffer is cleared. The initial value is 0&lt;br&gt;&lt;b&gt;green&lt;/b&gt;: Green value used when the color buffer is cleared. The initial value is 0&lt;br&gt;&lt;b&gt;blue&lt;/b&gt;: Blue value used when the color buffer is cleared. The initial value is 0&lt;br&gt;&lt;br&gt;Sets the color values used by a following CLEAR.</translation>
    </message>
    <message>
        <source>DESCRIPTION_TAG.</source>
        <oldsource>DESCRIPTION_TAG</oldsource>
        <translation type="vanished">&lt;b&gt;TAG&lt;/b&gt;(&lt;i&gt;s&lt;/i&gt;)&lt;br&gt;&lt;b&gt;s&lt;/b&gt;: Tag value. Valid value range is from 1 to 255.&lt;br&gt;&lt;br&gt;Attach the tag value for the following graphics objects drawn on the screen. The initial tag buffer value is 255.</translation>
    </message>
    <message>
        <source>DESCRIPTION_COLOR_RGB.</source>
        <oldsource>DESCRIPTION_COLOR_RGB</oldsource>
        <translation type="vanished">&lt;b&gt;COLOR_RGB&lt;/b&gt;(&lt;i&gt;red&lt;/i&gt;, &lt;i&gt;green&lt;/i&gt;, &lt;i&gt;blue&lt;/i&gt;)&lt;br&gt;&lt;b&gt;red&lt;/b&gt;: Red value for the current color. The initial value is 255&lt;br&gt;&lt;b&gt;green&lt;/b&gt;: Green value for the current color. The initial value is 255&lt;br&gt;&lt;b&gt;blue&lt;/b&gt;: Blue value for the current color. The initial value is 255&lt;br&gt;&lt;br&gt;Sets red, green and blue values of the FT800 color buffer which will be applied to the following draw operation.</translation>
    </message>
    <message>
        <source>DESCRIPTION_BITMAP_HANDLE.</source>
        <oldsource>DESCRIPTION_BITMAP_HANDLE</oldsource>
        <translation type="vanished">&lt;b&gt;BITMAP_HANDLE&lt;/b&gt;(&lt;i&gt;handle&lt;/i&gt;)&lt;br&gt;&lt;b&gt;handle&lt;/b&gt;: Bitmap handle. The initial value is 0. The valid value range is from 0 to 31.&lt;br&gt;&lt;br&gt;Specify the bitmap handle.&lt;br&gt;&lt;br&gt;Handles 16 to 31 are defined by the FT800 for built-in font and handle 15 is defined in the co-processor engine commands CMD_GRADIENT, CMD_BUTTON and CMD_KEYS. Users can define new bitmaps using handles from 0 to 14. If there is no co-processor engine command CMD_GRADIENT, CMD_BUTTON and CMD_KEYS in the current display list, users can even define a bitmap using handle 15.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CELL.</source>
        <oldsource>DESCRIPTION_CELL</oldsource>
        <translation type="vanished">&lt;b&gt;CELL&lt;/b&gt;(&lt;i&gt;cell&lt;/i&gt;)&lt;br&gt;&lt;b&gt;cell&lt;/b&gt;: Bitmap cell number. The initial value is 0&lt;br&gt;&lt;br&gt;Specify the bitmap cell number for the VERTEX2F command.</translation>
    </message>
    <message>
        <source>DESCRIPTION_BITMAP_LAYOUT.</source>
        <oldsource>DESCRIPTION_BITMAP_LAYOUT</oldsource>
        <translation type="vanished">&lt;b&gt;BITMAP_LAYOUT&lt;/b&gt;(&lt;i&gt;format&lt;/i&gt;, &lt;i&gt;linestride&lt;/i&gt;, &lt;i&gt;height&lt;/i&gt;)&lt;br&gt;&lt;b&gt;format&lt;/b&gt;: Bitmap pixel format. The bitmap formats supported are L1, L4, L8, RGB332, ARGB2, ARGB4, ARGB1555, RGB565 and PALETTED.&lt;br&gt;&lt;b&gt;linestride&lt;/b&gt;: Bitmap linestride, in bytes. For L1 format, the line stride must be a multiple of 8 bits; For L4 format the line stride must be multiple of 2 nibbles. (Aligned to byte).&lt;br&gt;&lt;b&gt;height&lt;/b&gt;: Bitmap height, in lines&lt;br&gt;&lt;br&gt;Specify the source bitmap memory format and layout for the current handle.</translation>
    </message>
    <message>
        <source>DESCRIPTION_BITMAP_SIZE.</source>
        <oldsource>DESCRIPTION_BITMAP_SIZE</oldsource>
        <translation type="vanished">&lt;b&gt;BITMAP_SIZE&lt;/b&gt;(&lt;i&gt;filter&lt;/i&gt;, &lt;i&gt;wrapx&lt;/i&gt;, &lt;i&gt;wrapy&lt;/i&gt;, &lt;i&gt;width&lt;/i&gt;, &lt;i&gt;height&lt;/i&gt;)&lt;br&gt;&lt;b&gt;filter&lt;/b&gt;: Bitmap filtering mode, one of NEAREST or BILINEAR&lt;br&gt;&lt;b&gt;wrapx&lt;/b&gt;: Bitmap x wrap mode, one of REPEAT or BORDER&lt;br&gt;&lt;b&gt;wrapy&lt;/b&gt;: Bitmap y wrap mode, one of REPEAT or BORDER&lt;br&gt;&lt;b&gt;width&lt;/b&gt;: Drawn bitmap width, in pixels&lt;br&gt;&lt;b&gt;height&lt;/b&gt;: Drawn bitmap height, in pixels&lt;br&gt;&lt;br&gt;Specify the screen drawing of bitmaps for the current handle.</translation>
    </message>
    <message>
        <source>DESCRIPTION_ALPHA_FUNC.</source>
        <oldsource>DESCRIPTION_ALPHA_FUNC</oldsource>
        <translation type="vanished">&lt;b&gt;ALPHA_FUNC&lt;/b&gt;(&lt;i&gt;func&lt;/i&gt;, &lt;i&gt;ref&lt;/i&gt;)&lt;br&gt;&lt;b&gt;func&lt;/b&gt;: Specifies the test function, one of NEVER, LESS, LEQUAL, GREATER, GEQUAL, EQUAL, NOTEQUAL, or ALWAYS. The initial value is ALWAYS (7)&lt;br&gt;&lt;b&gt;ref&lt;/b&gt;: Specifies the reference value for the alpha test. The initial value is 0&lt;br&gt;&lt;br&gt;Specify the alpha test function.</translation>
    </message>
    <message>
        <source>DESCRIPTION_STENCIL_FUNC.</source>
        <oldsource>DESCRIPTION_STENCIL_FUNC</oldsource>
        <translation type="vanished">&lt;b&gt;STENCIL_FUNC&lt;/b&gt;(&lt;i&gt;func&lt;/i&gt;, &lt;i&gt;ref&lt;/i&gt;, &lt;i&gt;mask&lt;/i&gt;)&lt;br&gt;&lt;b&gt;func&lt;/b&gt;: Specifies the test function, one of NEVER, LESS, LEQUAL, GREATER, GEQUAL, EQUAL, NOTEQUAL, or ALWAYS. The initial value is ALWAYS. &lt;br&gt;&lt;b&gt;ref&lt;/b&gt;: Specifies the reference value for the stencil test. The initial value is 0&lt;br&gt;&lt;b&gt;mask&lt;/b&gt;: Specifies a mask that is ANDed with the reference value and the stored stencil value. The initial value is 255&lt;br&gt;&lt;br&gt;Set function and reference value for stencil testing.</translation>
    </message>
    <message>
        <source>DESCRIPTION_BLEND_FUNC.</source>
        <oldsource>DESCRIPTION_BLEND_FUNC</oldsource>
        <translation type="vanished">&lt;b&gt;BLEND_FUNC&lt;/b&gt;(&lt;i&gt;src&lt;/i&gt;, &lt;i&gt;dst&lt;/i&gt;)&lt;br&gt;&lt;b&gt;src&lt;/b&gt;: Specifies how the source blending factor is computed. One of ZERO, ONE, SRC_ALPHA, DST_ALPHA, ONE_MINUS_SRC_ALPHA or ONE_MINUS_DST_ALPHA. The initial value is SRC_ALPHA (2).&lt;br&gt;&lt;b&gt;dst&lt;/b&gt;: Specifies how the destination blending factor is computed, one of the same constants as src. The initial value is ONE_MINUS_SRC_ALPHA(4)&lt;br&gt;&lt;br&gt;Specify pixel arithmetic.</translation>
    </message>
    <message>
        <source>DESCRIPTION_STENCIL_OP.</source>
        <oldsource>DESCRIPTION_STENCIL_OP</oldsource>
        <translation type="vanished">&lt;b&gt;STENCIL_OP&lt;/b&gt;(&lt;i&gt;sfail&lt;/i&gt;, &lt;i&gt;spass&lt;/i&gt;)&lt;br&gt;&lt;b&gt;sfail&lt;/b&gt;: Specifies the action to take when the stencil test fails, one of KEEP, ZERO, REPLACE, INCR, DECR, and INVERT. The initial value is KEEP (1)&lt;br&gt;&lt;b&gt;spass&lt;/b&gt;: Specifies the action to take when the stencil test passes, one of the same constants as sfail. The initial value is KEEP (1)&lt;br&gt;&lt;br&gt;Set stencil test actions.</translation>
    </message>
    <message>
        <source>DESCRIPTION_POINT_SIZE.</source>
        <oldsource>DESCRIPTION_POINT_SIZE</oldsource>
        <translation type="vanished">&lt;b&gt;POINT_SIZE&lt;/b&gt;(&lt;i&gt;size&lt;/i&gt;)&lt;br&gt;&lt;b&gt;size&lt;/b&gt;: Point radius in 1/16 pixel. The initial value is 16.&lt;br&gt;&lt;br&gt;Sets the size of drawn points. The width is the distance from the center of the point to the outermost drawn pixel, in units of 1/16 pixels. The valid range is from 16 to 8191 with respect to 1/16th pixel unit.</translation>
    </message>
    <message>
        <source>DESCRIPTION_LINE_WIDTH.</source>
        <oldsource>DESCRIPTION_LINE_WIDTH</oldsource>
        <translation type="vanished">&lt;b&gt;LINE_WIDTH&lt;/b&gt;(&lt;i&gt;width&lt;/i&gt;)&lt;br&gt;&lt;b&gt;width&lt;/b&gt;: Line width in 1/16 pixel. The initial value is 16.&lt;br&gt;&lt;br&gt;Sets the width of drawn lines. The width is the distance from the center of the line to the outermost drawn pixel, in units of 1/16 pixel. The valid range is from 16 to 4095 in terms of 1/16th pixel units.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CLEAR_COLOR_A.</source>
        <oldsource>DESCRIPTION_CLEAR_COLOR_A</oldsource>
        <translation type="vanished">&lt;b&gt;CLEAR_COLOR_A&lt;/b&gt;(&lt;i&gt;alpha&lt;/i&gt;)&lt;br&gt;&lt;b&gt;alpha&lt;/b&gt;: Alpha value used when the color buffer is cleared. The initial value is 0.&lt;br&gt;&lt;br&gt;Specify clear value for the alpha channel.</translation>
    </message>
    <message>
        <source>DESCRIPTION_COLOR_A.</source>
        <oldsource>DESCRIPTION_COLOR_A</oldsource>
        <translation type="vanished">&lt;b&gt;COLOR_A&lt;/b&gt;(&lt;i&gt;alpha&lt;/i&gt;)&lt;br&gt;&lt;b&gt;alpha&lt;/b&gt;: Alpha for the current color. The initial value is 255&lt;br&gt;&lt;br&gt;Set the current color alpha.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CLEAR_STENCIL.</source>
        <oldsource>DESCRIPTION_CLEAR_STENCIL</oldsource>
        <translation type="vanished">&lt;b&gt;CLEAR_STENCIL&lt;/b&gt;(&lt;i&gt;s&lt;/i&gt;)&lt;br&gt;&lt;b&gt;s&lt;/b&gt;: Value used when the stencil buffer is cleared. The initial value is 0&lt;br&gt;&lt;br&gt;Specify clear value for the stencil buffer.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CLEAR_TAG.</source>
        <oldsource>DESCRIPTION_CLEAR_TAG</oldsource>
        <translation type="vanished">&lt;b&gt;CLEAR_TAG&lt;/b&gt;(&lt;i&gt;s&lt;/i&gt;)&lt;br&gt;&lt;b&gt;s&lt;/b&gt;: Value used when the tag buffer is cleared. The initial value is 0&lt;br&gt;&lt;br&gt;Specify clear value for the tag buffer.</translation>
    </message>
    <message>
        <source>DESCRIPTION_STENCIL_MASK.</source>
        <oldsource>DESCRIPTION_STENCIL_MASK</oldsource>
        <translation type="vanished">&lt;b&gt;STENCIL_MASK&lt;/b&gt;(&lt;i&gt;mask&lt;/i&gt;)&lt;br&gt;&lt;b&gt;mask&lt;/b&gt;: The mask used to enable writing stencil bits. The initial value is 255&lt;br&gt;&lt;br&gt;Control the writing of individual bits in the stencil planes.</translation>
    </message>
    <message>
        <source>DESCRIPTION_TAG_MASK.</source>
        <oldsource>DESCRIPTION_TAG_MASK</oldsource>
        <translation type="vanished">&lt;b&gt;TAG_MASK&lt;/b&gt;(&lt;i&gt;mask&lt;/i&gt;)&lt;br&gt;&lt;b&gt;mask&lt;/b&gt;: Allow updates to the tag buffer. The initial value is one and it means the tag buffer of the FT800 is updated with the value given by the TAG command. Therefore, the following graphics objects will be attached to the tag value given by the TAG command.&lt;br&gt;The value zero means the tag buffer of the FT800 is set as the default value,rather than the value given by TAG command in the display list.&lt;br&gt;&lt;br&gt;Control the writing of the tag buffer.</translation>
    </message>
    <message>
        <source>DESCRIPTION_BITMAP_TRANSFORM_A.</source>
        <oldsource>DESCRIPTION_BITMAP_TRANSFORM_A</oldsource>
        <translation type="vanished">&lt;b&gt;BITMAP_TRANSFORM_A&lt;/b&gt;(&lt;i&gt;a&lt;/i&gt;)&lt;br&gt;&lt;b&gt;a&lt;/b&gt;: Coefficient A of the bitmap transform matrix, in signed 8.8 bit fixed-point form. The initial value is 256&lt;br&gt;&lt;br&gt;Specify the A coefficient of the bitmap transform </translation>
    </message>
    <message>
        <source>DESCRIPTION_BITMAP_TRANSFORM_B.</source>
        <oldsource>DESCRIPTION_BITMAP_TRANSFORM_B</oldsource>
        <translation type="vanished">&lt;b&gt;BITMAP_TRANSFORM_B&lt;/b&gt;(&lt;i&gt;b&lt;/i&gt;)&lt;br&gt;&lt;b&gt;b&lt;/b&gt;: Coefficient B of the bitmap transform matrix, in signed 8.8 bit fixed-point form. The initial value is 0&lt;br&gt;&lt;br&gt;Specify the B coefficient of the bitmap transform matrix.</translation>
    </message>
    <message>
        <source>DESCRIPTION_BITMAP_TRANSFORM_C.</source>
        <oldsource>DESCRIPTION_BITMAP_TRANSFORM_C</oldsource>
        <translation type="vanished">&lt;b&gt;BITMAP_TRANSFORM_C&lt;/b&gt;(&lt;i&gt;c&lt;/i&gt;)&lt;br&gt;&lt;b&gt;c&lt;/b&gt;: Coefficient C of the bitmap transform matrix, in signed 15.8 bit fixed-point form. The initial value is 0&lt;br&gt;&lt;br&gt;Specify the C coefficient of the bitmap transform matrix.</translation>
    </message>
    <message>
        <source>DESCRIPTION_BITMAP_TRANSFORM_D.</source>
        <oldsource>DESCRIPTION_BITMAP_TRANSFORM_D</oldsource>
        <translation type="vanished">&lt;b&gt;BITMAP_TRANSFORM_D&lt;/b&gt;(&lt;i&gt;d&lt;/i&gt;)&lt;br&gt;&lt;b&gt;d&lt;/b&gt;: Coefficient D of the bitmap transform matrix, in signed 8.8 bit fixed-point form. The initial value is 0&lt;br&gt;&lt;br&gt;Specify the D coefficient of the bitmap transform matrix.</translation>
    </message>
    <message>
        <source>DESCRIPTION_BITMAP_TRANSFORM_E.</source>
        <oldsource>DESCRIPTION_BITMAP_TRANSFORM_E</oldsource>
        <translation type="vanished">&lt;b&gt;BITMAP_TRANSFORM_E&lt;/b&gt;(&lt;i&gt;e&lt;/i&gt;)&lt;br&gt;&lt;b&gt;e&lt;/b&gt;: Coefficient E of the bitmap transform matrix, in signed 8.8 bit fixed-point form. The initial value is 256&lt;br&gt;&lt;br&gt;Specify the E coefficient of the bitmap transform matrix.</translation>
    </message>
    <message>
        <source>DESCRIPTION_BITMAP_TRANSFORM_F.</source>
        <oldsource>DESCRIPTION_BITMAP_TRANSFORM_F</oldsource>
        <translation type="vanished">&lt;b&gt;BITMAP_TRANSFORM_F&lt;/b&gt;(&lt;i&gt;f&lt;/i&gt;)&lt;br&gt;&lt;b&gt;f&lt;/b&gt;: Coefficient F of the bitmap transform matrix, in signed 15.8 bit fixed-point form. The initial value is 0&lt;br&gt;&lt;br&gt;Specify the F coefficient of the bitmap transform matrix.</translation>
    </message>
    <message>
        <source>DESCRIPTION_SCISSOR_XY.</source>
        <oldsource>DESCRIPTION_SCISSOR_XY</oldsource>
        <translation type="vanished">&lt;b&gt;SCISSOR_XY&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: The x coordinate of the scissor clip rectangle, in pixels. The initial value is 0&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: The y coordinate of the scissor clip rectangle, in pixels. The initial value is 0&lt;br&gt;&lt;br&gt;Sets the top-left position of the scissor clip rectangle, which limits the drawing area.</translation>
    </message>
    <message>
        <source>DESCRIPTION_SCISSOR_SIZE.</source>
        <oldsource>DESCRIPTION_SCISSOR_SIZE</oldsource>
        <translation type="vanished">&lt;b&gt;SCISSOR_SIZE&lt;/b&gt;(&lt;i&gt;width&lt;/i&gt;, &lt;i&gt;height&lt;/i&gt;)&lt;br&gt;&lt;b&gt;width&lt;/b&gt;: The width of the scissor clip rectangle, in pixels. The initial value is 512. The valid value range is from 0 to 512.&lt;br&gt;&lt;b&gt;height&lt;/b&gt;: The height of the scissor clip rectangle, in pixels. The initial value is 512. The valid value range is from 0 to 512.&lt;br&gt;&lt;br&gt;Sets the width and height of the scissor clip rectangle, which limits the drawing area.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CALL.</source>
        <oldsource>DESCRIPTION_CALL</oldsource>
        <translation type="vanished">&lt;b&gt;CALL&lt;/b&gt;(&lt;i&gt;dest&lt;/i&gt;)&lt;br&gt;&lt;b&gt;dest&lt;/b&gt;: The destination address in RAM_DL which the display command is to be switched. FT800 has the stack to store the return address. To come back to the next command of source address, the RETURN command can help.&lt;br&gt;&lt;br&gt;Execute a sequence of commands at another location in the display list&lt;br&gt;&lt;br&gt;CALL and RETURN have a 4 level stack in addition to the current pointer. Any additional CALL/RETURN done will lead to unexpected behavior.</translation>
    </message>
    <message>
        <source>DESCRIPTION_JUMP.</source>
        <oldsource>DESCRIPTION_JUMP</oldsource>
        <translation type="vanished">&lt;b&gt;JUMP&lt;/b&gt;(&lt;i&gt;dest&lt;/i&gt;)&lt;br&gt;&lt;b&gt;dest&lt;/b&gt;: Display list address to be jumped.&lt;br&gt;&lt;br&gt;Execute commands at another location in the display list.</translation>
    </message>
    <message>
        <source>DESCRIPTION_BEGIN.</source>
        <oldsource>DESCRIPTION_BEGIN</oldsource>
        <translation type="vanished">&lt;b&gt;BEGIN&lt;/b&gt;(&lt;i&gt;prim&lt;/i&gt;)&lt;br&gt;&lt;b&gt;prim&lt;/b&gt;: Graphics primitive. The valid value is defined as: BITMAPS, POINTS, LINES, LINE_STRIP, EDGE_STRIP_R, EDGE_STRIP_L, EDGE_STRIP_A, EDGE_STRIP_B, RECTS&lt;br&gt;&lt;br&gt;Begin drawing a graphics primitive.</translation>
    </message>
    <message>
        <source>DESCRIPTION_COLOR_MASK.</source>
        <oldsource>DESCRIPTION_COLOR_MASK</oldsource>
        <translation type="vanished">&lt;b&gt;COLOR_MASK&lt;/b&gt;(&lt;i&gt;r&lt;/i&gt;, &lt;i&gt;g&lt;/i&gt;, &lt;i&gt;b&lt;/i&gt;, &lt;i&gt;a&lt;/i&gt;)&lt;br&gt;&lt;b&gt;r&lt;/b&gt;: Enable or disable the red channel update of the FT800 color buffer. The initial value is 1 and means enable.&lt;br&gt;&lt;b&gt;g&lt;/b&gt;: Enable or disable the green channel update of the FT800 color buffer. The initial value is 1 and means enable.&lt;br&gt;&lt;b&gt;b&lt;/b&gt;: Enable or disable the blue channel update of the FT800 color buffer. The initial value is 1 and means enable.&lt;br&gt;&lt;b&gt;a&lt;/b&gt;: Enable or disable the alpha channel update of the FT800 color buffer. The initial value is 1 and means enable.&lt;br&gt;&lt;br&gt;The color mask controls whether the color values of a pixel are updated. Sometimes it is used to selectively update only the red, green, blue or alpha channels of the image. More often, it is used to completely disable color updates while updating the tag and stencil buffers.</translation>
    </message>
    <message>
        <source>DESCRIPTION_END.</source>
        <oldsource>DESCRIPTION_END</oldsource>
        <translation type="vanished">&lt;b&gt;END&lt;/b&gt;()&lt;br&gt;&lt;br&gt;End drawing a graphics primitive.&lt;br&gt;&lt;br&gt;It is recommended to have an END for each BEGIN. Whereas advanced users can avoid the usage of END in order to save extra graphics instructions in the display list RAM.</translation>
    </message>
    <message>
        <source>DESCRIPTION_SAVE_CONTEXT.</source>
        <oldsource>DESCRIPTION_SAVE_CONTEXT</oldsource>
        <translation type="vanished">&lt;b&gt;SAVE_CONTEXT&lt;/b&gt;()&lt;br&gt;&lt;br&gt;Push the current graphics context on the context stack.&lt;br&gt;&lt;br&gt;Any extra SAVE_CONTEXT will throw away the earliest saved context.</translation>
    </message>
    <message>
        <source>DESCRIPTION_RESTORE_CONTEXT.</source>
        <oldsource>DESCRIPTION_RESTORE_CONTEXT</oldsource>
        <translation type="vanished">&lt;b&gt;RESTORE_CONTEXT&lt;/b&gt;()&lt;br&gt;&lt;br&gt;Restore the current graphics context from the context stack.&lt;br&gt;&lt;br&gt;Any extra RESTORE_CONTEXT will load the default values into the present context.</translation>
    </message>
    <message>
        <source>DESCRIPTION_RETURN.</source>
        <oldsource>DESCRIPTION_RETURN</oldsource>
        <translation type="vanished">&lt;b&gt;RETURN&lt;/b&gt;()&lt;br&gt;&lt;br&gt;Return from a previous CALL command.&lt;br&gt;&lt;br&gt;CALL and RETURN have 4 levels of stack in addition to the current pointer. Any additional CALL/RETURN done will lead to unexpected behavior.</translation>
    </message>
    <message>
        <source>DESCRIPTION_MACRO.</source>
        <oldsource>DESCRIPTION_MACRO</oldsource>
        <translation type="vanished">&lt;b&gt;MACRO&lt;/b&gt;(&lt;i&gt;m&lt;/i&gt;)&lt;br&gt;&lt;b&gt;m&lt;/b&gt;: Macro register to read. Value 0 means the FT800 will fetch the command from REG_MACRO_0 to execute. Value 1 means the FT800 will fetch the command from REG_MACRO_1 to execute. The content of REG_MACRO_0 or REG_MACRO_1 shall be a valid display list command, otherwise the behavior is undefined.&lt;br&gt;&lt;br&gt;Execute a single command from a macro register.</translation>
    </message>
    <message>
        <source>DESCRIPTION_CLEAR.</source>
        <oldsource>DESCRIPTION_CLEAR</oldsource>
        <translation type="vanished">&lt;b&gt;CLEAR&lt;/b&gt;(&lt;i&gt;c&lt;/i&gt;, &lt;i&gt;s&lt;/i&gt;, &lt;i&gt;t&lt;/i&gt;)&lt;br&gt;&lt;b&gt;c&lt;/b&gt;: Clear color buffer. Setting this bit to 1 will clear the color buffer of the FT800 to the preset value. Setting this bit to 0 will maintain the color buffer of the FT800 with an unchanged value. The preset value is defined in command CLEAR_COLOR_RGB for RGB channel and CLEAR_COLOR_A for alpha channel.&lt;br&gt;&lt;b&gt;s&lt;/b&gt;: Clear stencil buffer. Setting this bit to 1 will clear the stencil buffer of the FT800 to the preset value. Setting this bit to 0 will maintain the stencil buffer of the FT800 with an unchanged value. The preset value is defined in command CLEAR_STENCIL.&lt;br&gt;&lt;b&gt;t&lt;/b&gt;: Clear tag buffer. Setting this bit to 1 will clear the tag buffer of the FT800 to the preset value. Setting this bit to 0 will maintain the tag buffer of the FT800 with an unchanged value. The preset value is defined in command CLEAR_TAG.&lt;br&gt;&lt;br&gt;Clear buffers to preset values.</translation>
    </message>
    <message>
        <source>&lt;/i&gt;Not yet implemented.&lt;/i&gt;</source>
        <translation type="vanished">&lt;/i&gt;Not yet implemented.&lt;/i&gt;</translation>
    </message>
</context>
<context>
    <name>FTEDITOR</name>
    <message>
        <location filename="main_window.cpp" line="2032"/>
        <source>Start typing in the &lt;b&gt;Coprocessor&lt;/b&gt; editor, or drag and drop items from the &lt;b&gt;Toolbox&lt;/b&gt; onto the display viewport.</source>
        <translation type="unfinished">Start typing in the &lt;b&gt;Coprocessor&lt;/b&gt; editor, or drag and drop items from the &lt;b&gt;Toolbox&lt;/b&gt; onto the display viewport.</translation>
    </message>
</context>
<context>
    <name>FTEDITOR::ContentManager</name>
    <message>
        <location filename="content_manager.cpp" line="227"/>
        <source>Status</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="228"/>
        <source>Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="238"/>
        <source>Add</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="246"/>
        <source>Remove</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="258"/>
        <source>Rebuild All</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="260"/>
        <source>Rebuilds all content that is out of date</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="275"/>
        <source>Content</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="281"/>
        <source>Source file: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="286"/>
        <source>Name: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="291"/>
        <source>Image</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="292"/>
        <location filename="content_manager.cpp" line="347"/>
        <source>Raw</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="293"/>
        <location filename="content_manager.cpp" line="323"/>
        <source>Font</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="295"/>
        <source>Converter: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="302"/>
        <source>Image Settings</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="308"/>
        <location filename="content_manager.cpp" line="329"/>
        <source>Format: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="315"/>
        <source>Image Preview</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="335"/>
        <source>Size: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="339"/>
        <source>Charset: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="353"/>
        <source>Start: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="360"/>
        <source>Length: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="367"/>
        <source>Memory Options</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="374"/>
        <source>Address: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="377"/>
        <source>Loaded: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="384"/>
        <source>Compressed: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="387"/>
        <source>Embedded: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="394"/>
        <source>&lt;i&gt;No content has been added to the project yet.&lt;br&gt;&lt;br&gt;Add new content to this project to automatically convert it to a hardware compatible format.&lt;/i&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="418"/>
        <source>Add content</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="515"/>
        <location filename="content_manager.cpp" line="557"/>
        <source>Remove content</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="721"/>
        <location filename="content_manager.cpp" line="2046"/>
        <source>Load Content</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="723"/>
        <location filename="content_manager.cpp" line="2048"/>
        <source>All files (*.*)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="832"/>
        <source>Clear content</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="935"/>
        <source>Select a &lt;b&gt;Converter&lt;/b&gt; to be used for this file. Converted files will be stored in the folder where the project is saved.&lt;br&gt;&lt;br&gt;&lt;b&gt;Image&lt;/b&gt;: Converts an image to one of the supported formats.&lt;br&gt;&lt;b&gt;Raw&lt;/b&gt;: Does a direct binary copy.&lt;br&gt;&lt;b&gt;Raw JPEG&lt;/b&gt;: Does a raw binary copy and decodes the JPEG on the coprocessor.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="942"/>
        <source>&lt;b&gt;Error&lt;/b&gt;: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="955"/>
        <source>&lt;b&gt;Error&lt;/b&gt;: Failed to load image preview.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="964"/>
        <location filename="content_manager.cpp" line="1018"/>
        <location filename="content_manager.cpp" line="1047"/>
        <source>&lt;b&gt;Size: &lt;/b&gt; </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="965"/>
        <location filename="content_manager.cpp" line="1019"/>
        <location filename="content_manager.cpp" line="1048"/>
        <source>&lt;br&gt;&lt;b&gt;Compressed: &lt;/b&gt; </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="999"/>
        <location filename="content_manager.cpp" line="1051"/>
        <source>&lt;br&gt;&lt;b&gt;Width: &lt;/b&gt; </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="1000"/>
        <location filename="content_manager.cpp" line="1052"/>
        <source>&lt;br&gt;&lt;b&gt;Height: &lt;/b&gt; </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="1001"/>
        <location filename="content_manager.cpp" line="1053"/>
        <source>&lt;br&gt;&lt;b&gt;Stride: &lt;/b&gt; </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="1031"/>
        <source>&lt;b&gt;Not yet implemented&lt;/b&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="1972"/>
        <source>Change content source path</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="2022"/>
        <source>Change source path</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="2068"/>
        <source>Change content destination name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="2203"/>
        <source>Set content converter</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="2256"/>
        <source>Change converter</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="2292"/>
        <location filename="content_manager.cpp" line="2342"/>
        <source>Change image format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="2374"/>
        <source>Load content to memory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="2374"/>
        <source>Unload content from memory</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="2436"/>
        <location filename="content_manager.cpp" line="2494"/>
        <location filename="content_manager.cpp" line="2651"/>
        <location filename="content_manager.cpp" line="2726"/>
        <source>Change memory address</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="2539"/>
        <source>Store data compressed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="2539"/>
        <source>Store data uncompressed</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="2595"/>
        <source>Store data in header</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="2595"/>
        <source>Store data as file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="2801"/>
        <location filename="content_manager.cpp" line="2851"/>
        <source>Change font format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="2883"/>
        <location filename="content_manager.cpp" line="2933"/>
        <source>Change font size</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="content_manager.cpp" line="2963"/>
        <location filename="content_manager.cpp" line="3016"/>
        <source>Change font charset</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>FTEDITOR::DeviceManager</name>
    <message>
        <location filename="device_manager.cpp" line="64"/>
        <source>Connected</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="device_manager.cpp" line="65"/>
        <source>Device Name</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="device_manager.cpp" line="77"/>
        <source>Refresh the device list</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="device_manager.cpp" line="85"/>
        <source>Sync With Device</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="device_manager.cpp" line="86"/>
        <source>Sends the current memory and display list to the selected device</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="device_manager.cpp" line="92"/>
        <source>Connect</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="device_manager.cpp" line="93"/>
        <source>Connect the selected device</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="device_manager.cpp" line="99"/>
        <source>Disconnect</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="device_manager.cpp" line="100"/>
        <source>Disconnect from the selected device</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>FTEDITOR::InteractiveProperties</name>
    <message>
        <location filename="interactive_properties.cpp" line="561"/>
        <location filename="interactive_properties.cpp" line="1189"/>
        <source>Format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="561"/>
        <source>Set bitmap format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="602"/>
        <source>Unknown command &apos;&lt;i&gt;%s&lt;/i&gt;&apos;</source>
        <translation type="unfinished">Unknown command &apos;&lt;i&gt;%s&lt;/i&gt;&apos;</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="621"/>
        <source>DESCRIPTION_VERTEX2F.</source>
        <translation type="unfinished">&lt;b&gt;VERTEX2F&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: Signed x-coordinate in 1/16 pixel precision&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: Signed x-coordinate in 1/16 pixel precision&lt;br&gt;&lt;br&gt;Start the operation of graphics primitives at the specified screen coordinate, in 1/16th pixel precision.&lt;br&gt;&lt;br&gt;The range of coordinates can be from -16384 to +16383 in terms of 1/16 th pixel units. Please note the negative x coordinate value means the coordinate in the left virtual screen from (0, 0), while the negative y coordinate value means the coordinate in the upper virtual screen from (0, 0). If drawing on the negative coordinate position, the drawing operation will not be visible.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="632"/>
        <source>DESCRIPTION_VERTEX2II.</source>
        <translation type="unfinished">&lt;b&gt;VERTEX2II&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;handle&lt;/i&gt;, &lt;/i&gt;cell&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: x-coordinate in pixels&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: y-coordinate in pixels&lt;br&gt;&lt;b&gt;handle&lt;/b&gt;: Bitmap handle. The valid range is from 0 to 31. From 16 to 31, the bitmap handle is dedicated to the FT800 built-in font.&lt;br&gt;&lt;b&gt;cell&lt;/b&gt;: Cell number. Cell number is the index of bitmap with same bitmap layout and format. For example, for handle 31, the cell 65 means the character &quot;A&quot; in the largest built in font.&lt;br&gt;&lt;br&gt;Start the operation of graphics primitive at the specified coordinates. The handle and cell parameters will be ignored unless the graphics primitive is specified as bitmap by command BEGIN, prior to this command.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="647"/>
        <source>DESCRIPTION_CMD_DLSTART.</source>
        <translation type="unfinished">&lt;b&gt;CMD_DLSTART&lt;/b&gt;()&lt;br&gt;&lt;br&gt;When the co-processor engine executes this command, it waits until the display list is ready for writing, then sets REG_CMD_DL to zero.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="657"/>
        <source>DESCRIPTION_CMD_SWAP.</source>
        <translation type="unfinished">&lt;b&gt;CMD_SWAP&lt;/b&gt;()&lt;br&gt;&lt;br&gt;When the co-processor engine executes this command, it requests a display list swap by writing to REG_DLSWAP.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="667"/>
        <source>DESCRIPTION_CMD_INTERRUPT.</source>
        <translation type="unfinished">&lt;b&gt;CMD_INTERRUPT&lt;/b&gt;(&lt;i&gt;ms&lt;/i&gt;)&lt;br&gt;&lt;b&gt;ms&lt;/b&gt;: Delay before interrupt triggers, in milliseconds. The interrupt is guaranteed not to fire before this delay. If ms is zero, the interrupt fires immediately.&lt;br&gt;&lt;br&gt;When the co-processor engine executes this command, it triggers interrupt INT_CMDFLAG.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="677"/>
        <source>DESCRIPTION_CMD_BGCOLOR.</source>
        <translation type="unfinished">&lt;b&gt;CMD_BGCOLOR&lt;/b&gt;(&lt;i&gt;r&lt;/i&gt;, &lt;i&gt;g&lt;/i&gt;, &lt;i&gt;b&lt;/i&gt;)&lt;br&gt;&lt;b&gt;rgb&lt;/b&gt;: New background color, as a 24-bit RGB number. Red is the most significant 8 bits, blue is the least. So 0xff0000 is bright red.&lt;br&gt;Background color is applicable for things that the user cannot move. Example behind gauges and sliders etc.&lt;br&gt;&lt;br&gt;Set the background color.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="689"/>
        <source>DESCRIPTION_CMD_FGCOLOR.</source>
        <translation type="unfinished">&lt;b&gt;CMD_FGCOLOR&lt;/b&gt;(&lt;i&gt;r&lt;/i&gt;, &lt;i&gt;g&lt;/i&gt;, &lt;i&gt;b&lt;/i&gt;)&lt;br&gt;&lt;b&gt;rgb&lt;/b&gt;: New foreground color, as a 24-bit RGB number. Red is the most significant 8 bits, blue is the least. So 0xff0000 is bright red.&lt;br&gt;Foreground color is applicable for things that the user can move such as handles and buttons (&quot;affordances&quot;).&lt;br&gt;&lt;br&gt;Set the foreground color.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="701"/>
        <source>DESCRIPTION_CMD_GRADIENT.</source>
        <translation type="unfinished">&lt;b&gt;CMD_GRADIENT&lt;/b&gt;(&lt;i&gt;x0&lt;/i&gt;, &lt;i&gt;y0&lt;/i&gt;, &lt;i&gt;a0&lt;/i&gt;, &lt;i&gt;r0&lt;/i&gt;, &lt;i&gt;g0&lt;/i&gt;, &lt;i&gt;b0&lt;/i&gt;, &lt;i&gt;x1&lt;/i&gt;, &lt;i&gt;y1&lt;/i&gt;, &lt;i&gt;a1&lt;/i&gt;, &lt;i&gt;r1&lt;/i&gt;, &lt;i&gt;g1&lt;/i&gt;, &lt;i&gt;b1&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x0&lt;/b&gt;: x-coordinate of point 0, in pixels&lt;br&gt;&lt;b&gt;y0&lt;/b&gt;: y-coordinate of point 0, in pixels&lt;br&gt;&lt;b&gt;argb0&lt;/b&gt;: Color of point 0, as a 24-bit RGB number. R is the most significant 8 bits, B is the least. So 0xff0000 is bright red.&lt;br&gt;&lt;b&gt;x1&lt;/b&gt;: x-coordinate of point 1, in pixels&lt;br&gt;&lt;b&gt;y1&lt;/b&gt;: y-coordinate of point 1, in pixels&lt;br&gt;&lt;b&gt;argb1&lt;/b&gt;: Color of point 1.&lt;br&gt;&lt;br&gt;Draw a smooth color gradient.&lt;br&gt;&lt;br&gt;All the colours step values are calculated based on smooth curve interpolated from the RGB0 to RGB1 parameter. The smooth curve equation is independently calculated for all three colors and the equation used is R0 + t * (R1 - R0), where t is interpolated between 0 and 1. Gradient must be used with Scissor function to get the intended gradient display.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="716"/>
        <source>DESCRIPTION_CMD_TEXT.</source>
        <translation type="unfinished">&lt;b&gt;CMD_TEXT&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;font&lt;/i&gt;, &lt;i&gt;options&lt;/i&gt;, &lt;i&gt;s&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: x-coordinate of text base, in pixels&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: y-coordinate of text base, in pixels&lt;br&gt;&lt;b&gt;font&lt;/b&gt;: Font to use for text, 0-31. See ROM and RAM Fonts&lt;br&gt;&lt;b&gt;options&lt;/b&gt;: By default (x; y) is the top-left pixel of the text. OPT_CENTERX centers the text horizontally, OPT_CENTERY centers it vertically. OPT_CENTER centers the text in both directions. OPT_RIGHTX right-justifies the text, so that the x is the rightmost pixel.&lt;br&gt;&lt;b&gt;s&lt;/b&gt;: text&lt;br&gt;&lt;br&gt;Draw text.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="731"/>
        <source>DESCRIPTION_CMD_BUTTON.</source>
        <translation type="unfinished">&lt;b&gt;CMD_BUTTON&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;w&lt;/i&gt;, &lt;i&gt;h&lt;/i&gt;, &lt;i&gt;font&lt;/i&gt;, &lt;i&gt;options&lt;/i&gt;, &lt;i&gt;s&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: x-coordinate of button top-left, in pixels&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: y-coordinate of button top-left, in pixels&lt;br&gt;&lt;b&gt;w&lt;/b&gt;: width of button, in pixels&lt;br&gt;&lt;b&gt;h&lt;/b&gt;: height of button, in pixels&lt;br&gt;&lt;b&gt;font&lt;/b&gt;: Font to use for text, 0-31. See ROM and RAM Fonts&lt;br&gt;&lt;b&gt;options&lt;/b&gt;: By default the button is drawn with a 3D effect. OPT_FLAT removes the 3D effect.&lt;br&gt;&lt;b&gt;s&lt;/b&gt;: button label&lt;br&gt;&lt;br&gt;Draw a button.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="747"/>
        <source>DESCRIPTION_CMD_KEYS.</source>
        <translation type="unfinished">&lt;b&gt;CMD_KEYS&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;w&lt;/i&gt;, &lt;i&gt;h&lt;/i&gt;, &lt;i&gt;font&lt;/i&gt;, &lt;i&gt;options&lt;/i&gt;, &lt;i&gt;s&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: x-coordinate of keys top-left, in pixels&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: y-coordinate of keys top-left, in pixels&lt;br&gt;&lt;b&gt;w&lt;/b&gt;: width of keys, in pixels&lt;br&gt;&lt;b&gt;h&lt;/b&gt;: height of keys, in pixels&lt;br&gt;&lt;b&gt;font&lt;/b&gt;: Font to use for keys, 0-31. See ROM and RAM Fonts&lt;br&gt;&lt;b&gt;options&lt;/b&gt;: By default the keys are drawn with a 3D effect. OPT_FLAT removes the 3D effect. If OPT_CENTER is given the keys are drawn at minimum size centered within the w x h rectangle. Otherwise the keys are expanded so that they completely fill the available space. If an ASCII code is specified, that key is drawn &apos;pressed&apos; - i.e. in background color with any 3D effect removed..&lt;br&gt;&lt;b&gt;s&lt;/b&gt;: key labels, one character per key. The TAG value is set to the ASCII value of each key, so that key presses can be detected using the REG_TOUCH_TAG register.&lt;br&gt;&lt;br&gt;Draw a row of keys.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="764"/>
        <source>DESCRIPTION_CMD_PROGRESS.</source>
        <translation type="unfinished">&lt;b&gt;CMD_PROGRESS&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;w&lt;/i&gt;, &lt;i&gt;h&lt;/i&gt;, &lt;i&gt;options&lt;/i&gt;, &lt;i&gt;val&lt;/i&gt;, &lt;i&gt;range&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: x-coordinate of progress bar top-left, in pixels&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: y-coordinate of progress bar top-left, in pixels&lt;br&gt;&lt;b&gt;w&lt;/b&gt;: width of progress bar, in pixels&lt;br&gt;&lt;b&gt;h&lt;/b&gt;: height of progress bar, in pixels&lt;br&gt;&lt;b&gt;options&lt;/b&gt;: By default the progress bar is drawn with a 3D effect. OPT_FLAT removes the 3D effect&lt;br&gt;&lt;b&gt;val&lt;/b&gt;: Displayed value of progress bar, between 0 and range inclusive&lt;br&gt;&lt;b&gt;range&lt;/b&gt;: Maximum value&lt;br&gt;&lt;br&gt;Draw a progress bar.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="780"/>
        <source>DESCRIPTION_CMD_SLIDER.</source>
        <translation type="unfinished">&lt;b&gt;CMD_SLIDER&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;w&lt;/i&gt;, &lt;i&gt;h&lt;/i&gt;, &lt;i&gt;options&lt;/i&gt;, &lt;i&gt;val&lt;/i&gt;, &lt;i&gt;range&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: x-coordinate of slider top-left, in pixels&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: y-coordinate of slider top-left, in pixels&lt;br&gt;&lt;b&gt;w&lt;/b&gt;: width of slider, in pixels&lt;br&gt;&lt;b&gt;h&lt;/b&gt;: height of slider, in pixels&lt;br&gt;&lt;b&gt;options&lt;/b&gt;: By default the slider is drawn with a 3D effect. OPT_FLAT removes the 3D effect&lt;br&gt;&lt;b&gt;val&lt;/b&gt;: Displayed value of slider, between 0 and range inclusive&lt;br&gt;&lt;b&gt;range&lt;/b&gt;: Maximum value&lt;br&gt;&lt;br&gt;Draw a slider.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="796"/>
        <source>DESCRIPTION_CMD_SCROLLBAR.</source>
        <translation type="unfinished">&lt;b&gt;CMD_SCROLLBAR&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;w&lt;/i&gt;, &lt;i&gt;h&lt;/i&gt;, &lt;i&gt;options&lt;/i&gt;, &lt;i&gt;val&lt;/i&gt;, &lt;i&gt;size&lt;/i&gt;, &lt;i&gt;range&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: x-coordinate of scroll bar top-left, in pixels&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: y-coordinate of scroll bar top-left, in pixels&lt;br&gt;&lt;b&gt;w&lt;/b&gt;: width of scroll bar, in pixels. If width is greater, the scroll bar is drawn horizontally&lt;br&gt;&lt;b&gt;h&lt;/b&gt;: height of scroll bar, in pixels. If height is greater, the scroll bar is drawn vertically&lt;br&gt;&lt;b&gt;options&lt;/b&gt;: By default the scroll bar is drawn with a 3D effect. OPT_FLAT removes the 3D effect&lt;br&gt;&lt;b&gt;val&lt;/b&gt;: Displayed value of scroll bar, between 0 and range inclusive&lt;br&gt;&lt;b&gt;size&lt;/b&gt;: Size&lt;br&gt;&lt;b&gt;range&lt;/b&gt;: Maximum value&lt;br&gt;&lt;br&gt;Draw a scroll bar.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="813"/>
        <source>DESCRIPTION_CMD_TOGGLE.</source>
        <translation type="unfinished">&lt;b&gt;CMD_TOGGLE&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;w&lt;/i&gt;, &lt;i&gt;f&lt;/i&gt;, &lt;i&gt;options&lt;/i&gt;, &lt;i&gt;state&lt;/i&gt;, &lt;i&gt;s&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: x-coordinate of top-left of toggle, in pixels&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: y-coordinate of top-left of toggle, in pixels&lt;br&gt;&lt;b&gt;w&lt;/b&gt;: width of toggle, in pixels&lt;br&gt;&lt;b&gt;f&lt;/b&gt;: font to use for text, 0-31. See ROM and RAM Fonts&lt;br&gt;&lt;b&gt;options&lt;/b&gt;: By default the toggle bar is drawn with a 3D effect. OPT_FLAT removes the 3D effect&lt;br&gt;&lt;b&gt;state&lt;/b&gt;: state of the toggle: 0 is off, 65535 is on.&lt;br&gt;&lt;b&gt;s&lt;/b&gt;: String label for toggle. A character value of 255 (in C it can be written as �) separates the two labels.&lt;br&gt;&lt;br&gt;Draw a toggle switch.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="830"/>
        <source>DESCRIPTION_CMD_GAUGE</source>
        <translation type="unfinished">&lt;b&gt;CMD_GAUGE&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;r&lt;/i&gt;, &lt;i&gt;options&lt;/i&gt;, &lt;i&gt;major&lt;/i&gt;, &lt;i&gt;minor&lt;/i&gt;, &lt;i&gt;val&lt;/i&gt;, &lt;i&gt;range&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: X-coordinate of gauge center, in pixels&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: Y-coordinate of gauge center, in pixels&lt;br&gt;&lt;b&gt;r&lt;/b&gt;: Radius of the gauge, in pixels&lt;br&gt;&lt;b&gt;options&lt;/b&gt;: By default the gauge dial is drawn with a 3D effect. OPT_FLAT removes the 3D effect. With option OPT_NOBACK, the background is not drawn. With option OPT_NOTICKS, the tick marks are not drawn. With option OPT_NOPOINTER, the pointer is not drawn.&lt;br&gt;&lt;b&gt;major&lt;/b&gt;: Number of major subdivisions on the dial, 1-10&lt;br&gt;&lt;b&gt;minor&lt;/b&gt;: Number of minor subdivisions on the dial, 1-10&lt;br&gt;&lt;b&gt;val&lt;/b&gt;: Gauge indicated value, between 0 and range, inclusive&lt;br&gt;&lt;b&gt;range&lt;/b&gt;: Maximum value&lt;br&gt;&lt;br&gt;Draw a gauge.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="848"/>
        <source>DESCRIPTION_CMD_CLOCK.</source>
        <translation type="unfinished">&lt;b&gt;CMD_CLOCK&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;r&lt;/i&gt;, &lt;i&gt;options&lt;/i&gt;, &lt;i&gt;h&lt;/i&gt;, &lt;i&gt;m&lt;/i&gt;, &lt;i&gt;s&lt;/i&gt;, &lt;i&gt;ms&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: X-coordinate of clock center, in pixels&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: Y-coordinate of clock center, in pixels&lt;br&gt;&lt;b&gt;r&lt;/b&gt;: Radius of the clock, in pixels&lt;br&gt;&lt;b&gt;options&lt;/b&gt;: By default the clock dial is drawn with a 3D effect. OPT_FLAT removes the 3D effect. With option OPT_NOBACK, the background is not drawn. With option OPT_NOTICKS, the twelve hour ticks are not drawn. With option OPT_NOSECS, the seconds hand is not drawn. With option OPT_NOHANDS, no hands are drawn. With option OPT_NOHM, no hour and minutes hands are drawn.&lt;br&gt;&lt;b&gt;h&lt;/b&gt;: hours&lt;br&gt;&lt;b&gt;m&lt;/b&gt;: minutes&lt;br&gt;&lt;b&gt;s&lt;/b&gt;: seconds&lt;br&gt;&lt;b&gt;ms&lt;/b&gt;: milliseconds&lt;br&gt;&lt;br&gt;Draw a clock.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="866"/>
        <source>DESCRIPTION_CMD_CALIBRATE.</source>
        <translation type="unfinished">&lt;b&gt;CMD_CALIBRATE&lt;/b&gt;(&lt;i&gt;result&lt;/i&gt;)&lt;br&gt;&lt;b&gt;result&lt;/b&gt;: output parameter; written with 0 on failure&lt;br&gt;&lt;br&gt;The calibration procedure collects three touches from the touch screen, then computes and loads an appropriate matrix into REG_TOUCH_TRANSFORM_A-F. To use it, create a display list and then use CMD_CALIBRATE. The co-processor engine overlays the touch targets on the current display list, gathers the calibration input and updates REG_TOUCH_TRANSFORM_A-F.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="877"/>
        <source>DESCRIPTION_CMD_SPINNER.</source>
        <translation type="unfinished">&lt;b&gt;CMD_SPINNER&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;style&lt;/i&gt;, &lt;i&gt;scale&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: x&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: y&lt;br&gt;&lt;b&gt;style&lt;/b&gt;: style&lt;br&gt;&lt;b&gt;scale&lt;/b&gt;: scale&lt;br&gt;&lt;br&gt;The spinner is an animated overlay that shows the user that some task is continuing. To trigger the spinner, create a display list and then use CMD_SPINNER. The co-processor engine overlays the spinner on the current display list, swaps the display list to make it visible, then continuously animates until it receives CMD_STOP. REG_MACRO_0 and REG_MACRO_1 registers are utilized to perform the animation kind of effect. The frequency of points movement is wrt display frame rate configured.&lt;br&gt;Typically for 480x272 display panels the display rate is ~60fps. For style 0 and 60fps, the point repeats the sequence within 2 seconds. For style 1 and 60fps, the point repeats the sequence within 1.25 seconds. For style 2 and 60fps, the clock hand repeats the sequence within 2 seconds. For style 3 and 60fps, the moving dots repeat the sequence within 1 second.&lt;br&gt;Note that only one of CMD_SKETCH, CMD_CSKETCH, CMD_SCREENSAVER, or CMD_SPINNER can be active at one time.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="891"/>
        <source>DESCRIPTION_CMD_STOP.</source>
        <translation type="unfinished">&lt;b&gt;CMD_STOP&lt;/b&gt;()&lt;br&gt;&lt;br&gt;Stop any spinner, screensaver or sketch.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="901"/>
        <source>DESCRIPTION_CMD_MEMSET.</source>
        <translation type="unfinished">&lt;b&gt;CMD_MEMSET&lt;/b&gt;(&lt;i&gt;ptr&lt;/i&gt;, &lt;i&gt;value&lt;/i&gt;, &lt;i&gt;num&lt;/i&gt;)&lt;br&gt;&lt;b&gt;ptr&lt;/b&gt;: Starting address of the memory block&lt;br&gt;&lt;b&gt;value&lt;/b&gt;: Value to be written to memory&lt;br&gt;&lt;b&gt;num&lt;/b&gt;: Number of bytes in the memory block&lt;br&gt;&lt;br&gt;Fill memory with a byte value.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="915"/>
        <source>DESCRIPTION_CMD_MEMZERO.</source>
        <translation type="unfinished">&lt;b&gt;CMD_MEMZERO&lt;/b&gt;(&lt;i&gt;ptr&lt;/i&gt;, &lt;i&gt;num&lt;/i&gt;)&lt;br&gt;&lt;b&gt;ptr&lt;/b&gt;: Starting address of the memory block&lt;br&gt;&lt;b&gt;num&lt;/b&gt;: Number of bytes in the memory block&lt;br&gt;&lt;br&gt;Write zero to a block of memory.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="928"/>
        <source>DESCRIPTION_CMD_MEMCPY.</source>
        <translation type="unfinished">&lt;b&gt;CMD_MEMCPY&lt;/b&gt;(&lt;i&gt;dest&lt;/i&gt;, &lt;i&gt;src&lt;/i&gt;, &lt;i&gt;num&lt;/i&gt;)&lt;br&gt;&lt;b&gt;dest&lt;/b&gt;: address of the destination memory block&lt;br&gt;&lt;b&gt;src&lt;/b&gt;: address of the source memory block&lt;br&gt;&lt;b&gt;num&lt;/b&gt;: Number of bytes in the memory block&lt;br&gt;&lt;br&gt;Copy a block of memory.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="942"/>
        <source>DESCRIPTION_CMD_APPEND.</source>
        <translation type="unfinished">&lt;b&gt;CMD_APPEND&lt;/b&gt;(&lt;i&gt;ptr&lt;/i&gt;, &lt;i&gt;num&lt;/i&gt;)&lt;br&gt;&lt;b&gt;ptr&lt;/b&gt;: Start of source commands in main memory&lt;br&gt;&lt;b&gt;num&lt;/b&gt;: Number of bytes to copy. This must be a multiple of 4&lt;br&gt;&lt;br&gt;Append memory to display list.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="955"/>
        <source>DESCRIPTION_CMD_SNAPSHOT.</source>
        <translation type="unfinished">&lt;b&gt;CMD_SNAPSHOT&lt;/b&gt;(&lt;i&gt;ptr&lt;/i&gt;)&lt;br&gt;&lt;b&gt;ptr&lt;/b&gt;: Snapshot destination address, in main memory&lt;br&gt;&lt;br&gt;Take a snapshot of the current screen.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="967"/>
        <source>DESCRIPTION_CMD_LOADIDENTITY.</source>
        <translation type="unfinished">&lt;b&gt;CMD_LOADIDENTITY&lt;/b&gt;()&lt;br&gt;&lt;br&gt;Set the current matrix to identity.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="977"/>
        <source>DESCRIPTION_CMD_TRANSLATE.</source>
        <translation type="unfinished">&lt;b&gt;CMD_TRANSLATE&lt;/b&gt;(&lt;i&gt;tx&lt;/i&gt;, &lt;i&gt;ty&lt;/i&gt;)&lt;br&gt;&lt;b&gt;tx&lt;/b&gt;: x translate factor, in signed 16.16 bit fixed-point form.&lt;br&gt;&lt;b&gt;ty&lt;/b&gt;: y translate factor, in signed 16.16 bit fixed-point form.&lt;br&gt;&lt;br&gt;Apply a translation to the current matrix.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="990"/>
        <source>DESCRIPTION_CMD_SCALE.</source>
        <translation type="unfinished">&lt;b&gt;CMD_SCALE&lt;/b&gt;(&lt;i&gt;sx&lt;/i&gt;, &lt;i&gt;sy&lt;/i&gt;)&lt;br&gt;&lt;b&gt;sx&lt;/b&gt;: x scale factor, in signed 16.16 bit fixed-point form.&lt;br&gt;&lt;b&gt;sy&lt;/b&gt;: y scale factor, in signed 16.16 bit fixed-point form.&lt;br&gt;&lt;br&gt;Apply a scale to the current matrix.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1003"/>
        <source>DESCRIPTION_CMD_ROTATE.</source>
        <translation type="unfinished">&lt;b&gt;CMD_ROTATE&lt;/b&gt;(&lt;i&gt;a&lt;/i&gt;)&lt;br&gt;&lt;b&gt;a&lt;/b&gt;: Clockwise rotation angle, in units of 1/65536 of a circle.&lt;br&gt;&lt;br&gt;Apply a rotation to the current matrix.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1015"/>
        <source>DESCRIPTION_CMD_SETMATRIX.</source>
        <translation type="unfinished">&lt;b&gt;CMD_SETMATRIX&lt;/b&gt;()&lt;br&gt;&lt;br&gt;Write the current matrix as a bitmap transform.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1025"/>
        <source>DESCRIPTION_CMD_SETFONT.</source>
        <translation type="unfinished">&lt;b&gt;CMD_SETFONT&lt;/b&gt;(&lt;i&gt;font&lt;/i&gt;, &lt;i&gt;ptr&lt;/i&gt;)&lt;br&gt;&lt;b&gt;font&lt;/b&gt;: font&lt;br&gt;&lt;b&gt;ptr&lt;/b&gt;: ptr&lt;br&gt;&lt;br&gt;To use a custom font with the co-processor engine objects, create the font definition data in FT800 RAM and issue CMD_SETFONT, as described in ROM and RAM Fonts.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1038"/>
        <source>DESCRIPTION_CMD_TRACK.</source>
        <translation type="unfinished">&lt;b&gt;CMD_TRACK&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;w&lt;/i&gt;, &lt;i&gt;h&lt;/i&gt;, &lt;i&gt;tag&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: x-coordinate of track area top-left, in pixels&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: y-coordinate of track area top-left, in pixels&lt;br&gt;&lt;b&gt;w&lt;/b&gt;: Width of track area, in pixels&lt;br&gt;&lt;b&gt;h&lt;/b&gt;: Height of track area, in pixels.&lt;br&gt;A w and h of (1,1) means that the tracker is rotary, and reports an angle value in REG_TRACKER. (0,0) disables the tracker. Other values mean that the tracker is linear, and reports values along its length from 0 to 65535 in REG_TRACKER.&lt;br&gt;&lt;b&gt;tag&lt;/b&gt;: tag for this track, 1-255&lt;br&gt;&lt;br&gt;The co-processor engine can assist the MCU in tracking touches on graphical objects. For example touches on dial objects can be reported as angles, saving MCU computation. To do this the MCU draws the object using a chosen tag value, and registers a track area for that tag.&lt;br&gt;From then on any touch on that object is reported in REG_TRACKER. The MCU can detect any touch on the object by reading the 32-bit value in REG_TRACKER. The low 8 bits give the current tag, or zero if there is no touch. The high sixteen bits give the tracked value.&lt;br&gt;For a rotary tracker - used for clocks, gauges and dials - this value is the angle of the touch point relative to the object center, in units of 1=65536 of a circle. 0 means that the angle is straight down, 0x4000 left, 0x8000 up, and 0xc000 right.&lt;br&gt;For a linear tracker - used for sliders and scrollbars - this value is the distance along the tracked object, from 0 to 65535.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1052"/>
        <source>DESCRIPTION_CMD_DIAL.</source>
        <translation type="unfinished">&lt;b&gt;CMD_DIAL&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;r&lt;/i&gt;, &lt;i&gt;options&lt;/i&gt;, &lt;i&gt;val&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: x-coordinate of dial center, in pixels&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: y-coordinate of dial center, in pixels&lt;br&gt;&lt;b&gt;r&lt;/b&gt;: radius of dial, in pixels&lt;br&gt;&lt;b&gt;options&lt;/b&gt;: By default the dial is drawn with a 3D effect. OPT_FLAT removes the 3D effect&lt;br&gt;&lt;b&gt;val&lt;/b&gt;: Displayed value of slider, between 0 and 65535 inclusive. 0 means that the dial points straight down, 0x4000 left, 0x8000 up, and 0xc000 right.&lt;br&gt;&lt;br&gt;Draw a rotary dial control.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1067"/>
        <source>DESCRIPTION_CMD_NUMBER.</source>
        <translation type="unfinished">&lt;b&gt;CMD_NUMBER&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;font&lt;/i&gt;, &lt;i&gt;options&lt;/i&gt;, &lt;i&gt;n&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: x-coordinate of text base, in pixels&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: y-coordinate of text base, in pixels&lt;br&gt;&lt;b&gt;font&lt;/b&gt;: Font to use for text, 0-31. See ROM and RAM Fonts&lt;br&gt;&lt;b&gt;options&lt;/b&gt;: By default (x; y) is the top-left pixel of the text. OPT_CENTERX centers the text horizontally, OPT_CENTERY centers it vertically. OPT_CENTER centers the text in both directions. OPT_RIGHTX right-justifies the text, so that the x is the rightmost pixel. By default the number is displayed with no leading zeroes, but if a width 1-9 is specified in the options, then the number is padded if necessary with leading zeroes so that it has the given width. If OPT_SIGNED is given, the number is treated as signed, and prefixed by a minus sign if negative.&lt;br&gt;&lt;b&gt;n&lt;/b&gt;: The number to display, either unsigned or signed 32-bit&lt;br&gt;&lt;br&gt;Draw text.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1082"/>
        <source>DESCRIPTION_CMD_SCREENSAVER.</source>
        <translation type="unfinished">&lt;b&gt;CMD_SCREENSAVER&lt;/b&gt;()&lt;br&gt;&lt;br&gt;After the screensaver command, the co-processor engine continuously updates REG_MACRO_0 with VERTEX2F with varying (x; y) coordinates. With an appropriate display list, this causes a bitmap to move around the screen without any MCU work.&lt;br&gt;Command CMD_STOP stops the update process.&lt;br&gt;Note that only one of CMD_SKETCH, CMD_CSKETCH, CMD_SCREENSAVER, or CMD_SPINNER can be active at one time.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1092"/>
        <source>DESCRIPTION_CMD_SKETCH.</source>
        <translation type="unfinished">&lt;b&gt;CMD_SKETCH&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;w&lt;/i&gt;, &lt;i&gt;h&lt;/i&gt;, &lt;i&gt;ptr&lt;/i&gt;, &lt;i&gt;format&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: x-coordinate of sketch area, in pixels&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: y-coordinate of sketch area, in pixels&lt;br&gt;&lt;b&gt;w&lt;/b&gt;: Width of sketch area, in pixels&lt;br&gt;&lt;b&gt;h&lt;/b&gt;: Height of sketch area, in pixels&lt;br&gt;&lt;b&gt;ptr&lt;/b&gt;: Base address of sketch bitmap&lt;br&gt;&lt;b&gt;format&lt;/b&gt;: Format of sketch bitmap, either L1 or L8&lt;br&gt;&lt;br&gt;After the sketch command, the co-processor engine continuously samples the touch inputs and paints pixels into a bitmap, according to the touch (x; y). This means that the user touch inputs are drawn into the bitmap without any need for MCU work.&lt;br&gt;Command CMD_STOP stops the update process.&lt;br&gt;Note that only one of CMD_SKETCH, CMD_CSKETCH, CMD_SCREENSAVER, or CMD_SPINNER can be active at one time.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1107"/>
        <source>DESCRIPTION_CMD_CSKETCH.</source>
        <translation type="unfinished">&lt;b&gt;CMD_CSKETCH&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;, &lt;i&gt;w&lt;/i&gt;, &lt;i&gt;h&lt;/i&gt;, &lt;i&gt;ptr&lt;/i&gt;, &lt;i&gt;format&lt;/i&gt;, &lt;i&gt;freq&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: x-coordinate of sketch area, in pixels&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: y-coordinate of sketch area, in pixels&lt;br&gt;&lt;b&gt;w&lt;/b&gt;: Width of sketch area, in pixels&lt;br&gt;&lt;b&gt;h&lt;/b&gt;: Height of sketch area, in pixels&lt;br&gt;&lt;b&gt;ptr&lt;/b&gt;: Base address of sketch bitmap&lt;br&gt;&lt;b&gt;format&lt;/b&gt;: Format of sketch bitmap, either L1 or L8&lt;br&gt;&lt;b&gt;freq&lt;/b&gt;: The oversampling frequency. The typical value is 1500 to make sure the lines are connected smoothly. The value zero means no oversampling operation&lt;br&gt;&lt;br&gt;This command is only valid for FT801 silicon. FT801 co-processor will oversample the coordinates reported by the capacitive touch panel in the frequency of �freq� and forms the lines with a smoother effect.&lt;br&gt;&lt;br&gt;After the sketch command, the co-processor engine continuously samples the touch inputs and paints pixels into a bitmap, according to the touch (x; y). This means that the user touch inputs are drawn into the bitmap without any need for MCU work.&lt;br&gt;Command CMD_STOP stops the update process.&lt;br&gt;Note that only one of CMD_SKETCH, CMD_CSKETCH, CMD_SCREENSAVER, or CMD_SPINNER can be active at one time.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1123"/>
        <source>DESCRIPTION_CMD_LOGO.</source>
        <translation type="unfinished">&lt;b&gt;CMD_LOGO&lt;/b&gt;()&lt;br&gt;&lt;br&gt;Play device logo animation.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1133"/>
        <source>DESCRIPTION_CMD_COLDSTART.</source>
        <translation type="unfinished">&lt;b&gt;CMD_COLDSTART&lt;/b&gt;()&lt;br&gt;&lt;br&gt;Set co-processor engine state to default values.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1143"/>
        <source>DESCRIPTION_CMD_GRADCOLOR.</source>
        <translation type="unfinished">&lt;b&gt;CMD_GRADCOLOR&lt;/b&gt;(&lt;i&gt;r&lt;/i&gt;, &lt;i&gt;g&lt;/i&gt;, &lt;i&gt;b&lt;/i&gt;)&lt;br&gt;&lt;b&gt;rgb&lt;/b&gt;: New highlight gradient color, as a 24-bit RGB number. Red is the most significant 8 bits, blue is the least. So 0xff0000 is bright red.&lt;br&gt;&lt;br&gt;Set the 3D button highlight color.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1166"/>
        <source>DESCRIPTION_CMD_SETROTATE.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1179"/>
        <source>DESCRIPTION_CMD_SNAPSHOT2.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1189"/>
        <source>Set snapshot format</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1200"/>
        <source>DESCRIPTION_CMD_SETBASE.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1204"/>
        <source>Base</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1204"/>
        <source>Set the base</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1219"/>
        <source>DESCRIPTION_CMD_SETFONT2.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1223"/>
        <location filename="interactive_properties.cpp" line="1237"/>
        <location filename="interactive_properties.cpp" line="1258"/>
        <source>Font</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1223"/>
        <location filename="interactive_properties.cpp" line="1237"/>
        <location filename="interactive_properties.cpp" line="1258"/>
        <source>Set font</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1225"/>
        <location filename="interactive_properties.cpp" line="1239"/>
        <source>First character</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1225"/>
        <location filename="interactive_properties.cpp" line="1239"/>
        <source>Set first character</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1233"/>
        <source>DESCRIPTION_CMD_SETSCRATCH.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1254"/>
        <source>DESCRIPTION_CMD_ROMFONT.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1259"/>
        <source>ROM Slot</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1259"/>
        <source>Set ROM slot</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1277"/>
        <source>DESCRIPTION_CMD_SETBITMAP.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1295"/>
        <source>DESCRIPTION_DISPLAY.</source>
        <translation type="unfinished">&lt;b&gt;DISPLAY&lt;/b&gt;()&lt;br&gt;&lt;br&gt;End the display list. FT800 will ignore all the commands following this command.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1305"/>
        <source>DESCRIPTION_BITMAP_SOURCE.</source>
        <translation type="unfinished">&lt;b&gt;BITMAP_SOURCE&lt;/b&gt;(&lt;i&gt;addr&lt;/i&gt;)&lt;br&gt;&lt;b&gt;addr&lt;/b&gt;: Bitmap address in graphics SRAM FT800, aligned with respect to the bitmap format.&lt;br&gt;For example, if the bitmap format is RGB565/ARGB4/ARGB1555, the bitmap source shall be aligned to 2 bytes.&lt;br&gt;&lt;br&gt;Specify the source address of bitmap data in FT800 graphics memory RAM_G.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1317"/>
        <source>DESCRIPTION_CLEAR_COLOR_RGB.</source>
        <translation type="unfinished">&lt;b&gt;CLEAR_COLOR_RGB&lt;/b&gt;(&lt;i&gt;red&lt;/i&gt;, &lt;i&gt;green&lt;/i&gt;, &lt;i&gt;blue&lt;/i&gt;)&lt;br&gt;&lt;b&gt;red&lt;/b&gt;: Red value used when the color buffer is cleared. The initial value is 0&lt;br&gt;&lt;b&gt;green&lt;/b&gt;: Green value used when the color buffer is cleared. The initial value is 0&lt;br&gt;&lt;b&gt;blue&lt;/b&gt;: Blue value used when the color buffer is cleared. The initial value is 0&lt;br&gt;&lt;br&gt;Sets the color values used by a following CLEAR.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1329"/>
        <source>DESCRIPTION_TAG.</source>
        <translation type="unfinished">&lt;b&gt;TAG&lt;/b&gt;(&lt;i&gt;s&lt;/i&gt;)&lt;br&gt;&lt;b&gt;s&lt;/b&gt;: Tag value. Valid value range is from 1 to 255.&lt;br&gt;&lt;br&gt;Attach the tag value for the following graphics objects drawn on the screen. The initial tag buffer value is 255.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1341"/>
        <source>DESCRIPTION_COLOR_RGB.</source>
        <translation type="unfinished">&lt;b&gt;COLOR_RGB&lt;/b&gt;(&lt;i&gt;red&lt;/i&gt;, &lt;i&gt;green&lt;/i&gt;, &lt;i&gt;blue&lt;/i&gt;)&lt;br&gt;&lt;b&gt;red&lt;/b&gt;: Red value for the current color. The initial value is 255&lt;br&gt;&lt;b&gt;green&lt;/b&gt;: Green value for the current color. The initial value is 255&lt;br&gt;&lt;b&gt;blue&lt;/b&gt;: Blue value for the current color. The initial value is 255&lt;br&gt;&lt;br&gt;Sets red, green and blue values of the FT800 color buffer which will be applied to the following draw operation.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1353"/>
        <source>DESCRIPTION_BITMAP_HANDLE.</source>
        <translation type="unfinished">&lt;b&gt;BITMAP_HANDLE&lt;/b&gt;(&lt;i&gt;handle&lt;/i&gt;)&lt;br&gt;&lt;b&gt;handle&lt;/b&gt;: Bitmap handle. The initial value is 0. The valid value range is from 0 to 31.&lt;br&gt;&lt;br&gt;Specify the bitmap handle.&lt;br&gt;&lt;br&gt;Handles 16 to 31 are defined by the FT800 for built-in font and handle 15 is defined in the co-processor engine commands CMD_GRADIENT, CMD_BUTTON and CMD_KEYS. Users can define new bitmaps using handles from 0 to 14. If there is no co-processor engine command CMD_GRADIENT, CMD_BUTTON and CMD_KEYS in the current display list, users can even define a bitmap using handle 15.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1365"/>
        <source>DESCRIPTION_CELL.</source>
        <translation type="unfinished">&lt;b&gt;CELL&lt;/b&gt;(&lt;i&gt;cell&lt;/i&gt;)&lt;br&gt;&lt;b&gt;cell&lt;/b&gt;: Bitmap cell number. The initial value is 0&lt;br&gt;&lt;br&gt;Specify the bitmap cell number for the VERTEX2F command.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1377"/>
        <source>DESCRIPTION_BITMAP_LAYOUT.</source>
        <translation type="unfinished">&lt;b&gt;BITMAP_LAYOUT&lt;/b&gt;(&lt;i&gt;format&lt;/i&gt;, &lt;i&gt;linestride&lt;/i&gt;, &lt;i&gt;height&lt;/i&gt;)&lt;br&gt;&lt;b&gt;format&lt;/b&gt;: Bitmap pixel format. The bitmap formats supported are L1, L4, L8, RGB332, ARGB2, ARGB4, ARGB1555, RGB565 and PALETTED.&lt;br&gt;&lt;b&gt;linestride&lt;/b&gt;: Bitmap linestride, in bytes. For L1 format, the line stride must be a multiple of 8 bits; For L4 format the line stride must be multiple of 2 nibbles. (Aligned to byte).&lt;br&gt;&lt;b&gt;height&lt;/b&gt;: Bitmap height, in lines&lt;br&gt;&lt;br&gt;Specify the source bitmap memory format and layout for the current handle.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1391"/>
        <source>DESCRIPTION_BITMAP_SIZE.</source>
        <translation type="unfinished">&lt;b&gt;BITMAP_SIZE&lt;/b&gt;(&lt;i&gt;filter&lt;/i&gt;, &lt;i&gt;wrapx&lt;/i&gt;, &lt;i&gt;wrapy&lt;/i&gt;, &lt;i&gt;width&lt;/i&gt;, &lt;i&gt;height&lt;/i&gt;)&lt;br&gt;&lt;b&gt;filter&lt;/b&gt;: Bitmap filtering mode, one of NEAREST or BILINEAR&lt;br&gt;&lt;b&gt;wrapx&lt;/b&gt;: Bitmap x wrap mode, one of REPEAT or BORDER&lt;br&gt;&lt;b&gt;wrapy&lt;/b&gt;: Bitmap y wrap mode, one of REPEAT or BORDER&lt;br&gt;&lt;b&gt;width&lt;/b&gt;: Drawn bitmap width, in pixels&lt;br&gt;&lt;b&gt;height&lt;/b&gt;: Drawn bitmap height, in pixels&lt;br&gt;&lt;br&gt;Specify the screen drawing of bitmaps for the current handle.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1407"/>
        <source>DESCRIPTION_ALPHA_FUNC.</source>
        <translation type="unfinished">&lt;b&gt;ALPHA_FUNC&lt;/b&gt;(&lt;i&gt;func&lt;/i&gt;, &lt;i&gt;ref&lt;/i&gt;)&lt;br&gt;&lt;b&gt;func&lt;/b&gt;: Specifies the test function, one of NEVER, LESS, LEQUAL, GREATER, GEQUAL, EQUAL, NOTEQUAL, or ALWAYS. The initial value is ALWAYS (7)&lt;br&gt;&lt;b&gt;ref&lt;/b&gt;: Specifies the reference value for the alpha test. The initial value is 0&lt;br&gt;&lt;br&gt;Specify the alpha test function.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1420"/>
        <source>DESCRIPTION_STENCIL_FUNC.</source>
        <translation type="unfinished">&lt;b&gt;STENCIL_FUNC&lt;/b&gt;(&lt;i&gt;func&lt;/i&gt;, &lt;i&gt;ref&lt;/i&gt;, &lt;i&gt;mask&lt;/i&gt;)&lt;br&gt;&lt;b&gt;func&lt;/b&gt;: Specifies the test function, one of NEVER, LESS, LEQUAL, GREATER, GEQUAL, EQUAL, NOTEQUAL, or ALWAYS. The initial value is ALWAYS. &lt;br&gt;&lt;b&gt;ref&lt;/b&gt;: Specifies the reference value for the stencil test. The initial value is 0&lt;br&gt;&lt;b&gt;mask&lt;/b&gt;: Specifies a mask that is ANDed with the reference value and the stored stencil value. The initial value is 255&lt;br&gt;&lt;br&gt;Set function and reference value for stencil testing.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1435"/>
        <source>DESCRIPTION_BLEND_FUNC.</source>
        <translation type="unfinished">&lt;b&gt;BLEND_FUNC&lt;/b&gt;(&lt;i&gt;src&lt;/i&gt;, &lt;i&gt;dst&lt;/i&gt;)&lt;br&gt;&lt;b&gt;src&lt;/b&gt;: Specifies how the source blending factor is computed. One of ZERO, ONE, SRC_ALPHA, DST_ALPHA, ONE_MINUS_SRC_ALPHA or ONE_MINUS_DST_ALPHA. The initial value is SRC_ALPHA (2).&lt;br&gt;&lt;b&gt;dst&lt;/b&gt;: Specifies how the destination blending factor is computed, one of the same constants as src. The initial value is ONE_MINUS_SRC_ALPHA(4)&lt;br&gt;&lt;br&gt;Specify pixel arithmetic.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1448"/>
        <source>DESCRIPTION_STENCIL_OP.</source>
        <translation type="unfinished">&lt;b&gt;STENCIL_OP&lt;/b&gt;(&lt;i&gt;sfail&lt;/i&gt;, &lt;i&gt;spass&lt;/i&gt;)&lt;br&gt;&lt;b&gt;sfail&lt;/b&gt;: Specifies the action to take when the stencil test fails, one of KEEP, ZERO, REPLACE, INCR, DECR, and INVERT. The initial value is KEEP (1)&lt;br&gt;&lt;b&gt;spass&lt;/b&gt;: Specifies the action to take when the stencil test passes, one of the same constants as sfail. The initial value is KEEP (1)&lt;br&gt;&lt;br&gt;Set stencil test actions.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1461"/>
        <source>DESCRIPTION_POINT_SIZE.</source>
        <translation type="unfinished">&lt;b&gt;POINT_SIZE&lt;/b&gt;(&lt;i&gt;size&lt;/i&gt;)&lt;br&gt;&lt;b&gt;size&lt;/b&gt;: Point radius in 1/16 pixel. The initial value is 16.&lt;br&gt;&lt;br&gt;Sets the size of drawn points. The width is the distance from the center of the point to the outermost drawn pixel, in units of 1/16 pixels. The valid range is from 16 to 8191 with respect to 1/16th pixel unit.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1473"/>
        <source>DESCRIPTION_LINE_WIDTH.</source>
        <translation type="unfinished">&lt;b&gt;LINE_WIDTH&lt;/b&gt;(&lt;i&gt;width&lt;/i&gt;)&lt;br&gt;&lt;b&gt;width&lt;/b&gt;: Line width in 1/16 pixel. The initial value is 16.&lt;br&gt;&lt;br&gt;Sets the width of drawn lines. The width is the distance from the center of the line to the outermost drawn pixel, in units of 1/16 pixel. The valid range is from 16 to 4095 in terms of 1/16th pixel units.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1485"/>
        <source>DESCRIPTION_CLEAR_COLOR_A.</source>
        <translation type="unfinished">&lt;b&gt;CLEAR_COLOR_A&lt;/b&gt;(&lt;i&gt;alpha&lt;/i&gt;)&lt;br&gt;&lt;b&gt;alpha&lt;/b&gt;: Alpha value used when the color buffer is cleared. The initial value is 0.&lt;br&gt;&lt;br&gt;Specify clear value for the alpha channel.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1497"/>
        <source>DESCRIPTION_COLOR_A.</source>
        <translation type="unfinished">&lt;b&gt;COLOR_A&lt;/b&gt;(&lt;i&gt;alpha&lt;/i&gt;)&lt;br&gt;&lt;b&gt;alpha&lt;/b&gt;: Alpha for the current color. The initial value is 255&lt;br&gt;&lt;br&gt;Set the current color alpha.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1509"/>
        <source>DESCRIPTION_CLEAR_STENCIL.</source>
        <translation type="unfinished">&lt;b&gt;CLEAR_STENCIL&lt;/b&gt;(&lt;i&gt;s&lt;/i&gt;)&lt;br&gt;&lt;b&gt;s&lt;/b&gt;: Value used when the stencil buffer is cleared. The initial value is 0&lt;br&gt;&lt;br&gt;Specify clear value for the stencil buffer.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1522"/>
        <source>DESCRIPTION_CLEAR_TAG.</source>
        <translation type="unfinished">&lt;b&gt;CLEAR_TAG&lt;/b&gt;(&lt;i&gt;s&lt;/i&gt;)&lt;br&gt;&lt;b&gt;s&lt;/b&gt;: Value used when the tag buffer is cleared. The initial value is 0&lt;br&gt;&lt;br&gt;Specify clear value for the tag buffer.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1534"/>
        <source>DESCRIPTION_STENCIL_MASK.</source>
        <translation type="unfinished">&lt;b&gt;STENCIL_MASK&lt;/b&gt;(&lt;i&gt;mask&lt;/i&gt;)&lt;br&gt;&lt;b&gt;mask&lt;/b&gt;: The mask used to enable writing stencil bits. The initial value is 255&lt;br&gt;&lt;br&gt;Control the writing of individual bits in the stencil planes.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1547"/>
        <source>DESCRIPTION_TAG_MASK.</source>
        <translation type="unfinished">&lt;b&gt;TAG_MASK&lt;/b&gt;(&lt;i&gt;mask&lt;/i&gt;)&lt;br&gt;&lt;b&gt;mask&lt;/b&gt;: Allow updates to the tag buffer. The initial value is one and it means the tag buffer of the FT800 is updated with the value given by the TAG command. Therefore, the following graphics objects will be attached to the tag value given by the TAG command.&lt;br&gt;The value zero means the tag buffer of the FT800 is set as the default value,rather than the value given by TAG command in the display list.&lt;br&gt;&lt;br&gt;Control the writing of the tag buffer.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1559"/>
        <source>DESCRIPTION_BITMAP_TRANSFORM_A.</source>
        <translation type="unfinished">&lt;b&gt;BITMAP_TRANSFORM_A&lt;/b&gt;(&lt;i&gt;a&lt;/i&gt;)&lt;br&gt;&lt;b&gt;a&lt;/b&gt;: Coefficient A of the bitmap transform matrix, in signed 8.8 bit fixed-point form. The initial value is 256&lt;br&gt;&lt;br&gt;Specify the A coefficient of the bitmap transform </translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1571"/>
        <source>DESCRIPTION_BITMAP_TRANSFORM_B.</source>
        <translation type="unfinished">&lt;b&gt;BITMAP_TRANSFORM_B&lt;/b&gt;(&lt;i&gt;b&lt;/i&gt;)&lt;br&gt;&lt;b&gt;b&lt;/b&gt;: Coefficient B of the bitmap transform matrix, in signed 8.8 bit fixed-point form. The initial value is 0&lt;br&gt;&lt;br&gt;Specify the B coefficient of the bitmap transform matrix.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1583"/>
        <source>DESCRIPTION_BITMAP_TRANSFORM_C.</source>
        <translation type="unfinished">&lt;b&gt;BITMAP_TRANSFORM_C&lt;/b&gt;(&lt;i&gt;c&lt;/i&gt;)&lt;br&gt;&lt;b&gt;c&lt;/b&gt;: Coefficient C of the bitmap transform matrix, in signed 15.8 bit fixed-point form. The initial value is 0&lt;br&gt;&lt;br&gt;Specify the C coefficient of the bitmap transform matrix.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1595"/>
        <source>DESCRIPTION_BITMAP_TRANSFORM_D.</source>
        <translation type="unfinished">&lt;b&gt;BITMAP_TRANSFORM_D&lt;/b&gt;(&lt;i&gt;d&lt;/i&gt;)&lt;br&gt;&lt;b&gt;d&lt;/b&gt;: Coefficient D of the bitmap transform matrix, in signed 8.8 bit fixed-point form. The initial value is 0&lt;br&gt;&lt;br&gt;Specify the D coefficient of the bitmap transform matrix.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1607"/>
        <source>DESCRIPTION_BITMAP_TRANSFORM_E.</source>
        <translation type="unfinished">&lt;b&gt;BITMAP_TRANSFORM_E&lt;/b&gt;(&lt;i&gt;e&lt;/i&gt;)&lt;br&gt;&lt;b&gt;e&lt;/b&gt;: Coefficient E of the bitmap transform matrix, in signed 8.8 bit fixed-point form. The initial value is 256&lt;br&gt;&lt;br&gt;Specify the E coefficient of the bitmap transform matrix.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1619"/>
        <source>DESCRIPTION_BITMAP_TRANSFORM_F.</source>
        <translation type="unfinished">&lt;b&gt;BITMAP_TRANSFORM_F&lt;/b&gt;(&lt;i&gt;f&lt;/i&gt;)&lt;br&gt;&lt;b&gt;f&lt;/b&gt;: Coefficient F of the bitmap transform matrix, in signed 15.8 bit fixed-point form. The initial value is 0&lt;br&gt;&lt;br&gt;Specify the F coefficient of the bitmap transform matrix.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1631"/>
        <source>DESCRIPTION_SCISSOR_XY.</source>
        <translation type="unfinished">&lt;b&gt;SCISSOR_XY&lt;/b&gt;(&lt;i&gt;x&lt;/i&gt;, &lt;i&gt;y&lt;/i&gt;)&lt;br&gt;&lt;b&gt;x&lt;/b&gt;: The x coordinate of the scissor clip rectangle, in pixels. The initial value is 0&lt;br&gt;&lt;b&gt;y&lt;/b&gt;: The y coordinate of the scissor clip rectangle, in pixels. The initial value is 0&lt;br&gt;&lt;br&gt;Sets the top-left position of the scissor clip rectangle, which limits the drawing area.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1643"/>
        <source>DESCRIPTION_SCISSOR_SIZE.</source>
        <translation type="unfinished">&lt;b&gt;SCISSOR_SIZE&lt;/b&gt;(&lt;i&gt;width&lt;/i&gt;, &lt;i&gt;height&lt;/i&gt;)&lt;br&gt;&lt;b&gt;width&lt;/b&gt;: The width of the scissor clip rectangle, in pixels. The initial value is 512. The valid value range is from 0 to 512.&lt;br&gt;&lt;b&gt;height&lt;/b&gt;: The height of the scissor clip rectangle, in pixels. The initial value is 512. The valid value range is from 0 to 512.&lt;br&gt;&lt;br&gt;Sets the width and height of the scissor clip rectangle, which limits the drawing area.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1655"/>
        <source>DESCRIPTION_CALL.</source>
        <translation type="unfinished">&lt;b&gt;CALL&lt;/b&gt;(&lt;i&gt;dest&lt;/i&gt;)&lt;br&gt;&lt;b&gt;dest&lt;/b&gt;: The destination address in RAM_DL which the display command is to be switched. FT800 has the stack to store the return address. To come back to the next command of source address, the RETURN command can help.&lt;br&gt;&lt;br&gt;Execute a sequence of commands at another location in the display list&lt;br&gt;&lt;br&gt;CALL and RETURN have a 4 level stack in addition to the current pointer. Any additional CALL/RETURN done will lead to unexpected behavior.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1667"/>
        <source>DESCRIPTION_JUMP.</source>
        <translation type="unfinished">&lt;b&gt;JUMP&lt;/b&gt;(&lt;i&gt;dest&lt;/i&gt;)&lt;br&gt;&lt;b&gt;dest&lt;/b&gt;: Display list address to be jumped.&lt;br&gt;&lt;br&gt;Execute commands at another location in the display list.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1679"/>
        <source>DESCRIPTION_BEGIN.</source>
        <translation type="unfinished">&lt;b&gt;BEGIN&lt;/b&gt;(&lt;i&gt;prim&lt;/i&gt;)&lt;br&gt;&lt;b&gt;prim&lt;/b&gt;: Graphics primitive. The valid value is defined as: BITMAPS, POINTS, LINES, LINE_STRIP, EDGE_STRIP_R, EDGE_STRIP_L, EDGE_STRIP_A, EDGE_STRIP_B, RECTS&lt;br&gt;&lt;br&gt;Begin drawing a graphics primitive.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1691"/>
        <source>DESCRIPTION_COLOR_MASK.</source>
        <translation type="unfinished">&lt;b&gt;COLOR_MASK&lt;/b&gt;(&lt;i&gt;r&lt;/i&gt;, &lt;i&gt;g&lt;/i&gt;, &lt;i&gt;b&lt;/i&gt;, &lt;i&gt;a&lt;/i&gt;)&lt;br&gt;&lt;b&gt;r&lt;/b&gt;: Enable or disable the red channel update of the FT800 color buffer. The initial value is 1 and means enable.&lt;br&gt;&lt;b&gt;g&lt;/b&gt;: Enable or disable the green channel update of the FT800 color buffer. The initial value is 1 and means enable.&lt;br&gt;&lt;b&gt;b&lt;/b&gt;: Enable or disable the blue channel update of the FT800 color buffer. The initial value is 1 and means enable.&lt;br&gt;&lt;b&gt;a&lt;/b&gt;: Enable or disable the alpha channel update of the FT800 color buffer. The initial value is 1 and means enable.&lt;br&gt;&lt;br&gt;The color mask controls whether the color values of a pixel are updated. Sometimes it is used to selectively update only the red, green, blue or alpha channels of the image. More often, it is used to completely disable color updates while updating the tag and stencil buffers.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1706"/>
        <source>DESCRIPTION_END.</source>
        <translation type="unfinished">&lt;b&gt;END&lt;/b&gt;()&lt;br&gt;&lt;br&gt;End drawing a graphics primitive.&lt;br&gt;&lt;br&gt;It is recommended to have an END for each BEGIN. Whereas advanced users can avoid the usage of END in order to save extra graphics instructions in the display list RAM.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1716"/>
        <source>DESCRIPTION_SAVE_CONTEXT.</source>
        <translation type="unfinished">&lt;b&gt;SAVE_CONTEXT&lt;/b&gt;()&lt;br&gt;&lt;br&gt;Push the current graphics context on the context stack.&lt;br&gt;&lt;br&gt;Any extra SAVE_CONTEXT will throw away the earliest saved context.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1726"/>
        <source>DESCRIPTION_RESTORE_CONTEXT.</source>
        <translation type="unfinished">&lt;b&gt;RESTORE_CONTEXT&lt;/b&gt;()&lt;br&gt;&lt;br&gt;Restore the current graphics context from the context stack.&lt;br&gt;&lt;br&gt;Any extra RESTORE_CONTEXT will load the default values into the present context.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1736"/>
        <source>DESCRIPTION_RETURN.</source>
        <translation type="unfinished">&lt;b&gt;RETURN&lt;/b&gt;()&lt;br&gt;&lt;br&gt;Return from a previous CALL command.&lt;br&gt;&lt;br&gt;CALL and RETURN have 4 levels of stack in addition to the current pointer. Any additional CALL/RETURN done will lead to unexpected behavior.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1746"/>
        <source>DESCRIPTION_MACRO.</source>
        <translation type="unfinished">&lt;b&gt;MACRO&lt;/b&gt;(&lt;i&gt;m&lt;/i&gt;)&lt;br&gt;&lt;b&gt;m&lt;/b&gt;: Macro register to read. Value 0 means the FT800 will fetch the command from REG_MACRO_0 to execute. Value 1 means the FT800 will fetch the command from REG_MACRO_1 to execute. The content of REG_MACRO_0 or REG_MACRO_1 shall be a valid display list command, otherwise the behavior is undefined.&lt;br&gt;&lt;br&gt;Execute a single command from a macro register.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1758"/>
        <source>DESCRIPTION_CLEAR.</source>
        <translation type="unfinished">&lt;b&gt;CLEAR&lt;/b&gt;(&lt;i&gt;c&lt;/i&gt;, &lt;i&gt;s&lt;/i&gt;, &lt;i&gt;t&lt;/i&gt;)&lt;br&gt;&lt;b&gt;c&lt;/b&gt;: Clear color buffer. Setting this bit to 1 will clear the color buffer of the FT800 to the preset value. Setting this bit to 0 will maintain the color buffer of the FT800 with an unchanged value. The preset value is defined in command CLEAR_COLOR_RGB for RGB channel and CLEAR_COLOR_A for alpha channel.&lt;br&gt;&lt;b&gt;s&lt;/b&gt;: Clear stencil buffer. Setting this bit to 1 will clear the stencil buffer of the FT800 to the preset value. Setting this bit to 0 will maintain the stencil buffer of the FT800 with an unchanged value. The preset value is defined in command CLEAR_STENCIL.&lt;br&gt;&lt;b&gt;t&lt;/b&gt;: Clear tag buffer. Setting this bit to 1 will clear the tag buffer of the FT800 to the preset value. Setting this bit to 0 will maintain the tag buffer of the FT800 with an unchanged value. The preset value is defined in command CLEAR_TAG.&lt;br&gt;&lt;br&gt;Clear buffers to preset values.</translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1772"/>
        <source>DESCRIPTION_VERTEX_FORMAT.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1784"/>
        <source>DESCRIPTION_BITMAP_LAYOUT_H.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1797"/>
        <source>DESCRIPTION_BITMAP_SIZE_H.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1810"/>
        <source>DESCRIPTION_PALETTE_SOURCE.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1822"/>
        <source>DESCRIPTION_VERTEX_TRANSLATE_X.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1834"/>
        <source>DESCRIPTION_VERTEX_TRANSLATE_Y.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1846"/>
        <source>DESCRIPTION_NOP.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_properties.cpp" line="1857"/>
        <source>&lt;/i&gt;Not yet implemented.&lt;/i&gt;</source>
        <translation type="unfinished">&lt;/i&gt;Not yet implemented.&lt;/i&gt;</translation>
    </message>
</context>
<context>
    <name>FTEDITOR::InteractiveViewport</name>
    <message>
        <location filename="interactive_viewport.cpp" line="90"/>
        <location filename="interactive_viewport.cpp" line="117"/>
        <source>Cursor</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_viewport.cpp" line="92"/>
        <source>Context dependent cursor</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_viewport.cpp" line="98"/>
        <source>Touch</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_viewport.cpp" line="100"/>
        <source>Use to cursor to touch the emulated display</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_viewport.cpp" line="105"/>
        <source>Trace</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_viewport.cpp" line="107"/>
        <source>Select a pixel to trace display commands</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_viewport.cpp" line="112"/>
        <source>Edit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_viewport.cpp" line="114"/>
        <source>Interactive editing tools</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_viewport.cpp" line="129"/>
        <source>Insert</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_viewport.cpp" line="131"/>
        <source>Place a new vertex or clone the selected widget directly on the screen</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_viewport.cpp" line="148"/>
        <source>Toolbar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_viewport.cpp" line="1219"/>
        <location filename="interactive_viewport.cpp" line="1496"/>
        <source>Move vertex</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_viewport.cpp" line="1565"/>
        <source>Move widget</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_viewport.cpp" line="1964"/>
        <source>&lt;b&gt;Error&lt;/b&gt;: No free bitmap handle available</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="interactive_viewport.cpp" line="1976"/>
        <source>Drag and drop primitive</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>FTEDITOR::MainWindow</name>
    <message>
        <location filename="main_window.cpp" line="830"/>
        <source>Run Python script</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="869"/>
        <source>Executed Python script &apos;%1&apos;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1043"/>
        <source>WARNING: Missing CLEAR instruction in display list</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1107"/>
        <source>New</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1108"/>
        <source>Create a new project</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1110"/>
        <source>Open</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1111"/>
        <source>Open an existing project</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1113"/>
        <source>Save</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1114"/>
        <source>Save the current project</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1116"/>
        <source>Save As</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1117"/>
        <source>Save the current project to a new file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1119"/>
        <location filename="main_window.cpp" line="2489"/>
        <source>Import</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1120"/>
        <source>Import file to a new project</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1121"/>
        <location filename="main_window.cpp" line="2609"/>
        <location filename="main_window.cpp" line="2658"/>
        <source>Export</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1122"/>
        <source>Export project to file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1123"/>
        <source>Reset Emulator</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1124"/>
        <source>Reset the emulated device</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1125"/>
        <location filename="main_window.cpp" line="2792"/>
        <source>Save Screenshot</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1126"/>
        <source>Save a screenshot of the emulator output</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1127"/>
        <source>Quit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1128"/>
        <source>Exit the application</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1129"/>
        <source>Manual</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1130"/>
        <source>Open the manual</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1131"/>
        <source>About</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1132"/>
        <source>Show information about the application</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1133"/>
        <location filename="main_window.cpp" line="2839"/>
        <source>3rd Party</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1134"/>
        <source>Show information about the 3rd party code and content</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1137"/>
        <source>Undo</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1138"/>
        <source>Reverses the last action</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1140"/>
        <source>Redo</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1141"/>
        <source>Reapply the action</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1143"/>
        <source>Dummy</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1144"/>
        <source>Does nothing</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1192"/>
        <source>File</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1193"/>
        <source>Edit</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1194"/>
        <source>Tools</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1195"/>
        <source>View</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1197"/>
        <source>Scripts</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1199"/>
        <source>Help</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1204"/>
        <source>MainBar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1223"/>
        <source>Ready</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1255"/>
        <source>Device</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1307"/>
        <source>BITMAP_HANDLE: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1318"/>
        <source>RAM_DL: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1329"/>
        <source>RAM_G: </source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1355"/>
        <location filename="main_window.cpp" line="1691"/>
        <source>Display List</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1449"/>
        <source>Steps</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1487"/>
        <source>Trace</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1532"/>
        <source>Display Size</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1541"/>
        <source>Horizontal</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1552"/>
        <source>Vertical</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1564"/>
        <source>Macro (REG_MACRO0, REG_MACRO1)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1690"/>
        <source>Inspector</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1692"/>
        <source>Coprocessor</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1693"/>
        <source>Project</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1695"/>
        <source>Devices</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1697"/>
        <source>Utilization</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1698"/>
        <source>Navigator</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1699"/>
        <source>Properties</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1700"/>
        <source>Toolbox</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1701"/>
        <source>Content</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1702"/>
        <source>Registers</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1703"/>
        <source>Controls</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="1710"/>
        <location filename="main_window.cpp" line="2063"/>
        <location filename="main_window.cpp" line="2077"/>
        <source>FTDI EVE Screen Editor</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="2078"/>
        <source>The project has been modified.
Do you want to save your changes?</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="2246"/>
        <source>Open Project</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="2247"/>
        <source>FT800 Editor Project, *.ft800proj (*.ft800proj)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="2300"/>
        <source>Opened FTDI EVE Screen Editor project</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="2404"/>
        <source>FTDI EVE Screen Editor Project, *.ft800proj (*.ft800proj)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="2406"/>
        <source>Save Project</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="2490"/>
        <location filename="main_window.cpp" line="2607"/>
        <source>Memory dump, *.vc1dump (*.vc1dump)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="2525"/>
        <source>Invalid header version: %i</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="2526"/>
        <location filename="main_window.cpp" line="2532"/>
        <location filename="main_window.cpp" line="2553"/>
        <location filename="main_window.cpp" line="2557"/>
        <location filename="main_window.cpp" line="2564"/>
        <source>Import .vc1dump</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="2532"/>
        <source>Incomplete header</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="2553"/>
        <source>Incomplete RAM_G</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="2557"/>
        <source>Incomplete RAM_PAL</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="2564"/>
        <source>Incomplete RAM_DL</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="2581"/>
        <source>Imported project from .vc1dump file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="2600"/>
        <source>Imported project from .vc1dump file.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="2650"/>
        <source>Exported project to .vc1dump file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="2653"/>
        <source>Exported project to .vc1dump file.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="2658"/>
        <source>Failed to write file</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="2789"/>
        <source>PNG image (*.png)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="2790"/>
        <source>JPG image (*.jpg)</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="2826"/>
        <source>About FTDI EVE Screen Editor</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="2828"/>
        <source>Copyright (C) 2013-2015  Future Technology Devices International Ltd&lt;br&gt;Author: Jan Boon &amp;lt;&lt;a href=&apos;mailto:jan.boon@kaetemi.be&apos;&gt;jan.boon@kaetemi.be&lt;/a&gt;&amp;gt;&lt;br&gt;&lt;br&gt;Support and updates: &lt;a href=&apos;http://www.ftdichip.com/Support/Utilities.htm&apos;&gt;http://www.ftdichip.com/Support/Utilities.htm&lt;/a&gt;</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="main_window.cpp" line="2839"/>
        <source>The Qt GUI Toolkit is Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
Contact: http://www.qt-project.org/legal
Qt is available under the LGPL.

Portions part of the examples of the Qt Toolkit, under the BSD license.
Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
Contact: http://www.qt-project.org/legal
THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS &quot;AS IS&quot; AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

Fugue Icons
(C) 2013 Yusuke Kamiyamane. All rights reserved.
These icons are licensed under a Creative CommonsAttribution 3.0 License.
&lt;http://creativecommons.org/licenses/by/3.0/&gt;</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>FTEDITOR::PropertiesEditor</name>
    <message>
        <location filename="properties_editor.cpp" line="131"/>
        <source>Information</source>
        <translation type="unfinished"></translation>
    </message>
</context>
<context>
    <name>FTEDITOR::Toolbox</name>
    <message>
        <location filename="toolbox.cpp" line="52"/>
        <source>Background</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="57"/>
        <source>Clear</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="61"/>
        <source>Clear Color RGB</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="65"/>
        <source>Clear Color Alpha</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="69"/>
        <source>Clear Stencil</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="73"/>
        <source>Clear Tag</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="79"/>
        <source>Primitives</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="83"/>
        <source>Bitmaps</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="87"/>
        <source>Points</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="91"/>
        <source>Lines</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="95"/>
        <source>Line Strip</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="99"/>
        <source>Edge Strip R</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="103"/>
        <source>Edge Strip L</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="107"/>
        <source>Edge Strip A</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="111"/>
        <source>Edge Strip B</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="115"/>
        <source>Rects</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="121"/>
        <source>Widgets</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="125"/>
        <source>Text</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="129"/>
        <source>Button</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="133"/>
        <source>Keys</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="137"/>
        <source>Progress</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="141"/>
        <source>Slider</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="145"/>
        <source>Scrollbar</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="149"/>
        <source>Toggle</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="153"/>
        <source>Gauge</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="157"/>
        <source>Clock</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="161"/>
        <source>Dial</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="165"/>
        <source>Number</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="169"/>
        <source>Spinner</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="173"/>
        <source>Screensaver</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="177"/>
        <source>Gradient</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="183"/>
        <source>Utilities</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="188"/>
        <source>Tracker</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="192"/>
        <source>Sketch</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="196"/>
        <source>Capacitive Sketch</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="203"/>
        <source>Graphics State</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="208"/>
        <source>Color RGB</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="212"/>
        <source>Color A</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="216"/>
        <source>Color Mask</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="220"/>
        <source>Foreground Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="225"/>
        <source>Background Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="230"/>
        <source>Gradient Color</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="235"/>
        <source>Line Width</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="239"/>
        <source>Point Size</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="243"/>
        <source>Blend Func</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="247"/>
        <source>Scissor Size</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="251"/>
        <source>Scissor XY</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="255"/>
        <source>Alpha Func</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="259"/>
        <source>Stencil Func</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="263"/>
        <source>Stencil Mask</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="267"/>
        <source>Stencil Op</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="271"/>
        <source>Tag</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="275"/>
        <source>Tag Mask</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="281"/>
        <source>Bitmap State</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="286"/>
        <source>Bitmap Handle</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="290"/>
        <source>Bitmap Source</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="294"/>
        <source>Bitmap Layout</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="298"/>
        <source>Bitmap Size</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="302"/>
        <source>Transform A</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="306"/>
        <source>Transform B</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="310"/>
        <source>Transform C</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="314"/>
        <source>Transform D</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="318"/>
        <source>Transform E</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="322"/>
        <source>Transform F</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="326"/>
        <source>Matrix Load Identity</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="331"/>
        <source>Matrix Translate</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="336"/>
        <source>Matrix Scale</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="341"/>
        <source>Matrix Rotate</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="346"/>
        <source>Matrix Set Current</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="370"/>
        <source>&lt;b&gt;BITMAPS&lt;/b&gt;&lt;br&gt;Rectangular pixel arrays, in various color format.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="376"/>
        <source>&lt;b&gt;POINTS&lt;/b&gt;&lt;br&gt;Anti-aliased points, point radius is 1-256 pixels.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="382"/>
        <source>&lt;b&gt;LINES&lt;/b&gt;&lt;br&gt;Anti-aliased lines, with width of 1-256 pixels (width is from center of the line to boundary).</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="388"/>
        <source>&lt;b&gt;LINE_STRIP&lt;/b&gt;&lt;br&gt;Anti-aliased lines, connected head-to-tail.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="397"/>
        <source>&lt;b&gt;EDGE STRIP A, B, L, R&lt;/b&gt;&lt;br&gt;Edge strips.</source>
        <translation type="unfinished"></translation>
    </message>
    <message>
        <location filename="toolbox.cpp" line="403"/>
        <source>&lt;b&gt;RECTS&lt;/b&gt;&lt;br&gt;Round-cornered rectangles, curvature of the corners can be adjusted using LINE_WIDTH.</source>
        <translation type="unfinished"></translation>
    </message>
</context>
</TS>
