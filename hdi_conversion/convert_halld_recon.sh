#!/bin/bash
hd_interface=/home/marki/git/hd_interface # specify the hd_interface clone
hdi_conversion=/home/marki/git/hd_utilities/hdi_conversion # specify the hdi_conversion directory of the hd_utilities clone
repo_dir=hdi_interface_from_hdr # name the halld_recon clone
rm -rf $repo_dir command.sh # clear the decks
git clone -b for_hdi_conversion git@github.com:markito3/halld_recon $repo_dir # clone halld_recon
#cp -pr halld_recon.save halld_recon # instead of clone for testing to save time
cd $repo_dir # enter the halld_recon clone
echo git filter-repo \\ > ../command.sh # add the git filter-repo command to command.sh
awk '{print "    --path "$1" \\"}' < $hdi_conversion/filter.txt >> ../command.sh # add directories to be passed through filter to command.sh
echo >> ../command.sh # display command.sh
. ../command.sh # execute command.sh, i.e., git filter-repo
git branch| awk '{print "git branch -D "$1}' | bash | grep -v "Deleted branch"
git mv src old_src
git commit -m "src moved to old_src" | grep -v "rename"
find $hd_interface -type f -name \*.cc | $hdi_conversion/move_cc.sh
find $hd_interface -type f -name \*.h | $hdi_conversion/move_h.sh
exit
git commit -m "move done" | grep -v "rename"
git rm -r old_src | grep -v "rm "
git commit -m "old_src deleted" | grep -v "delete mode"
git status
