#!/bin/bash
# Datum:   30.11.2012 02:38
# Autor:   xferra00

# resize every frame 0.5x
for f in `ls $1/config_*.txt.bmp`;do
    echo "resizing $f to M_$f"
    convert -geometry 50% $f $1/M_`basename $f`
done
