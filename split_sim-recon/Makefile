EXAMPLE_SPLIT_DIR = $(HD_UTILITIES_HOME)/split_sim-recon/example
SIM_REPO_DIR = gluex_sim
PWD=`pwd`

all: .scons_done

.sim-recon_clone_done:
	git clone ssh://git@github.com/jeffersonlab/sim-recon
	date >$@

.sim_clone_done: .sim-recon_clone_done
	git clone file://$(PWD)/sim-recon $(SIM_REPO_DIR)
	date > $@

version_split.xml:
	$(BUILD_SCRIPTS)/customize_version.pl -i /group/halld/www/halldweb/html/dist/version_jlab.xml -o version_split.xml -s $(PWD)/sim-recon -4 $(PWD)/hdgeant4 -a $(PWD)/gluex_root_analysis

.scons_sim-recon_done: version_split.xml .sim-recon_clone_done
	. $(BUILD_SCRIPTS)/gluex_env_jlab.sh version_split.xml ; \
	cd sim-recon/src ; \
	scons install $(SIM_RECON_SCONS_OPTIONS)

.filter_done: .sim_clone_done
	pushd $(SIM_REPO_DIR) ; git filter-branch --index-filter 'git rm --cached -qr --ignore-unmatch -- . && git reset -q $(GIT_COMMIT) -- src/programs/Simulation src/plugins/Simulation src/libraries/AMPTOOLS_AMPS src/libraries/AMPTOOLS_DATAIO src/libraries/AMPTOOLS_MCGEN' --prune-empty -- --all
	date > $@

.copy_done: .filter_done
	cp -v $(EXAMPLE_SPLIT_DIR)/src/SConstruct $(SIM_REPO_DIR)/src
	cp -rv $(EXAMPLE_SPLIT_DIR)/src/SBMS $(SIM_REPO_DIR)/src
	cp $(EXAMPLE_SPLIT_DIR)/src/libraries/SConscript $(SIM_REPO_DIR)/src/libraries
	cp $(EXAMPLE_SPLIT_DIR)/src/programs/SConscript $(SIM_REPO_DIR)/src/programs
	cp $(EXAMPLE_SPLIT_DIR)/src/plugins/SConscript $(SIM_REPO_DIR)/src/plugins
	date > $@

.scons_done: .scons_sim-recon_done .copy_done
	. $(BUILD_SCRIPTS)/gluex_env_jlab.sh version_split.xml ; \
	cd $(SIM_REPO_DIR)/src ; \
	scons install

clean:
	rm -rfv gluex_sim .*_done
