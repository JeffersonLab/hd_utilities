# Launch scripts for submitting large-scale reconstruction jobs at NERSC

 See more detailed documentation here: <https://halldweb.jlab.org/wiki/index.php/HOWTO_Execute_a_Launch_using_NERSC>.

## Call hierarchy of scripts

```bash
> do_my_launch.sh
  * Generates script_nersc_test.sh and jana_recon_nersc.config from templates
  * Creates and runs swif2 workflow at ifarm
  * Copies job scripts and JANA config to NERSC
  * For each run number: creates and executes exec_<run number>.sh script
  |
 1|-> exec_<run number>.sh
      |
     2|-> swif2 add-job (submits job at ifarm queue) + sbatch (submits job at NERSC queue)
          |
         3|-> @NERSC: script_nersc_multi_test.sh  <--  main job script run at NERSC
              |
             4|-> @NERSC: script_nersc_multi_test.py
                  |
                 5|-> @NERSC: srun run_shifter_multi.sh
                      |
                     6|-> @NERSC: shifter script_nersc_test.sh (run from inside a shifter container)
                          |
                         7|-> @NERSC: hd_root
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

* `/global/cfs/cdirs/m3120/launch-<batch>` directory on NERSC Community File System that contains job scripts and JANA config file(s). Is mapped to `/launch-<batch>` inside the job container.
* `/pscratch/sd/j/jlab/swif` directory on NERSC Scratch Space that serves as root directory for job output (= swif2 site path). Has a quota of 500 TB.
  * `/pscratch/sd/j/jlab/swif/jobs/gxproj4/${SLURM_JOB_NAME}/${SWIF_JOB_ATTEMPT_ID}` top-level working directory the job wakes up in (identical to `${SWIF_JOB_STAGE_DIR}` and `${SWIF_JOB_WORK_DIR}`)
    * `/pscratch/sd/j/jlab/swif/jobs/gxproj4/${SLURM_JOB_NAME}/${SWIF_JOB_ATTEMPT_ID}/subjob????` top-level working directory the container task wakes up in, where `????` is the 4-digit `${SLURM_NODEID}`
    * `/pscratch/sd/j/jlab/swif/jobs/gxproj4/${SLURM_JOB_NAME}/${SWIF_JOB_ATTEMPT_ID}/RUN??????/FILE???` directory linked to task directory above, where `???` is the 3-digit `${SLURM_NODEID}`; `RUN??????/FILE???` is copied back to JLab by swif2
* `${CSCRATCH}}/HALLD_MY`??? <!--TODO-->

### Container image

* `docker:jeffersonlab/gluex_almalinux_9:latest` contains everything to setup GlueX software environment from CVMFS.

### Globus collections

* `2f299200-6b58-4f81-bacd-b947873986db` `NERSC DTN jlab Collab`, for data inbound to and outbound from NERSC.
* `086540a1-9598-445e-8637-ac2f3997874f` `JLAB#SWIF`, for data outbound from JLab.
* `B0fca1ad-f485-4a00-8fcd-bca0b93a2a1c` `jlab#gw1`,  for data inbound to JLab.
