from flask import Flask, request

app = Flask(__name__)


@app.route("/")
def root_path():
    return "<p>Please use a GET or POST request on the proper endpoints!</p>"


@app.route("/register-device")
def register_device():
    if request.method == 'GET':
        mac = request.args.get('mac')
        device_id = "FDS234"
        print(f"Giving new device with MAC address {mac} id: {device_id}")
        return {'id': device_id}


@app.route("/mode", methods=['GET'])
def mode():
    if request.method == 'GET':
        print(f"Getting mode for device with id: {request.args.get('device-id')}")
        return {'ir': False, 'broadcast': False, 'rescan': False}


@app.route("/unswitch", methods=['POST'])
def unswitch():
    if request.method == 'POST':
        print(f'Unswitching for device with id: {request.args.get("device-id")}')
        print(request.data.decode('utf-8'))
        return "DONE"
    

@app.route("/upload", methods=['POST'])
def upload():
    if request.method == 'POST':
        print(f'Got {request.args.get("device-id")}\'s upload json:')
        print(request.data.decode('utf-8'))
        
        return "DONE"