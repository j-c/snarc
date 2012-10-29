#!/usr/bin/env python
# Access Gateway Controller
# @author: Luke Hovington

from datetime import datetime, timedelta
import time, calendar, os
import crc16, struct
import socket, select, sys
from response import *
import optparse
import threading

# Sanity check on startup
for descriptor in responseLookup:
    if len(descriptor) > int('FF', 16): # hex 255
        print "ERROR on descriptor", descriptor, "Description too long"
        sys.exit(1)
        
        if errorLookup[descriptor] > int('FF', 16): # hex 255
           print "ERROR on id", id, "id too long"
           sys.exit(1)

def client_parse(rfidInput, client_conn):
    clientReturn = None
    
    if rfidInput[0] in socket_options:
        clientReturn = socket_options[rfidInput[0]]['d'](rfidInput)
    else:
        clientReturn = {'response':response(rfidInput[0], 'InvalidOption')}
        
    if 'response' not in clientReturn:
        pass
    elif clientReturn['response'] is '':
        client_conn.send(response(rfidInput[0], 'ServerError'))
    else:
        print "Returning", [clientReturn['response']], "to the client"
        client_conn.send(clientReturn['response'])

def client_rxbyte(client, byte, conn):
    ''' Parse a byte received from a client. Once we receive an entire packet,
    send the packet out over serial. '''
    msgbuf = client['msgbuf']
    msgbuf.append(byte)
    print 'Recived char', byte
    # get packet length
    if len(msgbuf) > struct.calcsize('<B'):
        print "Input Size", len(msgbuf), socket_options[msgbuf[0]]['l']+struct.unpack('<B',msgbuf[1])[0]
        if len(msgbuf) == socket_options[msgbuf[0]]['l']+struct.unpack('<B',msgbuf[1])[0]:
            print "\nFull Packet", msgbuf[0]
            client_parse(msgbuf, conn)
            # clear the buffer
            client['msgbuf'] = []

# Rotate the token every 5 second
def token_rotation():
    while runThreads:
        with token_lock:
            tokens.pop()
            tokens.insert(0, gen_token())
            #print tokens
        time.sleep(5)

token_thread = threading.Thread(name='TokenRotator', target=token_rotation);
token_thread.start();

HOST = None               # Symbolic name meaning all available interfaces
s = None
PORT=8001

for res in socket.getaddrinfo(HOST, PORT, socket.AF_UNSPEC, socket.SOCK_STREAM, 0, socket.AI_PASSIVE):
    af, socktype, proto, canonname, sa = res
    try:
        s = socket.socket(af, socktype, proto)
    except socket.error, msg:
        s = None
        continue
    try:
        s.bind(sa)
        s.listen(10) # Listen for 10 clients
        s.settimeout(1)
    except socket.error, msg:
        s.close()
        s = None
        continue
    break
if s is None:
    runThreads = False
    print >> sys.stderr, 'Could not open socket'
    sys.exit(1)

clients = {}
try:
    select_timeout = 0.05
    while True:
        # wait until one of:
        #   1. A new client connects on one of the accept sockets
        #   2. New data comes in from one of the clients
        #   3. A timeout
        rr, rw, err = select.select([s] + clients.keys(), [], [], select_timeout)
        
        try:
            for sock in rr:
                # are there any new client connections?
                if sock == s:
                    (csock, (addr, port)) = sock.accept()
                    csock.setblocking(0)
                    clients[csock] = {'addr':addr, 'msgbuf':[]}
                    print >> sys.stderr, 'New connection from %s' % (addr)
                
                # have any of the clients sent us data?
                elif sock in clients:
                    try:
                        rxbytes = sock.recv(100)
                        if rxbytes == '':
                            raise socket.error
                        for byte in rxbytes:
                            client_rxbyte(clients[sock], byte, sock)
                    except socket.error:
                        # client died, or connection lost
                        c = clients.pop(sock)
                        print >> sys.stderr,  'Client from %s disconnected' % c['addr']
                        sock.close()
            
#            s.send(str(node_data_store))
        except socket.timeout:
            pass
except KeyboardInterrupt:
    runThreads = False
    print "\nPlease wait for connections to close.."
