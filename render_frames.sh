#!/bin/bash
# Soubor:  render_frames.sh
# Datum:   22.11.2012 16:11
# Autor:   xferra00

N_CORES=`cat /proc/cpuinfo | grep processor | wc -l`
echo "Detected $N_CORES cores for rendering.";
RES=128x72

[ "$#" == "1" ] || exit 1;

# if $CNT is even, run task in background - a dirty way, how to utilize two
# computing cores


CNT=1; 
for config in `ls $1/config_*.txt`; do
    if [ "$(($CNT % $N_CORES ))" = "0" ]; then
        ./rt -r $RES -c $config -o $1/`basename $config`.bmp;
    else
        ./rt -r $RES -c $config -o $1/`basename $config`.bmp &
    fi

    CNT=$(( $CNT + 1 ));
done
