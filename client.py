#!/usr/bin/env python
#-*- coding: utf-8 -*-

import socket
import sys
import thread

BUF_LEN = 1024


def recv_func(client):
    while True:
        try:
            data = client.recv(BUF_LEN)
        except:
            break
        if not data or not len(data):
            break
        print data
    client.close()
    sys.exit(0)


def process(client):
    thread.start_new_thread(recv_func, (client, ))
    while True:
        try:
            data = raw_input()
            if data == 'bye':
                break
            # data += '\n'
            client.sendall(data)
        except:
            break
    client.close()
    sys.exit(0)

if len(sys.argv) != 3:
    print 'Usage: ./client.py [ip] [port]'
    sys.exit(1)

HOST = sys.argv[1]
PORT = int(sys.argv[2])

client = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)
client.connect((HOST, PORT))

process(client)
client.close()
