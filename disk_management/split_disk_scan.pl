#!/usr/bin/env perl
$nscripts = 7;
foreach (0 .. $nscripts - 1) {
    print "$_\n";
    open($OUTPUT[$_], ">dir_list_${_}.txt");
}
open (FIND, "find /home/marki/ -mindepth 2 -maxdepth 2 -type d |");
while (<FIND>) {
    chomp;
    $dir = $_;
    $iscript = int(rand($nscripts));
    print {$OUTPUT[$iscript]} "$_\n";
}
foreach (0 .. $nscripts - 1) {
    print "$_\n";
    close($OUTPUT[$_]);
}
exit;

