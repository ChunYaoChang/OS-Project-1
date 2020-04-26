import os
mode = ['FIFO', 'PSJF', 'RR', 'SJF']
input_folder = "OS_PJ1_Test/"
output_folder = "output/"

os.system("gcc main.c -o main")

for i in range(len(mode)):
    for j in range(1, 6):
        os.system("sudo dmesg clean")
        os.system(f'./main < {input_folder}{mode[i]}_{j}.txt > {output_folder}{mode[i]}_{j}_stdout.txt')
        os.system(f'dmesg | grep Project1 > {output_folder}{mode[i]}_{j}_dmesg.txt')

os.system("sudo dmesg clean")
os.system(f'./main < {input_folder}TIME_MEASUREMENT.txt > {output_folder}TIME_MEASUREMENT_stdout.txt')
os.system(f'dmesg | grep Project1 > {output_folder}TIME_MEASUREMENT_dmesg.txt')