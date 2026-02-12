unix_time=$(date +%s)
co1=$(squeue | wc -l)
co2=$(squeue -u jlab | wc -l)
source make_list.sh
file="list-queue.txt"
co3=$(awk '{sum += $(NF-1)} END {print sum}' "$file")
file="list-jlab-queue.txt"
co4=$(awk '{sum += $(NF-1)} END {print sum}' "$file")
co5=$(squeue | grep "  R" | wc -l)
mkdir -p txt
echo "$unix_time $co1 $co2 $co3 $co4" >> txt/file.txt
echo "$unix_time $co1 $co2 $co3 $co4 $co5" >> txt/file_co.txt
