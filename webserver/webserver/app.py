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

from apscheduler.schedulers.background import BackgroundScheduler

HEARTBEAT_TIME: int = 10


app = Flask(__name__)
#socketio = SocketIO(app, binary=True, cors_allowed_origins='*', debug=True)
sock = Sock(app)


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
        


def updateDeviceState():
    # Firebase stuff
    print("temp")

def firebaseDeleteCollection(collection_ref, size):
    docs = collection_ref.list_documents(page_size=size)
    deleted = 0

    for doc in docs:
        doc.delete()
        deleted += 1

    if deleted >= size:
        firebaseDeleteCollection(collection_ref, size)

@sock.route('/echo')
def echo(ws):
    while True:
        data = ws.receive()
        ws.send(data)

# @socketio.on('connect')
# def connect():
#     print('Client connected\n\n\n\n\n\n\n\n\n\n\n')
#     socketio.send('YO WHATS UP')


# @socketio.on('disconnect')
# def connect():
#     print('Client DISCONNECTED NOOOO\n\n\n\n\n\n\n\n\n\n\n')


# @socketio.on('message')
# def upload_pcap(data):
#     print('Received data: ', data)
    # if request.method == 'POST':
    #     device_id = request.args.get('device-id')
    #     device_id = device_id.replace('"', '')
    #     path = request.args.get('path')
    #     chunk = request.args.get('offset')
    #     final = request.args.get('final')
    #     chunk_data = request.data

    #     print(f"Got {path} at chunk {chunk} from {device_id}. Final chunk status {final}")

    #     with open(os.path.join(app.config['UPLOAD_FOLDER'], f"{path}__{chunk}"), "wb") as f:
    #         f.write(chunk_data)
        
    #     if (final == "1"):
    #         final_path = os.path.join(app.config['UPLOAD_FOLDER'], path)
    #         if os.path.exists(final_path):
    #             final_path += "_1"

    #         with open(final_path, "wb") as outfile:
    #             for i in range(int(chunk) + 1):
    #                 with open(os.path.join(app.config['UPLOAD_FOLDER'], f"{path}__{i}"), 'rb') as infile:
    #                     data = infile.read()
    #                     outfile.write(data)
    #                 os.remove(os.path.join(app.config['UPLOAD_FOLDER'], f"{path}__{i}"))
    #     return "DONE"

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
    if request.method == 'GET':
        device_id = request.args.get('device-id')
        device_id = device_id.replace('"', '')
        print(f"Getting test mode for device with id: {device_id}")
        #return {'id': device_states[device_id]['test-params']['network_id'], 'type': device_states[device_id]['test-params']['test_type']}
        return {'id': 0, 'type': 'capture'}


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
    

# @app.route("/upload-pcap", methods=["POST"])
# def upload_pcap():
#     if request.method == 'POST':
#         device_id = request.args.get('device-id')
#         device_id = device_id.replace('"', '')
#         path = request.args.get('path')
#         chunk = request.args.get('offset')
#         final = request.args.get('final')
#         chunk_data = request.data

#         print(f"Got {path} at chunk {chunk} from {device_id}. Final chunk status {final}")

#         with open(os.path.join(app.config['UPLOAD_FOLDER'], f"{path}__{chunk}"), "wb") as f:
#             f.write(chunk_data)
        
#         if (final == "1"):
#             final_path = os.path.join(app.config['UPLOAD_FOLDER'], path)
#             if os.path.exists(final_path):
#                 final_path += "_1"

#             with open(final_path, "wb") as outfile:
#                 for i in range(int(chunk) + 1):
#                     with open(os.path.join(app.config['UPLOAD_FOLDER'], f"{path}__{i}"), 'rb') as infile:
#                         data = infile.read()
#                         outfile.write(data)
#                     os.remove(os.path.join(app.config['UPLOAD_FOLDER'], f"{path}__{i}"))
#         return "DONE"
    
scheduler = BackgroundScheduler()
scheduler.add_job(func=firebaseHeartbeat, trigger='interval', seconds=HEARTBEAT_TIME)
scheduler.start()

atexit.register(lambda: scheduler.shutdown())