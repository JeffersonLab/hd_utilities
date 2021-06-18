#!/usr/bin/perl

use Cwd;

$lowMass = 0.7;
$highMass = 3.0;
$nBins = 45;
$lowtMass = 0;
$hightMass = 1.2;
$ntBins = 4;

$fitName = "etaprimepi0";

# put a limit on the number of data events to process
# gen MC and acc MC smaples are not limited
$maxEvts = 1E9;

# this directory can be adjusted if you want to do the fit elsewhere
# but it needs to be an explicit path
$workingDir = getcwd();

# these files must exist in the working directory.  If you don't know how
# to generate them or don't have them, see the documentation in gen_3pi
# the Simulation area of the repository
$dataFile = "$workingDir/data_newhalldsim_SPD.root";
$accMCFile = "$workingDir/flat_newhalldsim.root";
$genMCFile = "$workingDir/flat_newhalldsim.root";

# this file sould be used for partially polarized or unpolarized beam fits
#$cfgTempl = "$workingDir/threepi_unpol_TEMPLATE.cfg";

# this file should be used when there is 100% beam polarization
$cfgTempl = "$workingDir/Mar_fit_etaprime_polarized.cfg";


### things below here probably don't need to be modified

# this is where the goodies for the fit will end up
$fitDir = "$workingDir/$fitName/";
mkdir $fitDir unless -d $fitDir;

chdir $fitDir;

# use the split_mass command line tool to divide up the
# data into bins of resonance mass

@dataParts = split /\//, $dataFile;
$dataTag = pop @dataParts;
$dataTag =~ s/\.root//;
system( "split_mass_t $dataFile $dataTag $lowMass $highMass $nBins $lowtMass $hightMass $ntBins  $maxEvts" );

@accMCParts = split /\//, $accMCFile;
$accMCTag = pop @accMCParts;
$accMCTag =~ s/\.root//;
system( "split_mass_t $accMCFile $accMCTag $lowMass $highMass $nBins  $lowtMass $hightMass $ntBins" );

@genMCParts = split /\//, $genMCFile;
$genMCTag = pop @genMCParts;
$genMCTag =~ s/\.root//;
system( "split_mass_t $genMCFile $genMCTag $lowMass $highMass $nBins $lowtMass $hightMass $ntBins" );



# make directories to perform the fits in
for( $i = 0; $i < $nBins; ++$i ){
for( $j = 0; $j < $ntBins; ++$j ){   

  mkdir "bin\_$i\_$j" unless -d "bin\_$i\_$j";
 
  system( "mv *\_$i\_$j.root bin\_$i\_$j" );

  chdir "bin\_$i\_$j";

  open( CFGOUT, ">bin\_$i\_$j.cfg" );
  open( CFGIN, $cfgTempl );

  while( <CFGIN> ){

    s/DATAFILE/$dataTag\_$i\_$j.root/;
    s/ACCMCFILE/$accMCTag\_$i\_$j.root/;
    s/GENMCFILE/$genMCTag\_$i\_$j.root/;
    s/NIFILE/bin\_$i\_$j.ni/;
    s/FITNAME/bin\_$i\_$j/;

    print CFGOUT $_;
  }

  close CFGOUT;
  close CFGIN;
  
  system( "touch param_init.cfg" );

  chdir $fitDir;
}
}
