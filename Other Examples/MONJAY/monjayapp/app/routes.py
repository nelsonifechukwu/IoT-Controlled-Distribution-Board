from app import app 
from flask import session, redirect, render_template, request, flash, url_for, Response 
from app.models import checklogin, delete_dist_data, insert_distribution_data, insert_into_fault
from app.models import insert_distribution_input, get_distribution_input, get_distribution_data, get_dist_state, update_dist_state, getfault

import json, random

API = 'xdol'

#login
@app.route('/', methods = ('GET', 'POST'))
@app.route('/login' , methods = ('GET', 'POST'))
def login():
    if (request.method=='POST'):
        username = request.form['username']
        password = request.form['password']

        succeed = checklogin(username, password)
        if (succeed):
            session['username'] = username
            #global m_user
            #m_user = username
            flash('Welcome! Login Successful')
            return redirect(url_for('plot'))
        else:
            flash('Error: Invalid Username or Password')
            return redirect(url_for('login'))
    else:
        return render_template('login.html')

#plot data
@app.route('/plot')
def plot():
    #create a session to secure this endpoint
    if "username" not in session:
        return redirect(url_for('login'))
    #data = get_distribution_data()
    return render_template('plot.html', name=session["username"])


#design the web_api
@app.route('/update/key=<api_key>/c1=<int:c1>/c2=<int:c2>/c3=<int:c3>/c4=<int:c4>/v=<int:v>', methods=['GET'])
def update(api_key, c1, c2, c3, c4, v):
    if (api_key == API):
        insert_distribution_data(c1, c2, c3, c4, v)
        return Response(status=200)
    else:
        return Response(status=500)

#fetch data to be plotted
@app.route('/data', methods=['GET', 'POST'])
def get_data():
    data = get_distribution_data()
    data = str(data)
    data = data.strip('[]()')
    return str(data)

#get button state (for the hardware)
@app.route('/get_state', methods = ['GET'])
def get_state():
    dist_state = get_dist_state('all')
    dist_state = str(dist_state)
    dist_state = dist_state.strip('[]()')
    return str(dist_state)

#update button state
@app.route('/updatestate/button=<id>', methods = ['GET', 'POST'])
def update_state(id):
    #get the previous state
    state = get_dist_state(id)
    state =  str(state)
    #Strip the string from the database and update the previous state
    state = state.strip('()[]').strip(',')
    newstate =  1-int(state)
    update_dist_state(newstate, id)
    return str(newstate)

#delete all data
@app.route('/delete', methods = ('GET', 'POST'))
def delete():
    if request.method == 'POST':
        delete_dist_data()
        insert_distribution_data(0,0,0,0,0)
        return redirect(url_for('plot'))

#get individual state
@app.route('/get_state/<id>', methods = ['GET'])
def get_id_state(id):
    dist_state = get_dist_state(id)
    dist_state = str(dist_state)
    dist_state = dist_state.strip('[]()')
    return str(dist_state)

#insert distribution control panel input
@app.route('/insert/<int:b1>/<int:b2>/<int:b3>/<int:b4>', methods=['GET','POST'])
def insert_input(b1, b2, b3, b4):
    insert_distribution_input(b1, b2, b3, b4)
    return Response(status=200)  

#get distribution input
@app.route('/get_distribution_input', methods = ['GET'])
def get_all_input():
    data = get_distribution_input('all')
    data = str(data)
    data = data.strip('[]()')
    return str(data)

@app.route('/get_distribution_input/<id>', methods = ['GET'])
def get_input(id):
    distribution_input = get_distribution_input(id)
    distribution_input = str(distribution_input)
    distribution_input = distribution_input.strip('[]()')
    return str(distribution_input)

@app.route('/fault=<id>', methods = ['GET', 'POST'])
def fault_state(id):
    insert_into_fault(id)
    return Response(status=200) 

@app.route('/get_fault', methods = ['GET'])
def fault():
    fault = getfault() 
    fault = str(fault)
    fault = fault.strip('[](),')
    return str(fault)

#logout of session
@app.route('/logout', methods=['POST'])
def log_out():
    session.pop("username") 
    return redirect(url_for('login'))