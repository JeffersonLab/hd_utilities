#!/bin/bash
perf record -g hd_root -PEVENTS_TO_KEEP=1000 -PPLUGINS=danarest /cache/halld/RunPeriod-2017-01/rawdata/Run030300/hd_rawdata_030300_000.evio
perf report --call-graph fractal --stdio > hd_root_call_graph.txt
