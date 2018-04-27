#!/usr/bin/env perl
#
# parameters
#
$starting_directory = $ARGV[0];
$directory_label = $ARGV[1];
if (! $starting_directory || !$directory_label) {
    print_usage_message();
    exit 1;
}
$dir_table_final = "${directory_label}_dir";
$file_table_final = "${directory_label}_file";
$update_time_table_final = "${directory_label}_updateTime";
$dir_table = $dir_table_final . "_temp";
$file_table = $file_table_final . "_temp";
$update_time_table = $update_time_table_final . "_temp";
#
# load perl modules
#
use DBI;
#
# connect to database
#
$host = 'localhost';
$user = 'diskmanager';
$password = '';
$database = 'diskManagement';
print "Connecting to $user\@$host, using $database.\n";
$dbh_db = DBI->connect("DBI:SQLite:dbname=diskManagement.sqlite");
if (defined $dbh_db) {
    print "Connection successful\n";
} else {
    die "Could not connect to the database server, exiting.\n";
}
#
# drop the tables
#
$sql = "drop table if exists $dir_table;";
make_query($dbh_db, \$sth);
$sql = "drop table if exists $file_table;";
make_query($dbh_db, \$sth);
$sql = "drop table if exists $update_time_table;";
make_query($dbh_db, \$sth);
#
# re-create tables
#
$sql = "create table $dir_table (
       id int primary key,
       dirname varchar(256),
       size int,
       uid smallint
);";
make_query($dbh_db, \$sth);
$sql = "create table $file_table (
       id int primary key,
       filename varchar(256),
       dirId int,
       atime datetime,
       size bigint,
       uid smallint
);";
make_query($dbh_db, \$sth);
$sql = "create table $update_time_table (
updateTime timestamp default current_timestamp not null
);";
make_query($dbh_db, \$sth);
#
# collect the data
#
$find_dir_command = "find $starting_directory -maxdepth 2 -xdev \\( \\(";
$find_dir_command .= " -path $starting_directory/.snapshot";
$find_dir_command .= " -o -path /volatile/halld/home/gxproj5/trackeff_ver99";
$find_dir_command .= " -o -path /volatile/halld/home/jzarling/April18_bggen_omega/log";
$find_dir_command .= " \\) -prune \\) -o \\( -type d -print \\) |";
print "find_dir_command = $find_dir_command\n";
open (FINDDIR, $find_dir_command);
$idebug = 0;
$jdebug = 0;
while ($dirname = <FINDDIR>) {
#    if (!($idebug%100)) {print "$idebug: dirname = $dirname";}
    print "$idebug: dirname = $dirname";
    chomp $dirname;
    @stat = stat($dirname);
    $sql = "insert into $dir_table (dirname, uid, size) values (\"$dirname\", $stat[4], $stat[7]);";
    #print $sql, "\n";
    make_query($dbh_db, \$sth);
    $sql = "select last_insert_rowid();";
    make_query($dbh_db, \$sth);
    @row = $sth->fetchrow_array;
    $last_id = $row[0];
    $dirname =~ s/ /\\ /g; # escape space
    $dirname =~ s/\(/\\\(/g; # escape (
    $dirname =~ s/\)/\\\)/g; # escape )
    $dirname =~ s/&/\\&/g; # escape &
    $dirname =~ s/\$/\\\$/g; # escape $
    $dirname =~ s/>/\\>/g; # escape >
    $dirname =~ s/</\\</g; # escape <
    open (FINDFILE, "find $dirname -maxdepth 1 -type f |");
    while ($filename = <FINDFILE>) {
	chomp $filename;
#	if (!($jdebug%1000)) {print "$jdebug: $filename\n";}
	print "$jdebug: $filename\n";
	@stat = stat($filename);
	if (@stat) {
	    @token = split(/\//, $filename);
	    $file_no_path = $token[$#token];
	    #print "file_no_path = $file_no_path\n";
	    $file_no_path =~ s/\\/\\\\/g; # escape backslash
	    $sql = "insert into $file_table (filename, dirId, atime, size, uid) values (\"$file_no_path\", $last_id, datetime($stat[8], \'unixepoch\'), $stat[7], $stat[4]);";
	    make_query($dbh_db, \$sth);
	} else {
	    print "cannot stat $filename in $dirname\n";
	}
	$jdebug++;
    }
    close(FINDFILE);
    $idebug++;
#    print "DEBUG: $idebug directories done\n";
#    if ($idebug == 1000) {last;}
}
$sql = "drop table if exists $dir_table_final;";
make_query($dbh_db, \$sth);
$sql = "drop table if exists $file_table_final;";
make_query($dbh_db, \$sth);
$sql = "drop table if exists $update_time_table_final;";
make_query($dbh_db, \$sth);
$sql = "alter table $dir_table rename to $dir_table_final;";
make_query($dbh_db, \$sth);
$sql = "alter table $file_table rename to $file_table_final;";
make_query($dbh_db, \$sth);
$sql = "alter table $update_time_table rename to $update_time_table_final;";
make_query($dbh_db, \$sth);
$sth = 0;
#
# disconnect from database server
#
print "disconnecting from server\n";
$rc = $dbh_db->disconnect();
print "disconnected\n";
#
# exit
#
print "exit time!\n";
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

sub print_usage_message {
    print <<EOM;
Usage:
  disk_database.pl <top level directory> <report label>
EOM
return;
}
#
# end of file
#
