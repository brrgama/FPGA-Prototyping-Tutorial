
#CLIENT
import socket
import numpy as np
import struct
import math

#Calculate the impedance value
def Zcalculation(Rsh, ch1, ch2):
    rmsCH1 = RMS(ch1)    
    rmsCH2_CH1 = RMS(arrSubtraction(ch2, ch1))
    return Rsh*(rmsCH1/rmsCH2_CH1)

#Calculate the RMS value   
def RMS(arr): 
    square = 0
    mean = 0.0
    root = 0.0      
    
    for i in range(0,len(arr)): 
        square += arr[i]**2    
      
    mean = (square / (float)(len(arr)))       
     
    root = math.sqrt(mean) 
      
    return root

#Calculate the difference between two lists
def arrSubtraction (arr1, arr2):    
    difference = [0 for i in range(len(arr1))]
    for i in range (0, len(arr1)):
        difference[i] = arr1[i] - arr2[i]
    return difference

#Generate list of sampling frequency
def samplingFrequency (generationFrequency):      
    samplingCode = [0 for n in range(len(generationFrequency))]
    for i in range (len(generationFrequency)):
        for j in range(16,0,-1):
            samplingFreq = 125 * pow(10,6) / pow(2,j)            
            if (samplingFreq/generationFrequency[i] >= 19):
                samplingCode[i] = j+1
                break            
    return samplingCode

#Save impedance list to text file
def saveToTxt(arr):
    Z_file = open("Impedance Spectrum.txt", "w")
    
    for item in arr:
        Z_file.write("%s\n" % item)
    Z_file.close()


#Initial sweep frequency
initialFrequency = 1000.0
#Final sweep frequency
finalFrequency = 1000000.0
#Number of points between initial and final frequency
n=100  
#Impedance vector
Z = [0 for i in range(n)]

#Sweep frequencies 
logspaceFrequency = np.logspace(np.log10(initialFrequency), np.log10(finalFrequency), n, endpoint=True, base=10.0, dtype=int) 
samplingFrequencyCode = samplingFrequency(logspaceFrequency) 
  
print("Performing the frequency sweep...\n")

for i in range (n):

    #AF_INET corresponds to IVP4 and SOCK_STREAM corresponds to TCP
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
    s.connect(('192.168.0.35',3030))#IP and Port of the server    

    msg = "SDAC"
    #Send command message to server 
    s.send(bytes(msg,"utf-8"))    
    msg = str(logspaceFrequency[i])
    #Send frequency value to server
    s.send(bytes(msg,"utf-8"))
        
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
    s.connect(('192.168.0.35',3030))#IP and Port of the server    

    msg = "SADC"
    #Send command message to server 
    s.send(bytes(msg,"utf-8"))    
    msg = str(samplingFrequencyCode[i])
    #Send frequency value to server
    s.send(bytes(msg,"utf-8"))   

    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
    s.connect(('192.168.0.35',3030))#IP and Port of the server  
              
    msg = "SAVE"
    #Send command message to server 
    s.send(bytes(msg,"utf-8"))    
 
    s = socket.socket(socket.AF_INET, socket.SOCK_STREAM) 
    s.connect(('192.168.0.35',3030))#IP and Port of the server  

    msg ="TRSF"
    chData = []
    ch1 = [0 for n in range(64)]
    ch2 = [0 for n in range(64)]
    #Send command message to server
    s.send(bytes(msg,"utf-8"))
    for j in range(64):
        #Receives data from channel 1
        msg = "RDY_"
        s.send(bytes(msg,"utf-8"))
        chData = s.recv(1024)            
        ch1[j] = int.from_bytes(chData, byteorder='little', signed=True)
                        
        #Receives data from channel 2
        msg = "RDY_"
        s.send(bytes(msg,"utf-8"))
        chData = s.recv(1024)
        ch2[j] = int.from_bytes(chData, byteorder='little', signed=True)

    for k in range(len(ch1)):
        ch1[k] = (ch1[k]/pow(2,12))
        ch2[k] = (ch2[k]/pow(2,12))
        
    Z[i] = Zcalculation(1000, ch1, ch2)
    Z_string = [str(x) for x in Z]
    saveToTxt(Z_string)

print("Frequency sweep is concluded")



   











                

        
