rm -r list-queue.txt
rm -r list-jlab-queue.txt
squeue > list-queue.txt
squeue -u jlab > list-jlab-queue.txt
