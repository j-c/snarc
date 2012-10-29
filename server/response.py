#!/usr/bin/env python
# Access Gateway Controller
# @author: Luke Hovington

import random, string, thread
import crc16, struct
from dob import *
import unittest

debug = False

# The first 19 are responces not errors
responseLookup = {
    'Unknown'      :0,
    'Ok'           :1,
    'Fail'         :2,
    
    'SwitchOn'     :10,
    'Resume'       :11,
    'SwitchOff'    :12,
    'MomentaryOn'  :13,
    
    'ChecksumFail' :20,
    'InvalidOption':21,  
    'UnknownError' :22,
    'ServerError'  :23,
}

socket_options = {}
def init_socket_options():
    global socket_options
    # Expected packet length minus the device name
    socket_options = {
        'T':{'l':2,  'd':sendToken},
        'H':{'l':11, 'd':hello},
        'B':{'l':4,  'd':heartBeat},
        '@':{'l':8,  'd':authenticate},
        'A':{'l':4,  'd':acknowledge},
    }

def debugPrint(*x):
    if debug:
        print x

def gen_token():
    foo = random.SystemRandom()
    length = 2
    chars = string.printable
    return ''.join(foo.choice(chars) for _ in xrange(length))

# Start with two random tokens, the first one will be passed back when asked.
# The second will be checked if the first fails (might be due to recently changed) 
tokens = [gen_token(), gen_token()]
token_lock = thread.allocate_lock()
runThreads = True

# Helper to construct return packets
def response(code, descriptor):
    if descriptor in responseLookup:
        if responseLookup[descriptor] > 19:
            return "%sE%s%s%s"%(code, struct.pack('<B',responseLookup[descriptor]), struct.pack('<B',len(descriptor)), descriptor)
        else:
            return "%s%s"%(code, struct.pack('<B',responseLookup[descriptor]))
    return "%sE%s%s%s"%(code, struct.pack('<B',responseLookup['ServerError']), struct.pack('<B',len('ServerError')), 'ServerError')


# Send Token Responce 'T'
def sendToken(rfidInput):
    global token_lock, tokens
    with token_lock:
        return {'response':'T'+ tokens[0] + struct.pack('<H', crc16.crc16('T'+tokens[0]))}

# Register Hello Packet, Update Dob and return
def hello(rfidInput):
    dnl = struct.unpack('<B',rfidInput[1])[0] # device name length
    packetcrc = struct.unpack('<H',''.join(rfidInput[-2:]))[0]
    device_salt = dob_get_salt("".join(rfidInput[2:dnl+2]))
        
    mac_addr = []
    for octet in rfidInput[dnl+2:dnl+8]:
        mac_addr.append("%02x" %struct.unpack('<B',octet)[0])
    
    debugPrint( "device name", "".join(rfidInput[2:dnl+2]) )
    debugPrint( "mac addr" , ":".join(mac_addr) )
    debugPrint( "Version Number", struct.unpack('<B',rfidInput[dnl+8])[0] )
    
    calculate_crc = []
    with token_lock:
        calculate_crc.append(crc16.crc16(device_salt+tokens[0]+''.join(rfidInput[:-2])))
        calculate_crc.append(crc16.crc16(device_salt+tokens[1]+''.join(rfidInput[:-2])))
    if packetcrc in calculate_crc:
        if dob_hello_check("".join(rfidInput[2:dnl+2]), ":".join(mac_addr), struct.unpack('<B',rfidInput[dnl+8])[0]):
            return {'response':response(rfidInput[0], 'Ok')}
        else:
            return {'response':response(rfidInput[0], 'Unknown')}
    else:
        return {'response':response(rfidInput[0], 'ChecksumFail')}

# Register Heart Beat with the database
def heartBeat(rfidInput):
    dnl = struct.unpack('<B',rfidInput[1])[0] # device name length
    if dob_heartBeat_register("".join(rfidInput[2:dnl+2])):
        return {'response':response(rfidInput[0], 'Ok')}
    else:
        return {'response':response(rfidInput[0], 'Fail')}
    
# Check user is allowed in
def authenticate(rfidInput):
    dnl = struct.unpack('<B',rfidInput[1])[0] # device name length
    packetcrc = struct.unpack('<H',''.join(rfidInput[-2:]))[0]
    device_salt = dob_get_salt("".join(rfidInput[2:dnl+2]))
    scanned_card = struct.unpack('<L', ''.join(rfidInput[-6:-2]))[0]
    
    calculate_crc = []
    tokens_list = []
    with token_lock:
        calculate_crc.append(crc16.crc16(device_salt+tokens[0]+''.join(rfidInput[:-2])))
        calculate_crc.append(crc16.crc16(device_salt+tokens[1]+''.join(rfidInput[:-2])))
        # Do a copy not DON'T point to the data
        tokens_list.append(tokens[0])
        tokens_list.append(tokens[1])
    
    if packetcrc in calculate_crc:
        if packetcrc == calculate_crc[0]:
            tokens_list.pop(1)
        else:
            tokens_list.pop(0)
        
        if dob_check_user("".join(rfidInput[2:dnl+2]), scanned_card):
            resp = response(rfidInput[0], 'Ok')
        else:
            resp = response(rfidInput[0], 'Fail')
    else:
        return {'response':response(rfidInput[0], 'ChecksumFail')}
    
    return {'response':resp + struct.pack('<H', crc16.crc16(device_salt+tokens_list[0]+resp))}

def acknowledge():
    return {}


class ReponseTestCase(unittest.TestCase):
    device_name = 'front-door'
    mac_address = "00:23:6c:95:d1:c6"
    version_num = 1
    device_salt = "djhldfsljhfdhjlkfsd"
    allowed_cards_list = [12344010]
    deny_cards_list    = [3090264]
    
    def checkResponce(self, expectedReturn, returnedData):
        if returnedData[0] == expectedReturn:
            if returnedData[1] == 'E': 
                debugPrint( "Error code returned.." )
                debugPrint( returnedData[:2], struct.unpack('<B', returnedData[2])[0], struct.unpack('<B', returnedData[3])[0], returnedData[4:] )
                return False
                
            debugPrint( "Response: %s%d"% (returnedData[0], struct.unpack('<B', returnedData[1])[0] ) )
            if struct.unpack('<B', returnedData[1])[0] == 1:
                return True
            else:
                debugPrint( "Test Failed.." )
        else:
            debugPrint( "Bad Responce:", [returnedData] )
        return False
    
    def test_sendToken_token(self):
        t = sendToken('T'+struct.pack('<B', 0))['response']
        with token_lock:
            self.assertEqual(''.join(t[1:-2]), tokens[0])
            
    def test_sendToken_crc(self):
        t = sendToken('T'+struct.pack('<B', 0))['response']
        self.assertEqual(crc16.crc16(''.join(t[:-2])), struct.unpack('<H', ''.join(t[-2:]))[0])

    def test_sendHello(self):
        device_len = struct.pack('<B', len(self.device_name))
        mac_add = ''
        for octet in self.mac_address.split(":"):
            mac_add += struct.pack('<B', int(octet, 16))
        v_num = struct.pack('<B', self.version_num)
        send_hello = 'H' + device_len + self.device_name + mac_add + v_num
        send_hello += struct.pack('<H', crc16.crc16(self.device_salt + tokens[0] + send_hello))
        
        h = hello(send_hello)['response']
        self.failUnless(self.checkResponce('H',h))    

    def test_heartBeat(self):
        device_len = struct.pack('<B', len(self.device_name))
        
        send_heartBeat = 'B' + device_len + self.device_name
        send_heartBeat += struct.pack('<H', crc16.crc16(send_heartBeat))
        
        b = heartBeat(send_heartBeat)['response']
        self.failUnless(self.checkResponce('B',b))
    
    def test_auth_CRC(self):
        device_len = struct.pack('<B', len(self.device_name))
        send_auth = '@' + device_len + self.device_name + struct.pack('<L', self.allowed_cards_list[0])
        send_auth += struct.pack('<H', crc16.crc16(self.device_salt + tokens[0] + send_auth))
        
        auth = authenticate(send_auth)['response']
        self.assertEqual(struct.unpack('<H', auth[-2:])[0], crc16.crc16(self.device_salt + tokens[0] + auth[:-2]))

    def test_auth_allowed(self):
        device_len = struct.pack('<B', len(self.device_name))
        for card in self.allowed_cards_list:
            send_auth = '@' + device_len + self.device_name + struct.pack('<L', card)
            send_auth += struct.pack('<H', crc16.crc16(self.device_salt + tokens[0] + send_auth))
            
            auth = authenticate(send_auth)['response']
            self.failUnless(self.checkResponce('@',auth))

    def test_auth_deny(self):
        device_len = struct.pack('<B', len(self.device_name))
        for card in self.deny_cards_list:
            send_auth = '@' + device_len + self.device_name + struct.pack('<L', card)
            send_auth += struct.pack('<H', crc16.crc16(self.device_salt + tokens[0] + send_auth))
            
            auth = authenticate(send_auth)['response']
            self.failIf(self.checkResponce('@',auth))

init_socket_options()

if __name__ == '__main__':
    unittest.main()