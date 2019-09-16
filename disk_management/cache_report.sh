#!/bin/bash
report_file=/tmp/cache_oldest.html
rm -f $report_file
cat >> $report_file <<EOF
<html>
<head>
<title>Hall D Cache Disk Report</title>
</head>
<body>
<h1>Hall D Cache Disk Report</h1>
<p>
EOF
date >> $report_file
cat >> $report_file <<EOF
</p>
<p>Total size (GB) of unpinned raw data files under /cache/halld</p>
EOF
query="select sum(size)/1024/1024/1024 as size_GB from file, directory, projectDisk where file.dir_index = directory.dir_index and projectDisk.disk_index = directory.disk_index and isCached = 1 and isTaped = 1 and root = \"/cache/halld\" and file.owner = \"halldata\" and file_index not in (select file_index from pin) order by file.mod_time;"
mysql --html -h scidbw -u dummy wdm -e "$query" >> $report_file
cat >> $report_file <<EOF
</p>
<p>Oldest unpinned, backed up files under /cache/halld</p>
EOF
query="select file.mod_time as mtime, file_name, file.owner, size, full_path from file, directory, projectDisk where file.dir_index = directory.dir_index and projectDisk.disk_index = directory.disk_index and isCached = 1 and isTaped = 1 and root = \"/cache/halld\" and file.owner != \"halldata\" and file_index not in (select file_index from pin) order by mtime limit 100;"
mysql --html -h scidbw -u dummy wdm -e "$query" >> $report_file
cat >> $report_file <<EOF
</body>
</html>
EOF
mv $report_file /group/halld/www/halldweb/html/disk_management/
