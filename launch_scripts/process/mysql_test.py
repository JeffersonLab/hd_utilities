import MySQLdb

# set up
db_conn = MySQLdb.connect(host='hallddb.jlab.org', user='datmon', passwd='', db='data_monitoring')
db = db_conn.cursor()

test_data = [ (1,"sean",2.),
              (2,"paul",5.),
              (3,"ryan",20.) ]

# test
#db.execute('CREATE TABLE IF NOT EXISTS test (id integer, name text, face real)')  ## 'if not exists' isn't working??
db.execute('CREATE TABLE test (id integer, name text, face real)')
db_conn.commit()

for dat in test_data:
    db.execute('INSERT INTO test VALUES (%s,,%s,%s)', dat)
db_conn.commit()

db_conn.close()
