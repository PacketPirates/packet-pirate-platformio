import os
import socket
from asyncio import IncompleteReadError

UPLOAD_FOLDER = "D:\\Documents\\uploads"
TCP_HOST, TCP_PORT = "localhost", 5001

class TCPHandler:
    def __init__(self, sock: socket.socket):
        self._sock = sock
        self._recv_buffer = bytearray()

    def read(self, num_bytes: int = -1) -> bytes:
        raise NotImplementedError

    def readexactly(self, num_bytes: int) -> bytes:
        buf = bytearray(num_bytes)
        pos = 0
        while pos < num_bytes:
            n = self._recv_into(memoryview(buf)[pos:])
            if n == 0:
                raise IncompleteReadError(bytes(buf[:pos]), num_bytes)
            pos += n
        return bytes(buf)

    def readline(self) -> bytes:
        return self.readuntil(b"\n")

    def readuntil(self, separator: bytes = b"\n") -> bytes:
        if len(separator) != 1:
            raise ValueError("Only separators of length 1 are supported.")

        chunk = bytearray(4096)
        start = 0
        buf = bytearray(len(self._recv_buffer))
        bytes_read = self._recv_into(memoryview(buf))
        assert bytes_read == len(buf)

        while True:
            idx = buf.find(separator, start)
            if idx != -1:
                break

            start = len(self._recv_buffer)
            bytes_read = self._recv_into(memoryview(chunk))
            buf += memoryview(chunk)[:bytes_read]

        result = bytes(buf[: idx + 1])
        self._recv_buffer = b"".join(
            (memoryview(buf)[idx + 1 :], self._recv_buffer)
        )
        return result

    def _recv_into(self, view: memoryview) -> int:
        bytes_read = min(len(view), len(self._recv_buffer))
        view[:bytes_read] = self._recv_buffer[:bytes_read]
        self._recv_buffer = self._recv_buffer[bytes_read:]
        if bytes_read == len(view):
            return bytes_read
        bytes_read += self._sock.recv_into(view[bytes_read:])
        return bytes_read

def tcpServe(callback):
    #with socketserver.TCPServer((TCP_HOST, TCP_PORT), TCPHandler) as server:
    #    print(f'Serving tcp on port {TCP_PORT}')
    #    server.serve_forever()
    with socket.create_server((TCP_HOST, TCP_PORT)) as server:
        print(f'Serving tcp on port {TCP_PORT}')
        server.listen()
        while True:
            print("Listening for new TCP connection...")
            sock = server.accept()[0]
            print("TCP connection active!")
            reader = TCPHandler(sock)

            data = []
            #correct = 0
            # Loop through one at a time until the magic numbers are found
            data = reader.readuntil(b'E')
            #print(data)
            dbytearray = bytearray(data)
            while dbytearray[-2] != 1 and dbytearray[-3] != 69:
                print("Found magic number but not at end yet")
                #print(dbytearray)
                dbytearray.extend(reader.readuntil(b'E'))
                
            sock.close()
            data = dbytearray
            #print(f"Final data: {list(data)}")
        
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
                currentChunk += data[device_id_len + 3 + filepath_len + 0 + i]

            finalChunk = bool(data[device_id_len + 3 + filepath_len + 8])

            buffer = []
            for i in range(2048):
                buffer.append(data[device_id_len + filepath_len + 12 + i])

            # The last index is part of the magic number
            buffer.pop()

            print(f"Got chunk {currentChunk} from {deviceId} with path {filepath}. Final status is {finalChunk}...")
            #print(f"Buffer {buffer}")

            with open(os.path.join(UPLOAD_FOLDER, f"{filepath}__{currentChunk}"), "wb") as f:
                f.write(bytes(buffer))
        
            if finalChunk:
                final_path = os.path.join(UPLOAD_FOLDER, filepath)
                if os.path.exists(final_path):
                    final_path += "_1"

                with open(final_path, "wb") as outfile:
                    for i in range(int(currentChunk) + 1):
                        with open(os.path.join(UPLOAD_FOLDER, f"{filepath}__{i}"), 'rb') as infile:
                            data = infile.read()
                            outfile.write(data)
                        os.remove(os.path.join(UPLOAD_FOLDER, f"{filepath}__{i}"))

                callback(final_path)
    