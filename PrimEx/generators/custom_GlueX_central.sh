source bashrc

# source GlueX software
export BUILD_SCRIPTS=/group/halld/Software/build_scripts
source $BUILD_SCRIPTS/gluex_env_boot_jlab.sh

#source $DEV/gluex_root_analysis/env_analysis.sh
# SET SOFTWARE VERSIONS/PATHS (e.g. $ROOTSYS, $CCDB_HOME, etc.)
export GLUEX_VERSION_XML=/group/halld/www/halldweb/html/halld_versions/version_4.23.0.xml

# SET FULL ENVIRONMENT
source $BUILD_SCRIPTS/gluex_env_jlab.sh $GLUEX_VERSION_XML


