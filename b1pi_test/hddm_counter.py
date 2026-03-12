import hddm_s
import sys

if len(sys.argv) < 2:
    print("Usage: python hddm_counter.py <filename>")
    sys.exit(1)

filename = sys.argv[1]

i=0
for rec in hddm_s.istream(filename):
    i=i+1
print(i)
