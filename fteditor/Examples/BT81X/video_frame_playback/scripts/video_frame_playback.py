import time
import interact as itc #Mandatory import

init_cmds = '''
CLEAR(1, 1, 1)
CMD_FLASHFAST()
CMD_FLASHSOURCE(4096)
CMD_VIDEOSTARTF()
BITMAP_HANDLE(0)
BITMAP_SOURCE(100)       // specify the video frame at offset #100
BITMAP_LAYOUT_H(0, 0)
BITMAP_LAYOUT(RGB565, 704, 240)
BITMAP_SIZE_H(0,0)
BITMAP_SIZE(NEAREST, BORDER, BORDER, 352, 240)
BEGIN(BITMAPS)
VERTEX2II(99, 74, 0, 0)
END()
DISPLAY()
'''

cmds = '''
CLEAR(1, 1, 1)
CMD_VIDEOFRAME(100, 0)
BEGIN(BITMAPS)
VERTEX2II(99, 74, 0, 0)
END()
DISPLAY()
'''

def main():
    itc.init() # Mandatory
    itc.transfer(init_cmds)
    loopCount = 0
    while loopCount < 100: 
        loopCount = loopCount + 1
        time.sleep(0.03)
        # This statement checks whether the script should continue or not.
        # Therefore, make sure to add this code statement in every loop.
        if itc.transfer(cmds) < 0:
            itc.deinit()
            return

if __name__ == "__main__":
    main()