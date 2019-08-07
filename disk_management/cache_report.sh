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
<p>Oldest 1000 files on /cache/halld</p>
EOF
query="select file.mod_time as mtime, file_name, file.owner, size, full_path from file, directory where file.dir_index = directory.dir_index and full_path like \"/cache/halld/%\" order by mtime limit 1000;"
mysql --html -h scidbw -u dummy wdm -e "$query" >> $report_file
cat >> $report_file <<EOF
</body>
</html>
EOF
mv $report_file /group/halld/www/halldweb/html/disk_management/
