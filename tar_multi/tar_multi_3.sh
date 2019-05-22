#! /bin/bash
# For this script it's advisable to use a shell, such as Bash,
# that supports a TAR_FD value greater than 9.
#
# Usage example:
# archive:
#
# tar cvf /scratch/Music.tar -F /home/marki/bin/tar_multi_2.sh --multi-volume \
#     -L 2G /home/marki/Music
#
# restore:
#
# tar xvf /scratch/Music.tar -F /home/marki/bin/tar_multi_2.sh --multi-volume
#

echo Preparing volume $TAR_VOLUME of $TAR_ARCHIVE.

name=`expr $TAR_ARCHIVE : '\(.*\):.*'`
case $TAR_SUBCOMMAND in
-c)       ;;
-d|-x|-t) test -r ${name:-$TAR_ARCHIVE}:$TAR_VOLUME || exit 1
          ;;
*)        exit 1
esac

echo ${name:-$TAR_ARCHIVE}:$TAR_VOLUME >&$TAR_FD
