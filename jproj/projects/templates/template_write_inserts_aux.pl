#!/usr/bin/env perl

$tablename = "PROJECT_aux";

while (<>) {
    chomp;
    @t0 = split;
    $jobId = $t0[0];
    $nevents = $t0[3];
    $timeCopy = $t0[4];
    $timePlugin = $t0[5];
    $segfault   = $t0[6];
    if ($jobId > 100000000 || $jobId < 10000000) {die "bad job id"}
    print "UPDATE $tablename SET nevents = $nevents, timeCopy = $timeCopy, timePlugin = $timePlugin, segfault = $segfault WHERE jobId = $jobId;\n";
}
exit;

