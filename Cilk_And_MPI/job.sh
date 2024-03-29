#!/bin/bash

#$ -V
#$ -cwd
#$ -q development
#$ -pe 8way 48
#$ -N distributedStencilMPI+CILK
#$ -o output-$JOB_NAME
#$ -e error-$JOB_NAME
#$ -M jaisong87@gmail.com
#$ -m be
#$ -l h_rt=01:00:00

export PATH=$PATH:$HOME/cilk/bin
export LD_LIBRARY_PATH=$LD_LIBRARY_PATH:$HOME/cilk/lib
ibrun ./distributedStencil < "../stencil0.in" 
