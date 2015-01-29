
s = """
========== ============= ===========
module     flash (bytes) RAM (bytes)
========== ============= ===========
a          45                11
========== ============= ===========
"""

import subprocess
def sizes(module):
    output = subprocess.check_output(["v3-elf-size", "../%s.o" % module])
    lines = output.split('\n')
    (text, data, bss) = [int(f) for f in lines[1].split()[:3]]
    return (text, data + bss)

op = open("sizes.rst", "w")
sep = ('=' * 13) + ' ' + ('=' * 13) + ' ' + ('=' * 13)
print >>op, sep
print >>op, '%-13s %-13s %-13s' % ('module', 'Flash (bytes)', 'RAM (bytes)')
print >>op, sep
for m in "pads gpio timer i2cm spim dcap uart sdhost interrupt pm".split():
    (c, d) = sizes(m)
    print >>op, '%-13s %-13s %-13s' % (m, c, d)
print >>op, sep
