import sqlite3 as sql
from datetime import datetime

def checklogin(username, password):
    con = sql.connect("zibah.db")
    cur = con.cursor()
    cur.execute("SELECT username, password FROM users")
    users = cur.fetchall()

    if username == users[0][0] and password == users[0][1]:
        success = True
    else:
        success = False
    con.commit()
    con.close()
    return success

def insert_distribution_data(c1, c2, c3, c4, v):
    con = sql.connect("zibah.db")
    cur = con.cursor()
    #add datetime to the query
    cur.execute("INSERT INTO distribution_data(c1, c2, c3, c4, v) VALUES (?,?,?,?,?)", (c1, c2, c3, c4, v))# datetime.now()
    con.commit()
    con.close()

def get_distribution_data():
    con = sql.connect("zibah.db")
    cur = con.cursor()
    #get the latest data
    cur.execute("SELECT * FROM distribution_data ORDER BY id DESC LIMIT 40")
    dist_data = cur.fetchall()
    con.commit()
    con.close()
    return dist_data

def get_dist_state(type):
    con = sql.connect("zibah.db")
    cur = con.cursor()

    if type == 'all':
        cur.execute("SELECT c1_state, c2_state, c3_state, c4_state FROM dist_state")
    else:
        cur.execute("SELECT {}_state FROM dist_state".format(type))

    dist_state = cur.fetchall()
    con.commit()
    con.close()
    return dist_state

def delete_dist_state():
    con = sql.connect("zibah.db")
    cur = con.cursor()
    cur.execute("DELETE FROM dist_state")
    con.commit()
    con.close()

def insert_dist_state(c1, c2, c3, c4, v):
    con = sql.connect("zibah.db")
    cur = con.cursor()
    #add datetime to the query
    cur.execute("INSERT INTO dist_state(c1_state, c2_state, c3_state, c4_state, v_state) VALUES (?,?,?,?,?)", (c1, c2, c3, c4, v))
    con.commit()
    con.close()

def update_dist_state(state, type):
    column_name = f"{type}_state"
    value = int(state)
    query = f"UPDATE dist_state SET {column_name} = {value}"

    with sql.connect("zibah.db") as con:
        cur = con.cursor()
        cur.execute(query)
        con.commit()

def delete_dist_data():
    con = sql.connect("zibah.db")
    cur = con.cursor()
    cur.execute("DELETE FROM distribution_data")
    con.commit()
    con.close()

