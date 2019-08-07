#!/bin/bash
report_file=/tmp/volatile_oldest.html
rm -f $report_file
cat >> $report_file <<EOF
<html>
<head>
<title>Hall D Volatile Disk Report</title>
</head>
<body>
<h1>Hall D Volatile Disk Report</h1>
<p>
EOF
date >> $report_file
cat >> $report_file <<EOF
</p>
<p>Oldest 1000 files under /volatile/halld</p>
EOF
query="select vfile.mod_time as mtime, file_name, vfile.owner, size, full_path from vfile, vdirectory where vfile.dir_index = vdirectory.dir_index and full_path like \"/volatile/halld/%\" order by mtime limit 1000;"
mysql --html -h scidbw -u dummy wdm -e "$query" >> $report_file
cat >> $report_file <<EOF
</body>
</html>
EOF
mv $report_file /group/halld/www/halldweb/html/disk_management/
