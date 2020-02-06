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
$host = 'hallddb';
$user = 'diskmanager';
$password = '';
$database = 'diskManagement';
print "Connecting to $user\@$host, using $database.\n";
$dbh_db = DBI->connect("DBI:mysql:$database:$host", $user, $password);
if (defined $dbh_db) {
    print "Connection successful\n";
} else {
    die "Could not connect to the database server, exiting.\n";
}
#
# drop the tables
#
$sql = "drop table if exists $dir_table, $file_table, $update_time_table;";
make_query($dbh_db, \$sth);
#
# re-create tables
#
$sql = "create table $dir_table (
       id int primary key auto_increment,
       dirname varchar(256),
       size int,
       uid smallint
);";
make_query($dbh_db, \$sth);
$sql = "create table $file_table (
       id int primary key auto_increment,
       filename varchar(256),
       dirId int,
       atime datetime,
       size bigint,
       uid smallint
);";
make_query($dbh_db, \$sth);
$sql = "create table $update_time_table (
updateTime timestamp
);";
make_query($dbh_db, \$sth);
$sql = "insert into $update_time_table set updateTime = NULL;";
make_query($dbh_db, \$sth);
#
# collect the data
#
$find_dir_command = "find $starting_directory -xdev \\( \\(";
$find_dir_command .= " -path $starting_directory/.snapshot";
$find_dir_command .= " -o -path /cache/halld/halld-scratch/REQUESTED_MC/LowE_anav01/etapippimpi0G4_20190612043545pm/root/trees/tree_pi0pippim__B4_genEtaRegge_051450_008.root";
$find_dir_command .= " -o -path /cache/halld/RunPeriod-2018-01/recon/ver02/p3pi_excl_skim/042117/p3pi_excl_skim_042117_204.root";
$find_dir_command .= " -o -path /cache/halld/RunPeriod-2018-01/recon/ver02/tree_bcal_hadronic_eff/042234/tree_bcal_hadronic_eff_042234_192.root";
$find_dir_command .= " -o -path /cache/halld/RunPeriod-2018-01/recon/ver02/dana_rest_coherent_peak/042103/dana_rest_coherent_peak_042103_013.hddm";
$find_dir_command .= " -o -path /cache/halld/RunPeriod-2018-01/recon/ver02/tree_sc_eff/042103/tree_sc_eff_042103_013.root";
$find_dir_command .= " -o -path /cache/halld/RunPeriod-2018-01/recon/ver02/tree_PSFlux/042117/tree_PSFlux_042117_204.root";
$find_dir_command .= " -o -path /cache/halld/offline_monitoring/RunPeriod-2018-08/ver14/converted_random/051306/converted_random_051306_002.hddm";
$find_dir_command .= " -o -path /cache/halld/RunPeriod-2018-08/rawdata/Run051545/hd_rawdata_051545_000.evio";
$find_dir_command .= " -o -path /cache/halld/RunPeriod-2017-01/recon/ver03/REST/030593/dana_rest_030593_022.hddm";
$find_dir_command .= " -o -path /volatile/halld/home/gxproj7/RunPeriod-2019-01/dirc_monitoring/analysis_REST/ver08_pass11/tree_pippimkpkm__B4/060727/tree_pippimkpkm__B4_060727_007.root";
$find_dir_command .= " -o -path /volatile/halld/home/gxproj7/RunPeriod-2019-01/dirc_monitoring/analysis_REST/ver08_pass11/tree_pipkmks__B4_M16/060735/tree_pipkmks__B4_M16_060735_001.root";
$find_dir_command .= " -o -path /volatile/halld/home/gxproj7/RunPeriod-2019-01/dirc_monitoring/analysis_REST/ver08_pass11/tree_pimkpks__B4_M16/060728/tree_pimkpks__B4_M16_060728_007.root";
$find_dir_command .= " -o -path /volatile/halld/home/gxproj7/RunPeriod-2019-01/dirc_monitoring/analysis_REST/ver08_pass11/tree_pippim__B4/060735/tree_pippim__B4_060735_002.root";
$find_dir_command .= " \\) -prune \\) -o \\( -type d -print \\) |";
print "find_dir_command = $find_dir_command\n";
open (FINDDIR, $find_dir_command);
$idebug = 0;
$jdebug = 0;
while ($dirname = <FINDDIR>) {
    if (!($idebug%100)) {print "$idebug: dirname = $dirname";}
    chomp $dirname;
    @stat = stat($dirname);
    $sql = "insert into $dir_table set dirname = \"$dirname\", uid = $stat[4], size = $stat[7];";
    #print $sql, "\n";
    make_query($dbh_db, \$sth);
    $sql = "select last_insert_id();";
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
	if (!($jdebug%1000)) {print "$jdebug: $filename\n";}
	@stat = stat($filename);
	if (@stat) {
	    @token = split(/\//, $filename);
	    $file_no_path = $token[$#token];
	    #print "file_no_path = $file_no_path\n";
	    $file_no_path =~ s/\\/\\\\/g; # escape backslash
	    $sql = "insert into $file_table set filename = \"$file_no_path\", dirId = $last_id, atime = from_unixtime($stat[8]), size = $stat[7], uid = $stat[4];";
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
$sql = "drop table if exists $dir_table_final, $file_table_final, $update_time_table_final;";
make_query($dbh_db, \$sth);
$sql = "rename table $dir_table to $dir_table_final;";
make_query($dbh_db, \$sth);
$sql = "rename table $file_table to $file_table_final;";
make_query($dbh_db, \$sth);
$sql = "rename table $update_time_table to $update_time_table_final;";
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
