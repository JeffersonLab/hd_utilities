#!/usr/bin/env perl

use CGI;                             # load CGI routines
use DBI;
use Getopt::Std;

parse_options();

$this_script = $0;

$directory_label = $ARGV[0];
$dir_table = $directory_label . "_dir";
$file_table = $directory_label . "_file";
$update_time_table = $directory_label . "_updateTime";

$user = "diskmanager";
$password = "";
$database = "diskManagement";
$hostname = "localhost";

$seconds_per_year = 365.25*24*60*60;

$dbh = DBI->connect("DBI:mysql:$database:$hostname", $user, $password);

if ($username) {
    $html_file_name = "${directory_label}_${username}_report.html";
} else {
    $html_file_name = "${directory_label}_report.html";
}
open(HTML, ">$html_file_name");

$q = new CGI;                        # create new CGI object
$title = "Disk Usage Report: $directory_label";
print HTML
    $q->start_html($title), # start the HTML
    $q->h1($title),         # level 1 header
    "<p>File ages are from last access time.\n";

$sql = "select updateTime from $update_time_table;";
make_query($dbh, \$sth);
@row = $sth->fetchrow_array;
print HTML "<p>Update time: $row[0]\n";

if ($userid) {
    $user_file_clause = "and $file_table.uid = $userid";
    $user_dir_clause = "and $dir_table.uid = $userid";
} else {
    $user_file_clause = "";
    $user_dir_clause = "";
    %users_to_report = ();
}

%largest_files_hash = ();
$largest_files_hash{title} = "Largest Files";
$largest_files_hash{comment} = "";
@lf_headings = ("Rank", "Size (GB)", "File", "Last Access Time", "User", "Directory");
$largest_files_hash{headings} = \@lf_headings;
$largest_files_hash{query} = "select format($file_table.size/1.e9, 2), filename, atime, $file_table.uid, dirname
			   from $dir_table, $file_table
			   where $dir_table.id = dirId $user_file_clause
			   order by $file_table.size desc
			   limit $nlines;";

%oldest_files_hash = ();
$oldest_files_hash{title} = "Oldest Files";
$oldest_files_hash{comment} = "";
@of_headings = ("Rank", "Last Access Time", "File", "Size (GB)", "User", "Directory");
$oldest_files_hash{headings} = \@of_headings;
$oldest_files_hash{query} = "select atime, filename, format($file_table.size/1.e9, 2), $file_table.uid, dirname
			  from $dir_table, $file_table
			  where $dir_table.id = dirId $user_file_clause
			  order by atime
			  limit $nlines;";

%sizagest_files_hash = ();
$sizagest_files_hash{title} = "Files with Greatest Size &times; Age";
$sizagest_files_hash{comment} = "";
@saf_headings = ("Rank", "Size&times;Age (GB-years)", "File", "Size (GB)", "Last Access Time", "User", "Directory");
$sizagest_files_hash{headings} = \@saf_headings;
$sizagest_files_hash{query} = "select format(($file_table.size*1.e-9)*(unix_timestamp(now()) - unix_timestamp(atime))/$seconds_per_year, 2) as gby, filename, format($file_table.size/1.e9, 2), atime, $file_table.uid, dirname
			    from $dir_table, $file_table 
			    where $dir_table.id = dirId $user_file_clause
			    order by cast($file_table.size as double)*cast((unix_timestamp(now()) - unix_timestamp(atime)) as double) desc
			    limit $nlines;";

%largest_dirs_hash = ();
$largest_dirs_hash{title} = "Largest Directories";
$largest_dirs_hash{comment} = "Excludes files in sub-directories";
@ld_headings = ("Rank", "Size (GB)", "Directory", "User");
$largest_dirs_hash{headings} = \@ld_headings;
$largest_dirs_hash{query} = "select format(sum($file_table.size)/1.e9, 2) as dirsize, dirname, $dir_table.uid
			  from $dir_table, $file_table
			  where dirId = $dir_table.id $user_dir_clause
			  group by dirId
			  order by sum(cast($file_table.size as double)) desc
			  limit $nlines;";

%oldest_dirs_hash = ();
$oldest_dirs_hash{title} = "Directories with Greatest Average Age";
$oldest_dirs_hash{comment} = "Excludes files in sub-directories, size-weighted average age of all files in directory";
@od_headings = ("Rank", "Age (years)", "Directory", "User");
$oldest_dirs_hash{headings} = \@od_headings;
$oldest_dirs_hash{query} = "select format(sum(($file_table.size*1.e-9)*(unix_timestamp(now()) - unix_timestamp(atime)))/sum($file_table.size)*1.e9/$seconds_per_year, 2) as aveage, dirname, $dir_table.uid
			 from $dir_table, $file_table
			 where $dir_table.id = dirId $user_dir_clause
			 group by dirId
			 order by sum(cast($file_table.size as double)*cast(unix_timestamp(now()) - unix_timestamp(atime) as double))/sum(cast($file_table.size as double)) desc
			 limit $nlines;";

%sizagest_dirs_hash = ();
$sizagest_dirs_hash{title} = "Directories with Greatest Size &times; Age";
$sizagest_dirs_hash{comment} = "Excludes files in sub-directories, age &times; size summed over all files in directory";
@sad_headings = ("Rank", "Size&times;Age (GB-years)", "Directory", "User");
$sizagest_dirs_hash{headings} = \@sad_headings;
$sizagest_dirs_hash{query} = "select format(sum(($file_table.size*1.e-9)*(unix_timestamp(now()) - unix_timestamp(atime)))/$seconds_per_year, 2) as sumgby, dirname, $dir_table.uid
			   from $dir_table, $file_table
			   where $dir_table.id = dirId $user_dir_clause
			   group by dirId
			   order by sum(cast($file_table.size as double)*cast(unix_timestamp(now()) - unix_timestamp(atime) as double)) desc
			   limit $nlines;";

%largest_users_hash = ();
$largest_users_hash{title} = "Largest Total File Size by User";
$largest_users_hash{comment} = "Sum of all files owned by user";
@lu_headings = ("Rank", "Total Size (GB)", "User");
$largest_users_hash{headings} = \@lu_headings;
$largest_users_hash{query} = "select format(sum($file_table.size)/1.e9, 2) as sumsize, $file_table.uid
			   from $file_table
			   group by $file_table.uid
			   order by sum(cast($file_table.size as double)) desc
			   limit $nlines;";

%oldest_users_hash = ();
$oldest_users_hash{title} = "Greatest Average File Age by User";
$oldest_users_hash{comment} = "Size-weighted average age of all files owned by user";
@ou_headings = ("Rank", "Age (years)", "User");
$oldest_users_hash{headings} = \@ou_headings;
$oldest_users_hash{query} = "select format(sum(($file_table.size*1.e-9)*(unix_timestamp(now())-unix_timestamp(atime)))/sum($file_table.size)*1.e9/$seconds_per_year, 2) as aveage, $file_table.uid
			  from $file_table
			  group by $file_table.uid
			  order by sum(cast($file_table.size as double)*cast(unix_timestamp(now())-unix_timestamp(atime) as double))/sum(cast($file_table.size as double)) desc
			  limit $nlines;";

%sizagest_users_hash = ();
$sizagest_users_hash{title} = "Largest Total Size &times; Age by User";
$sizagest_users_hash{comment} = "Age &times; size summed over all files owned by user";
@sau_headings = ("Rank", "Size&times;Age (GB-years)", "User");
$sizagest_users_hash{headings} = \@sau_headings;
$sizagest_users_hash{query} = "select format(sum(($file_table.size*1.e-9)*(unix_timestamp(now())-unix_timestamp(atime)))/$seconds_per_year, 2) as sumgby, $file_table.uid
			    from $file_table
			    group by $file_table.uid
			    order by sum(cast($file_table.size as double)*cast(unix_timestamp(now())-unix_timestamp(atime) as double)) desc
			    limit $nlines;";

%fileest_users_hash = ();
$fileest_users_hash{title} = "Largest Number of Files by User";
$fileest_users_hash{comment} = "";
@fu_headings = ("Rank", "Files", "User");
$fileest_users_hash{headings} = \@fu_headings;
$fileest_users_hash{query} = "select format(count(*), 0), uid, count(*) as c
                           from $file_table
                           group by uid
                           order by c desc
                           limit $nlines;";

if (!$userid) {
    do_one_section(\%sizagest_users_hash, "href");
    do_one_section(\%largest_users_hash, "href");
    do_one_section(\%fileest_users_hash, "href");
}
do_one_section(\%sizagest_dirs_hash);
do_one_section(\%largest_dirs_hash);
do_one_section(\%sizagest_files_hash);
do_one_section(\%largest_files_hash);

print HTML $q->end_html;                  # end the HTML
    
print HTML "\n";

if (!$userid) {
    foreach $user_key (keys %users_to_report) {
	$command =  "$this_script -u $user_key -n $nlines_users $directory_label";
	print "executing $command\n";
	system($command);
    }
}

exit;
#
# output html for one section
#
sub do_one_section {
    %section_hash = %{$_[0]};
    if ($_[1] eq "href") {
	$add_href = 1;
	$href_prefix = "<a href=\"${directory_label}_";
	$href_middle = "_report.html\">";
	$href_suffix = "</a>";
    } else {
	$add_href = 0;
    }
    print HTML $q->h2($section_hash{title}), "\n";
    my $comment = $section_hash{comment};
    if ($comment) {print HTML $comment, "\n";}
    $sql = $section_hash{query};
    make_query($dbh, \$sth);
    print HTML "<table border>\n";
    @hray = @{$section_hash{headings}};
    print HTML "<tr>";
    $iuser = -1; # no header is called "User"
    for ($ih = 0; $ih <= $#hray; $ih++) {
	$hthis = $hray[$ih];
	print HTML "<th>$hthis";
	if ($hthis eq "User") { # this column is a uid, mark it as such
	    $iuser = $ih;
	}
    }
    print HTML "\n";
    $i = 1;
    while (@row = $sth->fetchrow_array) {
	print "1 = $row[1], 4 = $row[4]\n";
	if ($iuser >= 0) {
	    $iu = $iuser - 1;
	    $user = getpwuid($row[$iu]);
	    if (! $user) {
		$user = "uid=$row[$iu]";
	    }
	    if ($add_href) {
		$row[$iu] = "${href_prefix}${user}${href_middle}${user}${href_suffix}";
		$users_to_report{$user} = 1;
	    } else {
		$row[$iu] = $user;
	    }
	}
	print HTML "<tr><td>$i";
	for ($ih = 0; $ih < $#hray; $ih++) {
	    print HTML "<td>$row[$ih]";
	}
	print HTML "\n";
	$i++;
    }
    print HTML "</table>\n";
}
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
#
# parse options
#
sub parse_options {
    getopts('hu:n:m:');
    if ($opt_h) {
	print_usage();
	exit 0;
    }
    if ($opt_u) {
	$username = $opt_u;
	if ($username =~ m/^uid=/) {
	    #print ("username is likely of the form uid=1234\n");
	    @tokens = split(/=/, $username);
	    $userid = $tokens[1]; 
	} else {
	    $userid = `id -u $username`;
	    chomp $userid;
	}
	if (!$userid) {$userid = 0;}
	#print "$username, $userid\n"
    } else {
	$username = "";
	$userid = 0;
    }
    #print "userid = $userid\n";
    if ($opt_n) {
	$nlines = $opt_n;
    } else {
	$nlines = 10; # default number of lines
    }
    if ($opt_m) {
	$nlines_users = $opt_m;
    } else {
	$nlines_users = 20; # default number of lines
    }
}
sub print_usage {
    print <<EOM;
disk_report.pl [-h] [-u username] [-n number-of-lines-per-category] database-tag
  example: disk_report.pl -u torvalds -n 20 work
EOM
}
#
# end of file
#
