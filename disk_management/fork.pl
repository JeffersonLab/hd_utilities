#!/usr/bin/env perl
use strict;
use warnings;

$SIG{INT} = $SIG{TERM} = sub { exit };

my $max_workers = shift || 1;
my $parent_pid = "$$";
my @dirs = ();
my @ndirs = ();
my ($ndir, $iworker);
foreach (0 .. $max_workers - 1) {
    $iworker = $_;
    print "opening file $iworker\n";
    open(DIRLIST, "dir_list_${iworker}.txt");
    $ndir = 0;
    while(<DIRLIST>) {
	print;
	chomp;
	$dirs[$iworker][$ndir] = $_; 
	$ndir++;
    }
    $ndirs[$iworker] = $ndir;
    close(DIRLIST);
}

my @children;
my $ichild = 0;
for (1..$max_workers) {
    my $pid = fork;
    if (!defined $pid) {
	warn "failed to fork: $!";
	kill 'TERM', @children;
	exit;
    }
    elsif ($pid) {
	push @children, $pid;
	$ichild++;
	next;
    }
    execute_payload();
    exit;
}
wait_children();

sub execute_payload {
    print "starting $ichild\n";
    my $dir;
    foreach (my $i = 0; $i < $ndirs[$ichild]; $i++) {
	print "$ichild $i ./disk_database.pl -a $dirs[$ichild][$i] marki_partial\n";
	system "./disk_database.pl -a $dirs[$ichild][$i] marki_partial";
    }
    print "ending $ichild\n";
}

sub wait_children {
    while (scalar @children) {
	my $pid = $children[0];
	my $kid = waitpid $pid, 0;
	warn "Reaped $pid ($kid)\n";
	shift @children;
    }
}

END {
    if ($parent_pid == $$) {
	wait_children();
    }
}
