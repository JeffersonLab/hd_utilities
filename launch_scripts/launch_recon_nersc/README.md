# Launch scripts for submitting large-scale reconstruction jobs at NERSC

 See more detailed documentation here: <https://halldweb.jlab.org/wiki/index.php/HOWTO_Execute_a_Launch_using_NERSC>.

## Call hierarchy of scripts

```bash
> do_my_launch.sh
  |
  |-> my_launch.sh
      * generates script_nersc_test.sh and jana_recon_nersc.config from templates
      * creates and runs swif2 workflow at ifarm
      * copies job scripts to NERSC
      * creates and executes exec_<run number>.sh scripts for each run number
      |
      |-> exec_<run number>.sh
          |
          |-> swif2 add-job (submits job at ifarm queue) + sbatch (submits job at NERSC queue)
              |
              |-> @NERSC: script_nersc_multi_test.sh
                  |
                  |-> @NERSC: script_nersc_multi_test.py
                      |
                      |-> @NERSC: srun run_shifter_multi.sh
                          |
                          |-> @NERSC: shifter script_nersc_test.sh (run from inside a shifter container)
                              |
                              |-> @NERSC: hd_root
```

## Used resources

* NERSC login host `perlmutter-p1.nersc.gov`.

### File system paths

#### ifarm

* `.` the name of the current directory is expected to have the form `launch-<batch>`, where `batch=<recon version>-<site>`, e.g. `launch-03-perl`.
* `/mss/halld/RunPeriod-<run period>/rawdata/Run<run number>` directory with input `.evio` files for given run number, e.g. `/mss/halld/RunPeriod-2025-01/rawdata/Run132313`.
* `/lustre/expphy/volatile/halld/offsite_prod/RunPeriod-<run period>/recon/<version>/RUN<6-digit run number>/FILE<3-digit file number>` output directory for given run and `.evio` file number.
* `/group/halld/Software/builds/Linux_Alma9-x86_64-gcc11.5.0-cntr/${HALLD_RECON_VERSION}/Linux_Alma9-x86_64-gcc11.5.0-cntr/setenv.sh` mounted inside the container from CVMFS.<!--TODO-->
* `/group/halld/www/halldweb/html/dist/{ccdb,rcdb}.sqlite` mounted inside the container from ???.<!--TODO-->
* `/group/halld/www/halldweb/html/resources` JANA resource directory.<!--TODO-->

#### NERSC

* `/global/cfs/cdirs/m3120/launch-<batch>` root directory for jobs at NERSC. Contains job scripts and JANA config file(s). Is mapped to `/launch-<batch>` inside the job containers.
* `/pscratch/sd/j/jlab/swif` output directory for jobs (= swif2 site path).
* `${CSRATCH}`??? <!--TODO-->

### Container image

* `docker:jeffersonlab/gluex_almalinux_9:latest` contains everything to setup GlueX software environment from CVMFS.
