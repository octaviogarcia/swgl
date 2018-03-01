#!/usr/bin/python3.5

from pathlib import *
from subprocess import call

p = Path('.')


cfiles = []
for f in p.iterdir():
 if f.suffix == '.c' and not f.is_dir():
  cfiles.append(f)


command = ["gcc","-Og","-g","-o"]
mid_flag = ["-c"]


ofiles = []

#compiling
for cfile in cfiles:
 ofile = cfile.with_suffix(".o")
 ofiles.append(ofile)
 call([*command,ofile.name,*mid_flag,cfile.name])

#linking

linking_flags = ["-lm","-lX11"]

ofiles_str = [x.name for x in ofiles]
call([*command,"main",*ofiles_str,*linking_flags])
