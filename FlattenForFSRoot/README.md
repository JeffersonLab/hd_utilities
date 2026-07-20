# Introduction

Following is the documentation on how to build ```FlattenForFSRoot``` in your own work directory in JLab's ifarm account.

# Prerequisites

Make sure that GlueX environment is set up. It is typically set by adding following lines in the ```.cshrc``` file if not already present.

```
setenv BUILD_SCRIPTS /group/halld/Software/build_scripts
source $BUILD_SCRIPTS/gluex_env_jlab.csh
```

# Installation

1. Move into the directory where you want to build it:
```
cd /path/to/work/directory
```

2. Clone the ```hd_utilities``` GitHub repository: 
```
git clone https://github.com/JeffersonLab/hd_utilities
```

3. Move into ```FlattenForFSRoot``` directory:
```
cd hd_utilities/FlattenForFSRoot/
```

4. Make sure there is a ```Makefile``` in this directory. Finally, run:
```
make
```

5. After this is complete, there will be a newly formed executable named ```flatten```.