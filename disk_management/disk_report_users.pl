#!/usr/bin/env perl

use DBI;
use Getopt::Std;

$directory_label = $ARGV[0];
$output_dir = $ARGV[1];
$dir_table = $directory_label . "_dir";
$file_table = $directory_label . "_file";
$update_time_table = $directory_label . "_updateTime";
$dmdir = "/home/marki/git/hd_utilities/disk_management";

$user = "diskmanager";
$password = "";
$database = "diskManagement";
$hostname = "localhost";

$seconds_per_year = 365.25*24*60*60;

$dbh = DBI->connect("DBI:mysql:$database:$hostname", $user, $password);

$sql = "select sum(($file_table.size*1.e-9)*(unix_timestamp(now())-unix_timestamp(atime))) as sumgby, $file_table.uid from $file_table group by $file_table.uid order by sumgby desc limit 10;";
make_query($dbh, \$sth);
$i = 1;
while (@row = $sth->fetchrow_array) {
    $age_size = round($row[0]/$seconds_per_year);
    $user = getpwuid($row[1]);
    if (! $user) {$user = "uid=$row[1]";}
    print "creating report for user = $user, rank = $i, GB-years = $age_size\n";
    system("$dmdir/disk_report.pl -u $user $directory_label > ${output_dir}/${directory_label}_${user}.html");
    $i++;
}

exit;
#
# make a query
#
sub make_query {    

    my($dbh, $sth_ref) = @_;
    $$sth_ref = $dbh->prepare($sql)
        or die "Can't prepare $sql: $dbh->errstr\n";
    
    $rv = $$sth_ref->execute
        or die "Can't execute the query $sql\n error: $sth->errstr\n";
    
    return 0;
}
sub round {
    my ($float) = @_;
    $rounded_float = sprintf("%.2f", $float);
    return $rounded_float;
}
#
# end of file
#
