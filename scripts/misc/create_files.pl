#!/usr/bin/env perl
$run = 9102;
$i_start = 701;
$i_end = 1000;
for ($i = $i_start; $i <= $i_end; $i++) {
    $command = "touch run_${run}_file_${i}.dat";
    print $command, "\n";
}
exit;
