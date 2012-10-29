#! /usr/bin/env python
# Gateway Unit Tester
# @author: Luke Hovington

import sys
import socket
import optparse
import time
import struct, crc16

p = optparse.OptionParser()
p.add_option('--host', dest='lis_addr', type='str', help='listener host')
p.add_option('--port', dest='lis_port', type='int', help='listener port')
p.set_defaults(lis_port=8001, lis_addr="localhost")

(opts, args) = p.parse_args()
globals().update(opts.__dict__)

device_name = 'front-door'
mac_address = "00:23:6c:95:d1:c6"
version_num = 1
device_salt = "djhldfsljhfdhjlkfsd"
rfid_cards  = [12344010, 3090264] # Number on cards, with leading zero's removed

s = None

for res in socket.getaddrinfo(lis_addr, lis_port, socket.AF_UNSPEC, socket.SOCK_STREAM):
    af, socktype, proto, canonname, sa = res
    try:
        s = socket.socket(af, socktype, proto)
        s.settimeout(10)
    except socket.error, msg:
        s = None
        continue
    try:
        s.connect(sa)
    except socket.error, msg:
        s.close()
        s = None
        continue
    break
    
if s is None:
    print 'could not open socket'
    sys.exit(1)

def checkResponce(expectedReturn, returnedData):
    if returnedData[0] == expectedReturn:
        if returnedData[1] == 'E': 
            print "Error code returned.."
            print returnedData[:2], struct.unpack('<B', returnedData[2])[0], struct.unpack('<B', returnedData[3])[0], returnedData[4:]
            return False
            
        print "Response: %s%d"% (returnedData[0], struct.unpack('<B', returnedData[1])[0] )
        if struct.unpack('<B', returnedData[1])[0] == 1:
            return True
        else:
            print "Test Failed.."
    else:
        print "Bad Responce:", [returnedData]
    return False





print "\n---- STARTING SERVER TEST ----\n"
print "Testing... Hello"

# ------------ Start Hello Message ------------

# Request a token
print "Requesting Token"
s.send('T'+struct.pack('<B', 0))
token = s.recv(1024)
print "Token Recived:",[token]
if struct.unpack('<H', token[3:])[0] == crc16.crc16(token[:3]):
    print "Token Valid - CRC passed"
else:
    print "Token INVALID", crc16.crc16(token[1:3]), struct.unpack('<H', token[3:])[0]


# Send Hello Message
device_len = struct.pack('<B', len(device_name))
mac_add = ''
for octet in mac_address.split(":"):
    mac_add += struct.pack('<B', int(octet, 16))
v_num = struct.pack('<B', version_num)
send_hello = 'H' + device_len + device_name + mac_add + v_num
send_hello += struct.pack('<H', crc16.crc16(device_salt + token[1:3] + send_hello))

s.send(send_hello)
hello = s.recv(1024)
if checkResponce('H', hello):
    print "Hello, PASS"
else:
    print "Hello, FAIL"



# ------------ Send a Heartbeat ------------
print "\nTesting... HeartBeat"
device_len = struct.pack('<B', len(device_name))

send_heartBeat = 'B' + device_len + device_name
send_heartBeat += struct.pack('<H', crc16.crc16(send_heartBeat))
s.send(send_heartBeat)

heartBeat = s.recv(1024)
if checkResponce('B', heartBeat):
    print "HeartBeat, PASS"
else:
    print "HeartBeat, FAIL"



# ------------ Start Authentication ------------
print "\nTesting... Authentication"
time.sleep(5) # wait 5 seconds
old_token = token

# Request a token
print "Requesting Token"
s.send('T'+struct.pack('<B', 0))
token = s.recv(1024)
print "Token Recived:",[token]
if struct.unpack('<H', token[3:])[0] == crc16.crc16(token[:3]):
    print "Token Valid - CRC passed"
else:
    print "Token INVALID", crc16.crc16(token[1:3]), struct.unpack('<H', token[3:])[0]

if old_token != token:
    print "Token has changed!!"
else:
    print "Token FALURE, expecting diffrent token"


# Send a Auth Request
device_len = struct.pack('<B', len(device_name))

for card in rfid_cards:
    send_auth = '@' + device_len + device_name + struct.pack('<L', card)
    send_auth += struct.pack('<H', crc16.crc16(device_salt + token[1:3] + send_auth))
    
    s.send(send_auth)
    auth = s.recv(1024)
            
    if struct.unpack('<H', auth[-2:])[0] == crc16.crc16(device_salt + token[1:3] + auth[:-2]):
        print "Auth - CRC passed"
    else:
        print "Auth CRC INVALID", crc16.crc16(device_salt + token[1:3] + auth[:-2]), struct.unpack('<H', auth[-2:])[0]   
        
    if checkResponce('@', auth):
        print "Auth, PASS", card
    else:
        print "Auth, FAIL", card


s.close()

print