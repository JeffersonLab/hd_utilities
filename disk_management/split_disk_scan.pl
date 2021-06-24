#!/usr/bin/env perl
$nscripts = 7;
foreach (0 .. $nscripts - 1) {
    print "$_\n";
    open($OUTPUT[$_], ">disk_script_${_}.sh");
}
open (FIND, "find /home/marki -mindepth 3 -type d |");
while (<FIND>) {
    chomp;
    $dir = $_;
    $iscript = int(rand($nscripts));
    print {$OUTPUT[$iscript]} "$iscript $_\n";
}
foreach (0 .. $nscripts - 1) {
    print "$_\n";
    close($OUTPUT[$_]);
}
exit;

