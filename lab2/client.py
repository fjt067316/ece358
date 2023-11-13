from socket import *
import random

host = "127.0.0.1"
port = 42069

client_socket = socket(AF_INET, SOCK_DGRAM)

def get_dns_header():
    # Define the DNS header fields
    random_id = random.getrandbits(16)
    id = random_id  # A randomly generated 16-bit identifier
    qr = 0  # qr = 0 for query, 1 for response
    opcode = 0x0000  # opcode = 0 (standard query)
    aa = 1  # aa = 0 (not authoritative)
    tc = 0  # tc = 0 (not truncated)
    rd = 1  # rd = 1 (recursion desired)
    ra = 0  # ra = 0 (no recursion available)
    z = 0  # z = 0
    rcode = 0  # rcode = 0 (no error)
    qdcount = 1  # qdcount = 1 (one entry in the question section)
    ancount = 0  # ancount = 0 (no answers)
    nscount = 0  # nscount = 0 (no name server records)
    arcount = 0  # arcount = 0 (no additional records)

    # Create a binary representation of the DNS header
    header = (
        (id).to_bytes(2, byteorder='big') +
        ((qr << 15) | (opcode << 11) | (aa << 10) | (tc << 9) | (rd << 8) | (ra << 7) | (z << 4) | (rcode)).to_bytes(2, byteorder='big') +
        (qdcount).to_bytes(2, byteorder='big') +
        (ancount).to_bytes(2, byteorder='big') +
        (nscount).to_bytes(2, byteorder='big') +
        (arcount).to_bytes(2, byteorder='big')
    )
    
    return bytes(header)

def get_question_header(query):
    # Define the values for the DNS question fields
    lst = query.split(".")
    qname = b""  # QNAME as bytes
    
    for word in lst:
        word_length = len(word) & 0xFF  # Ensure it's an 8-bit value
        qname += bytes([word_length])  # Append the 8-bit length
        qname += word.encode('ascii')  # Append the ASCII values for the word

    qtype = 0x0001  # qtype = 1 for type A (host address query)
    qclass = 0x0001  # qclass for IN (Internet)  

    # Create a binary representation
    question = qname + b"\x00" + qtype.to_bytes(2, byteorder='big') + qclass.to_bytes(2, byteorder='big')

    return question

def decode_answer(query, dns_resp, question_offset):
    if len(dns_resp) >= 12:
        header = dns_resp[:12]
        ancount = int.from_bytes(header[6:8], byteorder='big')
        
        offset = question_offset+12  # Initial offset to the start of answer sections

        for i in range(ancount):
            answer_section = dns_resp[offset:offset + 12]

            name = int.from_bytes(answer_section[0:2], byteorder='big')
            rtype = int.from_bytes(answer_section[2:4], byteorder='big')
            rclass = ''.join(chr(byte) for byte in answer_section[4:6])
            ttl = int.from_bytes(answer_section[6:10], byteorder='big')
            rdlength = int.from_bytes(answer_section[10:12], byteorder='big')
            rdata_bytes = dns_resp[offset + 12:offset + 12 + rdlength]

            # Convert the bytes to an IP address string
            rdata = '.'.join(str(byte) for byte in rdata_bytes)
            
            rtype_printed = rtype
            
            if rtype == 1:
                rtype_printed = 'A'
            elif rtype == 2:
                rtype_printed = 'NS'
            
            print(f"{query} : type {rtype_printed}, class {rclass}, TTL {ttl}, addr ({rdlength}) {rdata}")
            
            offset += 12 + rdlength
    
        
while True:
    query = input("Enter Domain Name: ")

    if query == "end":
        client_socket.sendto(b'end', (host, port))
        break
    
    question_header = get_question_header(query)
    
    # hex_q_header = ' '.join(format(byte, '02x') for byte in question_header)
    # print(hex_q_header)

    dns_header = get_dns_header()
    dns_query = dns_header + question_header
    client_socket.sendto(dns_query, (host, port))
    
    data, server_address = client_socket.recvfrom(2048)
    
    hex_data = ' '.join(format(byte, '02x') for byte in data)
    #print(hex_data)

    # hex_data = ''.join(format(byte, '02x') for byte in data)
    decode_answer(query, data, len(question_header))
    # print(data)

client_socket.close()
