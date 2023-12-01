from flask import Flask, request
import os

app = Flask(__name__)


app.config['UPLOAD_FOLDER'] = 'D:\\Documents\\uploads'


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
        device_id = request.args.get('device-id')
        print(f"Getting mode for device with id: {device_id}")
        return {'ir': False, 'broadcast': False, 'rescan': False, 'test': True}


@app.route("/get-test", methods=['GET'])
def get_test():
    if request.method == 'GET':
        device_id = request.args.get('device-id')
        print(f"Getting test mode for device with id: {device_id}")
        return {'id': 2, 'type': 'temp'}


@app.route("/unswitch", methods=['POST'])
def unswitch():
    if request.method == 'POST':
        device_id = request.args.get('device-id')
        print(f'Unswitching "{request.data.decode("utf-8")}" for device with id: {device_id}')
        return "DONE"
    

@app.route("/upload", methods=['POST'])
def upload():
    if request.method == 'POST':
        device_id = request.args.get('device-id')
        print(f'Got {device_id}\'s upload json:')
        print(request.data.decode('utf-8'))
        
        return "DONE"

 
@app.route("/upload-test-result", methods=['POST'])
def upload_test_result():
    if request.method == 'POST':
        device_id = request.args.get('device-id')
        print(f'Got {device_id}\'s test upload json:')
        print(request.data.decode('utf-8'))
        
        return "DONE"
    

@app.route("/upload-pcap", methods=["POST"])
def upload_pcap():
    if request.method == 'POST':
        device_id = request.args.get('device-id')
        path = request.args.get('path')
        chunk = request.args.get('offset')
        final = request.args.get('final')
        chunk_data = request.data

        print(f"Got {path} at chunk {chunk} from {device_id}. Final chunk status {final}")

        print(chunk_data)

        with open(os.path.join(app.config['UPLOAD_FOLDER'], f"{path}__{chunk}"), "wb") as f:
            f.write(chunk_data)
        
        if (final == "1"):
            with open(os.path.join(app.config['UPLOAD_FOLDER'], path), "wb") as outfile:
                for i in range(int(chunk) + 1):
                    with open(os.path.join(app.config['UPLOAD_FOLDER'], f"{path}__{i}"), 'rb') as infile:
                        data = infile.read()
                        outfile.write(data)
                    os.remove(os.path.join(app.config['UPLOAD_FOLDER'], f"{path}__{i}"))
        return "DONE"