#!/bin/bash
# Soubor:  SGE_submit.sh
# Datum:   01.12.2012 00:03
# Autor:   xferra00

export DEMODIR="demo_testing";
export RESOLUTION="640x480";
export FRAMESPERTASK="5";

# check if there is executable of our raytracer present
[ ! -e rt ] && { echo "Raytracer executable not found!"; exit 1;}

# create outputs/ for SGE output redirection
[ ! -e outputs ] && mkdir outputs;
[ -e outputs ] && rm -f outputs/*;

# how many frames we need to render
n_frames=`ls -1 $DEMODIR/config_*.txt | wc -l`;
[ "$n_frames" -ge "$FRAMESPERTASK" ] ||
    { echo "Not enough config files detected!"; exit 1;}

# submit SGE array job to render particular frames
echo "Submitting array job ...";
qsub -t 1-$n_frames:$FRAMESPERTASK -v FRAMESPERTASK -v HOME -v DEMODIR -v RESOLUTION -N pgrdemo SGE_render.sh
echo "Done.";
