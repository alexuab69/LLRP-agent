import socket

s = socket.socket()
s.connect(("169.254.116.254",5084))

print("Connected to LLRP reader")