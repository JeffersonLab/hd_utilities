#!/bin/bash
#
# Make a new version of halld_recon that has only source code that
# appears in hd_interface. All other source code is deleted. The
# resulting repo can be used as a basis for a new hd_interface with
# up-to-date versions of the code in halld_recon.
#
# specify the hd_interface clone
hd_interface=/home/marki/git/hd_interface
# specify the hdi_conversion directory of hd_utilities
hdi_conversion=/home/marki/git/hd_utilities/hdi_conversion
# specify where the halld_recon clone will go
repo_dir=hdi_interface_from_hdr
# clone halld_recon
rm -rf $repo_dir
git clone -b for_hdi_conversion git@github.com:markito3/halld_recon $repo_dir
# go to the halld_recon clone
cd $repo_dir
#
# Write ../command.sh which contains a "git filter-repo" command to
# drop all directories except those that contain code destined for
# hd_interface. See https://github.com/newren/git-filter-repo .
#
# add the git filter-repo command to command.sh
rm -f ../command.sh
echo git filter-repo \\ > ../command.sh
# add directories to be passed through filter to command.sh
awk '{print "    --path "$1" \\"}' < $hdi_conversion/filter.txt \
    >> ../command.sh
# add a blank line to command.sh
echo >> ../command.sh
# execute command.sh, i.e., do the git filter-repo
. ../command.sh
# delete all branches
git branch| awk '{print "git branch -D "$1}' | bash | grep -v "Deleted branch"
# move the "src" directory to "old_src", git-wise
git mv src old_src
git commit -m "src moved to old_src" | grep -v "rename"
# move *.cc files, present in hd_interface,  from old_src to src, git-wise
find $hd_interface -type f -name \*.cc | $hdi_conversion/move_cc.sh
# move *.h files, present in hd_interface,  from old_src to src, git-wise
find $hd_interface -type f -name \*.h | $hdi_conversion/move_h.sh
exit # for test, do not go further
# commit the changes
git commit -m "move done" | grep -v "rename"
# remove the old_src directory
git rm -r old_src | grep -v "rm "
# commit the removal
git commit -m "old_src deleted" | grep -v "delete mode"
# look at status
git status
