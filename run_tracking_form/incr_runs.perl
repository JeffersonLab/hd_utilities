#!/usr/bin/env perl
$run_start = $ARGV[0];
open(TEX,"checklist_run.tex");
$run = $run_start;
while($line = <TEX>) {
    if ($line =~ /runno/) {
	$line =~ s/runno/$run/;
	$run++;
    }
    print $line;
}
exit;
