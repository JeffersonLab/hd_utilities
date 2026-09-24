# Launch scripts for submitting large-scale Hall-D reconstruction jobs at NERSC

See more detailed documentation here: <https://halldweb.jlab.org/wiki/index.php/HOWTO_Execute_a_Launch_using_NERSC>.

## Call hierarchy of scripts

```
> submit_launch.py + environment file (example: launch.env)
  * Copies job scripts and JANA config to NERSC
  * Creates and runs swif2 workflow at JLab Farm
  * For each run number: creates and executes submit_job_for_RUN<run number>.sh script
  |
 1|-> submit_job_for_RUN<run number>.sh  <--  submits job
      |
     2|-> swif2 add-job (submits job to JLab Farm queue) -sbatch (submits job to NERSC queue)
          |
         3|-> @NERSC: script_job_wrapper.sh  <--  sets up Python environment
              |
             4|-> @NERSC: script_job.py  <--  job script; prepares work directories and starts tasks
                  |
                 5|-> @NERSC: srun shifter script_task.sh  <-- runs task script in a container on several NERSC nodes
                      |
                     6|-> @NERSC: script_task.sh <-- task script; runs multiple hd_root processes in parallel on a node
                          |
                         7|-> @NERSC: hd_root <-- processes data
```

## Used resources

* NERSC login host `perlmutter-p1.nersc.gov`.

### File system paths

#### JLab Farm

* `~gxproj4/NERSC/<start date>.recon.<run period>_<recon version>-perl/launch.<run period>_<recon version>-perl` directory with production scripts, e.g. `~gxproj4/NERSC/2026-05-31.recon.2021-11_ver05-perl/launch.2021-11_ver05-perl`.
* `/mss/halld/RunPeriod-<run period>/rawdata/Run<run number>` directory with input `.evio` files for given run number, e.g. `/mss/halld/RunPeriod-2025-01/rawdata/Run132313`.
* `/mss/halld/RunPeriod-<run period>/recon/<recon version>` final destination directory for all production output; each output type is collected in a separate subdirectory: `<output type>/??????/<output file>`, where `??????` is the 6-digit run number
* `/lustre/expphy/volatile/halld/offsite_prod/RunPeriod-<run period>/recon/` directory used for temporary storage of NERSC output for postprocessing
* Inside the software container that is run at NERSC, the `/group/halld/` tree is mounted via CVMFS; the following directories and files are used
  * `/group/halld/Software/build_scripts/gluex_env_boot_jlab.sh`
  * `/group/halld/www/halldweb/html/halld_versions`
  * `/group/halld/www/halldweb/html/dist/{ccdb,rcdb}.sqlite`
  * `/group/halld/www/halldweb/html/resources`

#### NERSC

* `/global/cfs/cdirs/m3120/launch.<run period>_<recon version>-perl` directory on NERSC Community File System that contains job scripts and JANA config file(s). Is mapped to `/launch.<run period>_<recon version>-perl` inside the job container.
* `/pscratch/sd/j/jlab/swif` directory on NERSC Scratch Space that serves as root directory for job output (= swif2 site path). Has a quota of 500 TB.
  * `/pscratch/sd/j/jlab/swif/input` directory where swif2 copies all input files to, using catalog IDs as file names.
  * `/pscratch/sd/j/jlab/swif/jobs/gxproj4/${SLURM_JOB_NAME}/${SWIF_JOB_ATTEMPT_ID}` working directory the job and the container task wakes up in (identical to `${SWIF_JOB_STAGE_DIR}` and `${SWIF_JOB_WORK_DIR}`); the content of this directory is copied back to the three job output directories at JLab by swif2: for successful `hd_root` processes, output and log files are copied to separate directories; all files of failed `hd_root` processes are copied to another directory
    * `/pscratch/sd/j/jlab/swif/jobs/gxproj4/${SLURM_JOB_NAME}/${SWIF_JOB_ATTEMPT_ID}/RUN??????/TASK???` working directory of the container task, where `???` is the 3-digit `${SLURM_PROCID}`.
      * `/pscratch/sd/j/jlab/swif/jobs/gxproj4/${SLURM_JOB_NAME}/${SWIF_JOB_ATTEMPT_ID}/RUN??????/TASK???/FILE???` working directory of the `hd_root` process that processes the EVIO file with the 3-digit file number `???`.

### Container image

* `docker:jeffersonlab/gluex_almalinux_9:latest` contains everything to set up GlueX software environment from CVMFS.

### Globus collections

* `2f299200-6b58-4f81-bacd-b947873986db` `NERSC DTN jlab Collab`, for data inbound to and outbound from NERSC.
* `086540a1-9598-445e-8637-ac2f3997874f` `JLAB#SWIF`, for data outbound from JLab.
* `b0fca1ad-f485-4a00-8fcd-bca0b93a2a1c` `jlab#gw1`, for data inbound to JLab.
