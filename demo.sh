#! /usr/bin/env bash

make

echo TIME_MEASUREMENT
sudo ./main < OS_PJ1_Test/TIME_MEASUREMENT.txt
sudo dmesg -c | grep Project1

echo FIFO_1
sudo ./main < OS_PJ1_Test/FIFO_1.txt
sudo dmesg -c | grep Project1

echo PSJF_2
sudo ./main < OS_PJ1_Test/PSJF_2.txt
sudo dmesg -c | grep Project1

echo RR_3
sudo ./main < OS_PJ1_Test/RR_3.txt
sudo dmesg -c | grep Project1

echo SJF_4
sudo ./main < OS_PJ1_Test/SJF_4.txt
sudo dmesg -c | grep Project1