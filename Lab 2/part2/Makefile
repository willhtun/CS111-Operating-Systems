{\rtf1\ansi\deff0\nouicompat{\fonttbl{\f0\fnil\fcharset0 Calibri;}}
{\*\generator Riched20 10.0.17134}\viewkind4\uc1 
\pard\sa200\sl240\slmult1\f0\fs24\lang9 #NAME: Wai Yan (Will) Htun\par
#ID: 104941153\par
#EMAIL: willhtun42@gmail.com\par
\par
default: build\par
build:\par
\tab gcc -pthread -Wall -Wextra -o lab2_list lab2_list.c SortedList.c\par
\par
clean\par
\tab rm -rf lab2_list *.o *.csv *.png *.perf profile*\par
\par
graphs: tests\par
\tab gnuplot lab2_list.gp\par
\par
profile: clean build\par
\tab -rm -f profile.gperf \par
\tab LD_PRELOAD=/usr/lib64/libprofiler.so CPUPROFILE=profile.gperf ./lab2_list --threads=12 --iterations=1000 --sync=s\par
\tab pprof --text lab2_list profile.gperf > profile.out\par
\tab pprof --list=listWork lab2_list profile.gperf >> profile.out\par
\tab -rm -f profile.gperf\par
\par
dist: clean build graphs profile\par
\tab tar -zcvf lab2b-104941153.tar.gz *.c *.h README Makefile *.png *.csv *.out *.gp\par
\par
tests: clean build\par
\tab # Test 1, 2\par
\tab ./lab2_list --iterations=1000 --sync=m --threads=1 >> lab2b_list.csv ; \\\par
\tab ./lab2_list --iterations=1000 --sync=m --threads=2 >> lab2b_list.csv ; \\\par
\tab ./lab2_list --iterations=1000 --sync=m --threads=4 >> lab2b_list.csv ; \\\par
\tab ./lab2_list --iterations=1000 --sync=m --threads=8 >> lab2b_list.csv ; \\\par
\tab ./lab2_list --iterations=1000 --sync=m --threads=12 >> lab2b_list.csv ; \\\par
\tab ./lab2_list --iterations=1000 --sync=m --threads=16> lab2b_list.csv ; \\\par
\tab ./lab2_list --iterations=1000 --sync=m --threads=24 >> lab2b_list.csv ; \\\par
\tab ./lab2_list --iterations=1000 --sync=s --threads=1 >> lab2b_list.csv ; \\\par
\tab ./lab2_list --iterations=1000 --sync=s --threads=2 >> lab2b_list.csv ; \\\par
\tab ./lab2_list --iterations=1000 --sync=s --threads=4 >> lab2b_list.csv ; \\\par
\tab ./lab2_list --iterations=1000 --sync=s --threads=8 >> lab2b_list.csv ; \\\par
\tab ./lab2_list --iterations=1000 --sync=s --threads=12 >> lab2b_list.csv ; \\\par
\tab ./lab2_list --iterations=1000 --sync=s --threads=16> lab2b_list.csv ; \\\par
\tab ./lab2_list --iterations=1000 --sync=s --threads=24 >> lab2b_list.csv ; \\\par
\tab # Test 3\par
\tab -./lab2_list --iterations=1 --threads=1 --yield=id --lists=4 >> lab2b_list.csv ; \\\par

\pard\li720\sa200\sl240\slmult1 -./lab2_list --iterations=1 --threads=4 --yield=id --lists=4 >> lab2b_list.csv ; \\\par
-./lab2_list --iterations=1 --threads=8 --yield=id --lists=4 >> lab2b_list.csv ; \\\par
-./lab2_list --iterations=1 --threads=12 --yield=id --lists=4 >> lab2b_list.csv ; \\\par
-./lab2_list --iterations=1 --threads=16 --yield=id --lists=4 >> lab2b_list.csv ; \\\par

\pard\sa200\sl240\slmult1\tab -./lab2_list --iterations=2 --threads=1 --yield=id --lists=4 >> lab2b_list.csv ; \\\par

\pard\li720\sa200\sl240\slmult1 -./lab2_list --iterations=2--threads=4 --yield=id --lists=4 >> lab2b_list.csv ; \\\par
-./lab2_list --iterations=2 --threads=8 --yield=id --lists=4 >> lab2b_list.csv ; \\\par
-./lab2_list --iterations=2 --threads=12 --yield=id --lists=4 >> lab2b_list.csv ; \\\par
-./lab2_list --iterations=2 --threads=16 --yield=id --lists=4 >> lab2b_list.csv ; \\\par

\pard\sa200\sl240\slmult1\tab -./lab2_list --iterations=4 --threads=1 --yield=id --lists=4 >> lab2b_list.csv ; \\\par

\pard\li720\sa200\sl240\slmult1 -./lab2_list --iterations=4 --threads=4 --yield=id --lists=4 >> lab2b_list.csv ; \\\par
-./lab2_list --iterations=4 --threads=8 --yield=id --lists=4 >> lab2b_list.csv ; \\\par
-./lab2_list --iterations=4 --threads=12 --yield=id --lists=4 >> lab2b_list.csv ; \\\par
-./lab2_list --iterations=4 --threads=16 --yield=id --lists=4 >> lab2b_list.csv ; \\\par

\pard\sa200\sl240\slmult1\tab -./lab2_list --iterations=8 --threads=1 --yield=id --lists=4 >> lab2b_list.csv ; \\\par

\pard\li720\sa200\sl240\slmult1 -./lab2_list --iterations=8 --threads=4 --yield=id --lists=4 >> lab2b_list.csv ; \\\par
-./lab2_list --iterations=8 --threads=8 --yield=id --lists=4 >> lab2b_list.csv ; \\\par
-./lab2_list --iterations=8 --threads=12 --yield=id --lists=4 >> lab2b_list.csv ; \\\par
-./lab2_list --iterations=8 --threads=16 --yield=id --lists=4 >> lab2b_list.csv ; \\\par

\pard\sa200\sl240\slmult1\tab -./lab2_list --iterations=16 --threads=1 --yield=id --lists=4 >> lab2b_list.csv ; \\\par

\pard\li720\sa200\sl240\slmult1 -./lab2_list --iterations=16 --threads=4 --yield=id --lists=4 >> lab2b_list.csv ; \\\par
-./lab2_list --iterations=16 --threads=8 --yield=id --lists=4 >> lab2b_list.csv ; \\\par
-./lab2_list --iterations=16 --threads=12 --yield=id --lists=4 >> lab2b_list.csv ; \\\par
-./lab2_list --iterations=16 --threads=16 --yield=id --lists=4 >> lab2b_list.csv ; \\\par
./lab2_list --iterations=10 --threads=1 --yield=id --lists=4 --sync=s>> lab2b_list.csv ; \\\par
./lab2_list --iterations=10 --threads=4 --yield=id --lists=4 --sync=s>> lab2b_list.csv ; \\\par
./lab2_list --iterations=10 --threads=8 --yield=id --lists=4 --sync=s>> lab2b_list.csv ; \\\par
./lab2_list --iterations=10 --threads=12 --yield=id --lists=4 --sync=s>> lab2b_list.csv ; \\\par
./lab2_list --iterations=10 --threads=16 --yield=id --lists=4 --sync=s>> lab2b_list.csv ; \\\par
./lab2_list --iterations=20 --threads=1 --yield=id --lists=4 --sync=s>> lab2b_list.csv ; \\\par
./lab2_list --iterations=20 --threads=4 --yield=id --lists=4 --sync=s>> lab2b_list.csv ; \\\par
./lab2_list --iterations=20 --threads=8 --yield=id --lists=4 --sync=s>> lab2b_list.csv ; \\\par
./lab2_list --iterations=20 --threads=12 --yield=id --lists=4 --sync=s>> lab2b_list.csv ; \\\par
./lab2_list --iterations=20 --threads=16 --yield=id --lists=4 --sync=s>> lab2b_list.csv ; \\\par
./lab2_list --iterations=40 --threads=1 --yield=id --lists=4 --sync=s>> lab2b_list.csv ; \par
./lab2_list --iterations=40 --threads=4 --yield=id --lists=4 --sync=s>> lab2b_list.csv ; \\\par
./lab2_list --iterations=40 --threads=8 --yield=id --lists=4 --sync=s>> lab2b_list.csv ; \\\par
./lab2_list --iterations=40 --threads=12 --yield=id --lists=4 --sync=s>> lab2b_list.csv ; \\\par
./lab2_list --iterations=40 --threads=16 --yield=id --lists=4 --sync=s>> lab2b_list.csv ; \\\par
./lab2_list --iterations=80 --threads=1 --yield=id --lists=4 --sync=s>> lab2b_list.csv ; \\\par
./lab2_list --iterations=80 --threads=4 --yield=id --lists=4 --sync=s>> lab2b_list.csv ; \\\par
./lab2_list --iterations=80 --threads=8 --yield=id --lists=4 --sync=s>> lab2b_list.csv ; \\\par
./lab2_list --iterations=80 --threads=12 --yield=id --lists=4 --sync=s>> lab2b_list.csv ; \\\par
./lab2_list --iterations=80 --threads=16 --yield=id --lists=4 --sync=s>> lab2b_list.csv ; \\\par
./lab2_list --iterations=10 --threads=1 --yield=id --lists=4 --sync=m>> lab2b_list.csv ; \\\par
./lab2_list --iterations=10 --threads=4 --yield=id --lists=4 --sync=m>> lab2b_list.csv ; \\\par
./lab2_list --iterations=10 --threads=8 --yield=id --lists=4 --sync=m>> lab2b_list.csv ; \\\par
./lab2_list --iterations=10 --threads=12 --yield=id --lists=4 --sync=m>> lab2b_list.csv ; \\\par
./lab2_list --iterations=10 --threads=16 --yield=id --lists=4 --sync=m>> lab2b_list.csv ; \\\par
./lab2_list --iterations=20 --threads=1 --yield=id --lists=4 --sync=m>> lab2b_list.csv ; \\\par
./lab2_list --iterations=20 --threads=4 --yield=id --lists=4 --sync=m>> lab2b_list.csv ; \\\par
./lab2_list --iterations=20 --threads=8 --yield=id --lists=4 --sync=m>> lab2b_list.csv ; \\\par
./lab2_list --iterations=20 --threads=12 --yield=id --lists=4 --sync=m>> lab2b_list.csv ; \\\par
./lab2_list --iterations=20 --threads=16 --yield=id --lists=4 --sync=m>> lab2b_list.csv ; \\\par
./lab2_list --iterations=40 --threads=1 --yield=id --lists=4 --sync=m>> lab2b_list.csv ; \par
./lab2_list --iterations=40 --threads=4 --yield=id --lists=4 --sync=m>> lab2b_list.csv ; \\\par
./lab2_list --iterations=40 --threads=8 --yield=id --lists=4 --sync=m>> lab2b_list.csv ; \\\par
./lab2_list --iterations=40 --threads=12 --yield=id --lists=4 --sync=m>> lab2b_list.csv ; \\\par
./lab2_list --iterations=40 --threads=16 --yield=id --lists=4 --sync=m>> lab2b_list.csv ; \\\par
./lab2_list --iterations=80 --threads=1 --yield=id --lists=4 --sync=m>> lab2b_list.csv ; \\\par
./lab2_list --iterations=80 --threads=4 --yield=id --lists=4 --sync=m>> lab2b_list.csv ; \\\par
./lab2_list --iterations=80 --threads=8 --yield=id --lists=4 --sync=m>> lab2b_list.csv ; \\\par
./lab2_list --iterations=80 --threads=12 --yield=id --lists=4 --sync=m>> lab2b_list.csv ; \\\par
./lab2_list --iterations=80 --threads=16 --yield=id --lists=4 --sync=m>> lab2b_list.csv ; \\\par
# Test 4\par
./lab2_list --sync=m --iterations=1000 --threads=1 --lists=1 >> lab2b_list.csv ; \\\par
./lab2_list --sync=m --iterations=1000 --threads=1 --lists=4 >> lab2b_list.csv ; \\\par
./lab2_list --sync=m --iterations=1000 --threads=1 --lists=8 >> lab2b_list.csv ; \\\par
./lab2_list --sync=m --iterations=1000 --threads=1 --lists=16 >> lab2b_list.csv ; \\\par
./lab2_list --sync=m --iterations=1000 --threads=2 --lists=1 >> lab2b_list.csv ; \\\par
./lab2_list --sync=m --iterations=1000 --threads=2 --lists=4 >> lab2b_list.csv ; \\\par
./lab2_list --sync=m --iterations=1000 --threads=2 --lists=8 >> lab2b_list.csv ; \\\par
./lab2_list --sync=m --iterations=1000 --threads=2 --lists=16 >> lab2b_list.csv ; \\\par
./lab2_list --sync=m --iterations=1000 --threads=4 --lists=1 >> lab2b_list.csv ; \\\par
./lab2_list --sync=m --iterations=1000 --threads=4 --lists=4 >> lab2b_list.csv ; \\\par
./lab2_list --sync=m --iterations=1000 --threads=4 --lists=8 >> lab2b_list.csv ; \\\par
./lab2_list --sync=m --iterations=1000 --threads=4 --lists=16 >> lab2b_list.csv ; \\\par
./lab2_list --sync=m --iterations=1000 --threads=8 --lists=1 >> lab2b_list.csv ; \\\par
./lab2_list --sync=m --iterations=1000 --threads=8 --lists=4 >> lab2b_list.csv ; \\\par
./lab2_list --sync=m --iterations=1000 --threads=8 --lists=8 >> lab2b_list.csv ; \\\par
./lab2_list --sync=m --iterations=1000 --threads=8 --lists=16 >> lab2b_list.csv ; \\\par
./lab2_list --sync=m --iterations=1000 --threads=12 --lists=1 >> lab2b_list.csv ; \\\par
./lab2_list --sync=m --iterations=1000 --threads=12 --lists=4 >> lab2b_list.csv ; \\\par
./lab2_list --sync=m --iterations=1000 --threads=12 --lists=8 >> lab2b_list.csv ; \\\par
./lab2_list --sync=m --iterations=1000 --threads=12 --lists=16 >> lab2b_list.csv ; \\\par
...\par
./lab2_list --sync=s --iterations=1000 --threads=1 --lists=1 >> lab2b_list.csv ; \\\par
./lab2_list --sync=s --iterations=1000 --threads=1 --lists=4 >> lab2b_list.csv ; \\\par
./lab2_list --sync=s --iterations=1000 --threads=1 --lists=8 >> lab2b_list.csv ; \\\par
./lab2_list --sync=s --iterations=1000 --threads=1 --lists=16 >> lab2b_list.csv ; \\\par
./lab2_list --sync=s --iterations=1000 --threads=2 --lists=1 >> lab2b_list.csv ; \\\par
./lab2_list --sync=s --iterations=1000 --threads=2 --lists=4 >> lab2b_list.csv ; \\\par
./lab2_list --sync=s --iterations=1000 --threads=2 --lists=8 >> lab2b_list.csv ; \\\par
./lab2_list --sync=s --iterations=1000 --threads=2 --lists=16 >> lab2b_list.csv ; \\\par
./lab2_list --sync=s --iterations=1000 --threads=4 --lists=1 >> lab2b_list.csv ; \\\par
./lab2_list --sync=s --iterations=1000 --threads=4 --lists=4 >> lab2b_list.csv ; \\\par
./lab2_list --sync=s --iterations=1000 --threads=4 --lists=8 >> lab2b_list.csv ; \\\par
./lab2_list --sync=s --iterations=1000 --threads=4 --lists=16 >> lab2b_list.csv ; \\\par
./lab2_list --sync=s --iterations=1000 --threads=8 --lists=1 >> lab2b_list.csv ; \\\par
./lab2_list --sync=s --iterations=1000 --threads=8 --lists=4 >> lab2b_list.csv ; \\\par
./lab2_list --sync=s --iterations=1000 --threads=8 --lists=8 >> lab2b_list.csv ; \\\par
./lab2_list --sync=s --iterations=1000 --threads=8 --lists=16 >> lab2b_list.csv ; \\\par
./lab2_list --sync=s --iterations=1000 --threads=12 --lists=1 >> lab2b_list.csv ; \\\par
./lab2_list --sync=s --iterations=1000 --threads=12 --lists=4 >> lab2b_list.csv ; \\\par
./lab2_list --sync=s --iterations=1000 --threads=12 --lists=8 >> lab2b_list.csv ; \\\par
./lab2_list --sync=s --iterations=1000 --threads=12 --lists=16 >> lab2b_list.csv ; \\\par
}
 