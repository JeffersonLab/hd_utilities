#!/bin/bash
rm -rf small_files
mkdir small_files
cp -v button_red.png button_blue.png small_files
cd small_files
rm -f top_level.tmp
mysql --silent -umarki -phybrid cacheInfo -e 'select count(*) as count, user from smallFile group by user order by count desc;' > top_level.tmp
rm -f index.html
echo \<h1\>Small Files on the Cache Disk\</h1\> > index.html
echo `date` >> index.html
echo \<table border\> >> index.html
echo \<tr\>\<th\>count\</th\>\<th\>user\</th\>\<th\>directories by name\</th\>\<th\>directories by count\</th\>\</tr\> >> index.html
awk '{print "<tr><td>"$1"</td><td>"$2"</td><td style=\"text-align:center\"><a href=\""$2"_by_dir.html\"><img src=\"button_red.png\"/></a></td><td style=\"text-align:center\"><a href=\""$2"_by_count.html\"><img src=\"button_blue.png\"/></a></td>"}' < top_level.tmp >> index.html
echo \</table\> >> index.html
rm -f users.tmp
mysql --silent -umarki -phybrid cacheInfo -e 'select unique user from smallFile;' > users.tmp
while read u; do
    echo "$u"
    rm -f sql.tmp
    echo select dir, count\(*\) as count, name as example_file from smallFile where user = \'$u\' group by dir order by dir\; > sql.tmp
    mysql --html -umarki -phybrid cacheInfo < sql.tmp > ${u}_by_dir.html
    rm -f sql.tmp
    echo select dir, count\(*\) as count, name as example_file from smallFile where user = \'$u\' group by dir order by count desc, dir\; > sql.tmp
    mysql --html -umarki -phybrid cacheInfo < sql.tmp > ${u}_by_count.html
done < users.tmp
