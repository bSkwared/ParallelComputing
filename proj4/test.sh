mpicc matrix1.c -o m1 -lm
mpicc matrix2.c -o m2 -lm
mpicc matrix3.c -o m3 -lm

#//./makemat 4 4
sleep 5s

mpiexec -f hosts -n 1 m1 matrix.in > 1.ot
mpiexec -f hosts -n 1 m2 matrix.in > 2.ot
mpiexec -f hosts -n 1 m3 matrix.in > 3.ot
mpiexec -f hosts -n 4 m3 matrix.in > 4.ot


diff 1.ot 2.ot
diff 1.ot 3.ot
diff 1.ot 4.ot
#diff 1.ot 8.ot

rm -f 2.ot 3.ot 4.ot 
rm -f m1 m2 m3
#8.ot
