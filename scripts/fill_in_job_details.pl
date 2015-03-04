#!/usr/bin/env perl


$project_in = $ARGV[0];
if (! $project_in) {
    die "ERROR: project argument was not provided";
}

# load perl modules
use DBI;

# connect to the database
$host = 'hallddb.jlab.org';
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

$table = $project_in . "Job";
$sql = "SELECT jobId from $table WHERE username IS NULL OR (username IS NOT NULL AND status != \"DONE\");";
make_query($dbh_db, \$sth_jobid);
$count = 0;
while (@row = $sth_jobid->fetchrow_array) {
    $jobid = $row[0];
    $count++;
    if ($count%100 == 0) {print "$count, $jobid\n";}
    %jobhash = get_job_hash($jobid);
    $sql = "UPDATE $table SET\n";
    $first = 1;
    foreach $key (keys(%jobhash)) {
	$value = $jobhash{$key};
	if ($key eq "walltime"
	    || $key eq "cput"
	    || $key eq "files"
	    || $key eq "mem"
	    || $key eq "vmem"
	    ) {
	    $value = "\"" . $value . "\"";
	}
	if ($key =~ m/^time/) {
	    $value = substr($value, 0, 10);
	    $value = "FROM_UNIXTIME($value)"
	}
	if ($key ne "id") {
	    if ($first) {$first = 0;} else {$sql .= ",\n";}
	    $sql .= "  $key = $value";
	}
    }
    $sql .= " WHERE jobId = $jobid;\n";
#    print "sql = $sql";
    make_query($dbh_db, \$sth_insert);
}
print "updated job info on $count row(s)\n";
exit;

sub get_job_hash {
    my ($jobid) = @_;
    my %jobhash = ();
    open(WGET, "wget -q -O- \"http://scicomp.jlab.org/scicomp/AugerJobServlet?requested=jobDetails&id=$jobid\" |");
    $line = <WGET>;
#    print $line;
    chomp $line;
    @tok0 = split(/\[\{/, $line);
#    print "tok00 = $tok0[0]\n";
#    print "tok01 = $tok0[1]\n";
#    print "tok02 = $tok0[2]\n";
#    print "tok03 = $tok0[3]\n";
    @tok3 = split(/\}\]/, $tok0[1]);
#    print "tok30 = $tok3[0]\n";
    @tok1 = split(/,\"/, $tok3[0]);
#    print "tok10 = $tok1[0]\n";
#    print "tok11 = $tok1[1]\n";
#    print "tok12 = $tok1[2]\n";
    for ($i = 0; $i <= $#tok1; $i++) {
#	print "$i $tok1[$i]\n";
	@tok2 = split(/\":/, $tok1[$i]);
#	print "tok20 = $tok2[0]\n";
#	print "tok21 = $tok2[1]\n";
	$key = $tok2[0];
	$key =~ s/\"//g;
#	print "key = $key\n";
	if ($key eq "resourcesUsed") {
#	    print "resources are $tok2[1]\n";
	    $res = $tok2[1];
	    $res =~ s/\"//g;
	    @tok4 = split(/,/, $res);
#	    print "tok40 = $tok4[0]\n";
#	    print "tok41 = $tok4[1]\n";
	    for ($j = 0; $j <= $#tok4; $j++) {
		@tok5 = split(/\\u003d/, $tok4[$j]);
		$keyres = $tok5[0];
		$valueres = $tok5[1];
#		print "keyres = $keyres, valueres = $valueres\n";
		$jobhash{$keyres} = $valueres;
	    }
	} else {
	    $value = $tok2[1];
#	    print "value = $value\n";
	    $jobhash{$key} = $value;
	}
    }
    return %jobhash;
}

sub make_query {    

    my($dbh, $sth_ref) = @_;
    $$sth_ref = $dbh->prepare($sql)
        or die "Can't prepare $sql: $dbh->errstr\n";
    
    $rv = $$sth_ref->execute
        or die "Can't execute the query $sql\n error: $sth->errstr\n";
    
    return 0;

}
