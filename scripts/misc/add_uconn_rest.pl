#!/usr/bin/env perl

# load perl modules
use DBI;

$lsl_file = $ARGV[0];

if (! -f $lsl_file) {die "input file not found, looked for \"$lsl_file\"";}

# connect to the database
$host = 'halldweb1.jlab.org';
$user = 'farmer';
$password = '';
$database = 'farming';

print "Connecting to $user\@$host, using $database.\n";
$dbh_db = DBI->connect("DBI:mysql:$database:$host", $user, $password);
if (defined $dbh_db) {
    print "Connection successful\n";
} else {
    die "Could not connect to the database server, exiting.\n";
}

open (LSL, $lsl_file);
open (INSERT, ">insert.sql");
$count_lines = 0;
$count_rest = 0;
$count_found = 0;
while (<LSL>) {
    $count_lines++;
    if ($count_lines%5000 == 0) {print; print_status();}
    if (/dana_rest/) {
	$count_rest++;
        #print;
        chomp;
        @tok0 = split(/dana_rest_/);
        #print "$tok0[1]\n";
        @tok1 = split(/.hddm/, $tok0[1]);
        #print "$tok1[0]\n";
        @tok2 = split(/_/, $tok1[0]);
	$run_in = $tok2[0]; $file_in = $tok2[1];
	$sql = "select run, file from dc2_uconn_rest where run = $run_in and file = $file_in;";
	#print "sql = $sql\n";
	make_query($dbh_db, \$sth);
	$count = 0;
	while (@row = $sth->fetchrow_array) {
	    $count++;
	    $run_out = $row[0];
	    $file_out = $row[1];
	}
	if ($count > 1) {die 'more than one, cannot happen';}
	if ($count) {
	    $count_found++;
	} else {
	    print INSERT "insert into dc2_uconn_rest set run = $run_in, file = $file_in;\n";
	}
    }
}
close(LSL);
close(INSERT);

print_status();

exit;

sub print_status {
    print "lines = $count_lines\n";
    print "rest = $count_rest\n";
    print "found = $count_found\n";
}

sub make_query {    

    my($dbh, $sth_ref) = @_;
    $$sth_ref = $dbh->prepare($sql)
        or die "Can't prepare $sql: $dbh->errstr\n";
    
    $rv = $$sth_ref->execute
        or die "Can't execute the query $sql\n error: $sth->errstr\n";
    
    return 0;

}
