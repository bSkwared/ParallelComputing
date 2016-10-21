#!/bin/bash
sleep 5h
HOSTS="hosts"

PAR_EXE="pipar"
SIM_EXE="pisim"

PAR_OUT="dump_$(date +%H-%M_%m-%d)_par"
SIM_OUT="dump_$(date +%H-%M_%m-%d)_sim"

>$PAR_OUT
>$SIM_OUT

NUMPROCS=12
NUMPARAM=1
for i in $(seq 1 10); do
    POWTEN=10
    OFFSET=20
    echo "$i"
    
    for j in $(seq 1 8); do
#    for j in $(seq 1 5); do
        echo "  $j"
        for k in $(seq 1 5); do
            echo "    $k"
            let OFFSET=$POWTEN*2*$k
            for l in $(echo 1 2 4 6 8 10 12); do
                let N=$OFFSET
                TMP=$(mpiexec -f hosts -n $l $SIM_EXE $N)
                printf "%s\n" $TMP >> $SIM_OUT

                TMP=$(mpiexec -f hosts -n $l $PAR_EXE $N)
                printf "%s\n" $TMP >> $PAR_OUT
                #sleep 1s
            done
        done
        let POWTEN=$POWTEN*10
    done
done
