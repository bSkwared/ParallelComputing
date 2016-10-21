#!/bin/bash

HOSTS="hosts"

PAR_EXE="pipar"
SIM_EXE="simpi"

PAR_OUT="dump_$(date +%H-%M_%m-%d)_par"
SIM_OUT="dump_$(date +%H-%M_%m-%d)_sim"

>$PAR_OUT
>$SIM_OUT

NUMPROCS=1
NUMPARAM=1


for i in $(seq 1 10); do
    POWTEN=10
    OFFSET=20
    
    for j in $(seq 1 10); do
        let POWTEN=$POWTEN*$POWTEN
        for k in $(seq 0 4); do
            let OFFSET=$POWTEN*2*$k
            for l in $(seq 1 $NUMPROCS); do
                let N=$POWTEN+$OFFSET
                
                TMP=$(mpiexec -f hosts -n $l $SIM_EXE $N)
                printf "%s\n" $TMP #>> $SIM_OUT
                sleep 5m
                TMP=$(mpiexec -f hosts -n $l $PAR_EXE $N)
                printf "%s\n" $TMP #>> $PAR_OUT
                sleep 10s
            done
        done
    done
done
