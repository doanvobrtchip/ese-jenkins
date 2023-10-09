import time
import interact as itc #Mandatory import

cmds = '''
CLEAR(1, 1, 1)
CMD_TOGGLE({0}, 296, 40, 27, 0, 0, "on\\xFFoff")
CMD_BUTTON(79, {1}, 120, 36, 27, 0, "Button")
CMD_DIAL(477, 246, 36, 0, {2})
'''

def main():
    itc.init() # Mandatory
    result = ''
    x = 0
    y = 0
    dialValue = 0 
    while True: 
        y = y + 2 if y <480 else 0
        x = x + 1 if x < 800 else 0
        dialValue = dialValue + 256 if dialValue < 65535 else 0
        result = cmds.format(x, y, dialValue)
        time.sleep(0.002)
        # This statement checks whether the script should continue or not.
        # Therefore, make sure to add this code statement in every loop.
        if itc.transfer(result) < 0:
            itc.deinit()
            return
            
if __name__ == "__main__":
    main()