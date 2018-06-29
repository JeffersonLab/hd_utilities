#!/apps/bin/perl -w
#
# purpose: the schema event.xsd and create the two files hits.xml
#          and reaction.xml using schema-hddm
#
# Note:    in order for this script to work schema-hddm and schema-hddm.xsl
#          are needed. Best run this script in ${HALLD_RECON_HOME}/src/programs/Utilities/hddm
#          and copy the two output xml files to ${HALLD_RECON_HOME}/src/libraries/HDDM/
#
# author: B. Zihlmann
#


use strict;
use FileHandle;

my $schemafile = "$ENV{HALLD_RECON_HOME}/src/libraries/HDDM/event.xsd";

if ($#ARGV==0){
    print "Use default schema file: $schemafile\n";
}

my $inf = "/tmp/xml.log";

my $cmd = "schema-hddm $schemafile > $inf";
system($cmd);

my $INF = new FileHandle("<$inf") or
    die "Error open file $inf\n";

my $ofr = "reaction.xml";
my $ofh = "hits.xml";

my $OF;
my $open = 0;
my $line1 = $INF->getline();
$line1 = "\<\?xml version=\"1.0\" encoding=\"UTF-8\"\?\>";

while (<$INF>){

    if (/\<reaction/){
	$OF = new FileHandle(">$ofr") or
	    die "Error open file $ofr\n";
	print $OF $line1;
	print $OF $_;
	$open=1;
    } elsif (/\<\/reaction/){
	print $OF $_;
	$OF->close();
	$open=0;
    } elsif (/\<hitView/){
	$OF = new FileHandle(">$ofh") or
	    die "Error open file $ofr\n";
	print $OF $line1;
	print $OF $_;
	$open=1;
    } elsif (/\<\/hitView/){
	print $OF $_;
	$OF->close();
	$open=0;
    } elsif ($open){
	print $OF $_;
    }

}
$INF->close();
$OF->close();

