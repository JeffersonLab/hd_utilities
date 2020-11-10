#!/usr/bin/perl

use Cwd;

# be sure that these settings agree with what was used in the divideData script
$nMBins = 45;
$ntBins = 4;
$fitName = "etaprimepi0";

# this directory can be adjusted if you want to do the fit elsewhere
# but it needs to be an explicit path
$workingDir = getcwd();

# this is the name of the file that will be used to store values used
# to see the parameters inthe fit
$seedFile = "param_init.cfg";

### things below here probably don't need to be modified

$fitDir = "$workingDir/$fitName";
$lastParams = "$fitDir/$seedFile";

for( $i = 0; $i < $nMBins; ++$i ){
    for( $j = 0; $j < $ntBins; ++$j ){
  
  chdir $fitDir;
  chdir "bin\_$i\_$j";
  
  print "Fitting in bin $i\_$j...\n";
  
  system( "fit -c bin\_$i\_$j.cfg -s $seedFile > bin\_$i\_$j.log" );
  
  if( -e "$seedFile" ){ system( "cp -f $seedFile .." ); }
  
  chdir $fitDir;
    }
}

