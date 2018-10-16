#NAME: Wai Yan (Will) Htun
#104941153
#EMAIL: willhtun42@gmail.com
#
#! /usr/bin/gnuplot
#
# purpose:
#	 generate data reduction graphs for the multi-threaded list project
#
# input: lab2_list.csv
#	1. test name
#	2. # threads
#	3. # iterations per thread
#	4. # lists
#	5. # operations performed (threads x iterations x (ins + lookup + delete))
#	6. run time (ns)
#	7. run time per operation (ns)
#	8. average lock wait time
#
# output:
#	lab2_list-1.png ... cost per operation vs threads and iterations
#	lab2_list-2.png ... threads and iterations that run (un-protected) w/o failure
#	lab2_list-3.png ... threads and iterations that run (protected) w/o failure
#	lab2_list-4.png ... cost per operation vs number of threads
#
# Note:
#	Managing data is simplified by keeping all of the results in a single
#	file.  But this means that the individual graphing commands have to
#	grep to select only the data they want.
#
#	Early in your implementation, you will not have data for all of the
#	tests, and the later sections may generate errors for missing data.
#

# general plot parameters
set terminal png
set datafile separator ","

# how many threads/iterations we can run without failure (w/o yielding)
set title "List-1: Throughput For Each Sync Method"
set xlabel "Number of Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput (operations per ns)"
set logscale y 10
set output 'lab2b_1.png'

# grep out only single threaded, un-protected, non-yield results
plot \
     "< grep 'list-none-m,[0-9]\\+,1000,1,' lab2_list.csv" \
	using ($2):(1000000000/($7)) \
	title 'list op with mutex' with linespoints lc rgb 'red', \
     "< grep 'list-none-s,[0-9]\\+,1000,1,' lab2_list.csv" \
	using ($2):(1000000000/($7)) \
	title 'list op with spin' with linespoints lc rgb 'green'


set title "List-2: Average Time Per Operation For Each Thread With Mutex Lock"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Average Time Per Operation (ns)"
set logscale y 10
set output 'lab2_2.png'
# note that unsuccessful runs should have produced no output
plot \
     "< grep 'list-none-m,[0-9]\\+,1000,1,' lab2b_list.csv" \
        using ($2):($7) \
        title 'operation time' with linespoints lc rgb 'red', \
     "< grep 'list-none-m,[0-9]\\+,1000,1,' lab2b_list.csv" \
        using ($2):($8) \
        title 'lock wait time' with linespoints lc rgb 'green'    
 
set title "List-3: Protected Iterations that run without failure"
unset logscale x
set xrange [0:5]
set xlabel "Yields"
set xtics("" 0, "yield=i" 1, "yield=d" 2, "yield=il" 3, "yield=dl" 4, "" 5)
set ylabel "successful iterations"
set logscale y 10
set output 'lab2_list-3.png'
plot \
    "< grep 'list-i-none,12,' lab2_list.csv" using (1):($3) \
	with points lc rgb "red" title "unprotected, T=12", \
    "< grep 'list-d-none,12,' lab2_list.csv" using (2):($3) \
	with points lc rgb "red" title "", \
    "< grep 'list-il-none,12,' lab2_list.csv" using (3):($3) \
	with points lc rgb "red" title "", \
    "< grep 'list-dl-none,12,' lab2_list.csv" using (4):($3) \
	with points lc rgb "red" title "", \
    "< grep 'list-i-m,12,' lab2_list.csv" using (1):($3) \
	with points lc rgb "green" title "Mutex, T=12", \
    "< grep 'list-d-m,12,' lab2_list.csv" using (2):($3) \
	with points lc rgb "green" title "", \
    "< grep 'list-il-m,12,' lab2_list.csv" using (3):($3) \
	with points lc rgb "green" title "", \
    "< grep 'list-dl-m,12,' lab2_list.csv" using (4):($3) \
	with points lc rgb "green" title "", \
    "< grep 'list-i-s,12,' lab2_list.csv" using (1):($3) \
	with points lc rgb "blue" title "Spin-Lock, T=12", \
    "< grep 'list-d-s,12,' lab2_list.csv" using (2):($3) \
	with points lc rgb "blue" title "", \
    "< grep 'list-il-s,12,' lab2_list.csv" using (3):($3) \
	with points lc rgb "blue" title "", \
    "< grep 'list-dl-s,12,' lab2_list.csv" using (4):($3) \
	with points lc rgb "blue" title ""
#
# "no valid points" is possible if even a single iteration can't run
#

# unset the kinky x axis
unset xtics
set xtics

set title "List-4: Scalability of synchronization mechanisms"
set xlabel "Threads"
set logscale x 2
unset xrange
set xrange [0.75:]
set ylabel "Length-adjusted cost per operation(ns)"
set logscale y
set output 'lab2_list-4.png'
set key left top
plot \
     "< grep -e 'list-none-m,[0-9]*,1000,' lab2_list.csv" using ($2):($7)/(($3)/4) \
	title '(adjusted) list w/mutex' with linespoints lc rgb 'blue', \
     "< grep -e 'list-none-s,[0-9]*,1000,' lab2_list.csv" using ($2):($7)/(($3)/4) \
	title '(adjusted) list w/spin-lock' with linespoints lc rgb 'green'
