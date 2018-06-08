#!/usr/bin/env perl
$file = $ARGV[0];
$tag = $ARGV[1];
$string = "<" . $tag . " ";
$tag_count = `hddm-xml $file | grep \'$string\' | wc -l`;
chomp $tag_count;
print $tag_count, "\n";
exit;
