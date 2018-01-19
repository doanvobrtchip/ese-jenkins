import Image
import sys

(w, h) = (480, 272)
f = open(sys.argv[1], "rb")
frame = 0
while True:
    fn = "%04d.png" % frame
    print fn
    bgra = f.read(4 * w * h)
    if not bgra:
        break
    (b,g,r,a) = [Image.fromstring("L", (w, h), bgra[i::4]) for i in range(4)]
    Image.merge("RGB", (r,g,b)).save(fn)
    frame += 1
