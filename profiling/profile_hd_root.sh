#!/bin/bash
perf record -g hd_root -PEVENTS_TO_KEEP=1000 -PPLUGINS=danarest /data/gluex/rawdata/hd_rawdata_030300_001.evio
perf report --call-graph fractal --stdio > hd_root_call_graph.txt
