
import serial,time #You need the pyserial library
import struct

ser = serial.Serial('/dev/ttyACM0', 250000, timeout=0)
#time.sleep(10);#my arduino bugs if data is written to the port after opening it
romsize=1024

print("      ________    _________    _________    _________    _____________ ")
print("    /  ______/|  /  ___   /|  /  ___   /|  /   __   /|  /            /|")
print("   /  /_____ |/ /  /__/  / / /  /__/  / / /  /  /  / / /  /\   /\   / /")
print("  /  ______/|  /   _____/ / /     ___/ / /  /  /  / / /  / /  / /  / / ")
print(" /  /______|/ /  / _____|/ /  /\  \__|/ /  /__/  / / /  / /  / /  / /  ")
print("/________/|  /__/ /       /__/ /\__/|  /________/ / /__/ /__/ /__/ /   ")
print("|________|/  |__|/        |__|/ |__|/  |________|/  |__|/|__|/|__|/    ")
print("\n")
print("  Robson Couto 2016")
print("  www.dragaosemchama.com.br")
print("  github.com/robsoncouto/eprom\n")

#Default value, 1MB chip:
romsize=1*1024*1024
numsectors=int(romsize/128) # I am sending data in 128 byte chunks


while True:

    print("  What would you like to do?      ")
    print("                                  ")
    print("          1-Read eprom            ")
    print("          2-burn eprom            ")
    print("          3-about this script     ")
    print("          4-blank check           ")
    print("          5-select chip size      ")
    print("                                  ")
    print("          6-quit                \n")

    option=int(input("Please insert a number:"))

    if(option==1):
        name=input("What the name of the file?")

        f = open(name, 'w')
        f.close()
        ser.flushInput()
        ser.write(b"\x55")
        ser.write(bytes("r","ASCII"))
        numBytes=0
        f = open(name, 'ab')
        while (numBytes<romsize):
            while ser.inWaiting()==0:
                print("Reading from eprom. Current porcentage:{:.2%}".format(numBytes/romsize),end='\r')
                time.sleep(0.1)
            data = ser.read(1)#must read the bytes and put in a file
            f.write(data)
            numBytes=numBytes+1
        f.close()
        print("Done\n")
    if(option==2):
        name=input("What's the name of the file?")
        print(name)
        f = open(name, 'rb')
        for i in range(numsectors):
            ser.write(b"\x55")
            ser.write(bytes("w","ASCII" ))
            time.sleep(0.001)
            ser.write(struct.pack(">B",i>>8))
            CHK=i>>8
            time.sleep(0.001)
            ser.write(struct.pack(">B",i&0xFF))
            CHK^=i&0xFF
            time.sleep(0.001)
            data=f.read(128);
            #print(data)
            for j in range(len(data)):
                 CHK=CHK^data[j]
            time.sleep(0.001)
            print("Writing data. Current porcentage:{:.2%}".format(i/numsectors),end='\r')
            #print("CHK:", CHK)
            response=~CHK
            while response!=CHK:
                ser.write(data)
                ser.write(struct.pack(">B",CHK&0xFF))
                timeout=30
                while ser.inWaiting()==0:
                    time.sleep(0.01)
                    timeout=timeout-1
                    if timeout==0:
                        print("could not get a response, please start again\n")
                        break
                response=ord(ser.read(1))
                if response!=CHK:
                    print("wrong checksum, sending chunk again\n")
        f.close()

    if(option==3):

        print("\nA more detailed write up about this project is available at www.dragaosemchama.com.br")
        print("This script goes together with a Arduino sketch, both are used to read and program")
        print("eproms on the cheap.")
        print("Written by Robson Couto\n")
    if(option==4):
        ser.flushInput()
        ser.write(b"\x55")
        ser.write(bytes("r","ASCII"))
        numBytes=0
        blank=1
        while (numBytes<romsize):
            while ser.inWaiting()==0:
                print("Reading from eprom. Current porcentage:{:.2%}".format(numBytes/romsize),end='\r')
                time.sleep(0.1)
            data = ser.read(1)
            numBytes=numBytes+1
            if ord(data)!=255:
                blank=0
                break
        if blank==1:
            print("\nThe chip is blank\n")
        else:
            print("\nThe chip seems to contain data\n")
        print("Done\n")
    if(option==5):
        print("Current eprom size:",romsize/(1024*1024),"MB\n")
        megs=float(input("Please insert the size of the eprom in Megabytes"))
        romsize=megs*1024*1024
        numsectors=int(romsize/128) # I am sending data in 128 byte chunks
        print("Eprom size changed to ",romsize/(1024*1024),"MB\n")


    if(option==6):
        print("See ya!")
        break
