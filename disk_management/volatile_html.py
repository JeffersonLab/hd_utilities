#!/usr/bin/env python
import mysql.connector
import time

now = time.strftime('%c')
datetime = str(now)

db = mysql.connector.connect(
  host="scidbw.jlab.org",
  user="dummy",
  passwd="",
  database="wdm"
)

dirlist = []

def checkdir(dir):
  if dir not in dirlist:
    dirlist.append(dir)
    return True
  else:
    return False

title = 'Directories with the Oldest Volatile Files'

print('<html>')
print('<head><title>' + title + '</title></head>')
print('<body>')
print('<h1>' + title + '</h1>')
print('<p>' + datetime + '</p>')
print('<table border>')
print('<tr><th>running directory count</th><th>running sum of <br/> size of old files (GB)</th><th>directories with oldest files</th><th>running file count</th><th>oldest file in directory</th><th>file mod time</th><th>file owner</th></tr>')

cursor = db.cursor()

cursor.execute('select vfile.mod_time as mtime, file_name, vfile.owner, size, full_path from vfile, vdirectory, projectDisk where vfile.dir_index = vdirectory.dir_index and projectDisk.disk_index = vdirectory.disk_index and root = "/volatile/halld" order by mtime limit 100000')

result = cursor.fetchall()

count = 0
count_dir = 0
sum = float(0);
for i in range(len(result)):
  count += 1
  line_array = result[i]
  sum += float(line_array[3])
  dir = str(line_array[4])
  if checkdir(dir):
    count_dir += 1
    line_str_array = []
    for j in range(len(line_array)):
      line_str_array.append(str(line_array[j]))
    line = '<tr>' + '<td>' + str(count_dir) + '</td>' + '<td>' + str(sum/1024.0/1024.0/1024.0) + '</td>' + '<td>' + line_str_array[4] + '</td>' + '<td>' + str(count) + '</td>' + '<td>' + line_str_array[1] + '</td>' + '<td>' + line_str_array[0] + '</td>' + '<td>' + line_str_array[2] + '</td>' + '</tr>'
    print(line)

print('</table>')
print('</body>')
print('</html>')


