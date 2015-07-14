#!/usr/bin/env perl

$tablename = "cross_analysis_table_2015_03";

while (<>) {
    chomp;
    @t0 = split;
    $run = $t0[0];
    $file = $t0[1];
    print "INSERT INTO $tablename SET run = $run, file = $file;\n";
}
exit;

