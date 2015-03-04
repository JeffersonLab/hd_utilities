#!/usr/bin/env perl
$project = 'dc_01';
for ($file_number = 10001; $file_number <= 50000; $file_number++) {
    print "INSERT INTO $project SET run=9000, file = $file_number, submitted=0;\n";
}
