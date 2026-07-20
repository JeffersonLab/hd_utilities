#!/usr/bin/env bash

if [ -d /group ]; then
  echo "I am running in singularity."
else
  echo "Something went wrong with setting up singularity! Abort."
  exit 9
fi

VERSION=version.xml

CMDLINE_ARGS=""
# prepare command line arguments for evaluation
#for arg in "$@"; do
while [[ $# -gt 0 ]]; do
    key="$1"
    case $key in
	-v|--version)
	    VERSION="$2"
	    shift # past argument
	    shift # past value
	    ;;
	-s|--sqlite)
	    SQLITE=true
	    shift # past argument
	    ;;
	*)
	    CMDLINE_ARGS="${CMDLINE_ARGS} $1"
	    shift # past argument
	    ;;
    esac
done

if [ -e /group/halld/www/halldweb/html/halld_versions/"$VERSION" ] ; then
    echo "source /group/halld/Software/build_scripts/gluex_env_jlab.sh /group/halld/www/halldweb/html/halld_versions/"$VERSION
    source /group/halld/Software/build_scripts/gluex_env_jlab.sh /group/halld/www/halldweb/html/halld_versions/$VERSION
else
    echo $VERSION "does not exist! Abort."
    exit 9
fi

if [ "$SQLITE" = true ] ; then
    export JANA_CALIB_URL=sqlite:///$PWD/ccdb.sqlite
    export CCDB_CONNECTION=sqlite:///$PWD/ccdb.sqlite
fi

printenv

echo $CMDLINE_ARGS
$CMDLINE_ARGS
