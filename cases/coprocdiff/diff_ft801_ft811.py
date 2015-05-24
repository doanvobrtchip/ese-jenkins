
#compare -metric AE actual/party_1.png expected/party_1.png diff/compare_1.png

dira = "ft801"
dirb = "ft811"
dirdiff = "diff_ft801_ft811"
imagemagick = "C:/Program Files/ImageMagick-6.8.9-Q16"
diff = "C:/Program Files (x86)/Git/bin"

import os
import subprocess
# subprocess

for subdir, dirs, files in os.walk(dira):
	for file in files:
		subdira = subdir
		subdirb = subdir.replace(dira, dirb)
		subdirdiff = subdir.replace(dira, dirdiff)
		if file.endswith(".png"):
			print file
			subprocess.call([ imagemagick + "/compare.exe", "-metric", "AE", os.path.join(subdira, file), os.path.join(subdirb, file), os.path.join(subdirdiff, file) ])
		if file.endswith(".txt"):
			print file
			f = open(os.path.join(subdirdiff, file), "w")
			subprocess.call([ diff + "/diff.exe", os.path.join(subdira, file), os.path.join(subdirb, file) ], stdout=f)
