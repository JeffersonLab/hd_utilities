#!/bin/bash

function show_help
{
    cat <<EOF
Description:

Run b1pi_test.sh for a particular configuration of the GlueX software. The environment can be set in advance or a version.xml file can be supplied. In the latter case the file will be used to set-up the GlueX environment.

Usage:

  my_b1pi_test.sh [-n <number of events>] [-t <number of threads>] \\
    [-v <vertex string>] [-d <b1pi_test script directory>] \\
    [-v <version.xml file>]

Example:

  mkdir my_test_dir # make a test directory
  cd my_test_dir # cd into it
  export GLUEX_TOP /home/username/gluex_top # define location of the top of the 
                                            # GlueX software tree
  export B1PI_TEST_DIR /home/username/b1pi_test # define the location of the
                                                # b1pi_test files
  my_b1pi_test.sh -n 10000 -f /home/username/my_version.xml # do the test for
                                                            # 10 k events using
                                                            # versions in
                                                            # my_version.xml
EOF
}

b1pi_test_options=""

while getopts "h?v:f:n:t:d:s:" opt; do
    case "$opt" in
    h|\?)
        show_help
        exit 0
        ;;
    f)  version_file=$OPTARG
        ;;
    n)  NEVENTS=$OPTARG
	;;
    v)  VERTEX=$OPTARG
	;;
    t)  NTHREADS=$OPTARG
	;;
    d)  B1PI_TEST_DIR=$OPTARG
        b1pi_test_options="$b1pi_test_options -d $B1PI_TEST_DIR"
	;;
    s)  SEED=$OPTARG
    esac
done

shift $((OPTIND-1))

[ "$1" = "--" ] && shift

if [ ! -d "$B1PI_TEST_DIR" ]
    then
    echo "error: location of scripts and macros not found"
    echo "       location tried = \"$B1PI_TEST_DIR\""
    exit 1
fi

if [ ! -z "$NEVENTS" ]
    then
    b1pi_test_options="$b1pi_test_options -n $NEVENTS"
fi

if [ ! -z "$NTHREADS" ]
    then
    b1pi_test_options="$b1pi_test_options -t $NTHREADS"
fi

if [ ! -z "$VERTEX" ]
    then
    b1pi_test_options="$b1pi_test_options -v $VERTEX"
fi

if [ -z "$version_file" ]
    then
    echo "info from my_b1pi_test.sh: no version.xml specified, will rely on existing GlueX environment"
    if ! which genr8
	then
	echo "error in my_b1pi_test.sh: genr8 not in path"
	exit 1
    fi
else
    if [ ! -d "$BUILD_SCRIPTS" ]
	then
	echo "error in my_b1pi_test.sh: BUILD_SCRIPTS not found"
	echo "    using BUILD_SCRIPTS = \"$BUILD_SCRIPTS\""
	exit 2
    fi
    source $BUILD_SCRIPTS/gluex_env_version.sh $version_file    
fi

if [ ! -z "$SEED" ]
    then
    b1pi_test_options="$b1pi_test_options -s $SEED"
fi

if ! which genr8
    then
    echo "error in my_b1pi_test.sh: environment setting failed, genr8 not in path"
    exit 3
fi

command="$B1PI_TEST_DIR/b1pi_test.sh $b1pi_test_options"
echo "info from my_b1pi_test.sh: running test, command = \"$command\""
$command
