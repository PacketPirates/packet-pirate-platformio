import time
from flask import Flask, request
#from flask_socketio import SocketIO
from flask_sock import Sock
import json
import os
import pathlib
import atexit
import firebase_admin
from firebase_admin import firestore
import subprocess
import requests

from .tcpserver import tcpServe

import threading

from apscheduler.schedulers.background import BackgroundScheduler

HEARTBEAT_TIME: int = 10
SECURITY_ENDPOINT = "http://147.182.195.19:5000"


app = Flask(__name__)
sock = Sock(app)

global test_device_id
global test_id
test_id = 0
test_device_id = ""


app.config['UPLOAD_FOLDER'] = 'D:\\Documents\\uploads'

firebase_creds = firebase_admin.credentials.Certificate(f"{pathlib.Path(__file__).parent.as_posix()}/secrets/firebase_admin_sdk_key.json")
firebase_app = firebase_admin.initialize_app(firebase_creds)
db: firestore.firestore.Client = firestore.client()

device_states = {}

def firebaseHeartbeat():
    print("Firebase heartbeat!")
    for _, (key, _) in enumerate(device_states.items()):
        remote_state = db.collection('devices').document(key).get().to_dict()
        device_states[key] = remote_state
        


def post_upload_integration(upload_path):
    global test_id
    global test_device_id

    print(upload_path)
    crack_r = requests.post(f'{SECURITY_ENDPOINT}/crackPcap', files={'file': f"@{upload_path}"})
    print(crack_r.text)
    crack_r_json = crack_r.json()
    hash = crack_r_json['hashes'][0]
    print(crack_r_json)
    time.sleep(2)
    hash_r = requests.post(f'{SECURITY_ENDPOINT}/getCrackedHash', json={'hash': hash})
    while hash_r.status_code != 200:
        print("Could not get hash! Waiting...")
        time.sleep(2)
        hash_r = requests.post(f'{SECURITY_ENDPOINT}/getCrackedHash', json={'hash': hash})
        print(hash_r.content)

    print(hash_r.text)
    print(f"ID: {test_device_id}, ch: {test_id}")

    id_str = str(test_id)

    passed_temp = db.collection('devices').document(test_device_id).collection('networks').document(id_str).get().to_dict()

    print(f"dict: {passed_temp}")
    passed_temp['tests_passed']['decryption'] = True
    passed_temp['tests_passed']['hash'] = hash_r.json()['result']
    db.collection('devices').document(test_device_id).collection('networks').document(id_str).set(passed_temp)


def firebaseDeleteCollection(collection_ref, size):
    docs = collection_ref.list_documents(page_size=size)
    deleted = 0

    for doc in docs:
        doc.delete()
        deleted += 1

    if deleted >= size:
        firebaseDeleteCollection(collection_ref, size)

@app.route("/")
def root_path():
    return "<p>Please use a GET or POST request on the proper endpoints!</p>"


@app.route("/register-device")
def register_device():
    if request.method == 'GET':
        mac = request.args.get('mac')
        mac = mac.replace(':', '_')
        device_id = mac
        print(f"Giving new device with MAC address {mac} id: {device_id}")
        device_states[device_id] = {
            'broadcast': False,
            'ir': False,
            'scan': False,
            'test': False,
            'test-params': {
                'network_id': None,
                'test_type': None
            },
            'networks': None
        }
        doc_ref = db.collection("devices").document(device_id)
        doc_ref.set(device_states[device_id])
        doc_ref.update({'networks': firestore.DELETE_FIELD})

        return {'id': device_id}


@app.route("/mode", methods=['GET'])
def mode():
    if request.method == 'GET':
        device_id = request.args.get('device-id')
        device_id = device_id.replace('"', '')
        print(f"Getting mode for device with id: {device_id}")
        return {'ir': device_states[device_id]['ir'], 
                'broadcast': device_states[device_id]['broadcast'], 
                'rescan': device_states[device_id]['scan'], 
                'test': device_states[device_id]['test']}


@app.route("/get-test", methods=['GET'])
def get_test():
    global test_id
    global test_device_id
    if request.method == 'GET':
        device_id = request.args.get('device-id')
        device_id = device_id.replace('"', '')
        print(f"Getting test mode for device with id: {device_id}")
        if (device_states[device_id]['test-params']['test_type'] == 'capture'):
                test_id = device_states[device_id]['test-params']['network_id']
                test_device_id = device_id
        return {'id': device_states[device_id]['test-params']['network_id'], 'type': device_states[device_id]['test-params']['test_type']}


@app.route("/unswitch", methods=['POST'])
def unswitch():
    if request.method == 'POST':
        device_id = request.args.get('device-id')
        device_id = device_id.replace('"', '')
        print(f'Unswitching "{request.data.decode("utf-8")}" for device with id: {device_id}')
        
        mode_str = request.data.decode("utf-8")
        mode_str = mode_str.replace("modes/", "")
        current_dict = db.collection('devices').document(device_id).get().to_dict()
        current_dict[mode_str] = False
        if (mode_str == 'test'):
            current_dict['test-params'] = { 'network_id': None, 'test_type': None }
        db.collection('devices').document(device_id).set(current_dict)
        device_states[device_id] = current_dict
        return "DONE"
    

@app.route("/upload", methods=['POST'])
def upload():
    if request.method == 'POST':
        device_id = request.args.get('device-id')
        device_id = device_id.replace('"', '')
        print(f'Got {device_id}\'s upload json:')
        request_data = request.data.decode('utf-8')
        request_data = request_data.removesuffix(',')
        print(request_data)

        new_networks = json.loads(request_data)
        networks_ref = db.collection('devices').document(device_id).collection('networks')
        firebaseDeleteCollection(networks_ref, len(new_networks))

        for _, (key, val) in enumerate(new_networks.items()):
            new_networks[key]['tests_passed'] = {'decryption': False, 'deauth': False}
            temp_ref = db.collection('devices').document(device_id).collection('networks').document(str(key))
            temp_ref.set(val)

        device_states[device_id]['networks'] = new_networks

        return "DONE"

 
@app.route("/upload-test-result", methods=['POST'])
def upload_test_result():
    if request.method == 'POST':
        device_id = request.args.get('device-id')
        device_id = device_id.replace('"', '')
        print(f'Got {device_id}\'s test upload json:')
        result = json.loads(request.data.decode('utf-8'))
        print(result)
        id_str = str(result['id'])
        print(f"{id_str} is of type {type(id_str)}")
        current_passed = db.collection('devices').document(device_id).collection('networks').document(id_str).get().to_dict()
        current_passed['tests_passed'][result['test']] = bool(result['result'])
        db.collection('devices').document(device_id).collection('networks').document(id_str).set(current_passed)
        return "DONE"
    
scheduler = BackgroundScheduler()
scheduler.add_job(func=firebaseHeartbeat, trigger='interval', seconds=HEARTBEAT_TIME)
scheduler.start()

atexit.register(lambda: scheduler.shutdown())

tcp_thread = threading.Thread(target=tcpServe, args=(post_upload_integration,))
tcp_thread.daemon = True
tcp_thread.start()