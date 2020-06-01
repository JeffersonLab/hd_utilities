source /group/halld/Software/build_scripts/gluex_env_jlab.csh /group/halld/www/halldweb/html/halld_versions/version_4.20.0.xml

setenv JANA_CALIB_CONTEXT variation=mc

hd_root --config=/group/halld/www/halldweb/html/data_monitoring/pid_syst/jana_pid.config /cache/halld/gluex_simulations/REQUESTED_MC/pi0_jrs_2017_20200215072102am/hddm/dana_rest_gen_pi0_*
