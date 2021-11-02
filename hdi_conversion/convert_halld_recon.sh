#!/bin/bash
hd_interface=/home/marki/git/hd_interface
rm -rf halld_recon cc_to_move.txt h_to_move.txt
#git clone git@github.com:jeffersonlab/halld_recon
cp -pr halld_recon.save halld_recon # instead of clone for testing to save time
cd halld_recon
echo git filter-repo \\ > ../command.sh
awk '{print "    --path "$1" \\"}' < $hd_interface/conversion/filter.txt >> ../command.sh
echo >> ../command.sh
. ../command.sh
git branch| awk '{print "git branch -D "$1}' | bash
mkdir old
git mv src old
find $hd_interface -type f -name \*.cc > ../cc_to_move.txt
find $hd_interface -type f -name \*.h > ../h_to_move.txt
git add .
git commit -m "move done"
git rm -r old
git add .
git commit -m "old deleted"
git status
