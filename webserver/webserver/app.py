from flask import Flask, request

app = Flask(__name__)


@app.route("/")
def root_path():
    return "<p>Please use a GET or POST request on the proper endpoints!</p>"


@app.route("/mode", methods=['GET'])
def mode():
    if request.method == 'GET':
        print("Getting mode")
        return {'ir': True, 'broadcast': False, 'rescan': False}


@app.route("/unswitch", methods=['POST'])
def unswitch():
    if request.method == 'POST':
        print('Unswitching')
        print(request.data.decode('utf-8'))
        return "DONE"
    

@app.route("/upload", methods=['POST'])
def upload():
    if request.method == 'POST':
        print('Got upload json:')
        print(request.data.decode('utf-8'))
        return "DONE"