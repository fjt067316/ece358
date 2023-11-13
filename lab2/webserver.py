from socket import *
from datetime import datetime
import os

# Declare ip and port of server
host = "127.0.0.1"
port = 42069

# Server initializes a tcp socket with the host and port and starts listening
serverSocket = socket(AF_INET, SOCK_STREAM)
serverSocket.bind((host, port))
serverSocket.listen(1)

# Parses the http request method (get or head)
def get_method(msg):
    lines = msg.split('\r\n')
    words = lines[0].split()
    return words[0]

# Parses the http message and returns the requested file path 
def get_file(msg):
    lines = msg.split('\r\n')
    words = lines[0].split()
    return words[1].replace('/', '')

# Processes the http request and generates the respective response
def get_resp(request):
    method = get_method(request)
    if method == "GET":
        requested_file = get_file(request)
        try:
            file_last_modified = datetime.utcfromtimestamp(os.path.getmtime(requested_file)).strftime('%a, %d %b %Y %H:%M:%S GMT')
            with open(requested_file, 'rb') as file:
                file_content = file.read() # Get content of the requested file 
            response = (
                "HTTP/1.1 200 OK\r\n"
                "Connection: close\r\n"
                "Date: {}\r\n"
                "Server: MyHTTPServer\r\n"
                "Last-Modified: {}\r\n"
                "Content-Length: {}\r\n"
                "Content-Type: text/html\r\n"
                "\r\n"  # End of headers
            ).format(datetime.now().strftime('%a, %d %b %Y %H:%M:%S GMT'), file_last_modified, len(file_content))
            response += file_content.decode()  # Append the actual content for GET requests
            return response
        except FileNotFoundError:
            return "HTTP/1.1 404 Not Found\r\n\r\nFile not found"
    elif method == "HEAD":
        return "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n"

# Loop to accept incoming connection, receive data, and send back response
while True:
    conn, addr_client = serverSocket.accept()
    print(f"Connection from {addr_client}")
    recv_data = conn.recv(2048)
    print(recv_data.decode())
    resp = get_resp(recv_data.decode())
    conn.sendall(resp.encode())  # Encode the response string to bytes before sending
    conn.close()



