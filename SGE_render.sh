#!/bin/bash
#$ -S /bin/bash
#$ -l ram_free=100M,mem_free=100M
#$ -q all.q@@PCNxxx,all.q@@PCOxxx
#$ -N pgrdemo
#$ -o $HOME/src/PGR02/outputs/frame.$TASK_ID.out
#$ -j yes

echo "SGE_TASK_ID=$SGE_TASK_ID started"
echo "FRAMESPERTASK=$FRAMESPERTASK "
echo `seq $SGE_TASK_ID $(( $SGE_TASK_ID + $FRAMESPERTASK - 1))`

# rendering shouldnt take more than 1 hour
ulimit -t 3600

# try to cd into our project root
WORKDIR=$HOME/src/PGR02
cd $WORKDIR ||
    { echo "Cannot cd into WORKDIR ($WORKDIR)"; exit 1; }


for id in `seq $SGE_TASK_ID $(( $SGE_TASK_ID + $FRAMESPERTASK - 1 ))`;do
    config_fname="config_`printf '%05d' $id`.txt"
    echo "$id/$FRAMESPERTASK - Rendering $config_fname ...";

    # check if there is frame config file available for this $id
    [ -f $DEMODIR/$config_fname ] || { 
    echo "Config file $config_fname does not exist!"; continue; }

    # is there is already output, quit. This should not happen!
    [ -f $DEMODIR/$config_fname.bmp ] && {
    echo "$DEMODIR/$config_fname.bmp: file already present!"; continue; }

    # render the fingerprint of God
    ./rt -r $RESOLUTION -c $WORKDIR/$DEMODIR/$config_fname -o $WORKDIR/$DEMODIR/$config_fname.bmp
    echo "Rendering done (exit status $?)";
done
