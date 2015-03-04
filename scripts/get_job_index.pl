#!/usr/bin/env perl

use XML::Parser;

my $p = new XML::Parser(Style=>'Stream');

#$p->parsefile($ARGV[0]);
$p->parse(<>);

exit;

sub StartTag {
    my ($expat, $eltype) = @_;
    if ($eltype eq 'jobIndex') {
	$get_jobIndex = 1;
    }
}

sub EndTag {
    my ($expat, $eltype) = @_;
    if ($eltype eq 'jobIndex') {
	$get_jobIndex = 0;
    }
}

sub Text {
    my $text = $_;
    if ($get_jobIndex) {
	chomp $text;
	print "$text\n";
    }
    return;
}
