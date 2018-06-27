EXAMPLE_SPLIT_DIR = $(HD_UTILITIES_HOME)/split_sim-recon/example
SIM_REPO_DIR = gluex_sim
PWD=`pwd`
DIRECTORIES_TO_BE_MOVED = src/programs/Simulation src/plugins/Simulation \
	src/libraries/AMPTOOLS_AMPS src/libraries/AMPTOOLS_DATAIO \
	src/libraries/AMPTOOLS_MCGEN

all: .recon_remove_dirs .sim_push_done

.sim-recon_clone_done:
	git clone --bare ssh://git@github.com/jeffersonlab/sim-recon
	date >$@

.recon_push_done: .sim-recon_clone_done
	cd sim-recon.git ; \
	    git push --mirror git@github.com:jeffersonlab/gluex_recon.git
	date > $@

.recon_remove_dirs: .recon_push_done
	rm -rf gluex_recon
	git clone ssh://git@github.com/jeffersonlab/gluex_recon
	cd gluex_recon ; git rm -r $(DIRECTORIES_TO_BE_MOVED)
	cd gluex_recon ; git commit -m "sim directories removed"
	cd gluex_recon ; git push
	date > $@

.sim_clone_done: .sim-recon_clone_done
	git clone file://$(PWD)/sim-recon $(SIM_REPO_DIR)
	date > $@

.filter_done: .sim_clone_done
	pushd $(SIM_REPO_DIR) ; git filter-branch --index-filter 'git rm --cached -qr --ignore-unmatch -- . && git reset -q $(GIT_COMMIT) -- $(DIRECTORIES_TO_BE_MOVED)' --prune-empty -- --all
	date > $@

.copy_done: .filter_done
	cp -v $(EXAMPLE_SPLIT_DIR)/gluex_sim/src/SConstruct $(SIM_REPO_DIR)/src
	cp -rv $(EXAMPLE_SPLIT_DIR)/gluex_sim/src/SBMS $(SIM_REPO_DIR)/src
	cp $(EXAMPLE_SPLIT_DIR)/gluex_sim/src/libraries/SConscript $(SIM_REPO_DIR)/src/libraries
	cp $(EXAMPLE_SPLIT_DIR)/gluex_sim/src/programs/SConscript $(SIM_REPO_DIR)/src/programs
	cp $(EXAMPLE_SPLIT_DIR)/gluex_sim/src/plugins/SConscript $(SIM_REPO_DIR)/src/plugins
	date > $@

.commit_done: .copy_done
	cd $(SIM_REPO_DIR) ; git add .
	cd $(SIM_REPO_DIR) ; \
	    git commit -m "commit files added from examples directory"
	date > $@

.sim_push_done: .commit_done
	cd $(SIM_REPO_DIR) ; \
	    git push --mirror git@github.com:jeffersonlab/gluex_sim.git
	date > $@

.scons_done: .copy_done
	pushd $(SIM_REPO_DIR)/src ; scons install
	date > $@

clean:
	rm -rfv gluex_sim .*_done
