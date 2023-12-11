import sqlite3 as sql
from datetime import datetime

def checklogin(username, password):
    con = sql.connect("monjay.db")
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
    con = sql.connect("monjay.db")
    cur = con.cursor()
    #add datetime to the query
    cur.execute("INSERT INTO distribution_data(c1, c2, c3, c4, v) VALUES (?,?,?,?,?)", (c1, c2, c3, c4, v))
    con.commit()
    con.close()

def insert_distribution_input(d1, d2, d3, d4):
    d1 = f"{d1}"
    d2 = f"{d2}"
    d3 = f"{d3}"
    d4 = f"{d4}"
    con = sql.connect("monjay.db")
    cur = con.cursor()
    query = f"UPDATE ranges SET d1 = {d1}, d2 = {d2}, d3 = {d3}, d4 = {d4} WHERE id = 1"
    cur.execute(query)
    con.commit()
    con.close()

def get_distribution_input(id):
    id = f"{id}"
    con = sql.connect("monjay.db")
    cur = con.cursor()

    if id == 'all':
        query = f"SELECT d1, d2, d3, d4 FROM ranges WHERE id = 1"
    else:
        query = f"SELECT {id} FROM ranges WHERE id = 1"
    cur.execute(query)
    control_input = cur.fetchall()
    con.commit()
    con.close()
    return control_input

def get_distribution_data():
    con = sql.connect("monjay.db")
    cur = con.cursor()
    #get the latest data
    cur.execute("SELECT * FROM distribution_data ORDER BY id DESC LIMIT 40")
    dist_data = cur.fetchall()
    con.commit()
    con.close()
    return dist_data

def get_dist_state(type):
    con = sql.connect("monjay.db")
    cur = con.cursor()

    if type == 'all':
        cur.execute("SELECT c1_state, c2_state, c3_state, c4_state FROM dist_state WHERE id = 1")
    else:
        cur.execute("SELECT {}_state FROM dist_state WHERE id = 1".format(type))

    dist_state = cur.fetchall()
    con.commit()
    con.close()
    return dist_state

def update_dist_state(state, type):
    column_name = f"{type}_state"
    value = int(state)
    query = f"UPDATE dist_state SET {column_name} = {value} WHERE id = 1"

    with sql.connect("monjay.db") as con:
        cur = con.cursor()
        cur.execute(query)
        con.commit()

def insert_into_fault(id):
    #state = f"{id}"
    query = f"UPDATE fault SET fault_state = {id} WHERE id = 1"
    con = sql.connect("monjay.db")
    cur = con.cursor()
    cur.execute(query)
    con.commit()
    con.close()

def getfault():
    con = sql.connect("monjay.db")
    cur = con.cursor()
    cur.execute("SELECT fault_state from fault WHERE id = 1")
    fault = cur.fetchall()
    con.commit()
    con.close()
    return fault

def delete_dist_data():
    con = sql.connect("monjay.db")
    cur = con.cursor()
    cur.execute("DELETE FROM distribution_data")
    con.commit()
    con.close()

