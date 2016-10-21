#!/bin/bash

COUNTER=10
ITERVALUE=100000000
STUFF="start:"
LOWER=1
UPPER=2000000000
CURPREC=1
LASTPREC=$(mpiexec -f hosts -n 1 par_pi $LOWER | cut -d, -f 3)
UPREC=1

while [ $ITERVALUE -gt 0 ]; do
    CURPREC=$(mpiexec -f hosts -n 1 par_pi $LOWER | cut -d, -f 3)
    printf "cur: %s      lower: %s lastprec: %s\n" $CURPREC $LOWER $LASTPREC

    if [ $(echo $CURPREC'>'$LASTPREC | bc -l) -gt 0 ]; then
        LOWER=$(expr $LOWER - $ITERVALUE)
        ITERVALUE=$(expr $ITERVALUE / 10)
        printf "inside if\n"
    else
        LOWER=$(expr $LOWER + $ITERVALUE)
        LASTPREC=$CURPREC
        printf "inside else\n"
    fi
    
done



COUNTER=10
ITERVALUE=100000000
STUFF="start:"
LOWER=1
UPPER=2000000000
CURPREC=1
LASTPREC=$(mpiexec -f hosts -n 1 simpson_pi $LOWER | cut -d, -f 3)
UPREC=1

while [ $ITERVALUE -gt 0 ]; do
    CURPREC=$(mpiexec -f hosts -n 1 simpson_pi $LOWER | cut -d, -f 3)
    printf "cur: %s      lower: %s lastprec: %s\n" $CURPREC $LOWER $LASTPREC

    if [ $(echo $CURPREC'>'$LASTPREC | bc -l) -gt 0 ]; then
        LOWER=$(expr $LOWER - $ITERVALUE)
        ITERVALUE=$(expr $ITERVALUE / 10)
        printf "inside if\n"
    else
        LOWER=$(expr $LOWER + $ITERVALUE)
        LASTPREC=$CURPREC
        printf "inside else\n"
    fi
    
done







echo $LOWER





sleep 5m
    echo The counter is $COUNTER
    STUFF=$STUFF$(mpiexec -f hosts -n 1 par_pi $COUNTER >> testfile.txt)
    let COUNTER=COUNTER*10
echo hey
echo hey
echo hey
echo hey
printf "\n\n%s\n\n" $STUFF
