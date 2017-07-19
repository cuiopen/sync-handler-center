#!/usr/bin/python
#encoding=utf8

import socket
import struct
import hashlib
import sys
import time

def send_data(send_buf, ip = None, port = None, log = False):
    s = socket.socket(socket.AF_INET)
    s.connect((ip, port))
    if log: print("-------------NEW CONNECTION--------------")
    send_len = s.send(send_buf)
    if log: print("send_len:", send_len)
    recv_buf = s.recv(4)
    while len(recv_buf) < 4:
        recv_buf += s.recv(4 - len(recv_buf))
    recv_len = struct.unpack("=L", recv_buf[:4])[0]
    while len(recv_buf) < recv_len:
        recv_buf += s.recv(recv_len - len(recv_buf))
    if log: print("recv_len:", recv_len)
    s.close()
    return recv_buf

def pack(user_id,youxi_bi, trans_id):
    body_buf = struct.pack("=LLLLLLL16s", game_id, user_id, 1, 1, youxi_bi, trans_id, 0, "")
    head_buf = struct.pack("=LLHLL", 18 + len(body_buf), 0, 6001, 0, user_id)
    return head_buf + body_buf


def unpack(recv_buf):
    if len(recv_buf) < 18: raise Exception()
    return struct.unpack("=LLHLL", recv_buf[:18])

if __name__ == '__main__':
    for i in range(1, 20000):
        PORT = 21199
        user_id = 721018944
        youxi_bi = 45000
        trans_id = 2467
        #IP = '10.1.1.35'
        IP = '10.1.1.151'
        game_id = 2
        print unpack(send_data(pack(user_id, youxi_bi, trans_id),IP, PORT, log=True))
        time.sleep(0.001);

