#!/bin/tcsh -e

# Usage:
# create_cross_analysis_table.sh [run period] [version]

# initialize
set RUNPERIOD = ""
set VERSION   = ""

# read in arguments
set RUNPERIOD = $1
set VERSION   = $2

if ( $RUNPERIOD == "" || $VERSION == "" ) then

  echo "Usage:"
  echo "create_cross_analysis_table.sh [run period] [version]"
  exit
endif

# Make sure RUNPERIOD uses underscore, not hyphen
# (MySQL does not allow hyphens in table names)
set RUNPERIOD = `echo $RUNPERIOD | sed 's:-:_:'`

set TABLENAME = "cross_analysis_table_${RUNPERIOD}_ver${VERSION}"

# If table already exists, ask if overwrite
# Count how many tables have the name TABLENAME, ask to overwrite if > 0
set EXISTS = `mysql -hhallddb -ufarmer farming -e "SHOW TABLES LIKE '"${TABLENAME}"'" | wc -l`
if ( $EXISTS > 0 ) then
  while ( 1 )
    echo "overwrite table ${TABLENAME} ? (y/n)"
    set ANSWER = $<
    if ( $ANSWER == "n" ) then
      echo "exiting create_cross_analysis_table.sh"
      exit
    else if ( $ANSWER == "y" ) then
      # echo "dropping table ${TABLENAME}..."
      mysql -hhallddb -ufarmer farming -e "DROP TABLE IF EXISTS $TABLENAME"
      break
    endif
  end
endif

# Create table schema from template
cat template_table_schema.sql | sed "s:TABLENAME:${TABLENAME}:" > table_schema_${RUNPERIOD}_${VERSION}.sql

# Create table
mysql -hhallddb -ufarmer farming < table_schema_${RUNPERIOD}_${VERSION}.sql
rm -f table_schema_${RUNPERIOD}_${VERSION}.sql

echo "created table ${TABLENAME}..."
