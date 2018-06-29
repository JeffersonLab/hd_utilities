EXAMPLE_SPLIT_DIR = $(HD_UTILITIES_HOME)/split_sim-recon/example
PWD=`pwd`
SIM_REPO_DIR = $(PWD)/halld_sim
RECON_REPO_DIR = $(PWD)/halld_recon
SETUP_GLUEX = source $(BUILD_SCRIPTS)/gluex_env_jlab.sh ; unset HALLD_HOME
DIRECTORIES_TO_BE_MOVED = src/programs/Simulation \
	src/programs/AmplitudeAnalysis src/plugins/Simulation \
	src/libraries/AMPTOOLS_AMPS src/libraries/AMPTOOLS_DATAIO \
	src/libraries/AMPTOOLS_MCGEN

all: .sim_scons_done

no_build: .recon_push_done .sim_push_done

.sim-recon_clone_done:
	git clone --bare ssh://git@github.com/jeffersonlab/sim-recon
	date >$@

.recon_bare_push_done: .sim-recon_clone_done
	cd sim-recon.git ; \
	    git push --mirror git@github.com:jeffersonlab/halld_recon.git
	date > $@

.recon_clone_done: .recon_bare_push_done
	rm -rf $(RECON_REPO_DIR)
	git clone ssh://git@github.com/jeffersonlab/halld_recon
	date > $@

.recon_modify_done: .recon_clone_done
	cp -r $(EXAMPLE_SPLIT_DIR)/halld_recon/src halld_recon
	cd $(RECON_REPO_DIR) ; git rm -rq $(DIRECTORIES_TO_BE_MOVED)
	date > $@

.recon_commit_done: .recon_modify_done
	cd $(RECON_REPO_DIR) ; git add .
	cd $(RECON_REPO_DIR) ; \
	    git commit -q -m "halld_sim modified from sim-recon original"
	date > $@

.recon_push_done: .recon_commit_done
	cd $(RECON_REPO_DIR) ; git push
	date > $@

.recon_scons_done: .recon_push_done
	$(SETUP_GLUEX) ; \
	    echo $(HALLD_HOME) ; \
	    cd $(RECON_REPO_DIR)/src ; \
	    scons install -j3
	date > $@

.sim_clone_done: .sim-recon_clone_done
	git clone file://$(PWD)/sim-recon $(SIM_REPO_DIR)
	date > $@

.sim_filter_done: .sim_clone_done
	pushd $(SIM_REPO_DIR) ; git filter-branch --index-filter 'git rm --cached -qr --ignore-unmatch -- . && git reset -q $(GIT_COMMIT) -- $(DIRECTORIES_TO_BE_MOVED)' --prune-empty -- --all
	date > $@

.sim_copy_done: .sim_filter_done
	cp -rv $(EXAMPLE_SPLIT_DIR)/halld_sim/src $(SIM_REPO_DIR)
	date > $@

.sim_commit_done: .sim_copy_done
	cd $(SIM_REPO_DIR) ; git add .
	cd $(SIM_REPO_DIR) ; \
	    git commit -m "commit files added from examples directory"
	date > $@

.sim_push_done: .sim_commit_done
	cd $(SIM_REPO_DIR) ; \
	    git push --mirror git@github.com:jeffersonlab/halld_sim.git
	date > $@

.sim_scons_done: .sim_push_done .recon_scons_done
	$(SETUP_GLUEX) ; \
	    export HALLD_RECON_HOME=$(RECON_REPO_DIR) ; \
	    export HALLD_SIM_HOME=$(SIM_REPO_DIR) ; \
	    cd $(SIM_REPO_DIR)/src ; \
	    scons install SHOWBUILD=1
#	    scons install -j3
	date > $@

clean:
	rm -rf halld_sim halld_recon sim-recon.git .*_done
