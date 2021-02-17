#! /usr/bin/gnuplot
#NAME: Khoa Quach
#EMAIL: khoaquachschool@gmail.com
#ID: 105123806
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

# lab2b_1.png
set title "Throughput (synchronized lists) vs # of threads"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Throughput (operations/sec)"
set logscale y 10
set output 'lab2b_1.png'

# grep out only single threaded, un-protected, non-yield results
plot \
    "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Mutex' with linespoints lc rgb 'red', \
    "< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title 'Spin Lock' with linespoints lc rgb 'green'

# lab2b_2.png
set title "Wait for lock time. Avg. Operation time vs threads"
set xlabel "Threads"
set logscale x 2
set xrange [0.75:]
set ylabel "Mean Time/Operation(ns)"
set logscale y 10
set output 'lab2b_2.png'

plot \
    "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($7) \
	title 'Wait for lock' with linespoints lc rgb 'green', \
	"< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($8) \
	title 'Average Time Per Operation' with linespoints lc rgb 'red'


# lab2b_3.png     
set title "Correct Synchronization of Partitioned Lists"
set logscale x 2
set xrange [0.75:]
set xlabel "Threads"
set ylabel "Successful Iterations"
set logscale y 10
set output 'lab2b_3.png'
plot \
	"< grep 'list-id-none,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'Unprotected' with points lc rgb 'green', \
	"< grep 'list-id-s,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'Spin Lock' with points lc rgb 'red', \
	"< grep 'list-id-m,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
	title 'Mutex' with points lc rgb 'blue'

# lab2b_4.png   
set title "Mutex Syncrhonized Partitioned Lists: Throughput vs # of threads"
set logscale x 2
set xrange [0.75:]
set xlabel "Threads"
set ylabel "Throughput (operations/sec)"
set logscale y 10
set output 'lab2b_4.png'


plot \
	"< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '1 List' with linespoints lc rgb 'green', \
	"< grep 'list-none-m,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '4 List' with linespoints lc rgb 'red', \
	"< grep 'list-none-m,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '8 List' with linespoints lc rgb 'blue', \
	"< grep 'list-none-m,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '16 List' with linespoints lc rgb 'yellow'

# lab2b_5.png

set title "Spin Lock Syncrhonized Partitioned Lists:Throughput vs # of threads"
set logscale x 2
set xrange [0.75:]
set xlabel "Threads"
set ylabel "Throughput (operations/sec)"
set logscale y 10
set output 'lab2b_5.png'

plot \
	"< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '1 List' with linespoints lc rgb 'green', \
	"< grep 'list-none-s,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '4 List' with linespoints lc rgb 'red', \
	"< grep 'list-none-s,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '8 List' with linespoints lc rgb 'blue', \
	"< grep 'list-none-s,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
	title '16 List' with linespoints lc rgb 'yellow'