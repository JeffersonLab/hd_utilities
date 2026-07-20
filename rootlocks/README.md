
This script scans GlueX/Hall D plugin code to check for root locks surrounding histogram fills, following the [guidelines linked here](https://halldweb.jlab.org/wiki/index.php/Locking_in_JANA)

Usage:
python checklocks.py <filename or superdirectory>
       
where superdirectory is 2 levels above the plugin code

eg
```
python checklocks.py JEventProcessor_buggy_example.cc
```
or
```
python checklocks.py halld_recon/src/plugins/monitoring
```



