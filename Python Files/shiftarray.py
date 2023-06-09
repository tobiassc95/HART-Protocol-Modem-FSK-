import numpy as np
import matplotlib.pylab as plt

def main ():
    # h = [-0.000193843232,  0.000741663400,  0.00216954197,  0.00200690548,
	# 					 -0.00156648218, -0.00686607503, -0.00806394630,  0,
	# 					 0.0144164296,  0.0222006915,  0.00939122198, -0.0232666492,
	# 					 -0.0520410072, -0.0411515043,  0.0305206168,  0.146638914,
	# 					 0.255488475,  0.3,  0.255488475,  0.146638914,
	# 					 0.0305206168, -0.0411515043, -0.0520410072, -0.0232666492,
	# 					 0.00939122198,  0.0222006915,  0.0144164296,  0,
	# 					 -0.00806394630, -0.00686607503, -0.00156648218,  0.00200690548,
	# 					 0.00216954197,  0.000741663400, -0.000193843232]
    # #print(h)
    # plt.plot(h, 'o')
    # plt.grid(True)
    # plt.show()

    #load x
    x = np.zeros(5)
    for i in range(len(x)):
        x[i] = i+1
    print(x)

    #shift x
    for j in range(len(x)):
        temp1 = x[0]
        x[0] = 0
        for i in range(1, len(x)):
            temp2 = x[i]
            x[i] = temp1
            temp1 = temp2
        print(x)

if __name__ == "__main__":
    main()