/**********************************************************************

Grants:

grant all on cacheInfo.* to marki@localhost identified by '*****';
grant file on *.* to marki@localhost;

Invocation:

sed 's/\(.*\)\//\1 /' cache_small_files.txt > csf.tmp
mysql -umarki -p cacheInfo < load_small_file_info.sql

Example query:

select count(*) as c, user from smallFile group by user order by c desc;

**********************************************************************/

drop table if exists smallFile;
create table smallFile (
  dir varchar(1024),
  name varchar(1024),
  user varchar(32),
  atime int,
  mtime int,
  size int
);
load data local infile
  './csf.tmp' into table smallFile
  fields terminated by ' ';
