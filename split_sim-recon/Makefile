EXAMPLE_SPLIT_DIR = $(HD_UTILITIES_HOME)/split_sim-recon/example
SIM_REPO_DIR = gluex_sim
RECON_REPO_DIR = gluex_recon
PWD=`pwd`
DIRECTORIES_TO_BE_MOVED = src/programs/Simulation \
	src/programs/AmplitudeAnalysis src/plugins/Simulation \
	src/libraries/AMPTOOLS_AMPS src/libraries/AMPTOOLS_DATAIO \
	src/libraries/AMPTOOLS_MCGEN

all: .recon_push_done .sim_push_done

.sim-recon_clone_done:
	git clone --bare ssh://git@github.com/jeffersonlab/sim-recon
	date >$@

.recon_bare_push_done: .sim-recon_clone_done
	cd sim-recon.git ; \
	    git push --mirror git@github.com:jeffersonlab/gluex_recon.git
	date > $@

.recon_clone_done: .recon_bare_push_done
	rm -rf $(RECON_REPO_DIR)
	git clone ssh://git@github.com/jeffersonlab/gluex_recon

.recon_modify_done: .recon_clone_done
	cp -rv $(EXAMPLE_SPLIT_DIR)/gluex_recon/src gluex_recon
	cd $(RECON_REPO_DIR) ; git rm -r $(DIRECTORIES_TO_BE_MOVED)

.recon_commit_done: .recon_modify_done
	cd $(RECON_REPO_DIR) ; git add .
	cd $(RECON_REPO_DIR) ; \
	    git commit -m "gluex_sim modified from sim-recon original"

.recon_push_done: .recon_commit_done
	cd $(RECON_REPO_DIR) ; git push
	date > $@

.sim_clone_done: .sim-recon_clone_done
	git clone file://$(PWD)/sim-recon $(SIM_REPO_DIR)
	date > $@

.sim_filter_done: .sim_clone_done
	pushd $(SIM_REPO_DIR) ; git filter-branch --index-filter 'git rm --cached -qr --ignore-unmatch -- . && git reset -q $(GIT_COMMIT) -- $(DIRECTORIES_TO_BE_MOVED)' --prune-empty -- --all
	date > $@

.sim_copy_done: .sim_filter_done
	cp -rv $(EXAMPLE_SPLIT_DIR)/gluex_sim/src $(SIM_REPO_DIR)
	date > $@

.sim_commit_done: .sim_copy_done
	cd $(SIM_REPO_DIR) ; git add .
	cd $(SIM_REPO_DIR) ; \
	    git commit -m "commit files added from examples directory"
	date > $@

.sim_push_done: .sim_commit_done
	cd $(SIM_REPO_DIR) ; \
	    git push --mirror git@github.com:jeffersonlab/gluex_sim.git
	date > $@

.scons_done: .copy_done
	pushd $(SIM_REPO_DIR)/src ; scons install
	date > $@

clean:
	rm -rfv gluex_sim .*_done
