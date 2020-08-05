
#CLIENT
import socket

while(1):

    #AF_INET corresponds to IVP4 and SOCK_STREAM corresponds to TCP
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
    s.connect(('192.168.0.35',3030))#IP and Port of the server

    print("Connected to server")
    mode = input("Enter with 1 to start signal acquisition and with 2 to save the acquired data\n") 

    if mode == "1":
        msg = "SADC"
        #Send command message to server 
        s.send(bytes(msg,"utf-8"))
        sampligFrequency = input("Enter with samplig frequency code:\n")
        msg = sampligFrequency
        #Send frequency value to server
        s.send(bytes(msg,"utf-8"))   

    if mode == "2":
        msg = "SAVE"
        #Send command message to server 
        s.send(bytes(msg,"utf-8"))
        