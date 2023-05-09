import numpy as np
from scipy import signal
import matplotlib.pylab as plt

def main ():
    #from my program
    h = [-0.000193843232,  0.000741663400,  0.00216954197,  0.00200690548,
        -0.00156648218, -0.00686607503, -0.00806394630,  0,  0.0144164296,
        0.0222006915,  0.00939122198, -0.0232666492,  -0.0520410072,
        -0.0411515043,  0.0305206168,  0.146638914,  0.255488475,  0.3,
        0.255488475,  0.146638914,  0.0305206168, -0.0411515043, -0.0520410072,
        -0.0232666492,  0.00939122198,  0.0222006915,  0.0144164296,  0,
        -0.00806394630, -0.00686607503, -0.00156648218,  0.00200690548,
        0.00216954197,  0.000741663400, -0.000193843232]
    #from fdatool (matlab)
#     h = [  0.0001010395245011,  0.00117887892105, 0.002050177882161,0.0005712730519797,
#    -0.00385998542783,-0.007723459766351,-0.004927441981018,  0.00667963385154,
#     0.01936505662795,  0.01827009313005,-0.005301164628446,  -0.0397201726459,
#    -0.05298946183652, -0.01281561445299,  0.08535387880299,   0.2055360479448,
#       0.288231221002,    0.288231221002,   0.2055360479448,  0.08535387880299,
#    -0.01281561445299, -0.05298946183652,  -0.0397201726459,-0.005301164628446,
#     0.01827009313005,  0.01936505662795,  0.00667963385154,-0.004927441981018,
#   -0.007723459766351, -0.00385998542783,0.0005712730519797, 0.002050177882161,
#     0.00117887892105,0.0001010395245011]

    #fsk signal generator. 111111111100110100101111111111. 1s(idle),0(start),01101001(data),0(parity),1(stop),1s(idle)
    x = np.zeros(10*30) #each 10 samples we have a bit.
    t = np.linspace(0, len(x)/12000, len(x))
    for i in range(len(x)):
        if (i < 100):
            x[i] = 1.0*(np.sin(2*np.pi*1200*t[i]) + 0) #ten 1s
        elif (i >= 100 and i < 120):
            x[i] = 1.0*(np.sin(2*np.pi*2400*t[i]) + 0) #two 0s
        elif (i >= 120 and i < 140):
            x[i] = 1.0*(np.sin(2*np.pi*1200*t[i]) + 0) #two 1s
        elif (i >= 140 and i < 150):
            x[i] = 1.0*(np.sin(2*np.pi*2400*t[i]) + 0) #one 0
        elif (i >= 150 and i < 160):
            x[i] = 1.0*(np.sin(2*np.pi*1200*t[i]) + 0) #one 1
        elif (i >= 160 and i < 180):
            x[i] = 1.0*(np.sin(2*np.pi*2400*t[i]) + 0) #two 0s
        elif (i >= 180 and i < 190):
            x[i] = 1.0*(np.sin(2*np.pi*1200*t[i]) + 0) #one 1
        elif (i >= 190 and i < 200):
            x[i] = 1.0*(np.sin(2*np.pi*2400*t[i]) + 0) #one 0
        elif (i >= 200 and i < 300):
            x[i] = 1.0*(np.sin(2*np.pi*1200*t[i]) + 0) #ten 1

    #fsk signal generator delayed. 11001
    d = np.zeros(len(x))
    for i in range(len(x)):
        if(i < 5):
            d[i] = 0 
        else:
            d[i] = x[i-5] 
    
    #multipling delay
    x_ = x*d

    #signal processed.
    firx_ = np.zeros(len(h)) #fir input
    y_ = np.zeros(len(x_))
    y = np.zeros(len(y_)) #during comparator.
    frame = np.zeros(11) #after comparator. frame.
    startf = False #to build the frame.
    count = 0
    bit = 0
    for j in range(len(y_)):
        shiftarray(firx_, 1)
        firx_[0] = x_[j]
        for i in range(len(h)):
            y_[j] += h[i]*firx_[i]

        #comparator
        if (y_[j] > 0.25):
            y[j] = 0
        else:
            y[j] = 1

        #building the frame
        if(y[j] == 0): #start bit.
            startf = True
        if(startf == True):
            frame[bit] += y[j]
            count += 1
            if(count == 10):
                count = 0
                bit += 1
                if(bit == 11):
                    startf = False
                    print(frame)
                    for k in range(len(frame)):
                        if(frame[k] >= 5):
                            frame[k] = 1
                        else:
                            frame[k] = 0
                    
    print(frame)
    
    plt.ylabel("amp [V]")
    plt.xlabel("time [sec]")
    plt.plot(t, x, 'r')
    #plt.plot(t, d, 'g')
    #plt.plot(t, x_, 'b')
    plt.plot(t, y_, 'c')
    plt.plot(t, y, 'm')
    plt.grid(True)
    plt.show()

def shiftarray (x, shift):
    for j in range(shift):
        temp1 = x[0]
        x[0] = 0
        for i in range(1, len(x)):
            temp2 = x[i]
            x[i] = temp1
            temp1 = temp2

if __name__ == "__main__":
    main()