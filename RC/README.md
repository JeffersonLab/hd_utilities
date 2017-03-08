Scripts to help with Run Coordinator duties

- analyze_BTM.py - script to summarize BTA accounting information from HDLOG entries
  - LabTools-0.2.5.tar.gz - required package
  - BTM_28_2_17.data.txt.example - example input file

- count_events.py - simple example RCDB script (doesn't currently work?)
- bta_rep_acc.py - simple script to summarize BTA data from accelerator log
- bta_rep.py - simple script to summarize BTA data from HDLOG
  Two run modes:
    - "bta_rep.py -summary" prints a summary table for all existing days
    - "bta_rep.py N" prints a summary table for the RC notes wiki page for the N'th day

- plot_rcdb3_WB.py - Script for plotting triggers as a function of time and other RCDB quantities