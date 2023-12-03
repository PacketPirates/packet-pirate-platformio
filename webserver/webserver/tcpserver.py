import socketserver
import socket

class TCPHandler(socketserver.StreamRequestHandler):
    next_size = 0
    mode = 0    

    def handle(self):
        self.data = self.rfile.readline().strip()

        data = self.data
        print(data)
        if (data[-1] != 69):
            print("Invalid packet received")
        
        device_id_len = data[0]
        deviceId = ""
        for i in range(device_id_len):
            deviceId += chr(data[i + 1])
        deviceId = deviceId.replace('"', '')

        filepath_len = data[device_id_len + 1] + data[device_id_len + 2]
        filepath = ""
        for i in range(filepath_len):
            filepath += chr(data[device_id_len + 3 + i])

        currentChunk = 0
        for i in range(8):
            currentChunk += data[device_id_len + 3 + filepath_len + 1 + i]

        finalChunk = bool(data[device_id_len + 3 + filepath_len + 9])

        buffer = []
        for i in range(2048):
            buffer.append(data[device_id_len + filepath_len + 12 + i])

        print(f"Got chunk {currentChunk} from {deviceId} with path {filepath}. Final status is {finalChunk}...")
        print(buffer)
        

TCP_HOST, TCP_PORT = "localhost", 5001

def tcpServe():
    with socketserver.TCPServer((TCP_HOST, TCP_PORT), TCPHandler) as server:
        print(f'Serving tcp on port {TCP_PORT}')
        server.serve_forever()