import numpy as np
from scipy import signal
import matplotlib.pylab as plt

def main ():
    #datos para pasabajos.
    fs = 12000 #sample frequency.
    fp = 1200
    fa = 2200
    Ap = 3
    Aa = 50

    #calculamos el nuevo Aa
    dp = (10**(Ap/20) - 1)/(10**(Ap/20) + 1)
    da = 10**-(Aa/20)
    d = min(dp, da)
    Aa = -20*np.log10(d)

    #calculamos alfa
    if Aa <= 21:
        alfa = 0
    elif Aa > 21 and Aa <= 50:
        alfa = 0.5842*(Aa - 21)**0.4 + 0.07886*(Aa - 21)
    elif Aa > 50:
        alfa = 0.1102*(Aa - 8.7)

    #calculamos N
    if Aa <= 21:
        D = 0.9222
    elif Aa > 21:
        D = (Aa - 7.95)/14.36
    Bt = 2*np.pi*(fa - fp)
    N = np.ceil((2*np.pi*fs*D)/Bt + 1) #redondeamos para arriba.
    if N%2 == 0: #si es par
        N = N + 1 #lo convierto a impar
    print(N)

    #calculamos la respuesta impulsiva (h) de un filtro pasabajos ideal.
    wc = np.pi*(fp + fa)
    n = np.linspace(0, N-1, int(N))
    h = np.zeros(int(N))
    for i in range(len(n)):
        if n[i]-(N-1)/2 != 0:
            h[i] = 1/((n[i]-(N-1)/2)*np.pi)*np.sin((n[i]-(N-1)/2)*wc/fs)
        else:
            h[i] = wc/(np.pi*fs)

    #calculamos la respuesta impulsiva (h).
    h = h*np.kaiser(M = N, beta = alfa)

    #calculamos la respuesta al escalon (u).
    num = h
    den = np.zeros(int(N))
    den[0] = 1
    t, u = signal.dstep((num, den, 1/fs), n = N)

    #calculamos la respuesta en frecuencia (H)
    w, H = signal.freqz(h,1)
    f = (fs*w)/(2*np.pi)

    #ploteamos la respuesta en frecuencias.
    plt.figure(1)
    plt.ylabel("mag [dB]")
    plt.xlabel("freq [Hz]")
    plt.plot(f, 20*np.log10(np.abs(H)))
    plt.grid(True)
    #ploteamos la fase en frecuencias.
    plt.figure(2)
    plt.ylabel("angle [deg]")
    plt.xlabel("freq [Hz]")
    plt.plot(f, 180/np.pi*np.angle(H))
    plt.grid(True)
    #ploteamos la respuesta impulsiva.
    plt.figure(3)
    plt.ylabel("amp [V]")
    plt.xlabel("time [sec]")
    plt.plot(t, h, 'o')
    plt.grid(True)
    #ploteamos la respuesta al escalon.
    plt.figure(4)
    plt.ylabel("amp [V]")
    plt.xlabel("time [sec]")
    plt.step(t, np.squeeze(u), 'o')
    plt.grid(True)
    plt.show()

if __name__ == "__main__":
    main()