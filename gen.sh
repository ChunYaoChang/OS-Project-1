#! /usr/bin/env bash

dir_input="OS_PJ1_Test"
dir_output="output"

policy=('FIFO' 'PSJF' 'RR' 'SJF')

make
for i in "${policy[@]}"; do
	for j in {1..5}; do
    	sudo dmesg -c > /dev/null
		sudo ./main < ${dir_input}/${i}_${j}.txt > ${dir_output}/${i}_${j}_stdout.txt
		dmesg | grep Project1 > ${dir_output}/${i}_${j}_dmesg.txt
		echo "${i}_${j} is done"
	done
done

sudo dmesg -c > /dev/null
sudo ./main < ${dir_input}/TIME_MEASUREMENT.txt > ${dir_output}/TIME_MEASUREMENT_stdout.txt
dmesg | grep Project1 > ${dir_output}/TIME_MEASUREMENT_dmesg.txt
echo "TIME_MEASUREMENT is done"