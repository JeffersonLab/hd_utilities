#!/usr/bin/env perl

$directory_label = $ARGV[0];
$dir_table = $directory_label . "_dir";
$file_table = $directory_label . "_file";
$update_time_table = $directory_label . "_updateTime";

use CGI;                             # load CGI routines
use DBI;
$user = "diskmanager";
$password = "";
$database = "diskManagement";
$hostname = "localhost";

$seconds_per_year = 365.25*24*60*60;

$dbh = DBI->connect("DBI:mysql:$database:$hostname", $user, $password);

$q = new CGI;                        # create new CGI object
$title = "Disk Usage Report: $directory_label";
print
    $q->start_html($title), # start the HTML
    $q->h1($title),         # level 1 header
    "<p>File ages are from last access time.\n";

$sql = "select updateTime from $update_time_table;";
make_query($dbh, \$sth);
@row = $sth->fetchrow_array;
print "<p>Update time: $row[0]\n";

print $q->h2("Largest Files"), "\n";
              #############

$sql = "select $file_table.size, filename, atime, $file_table.uid, dirname from $dir_table, $file_table where $dir_table.id = dirId order by $file_table.size desc limit 10;";
make_query($dbh, \$sth);
print "<table border>\n";
print "<tr><th>Rank<th>Size (GB)<th>File<th>Last Access Time<th>Owner<th>Directory\n";
$i = 1;
while (@row = $sth->fetchrow_array) {
    $size = round($row[0]/1.e9);
    $user = getpwuid($row[3]);
    if (! $user) {$user = "uid=$row[3]";}
    print "<tr><td>$i<td>$size<td>$row[1]<td>$row[2]<td>$user<td>$row[4]\n";
    $i++;
}
print "</table>\n";

print $q->h2("Oldest Files"), "\n";
              ############

$sql = "select atime, filename, $file_table.size, $file_table.uid, dirname from $dir_table, $file_table where $dir_table.id = dirId order by atime limit 10;";
make_query($dbh, \$sth);
print "<table border>\n";
print "<tr><th>Rank<th>Last Access Time<th>File<th>Size (GB)<th>Owner<th>Directory\n";
$i = 1;
while (@row = $sth->fetchrow_array) {
    $size = round($row[2]/1.e9);
    $user = getpwuid($row[3]);
    if (! $user) {$user = "uid=$row[3]";}
    print "<tr><td>$i<td>$row[0]<td>$row[1]<td>$size<td>$user<td>$row[4]\n";
    $i++;
}
print "</table>\n";

print $q->h2("Files with Greatest Size &times; Age"), "\n";
              ####################################

$sql = "select ($file_table.size*1.e-9)*(unix_timestamp(now()) - unix_timestamp(atime)) as gby, filename, $file_table.size, atime, $dir_table.uid, dirname from $dir_table, $file_table where $dir_table.id = dirId order by gby desc limit 10;";
make_query($dbh, \$sth);
print "<table border>\n";
print "<tr><th>Rank<th>Size&times;Age (GB-years)<th>File<th>Size (GB)<th>Last Access Time<th>Owner<th>Directory\n";
$i = 1;
while (@row = $sth->fetchrow_array) {
    $age_size = round($row[0]/$seconds_per_year);
    $size = round($row[2]/1.e9);
    $user = getpwuid($row[4]);
    if (! $user) {$user = "uid=$row[4]";}
    print "<tr><td>$i<td>$age_size<td>$row[1]<td>$size<td>$row[3]<td>$user<td>$row[5]\n";
    $i++;
}
print "</table>\n";

print $q->h2("Largest Directories"), "\nExcludes files in sub-directories\n";
              ###################
$sql = "select sum($file_table.size) as dirsize, dirname, $dir_table.uid from $dir_table, $file_table where dirId = $dir_table.id group by dirId order by dirsize desc limit 10;";
make_query($dbh, \$sth);
print "<table border>\n";
print "<tr><th>Rank<th>Size (GB)<th>Directory<th>Owner\n";
$i = 1;
while (@row = $sth->fetchrow_array) {
    $size = round($row[0]/1.e9);
    $user = getpwuid($row[2]);
    if (! $user) {$user = "uid=$row[2]";}
    print "<tr><td>$i<td>$size<td>$row[1]<td>$user\n";
    $i++;
}
print "</table>\n";

print $q->h2("Directories with Greatest Average Age"),
              ##########################################
    "\nExcludes files in sub-directories, size-weighted average age of all files in directory\n";

$sql = "select sum(($file_table.size*1.e-9)*(unix_timestamp(now()) - unix_timestamp(atime)))/sum($file_table.size)*1.e9 as aveage, dirname, $dir_table.uid from $dir_table, $file_table where $dir_table.id = dirId group by dirId order by aveage desc limit 10;";
make_query($dbh, \$sth);
print "<table border>\n";
print "<tr><th>Rank<th>Age (years)<th>Directory<th>Owner\n";
$i = 1;
while (@row = $sth->fetchrow_array) {
    $age_size = round($row[0]/$seconds_per_year);
    $user = getpwuid($row[2]);
    if (! $user) {$user = "uid=$row[2]";}
    print "<tr><td>$i<td>$age_size<td>$row[1]<td>$user\n";
    $i++;
}
print "</table>\n";

print $q->h2("Directories with Greatest Size &times; Age"),
              ##########################################
    "\nExcludes files in sub-directories, age &times; size summed over all files in directory\n";

$sql = "select sum(($file_table.size*1.e-9)*(unix_timestamp(now()) - unix_timestamp(atime))) as sumgby, dirname, $dir_table.uid from $dir_table, $file_table where $dir_table.id = dirId group by dirId order by sumgby desc limit 10;";
make_query($dbh, \$sth);
print "<table border>\n";
print "<tr><th>Rank<th>Size&times;Age (GB-years)<th>Directory<th>Owner\n";
$i = 1;
while (@row = $sth->fetchrow_array) {
    $age_size = round($row[0]/$seconds_per_year);
    $user = getpwuid($row[2]);
    if (! $user) {$user = "uid=$row[2]";}
    print "<tr><td>$i<td>$age_size<td>$row[1]<td>$user\n";
    $i++;
}
print "</table>\n";

print $q->h2("Largest Total File Size by User"),
              ###############################
    "\nSum of all files owned by user";

$sql = "select sum($file_table.size) as sumsize, $file_table.uid from $file_table group by $file_table.uid order by sumsize desc limit 10;";
make_query($dbh, \$sth);
print "<table border>\n";
print "<tr><th>Rank<th>Total Size (GB)<th>User";
$i = 1;
while (@row = $sth->fetchrow_array) {
    $size = round($row[0]/1.e9);
    $user = getpwuid($row[1]);
    if (! $user) {$user = "uid=$row[1]";}
    print "<tr><td>$i<td>$size<td>$user\n";
    $i++;
}
print "</table>\n";

print $q->h2("Greatest Average File Age by User"),
              ######################################
    "Size-weighted average age of all files owned by user\n";

$sql = "select sum(($file_table.size*1.e-9)*(unix_timestamp(now())-unix_timestamp(atime)))/sum($file_table.size)*1.e9 as aveage, $file_table.uid from $file_table group by $file_table.uid order by aveage desc limit 10;";
make_query($dbh, \$sth);
print "<table border>\n";
print "<tr><th>Rank<th>Age (years)<th>User\n";
$i = 1;
while (@row = $sth->fetchrow_array) {
    $age_size = round($row[0]/$seconds_per_year);
    $user = getpwuid($row[1]);
    if (! $user) {$user = "uid=$row[1]";}
    print "<tr><td>$i<td>$age_size<td>$user\n";
    $i++;
}
print "</table>\n";

print $q->h2("Largest Total Size &times; Age by User"),
              ######################################
    "\nAge &times; size summed over all files owned by user\n";

$sql = "select sum(($file_table.size*1.e-9)*(unix_timestamp(now())-unix_timestamp(atime))) as sumgby, $file_table.uid from $file_table group by $file_table.uid order by sumgby desc limit 10;";
make_query($dbh, \$sth);
print "<table border>\n";
print "<tr><th>Rank<th>Size&times;Age (GB-years)<th>User\n";
$i = 1;
while (@row = $sth->fetchrow_array) {
    $age_size = round($row[0]/$seconds_per_year);
    $user = getpwuid($row[1]);
    if (! $user) {$user = "uid=$row[1]";}
    print "<tr><td>$i<td>$age_size<td>$user\n";
    $i++;
}
print "</table>\n";

    print $q->end_html;                  # end the HTML
    
    print "\n";

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
