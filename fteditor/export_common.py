import re

def splitCommand(command):
    p = re.search(r'\(.*?((".*?".*?\))|(\)))', command)
    return (command[0:p.start()], p[0][1:-1], command[p.end():])

def parseCommand(command, functionMap, convertArgsFunc):
    name, args, comment = splitCommand(command)    
    name = functionMap.get(name.upper(), name.upper())
    args = convertArgsFunc(args)
    m = re.match('\s*?//.*$', comment)
    comment = m[0] if m else ''
    return name, args, comment