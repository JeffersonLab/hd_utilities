import sqlite3

# set up
db_conn = sqlite3.connect('test.sqlite')
db = db_conn.cursor()

test_data = [ (1,"sean",2.),
              (2,"paul",5.),
              (3,"ryan",20.) ]

# test
db.execute('CREATE TABLE IF NOT EXISTS test (id integer, name text, face real)')
db_conn.commit()

db.executemany('INSERT INTO test VALUES (?,?,?)', test_data)
db_conn.commit()

db_conn.close()
