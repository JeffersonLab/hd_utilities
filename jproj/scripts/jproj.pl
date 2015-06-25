#!/usr/bin/env perl

# load perl modules

use DBI;
use File::Path 'rmtree';

# output file directory
#$jsub_file_path = "/tmp";

if ($#ARGV == -1) {
    print_usage();
    exit 0;
}

$project = $ARGV[0];
$action = $ARGV[1];

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

if ($action eq 'create') {
    create();
} elsif ($action eq 'update') {
    update();
} elsif ($action eq 'update_output') {
    update_output();
} elsif ($action eq 'update_silo') {
    update_silo();
} elsif ($action eq 'update_cache') {
    update_cache();
} elsif ($action eq 'submit') {
    submit();
} elsif ($action eq 'unsubmit') {
    unsubmit();
} elsif ($action eq 'jput') {
    jput();
} elsif ($action eq 'jcache') {
    jcache();
} else {
    print "no valid action requested, action = $action\n";
}

print "disconnecting from server\n";
$rc = $dbh_db->disconnect;

sub create {
    print "starting create\n";
    $sql = 
"CREATE TABLE $project (
  run int(11) NOT NULL default '0',
  file int(11) NOT NULL default '0',
  tape_volser int(11) NOT NULL default '0',
  tape_fileposition int(11) NOT NULL default '0',
  submitted tinyint(4) NOT NULL default '0',
  output tinyint(4) NOT NULL default '0',
  jput_submitted tinyint(4) NOT NULL default '0',
  silo tinyint(4) NOT NULL default '0',
  jcache_submitted tinyint(4) NOT NULL default '0',
  cache tinyint(4) NOT NULL default '0',
  mod_time timestamp NOT NULL,
  PRIMARY KEY  (run,file)
) ENGINE=MyISAM;";
    make_query($dbh_db, \$sth);
    $sql = 
"CREATE TABLE ${project}Job (
  `id` int(11) NOT NULL AUTO_INCREMENT,
  `run` int(11) DEFAULT NULL,
  `file` int(11) DEFAULT NULL,
  `jobId` int(11) DEFAULT NULL,
  `timeChange` timestamp NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `username` varchar(64) DEFAULT NULL,
  `project` varchar(64) DEFAULT NULL,
  `name` varchar(64) DEFAULT NULL,
  `queue` varchar(64) DEFAULT NULL,
  `hostname` varchar(64) DEFAULT NULL,
  `nodeTags` varchar(64) DEFAULT NULL,
  `coresRequested` int(11) DEFAULT NULL,
  `memoryRequested` int(11) DEFAULT NULL,
  `status` varchar(64) DEFAULT NULL,
  `exitCode` int(11) DEFAULT NULL,
  `result` varchar(64) DEFAULT NULL,
  `timeSubmitted` datetime DEFAULT NULL,
  `timeDependency` datetime DEFAULT NULL,
  `timePending` datetime DEFAULT NULL,
  `timeStagingIn` datetime DEFAULT NULL,
  `timeActive` datetime DEFAULT NULL,
  `timeStagingOut` datetime DEFAULT NULL,
  `timeComplete` datetime DEFAULT NULL,
  `walltime` varchar(8) DEFAULT NULL,
  `cput` varchar(8) DEFAULT NULL,
  `mem` varchar(64) DEFAULT NULL,
  `vmem` varchar(64) DEFAULT NULL,
  `script` varchar(1024) DEFAULT NULL,
  `files` varchar(1024) DEFAULT NULL,
  `error` varchar(1024) DEFAULT NULL,
  PRIMARY KEY (`id`)
) ENGINE=MyISAM;";

    make_query($dbh_db, \$sth);
    $run_number = $ARGV[2];
    $number_of_files = $ARGV[3];
    if ($number_of_files ne '') {
	print "create: $number_of_files runs requested\n";
	for ($findex = 1; $findex <= $number_of_files; $findex++) {
	    $file_number = $findex;
	    $sql = "INSERT INTO $project SET run = $run_number, file = $file_number, submitted=0";
	    make_query($dbh_db, \$sth);
    }
    } else{
	print "create: no runs requested, none will be created\n";
    }
}

sub update {

    open(CONFIG, "${project}.jproj");
    $input_string = <CONFIG>;
    chomp $input_string;
    print "$input_string\n";
    @token = split(/\//, $input_string);
    $name = $token[$#token];
    $name_escaped = $name;
    $name_escaped =~ s/\*/\\\*/g;
    @token2 = split(/$name_escaped/, $input_string);
    $dir = @token2[0];
    print "dir = $dir name = $name\n";
    @token3 = split(/\*/, $name);
    $prerun = $token3[0];
    $separator = $token3[1];
    $postfile = $token3[2];
    print "prerun = $prerun separator = $separator postfile = $postfile\n";
    $file_number_requested = $ARGV[2];
    if ($file_number_requested ne '') {
	print "file number requested = $file_number_requested\n";
    }
    open(FIND, "find $dir -maxdepth 1 -name \"$name\" |");
    while ($file = <FIND>) {
	chomp $file;
	if(index($file, 'Backup') != -1){
	    next; # "Backup" in name, don't use
	}
	print "file = $file\n";
        @field = split(/\//, $file);
	$this_name = $field[$#field];
	print "this_name(1) = $this_name\n";
	$run_in_dir_name = $field[$#field -1];
	$run_in_dir_name =~ s/Run//;
	# print "run_in_dir_name = $run_in_dir_name\n";
	open(MSSFILE, $file);
	my $tapevolser = 0;
	my $tapefileposition = 0;
	while($line = <MSSFILE>) {
	    chomp $line;
	    if ($line =~ /^volser/) {
		@linesplit = split('=', $line);
		$tapevolser = @linesplit[1];
	    }
	    if ($line =~ /^filePosition/) {
                @linesplit = split('=', $line);
                $tapefileposition = @linesplit[1];
            }
	}
	@token4 = split(/$prerun/, $this_name);
	$this_name = $token4[1];
	print "this_name(2) = $this_name\n";
	if ($postfile) {
	    @token5 = split(/$postfile/, $this_name);
	    $this_name = @token5[0];
	}
	print "this_name(3) = $this_name\n";
	print "volser, filepos = $tapevolser, $tapefileposition\n";
	@token6 = split(/$separator/, $this_name);
	$run = $token6[0];
	$file_number = $token6[1];
	print "run = $run file_number = $file_number \n";

	if ($run != $run_in_dir_name) {
	    # Found a file where the directory name RunRRRRRR does not
            # match the one for the file.
	    print "skipping file $file due to mismatch of run number!!\n";
	    next;
	}

	if ($file_number_requested eq '' || $file_number_requested == $file_number) {
	    $sql = "SELECT * FROM $project WHERE run = $run and $file_number = file";
	    make_query($dbh_db, \$sth);
	    $nrow = 0;
	    while ($hashref = $sth->fetchrow_hashref) {
		$nrow++;
	    }
	    if ($nrow == 0) {
		print "new run: $run, file: $file_number\n";
		$sql = "INSERT INTO $project SET run=$run, file = $file_number, tape_volser = $tapevolser, tape_fileposition = $tapefileposition, submitted=0";
		make_query($dbh_db, \$sth);
	    } elsif ($nrow > 1) {
		die "error too many entries for run $run"; 
	    }
	}
    }
    close(FIND);

}

sub update_output {
    $output_dir = $ARGV[2];
    $pattern_run_only = $ARGV[3];
    if ($pattern_run_only ne '') {
	print "file pattern will include only run number\n";
    }
    $sql = "SELECT run, file FROM $project WHERE submitted = 1 AND output = 0 order by run, file";
    make_query($dbh_db, \$sth);
    $nprocessed = 0;
    $nfound = 0;
    while (@row = $sth->fetchrow_array) {
	$run = sprintf("%05d", $row[0]);
	$file = sprintf("%07d", $row[1]);
	if ($pattern_run_only) {
	    $file_pattern = $run;
	} else {
	    $file_pattern = $run . '_' . $file;
	}
	open(FIND, "find $output_dir -maxdepth 1 -name \*$file_pattern\* |");
	$nfile = 0;
	while ($filefound = <FIND>) {
	    $filename = $filefound; # for use outside loop
	    $nfile++;
	}
	close(FIND);
	chomp $filename;
	$output = 0;
	if ($nfile == 1) {
	    if (-f $filename) {
		$output = 1;
		$nfound++;
	    } else {
		print "removing dead link: $filename\n";
		unlink $filename;
	    }
	} elsif ($nfile > 1) {
	    print "Run $run File $file has more than one output files\n";
	}
	$sql = "UPDATE $project SET output = $output WHERE run=$run and file=$file";
	make_query($dbh_db, \$sth2);
	$nprocessed++;
	if ($nprocessed%100 == 0) {
	    print "last pattern = $file_pattern, processed = $nprocessed, found = $nfound\n";
	}
    }
    print "last pattern = $file_pattern, processed = $nprocessed, found = $nfound\n";
}

sub update_silo {
    $silo_dir = $ARGV[2];
    $pattern_run_only = $ARGV[3];
    if ($pattern_run_only ne '') {
	print "file pattern will include only run number\n";
    }
    $sql = "SELECT run, file FROM $project WHERE jput_submitted = 1 AND silo = 0 order by run, file\;";
    make_query($dbh_db, \$sth);
    $nprocessed = 0;
    $nfound = 0;
    while (@row = $sth->fetchrow_array) {
	$run = sprintf("%05d", $row[0]);
	$file = sprintf("%07d", $row[1]);
	if ($pattern_run_only) {
	    $file_pattern = $run;
	} else {
	    $file_pattern = $run . '_' . $file;
	}
	open(FIND, "find $silo_dir -maxdepth 1 -name \*$file_pattern\* |");
	$nfile = 0;
	while ($filefound = <FIND>) {
	    $filename = $filefound; # for use outside loop
	    $nfile++;
	}
	close(FIND);
	chomp $filename;
	$silo = 0;
	if ($nfile == 1) {
	    if (-f $filename) {
		$silo = 1;
		$nfound++;
	    }
	} elsif ($nfile > 1) {
	    print "Run $run File $file has more than one output files\n";
	}
	$sql = "UPDATE $project SET silo = $silo WHERE run=$run and file=$file";
	make_query($dbh_db, \$sth2);
	$nprocessed++;
	if ($nprocessed%100 == 0) {
	    print "last pattern = $file_pattern, processed = $nprocessed, found = $nfound\n";
	}
    }
    print "last pattern = $file_pattern, processed = $nprocessed, found = $nfound\n";
}

sub update_cache {
    $cache_dir = $ARGV[2];
    $pattern_run_only = $ARGV[3];
    if ($pattern_run_only ne '') {
	print "file pattern will include only run number\n";
    }
    $sql = "SELECT run, file FROM $project";
    make_query($dbh_db, \$sth);
    $nprocessed = 0;
    $nfound = 0;
    while (@row = $sth->fetchrow_array) {
	$run = sprintf("%05d", $row[0]);
	$file = sprintf("%07d", $row[1]);
	if ($pattern_run_only) {
	    $file_pattern = $run;
	} else {
	    $file_pattern = $run . '_' . $file;
	}
	open(FIND, "find $cache_dir -maxdepth 1 -name \*$file_pattern\* |");
	$nfile = 0;
	while ($filefound = <FIND>) {
	    $filename = $filefound; # for use outside loop
	    $nfile++;
	}
	close(FIND);
	chomp $filename;
	$cache = 0;
	if ($nfile == 1) {
	    if (-f $filename) {
		$cache = 1;
		$nfound++;
	    }
	} elsif ($nfile > 1) {
	    print "Run $run File $file has more than one output files\n";
	}
	$sql = "UPDATE $project SET cache = $cache WHERE run=$run and file=$file";
	make_query($dbh_db, \$sth2);
	$nprocessed++;
	if ($nprocessed%100 == 0) {
	    print "last pattern = $file_pattern, processed = $nprocessed, found = $nfound\n";
	}
    }
    print "last pattern = $file_pattern, processed = $nprocessed, found = $nfound\n";
}

sub submit {
    $limit = $ARGV[2];
    $run_choice = $ARGV[3];
    if ($limit == 0 or $limit eq '') {
	$limit = 1000000;
    }
    print "limit = $limit\n";

    if ($run_choice) {
#	$sql = "SELECT run, file FROM $project WHERE submitted=0 AND run=$run_choice order by file limit $limit";
	$sql = "SELECT run, file FROM $project WHERE submitted=0 AND run=$run_choice order by tape_volser, tape_fileposition limit $limit";
    } else {
#	$sql = "SELECT run, file FROM $project WHERE submitted=0 order by run, file limit $limit";
	$sql = "SELECT run, file FROM $project WHERE submitted=0 order by tape_volser, tape_fileposition limit $limit";
    }
    make_query($dbh_db, \$sth);
    $num_jobs = 0;
    while (@row = $sth->fetchrow_array) {
	$run_array[$num_jobs] = $row[0];
	$file_array[$num_jobs] = $row[1];
	$num_jobs++;
    }
    # Now, re-arrange jobs such that they are submitted in the optimal order for retrieval from tape: 
        # Different tape movers should read data from different tapes, so submit jobs in a jagged order.
    my ($new_run_array_ref, $new_file_array_ref) = rearrange_files(\@run_array, \@file_array, $num_jobs);
    my @new_run_array = @$new_run_array_ref;
    my @new_file_array = @$new_file_array_ref;
    # make temporary directory to house temporary xml files
    mkdir "temp";
    for ($j = 0; $j < $num_jobs; $j++) {
	$run_this = $new_run_array[$j];
	$file_this = $new_file_array[$j];
	printf ">>>submitting run $run_this file $file_this<<<\n";
	$jobIndex = submit_one($run_this, $file_this);
	# print "DEBUG: jobindex returned from submit_one = $jobIndex\n";
	if($jobIndex ne '') {
	   $sql = "UPDATE $project SET submitted=1 WHERE run=$run_this and file=$file_this";
	   make_query($dbh_db, \$sth);
	   $sql = "INSERT ${project}Job SET run=$run_this, file=$file_this, jobId = $jobIndex";
	   make_query($dbh_db, \$sth);
	}
    }
    rmtree(["temp"]);
}

sub rearrange_files {
    my ($run_array_ref, $file_array_ref, $num_jobs) = @_;
    my @run_array = @$run_array_ref;
    my @file_array = @$file_array_ref;
    $num_tape_movers = 13;
    $num_jobs_per_tape_mover = int($num_jobs / $num_tape_movers);
    $num_organized_jobs = $num_jobs_per_tape_mover * $num_tape_movers;
    my @new_run_array;
    $#new_run_array = $num_jobs;
    my @new_file_array;
    $#new_file_array = $num_jobs;
    my $tape_mover_index = 0;
    my $tape_mover_job_index = 0;
    print "num_jobs = $num_jobs, num_jobs_per_tape_mover = $num_jobs_per_tape_mover\n";
    for ($j = 0; $j < $num_organized_jobs; $j++) {
        my $file_index = ($tape_mover_index * $num_jobs_per_tape_mover) + $tape_mover_job_index;
        $new_run_array[$j] = $run_array[$file_index];
        $new_file_array[$j] = $file_array[$file_index];
        # print "PUSH: j = $j, tape_mover_index = $tape_mover_index, job_index = $tape_mover_job_index, file_index = $file_index, run = $new_run_array[$#new_run_array], file = $new_file_array[$#new_file_array]\n";
        $tape_mover_index++;
        if($tape_mover_index eq $num_tape_movers) {
            $tape_mover_index = 0;
            $tape_mover_job_index++;
        }
    }
    # push back remainder of jobs
    for ($j = $num_organized_jobs; $j < $num_jobs; $j++) {
        $new_run_array[$j] = $run_array[$j];
        $new_file_array[$j] = $file_array[$j];
	# print "set: j = $j, new run = $new_run_array[$j], new file = $new_file_array[$j]\n";
    }
    return (\@new_run_array, \@new_file_array);
}

sub submit_one {
    my($run_in, $file_in) = @_;
    my $jobIndex = "job index undefined";
    $run = sprintf("%06d", $run_in);
    $file = sprintf("%03d", $file_in);
#    $jsub_file = "$jsub_file_path/${project}_${run}_${file}.jsub";
    $jsub_file = "temp/${run}_${file}.jsub";
    if (-e $jsub_file) {
       unlink $jsub_file;
    }
    open(JSUB, ">$jsub_file");
    $jsub_file_template = "$project.jsub";
    if (-e $jsub_file_template) {
	open(JSUB_TEMPLATE, "$jsub_file_template");
	while ($line = <JSUB_TEMPLATE>) {
	    $line =~ s/{project}/$project/g;
	    $line =~ s/{run_number}/$run/g;
	    $line =~ s/{file_number}/$file/g;
	    print JSUB $line;
	}
	close(JSUB);
	close(JSUB_TEMPLATE);
	$submit_command = "jsub -xml $jsub_file | perl -n -e 'if(/jsub/) {print;}' | get_job_index.pl";
	$jobIndex = `$submit_command`;
	chomp $jobIndex;
	# print "DEBUG jobIndex = $jobIndex\n";
    } else {
	die "error: jsub file template $jsub_file_template does not exist";
    }
    #print "DEBUG right before return, jobIndex = $jobIndex\n";
    return $jobIndex;
}

sub unsubmit {
    $run = $ARGV[2];
    $file = $ARGV[3];
    print "run = $run, file = $file\n";
    $sql = "SELECT submitted FROM $project WHERE run = $run AND file = $file";
    make_query($dbh_db, \$sth);
    $submitted = 0;
    $nrow = 0;
    while (@column = $sth->fetchrow_array) {
	$nrow++;
	$submitted = $column[0];
    }
    if ($nrow > 1) {die "more than one entry for run/file";}
    if ($submitted != 1) {die "job never submitted or run/file does not exist";}
    $sql = "UPDATE $project SET submitted = 0 WHERE run = $run AND file = $file";
    make_query($dbh_db, \$sth);
}

sub jput {
    $output_dir = $ARGV[2];
    $silo_dir = $ARGV[3];
    $pattern_run_only = $ARGV[4];
    $nfile_max = $ARGV[5];
    if ($silo_dir !~ /\/$/) {
	$silo_dir .= '/';     # add trailing "/"
    }
    if ($pattern_run_only) {
	print "file pattern will include only run number\n";
    }
    $sql = "SELECT run, file FROM $project WHERE submitted = 1 AND output = 1 AND jput_submitted = 0 order by run, file";
    if ($nfile_max) {
	$sql .= " limit $nfile_max"
    }
    make_query($dbh_db, \$sth);
    $nfile = 0;
    while (@column = $sth->fetchrow_array) {
	if ($nfile%100 == 0) {
	    if ($nfile != 0) {
		jput_it();
	    }
	    $command = "cd $output_dir ; jput";
	}
	$run = sprintf("%06d", $column[0]);
	$file = sprintf("%03d", $column[1]);
	if ($pattern_run_only) {
	    $file_pattern = $run;
	} else {
	    $file_pattern = $run . '_' . $file;
	}
	$command .= " \*$file_pattern\*";
	$sql = "UPDATE $project SET jput_submitted = 1 WHERE run = $run AND file = $file";
	make_query($dbh_db, \$sth2);
	$nfile++;
    }
    jput_it();
    print "jput $nfile files\n";
}

sub jput_it {
# called from multiple places, whenever $sql is ready to finish and ship
    $command .= " $silo_dir";
    system $command;
}

sub jcache {
    $silo_dir = $ARGV[2];
    $pattern_run_only = $ARGV[3];
    if ($pattern_run_only ne '') {
	print "file pattern will include only run number\n";
    }
#    $sql = "SELECT run, file FROM $project WHERE submitted = 1 AND silo = 1 AND jcache_submitted = 0";
    $sql = "SELECT run, file FROM $project WHERE submitted=0 AND jcache_submitted=0 order by tape_volser, tape_fileposition";
    make_query($dbh_db, \$sth);
    $num_jobs = 0;
    while (@row = $sth->fetchrow_array) {
        $run_array[$num_jobs] = $row[0];
        $file_array[$num_jobs] = $row[1];
        $num_jobs++;
    }
    # Now, re-arrange jobs such that they are submitted in the optimal order for retrieval from tape:
        # Different tape movers should read data from different tapes, so submit jobs in a jagged order.
    my ($new_run_array_ref, $new_file_array_ref) = rearrange_files(\@run_array, \@file_array, $num_jobs);
    my @new_run_array = @$new_run_array_ref;
    my @new_file_array = @$new_file_array_ref;

    for ($j = 0; $j < $num_jobs; $j++) {
        $run_this = sprintf("%06d", $new_run_array[$j]);
        $file_this = sprintf("%03d", $new_file_array[$j]);
        printf ">>>caching run $run_this file $file_this<<<\n";
	$command = "jcache submit halld ";
	if ($pattern_run_only) {
	    $file_pattern = $run_this;
	} else {
	    $file_pattern = $run_this . '_' . $file_this;
	}
	$command .= " $silo_dir/Run$run_this/\*$file_pattern\*";
	jcache_it();
	$sql = "UPDATE $project SET jcache_submitted=1 WHERE run=$run_this AND file=$file_this";
	make_query($dbh_db, \$sth2);
    }
}

sub jcache_it {
    system $command;
}

sub make_query {    

    my($dbh, $sth_ref) = @_;
    $$sth_ref = $dbh->prepare($sql)
        or die "Can't prepare $sql: $dbh->errstr\n";
    
    $rv = $$sth_ref->execute
        or die "Can't execute the query $sql\n error: $sth->errstr\n";
    
    return 0;

}

sub print_usage {
    print <<EOM;
jproj.pl: manages a JLab batch farm project

usage:

jproj.pl <project> <action> <arg1> <arg2> ...

actions:

create
    arg1: run number
    arg2: number of files in the project
    Note: use run number and number of files only if project is not driven
          by input data files, "update" action will then never be necessary
          for this project

update
    arg1: file number to use; if omitted all file numbers will be used

update_output
    arg1: output link directory
    arg2: if present and non-zero, use only run number in file pattern search

update_silo
    arg1: mss directory
    arg2: if present and non-zero, use only run number in file pattern search

update_cache
    arg1: cache directory
    arg2: if present and non-zero, use only run number in file pattern search

submit
    arg1: limit on number of submissions
    arg2: run choice, submit only this run

unsubmit
    arg1: run number
    arg2: file number

jput
    arg1: output link directory
    arg2: mss directory
    arg3: if present and non-zero, use only run number in file pattern for jput
    arg4: if present and non-zero, jput at most this many files

jcache
    arg1: mss directory
    arg2: if present and non-zero, use only run number in file pattern for jcache
EOM
}
