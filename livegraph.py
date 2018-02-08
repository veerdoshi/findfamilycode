import urllib, json
import matplotlib.pyplot as plt
#matplotlib.use('TkAgg')
#import sys
#print sys.getdefaultencoding()
#from http.server import SimpleHTTPRequestHandler, HTTPServer
import matplotlib.animation as animation
import random
import time
import mpld3
from twilio.rest import Client


fig = plt.figure()
ax1 = fig.add_subplot(1, 1, 1)

account_sid = "********************************"
auth_token  = "********************************"
client = Client(account_sid, auth_token)

def animate(i):
    graphdata = []
    x = 32
    textsend = True
    sumof = 0
    calibration = 0
#    serveraddress = HTTPServer('127.0.0.1', 8888)
#    httpd = HTTPServer(serveraddress, SimpleHTTPRequestHandler)
    for i in range (1, x):
        data = urllib.urlopen("http://findfamily.co/quakes").read()
        output = json.loads(data)
        graphelement = output["quakes"][i]["magnitude"]
        print(graphelement)
        if (i<20):
            sumof = sumof + graphelement
        if (i==20):
            calibration = sumof/20
        if (i>20):
            if ((graphelement > (calibration + 50)) or (graphelement < (calibration - 50))):
                message = client.messages.create(
                    to="+14082187162",
                    from_="+14083594824 ",
                    body="Emergency Alert. Earthquake in San Jose, CA. Get to safety.")
                print("Alert Sent")

        neggraphelement = graphelement * -1
        graphdata.append(graphelement)
        graphdata.append(neggraphelement)

    ax1.clear()
    ax1.plot(graphdata, color="black")
    mpld3.show(fig=None, ip='127.0.0.1', port=8888, n_retries=50, local=True, open_browser=True, http_server=None)[source]
    time.sleep(2)
    #httpd.shutdown()


ani = animation.FuncAnimation(fig, animate, interval=1000)
plt.show()
