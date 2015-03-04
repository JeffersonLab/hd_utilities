#!/usr/bin/env perl

$tablename = "offline_monitoring_RunPeriod2014_10_ver11_hd_rawdata_aux";

while (<>) {
    chomp;
    @t0 = split;
    $jobId = $t0[0];
    $nevents = $t0[3];
    $timeCopy = $t0[4];
    $timePlugin = $t0[5];
    if ($jobId > 100000000 || $jobId < 10000000) {die "bad job id"}
    print "INSERT INTO $tablename SET jobId = $jobId, nevents = $nevents, timeCopy = $timeCopy, timePlugin = $timePlugin;\n";
}
exit;

