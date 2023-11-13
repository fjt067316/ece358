from socket import *
from datetime import datetime
import random

host = "127.0.0.1"
port = 42069

serverSocket = socket(AF_INET, SOCK_DGRAM)
serverSocket.bind((host, port))

# key is "google.com" value is (google.com, A, IN, 260, 192.165.1.1)
domain_map = {
    "google.com": [
        ("google.com", 1, "IN", 260, "192.165.1.1"),
        ("google.com", 1, "IN", 260, "192.165.1.10")
    ],
    "youtube.com" : [("youtube.com", 1, "IN", 160, "192.165.1.2")],
    "uwaterloo.ca" : [("uwaterloo.ca", 1, "IN", 160, "192.165.1.3")],
    "wikipedia.org" : [("wikipedia.org", 1, "IN", 160, "192.165.1.4")],
    "amazon.ca" : [("amazon.ca", 1, "IN", 160, "192.165.1.5")],
}

def get_dns_header(query):
    random_id = random.getrandbits(16)

    id = random_id  # A randomly generated 16-bit identifier
    qr = 1  # qr = 1 for response 0 for query
    opcode = 0  # opcode = 0 (standard query)
    aa = 1  # aa = 1 (bit means valid response)
    tc = 0  # tc = 0 (was response message truncated?)
    rd = 0  # rd = 0 (no recursion requested from client)
    ra = 0  # ra = 0 (no recursion available)
    z = 0  # z = 0
    rcode = 0  # rcode = 0 (no error)
    qdcount = 1  # qdcount = 1 (one entry in the question section)
    ancount = len(domain_map[query])  # ancount = number of resource records in the answer section
    nscount = 0  # nscount = 0
    arcount = 0  # arcount = 0

    # Create a binary representation of the DNS response header
    header = (
        (id).to_bytes(2, byteorder='big') +
        ((qr << 15) | (opcode << 11) | (aa << 10) | (tc << 9) | (rd << 8) | (ra << 7) | (z << 4) | (rcode)).to_bytes(2, byteorder='big') +
        (qdcount).to_bytes(2, byteorder='big') +
        (ancount).to_bytes(2, byteorder='big') +
        (nscount).to_bytes(2, byteorder='big') +
        (arcount).to_bytes(2, byteorder='big')
    )
    
    return header


def get_dns_query(recv_data):
    
    if len(recv_data) >= 12: # check if there is at least DNS header of 12 Bytes
        dns_header = recv_data[:12]
        question_section = recv_data[12:]
        
        # Parsing the QNAME field (domain name)
        qname = b""
        pointer = 0
        while True:
            label_length = question_section[pointer]
            if label_length == 0:
                # Zero-length label indicates the end of the domain name
                break
            label = question_section[pointer + 1 : pointer + 1 + label_length]
            qname += label + b"."
            pointer += label_length + 1

        # Remove the trailing dot if it exists
        if qname.endswith(b"."):
            qname = qname[:-1]

        return qname.decode('ascii')
    

def generate_dns_answer_section(domain):
    # Create the binary representation of the DNS answer section
    ans = b""
    if domain in domain_map:
        for record in domain_map[domain]:
            # Get the values from the dictionary
            values_list = record  # Assuming the first entry in the list

            # Extract the relevant information
            NAME = values_list[0]
            TYPE = int(values_list[1])
            CLASS = bytes(values_list[2].encode('ascii')) # Class IN
            TTL = values_list[3]  # TTL value from the dictionary
            RDATA = bytes(int(byte) for byte in values_list[4].split('.'))
            RDLENGTH = len(RDATA)  # Length of RDATA (IPv4 address)

            answer_section = (
                # NAME (c00c in hex)
                bytes([0xC0, 0x0C]) +
                # TYPE (two octets containing TYPE A value, e.g., 0x0001 for A)
                TYPE.to_bytes(2, byteorder='big') +
                # CLASS (two octets containing IN value, e.g., 0x0001 for IN)
                CLASS +
                # TTL (32-bit unsigned integer)
                TTL.to_bytes(4, byteorder='big') +
                # RDLENGTH (16-bit unsigned integer)
                RDLENGTH.to_bytes(2, byteorder='big') +
                # RDATA (variable length string of octets)
                RDATA 
            )
            
            ans += answer_section

    
    return ans

while True:
    recv_data, addr_client = serverSocket.recvfrom(2048)

    if recv_data == b'end':
        break
    
    hex_data = ' '.join(format(byte, '02x') for byte in recv_data)
    print("Request:")
    print(hex_data)
    
    qname = get_dns_query(recv_data)
    #print(qname)
    header = get_dns_header(qname)
    
    
    question_section = recv_data[12:]
    
    resp = header + question_section + generate_dns_answer_section(qname)
    hex_resp = ' '.join(format(byte, '02x') for byte in resp)
    print("Response:")
    print(hex_resp)
    serverSocket.sendto(resp, addr_client)

