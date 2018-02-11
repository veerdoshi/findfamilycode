import urllib, json
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import time
import mpld3
from twilio.rest import Client


fig = plt.figure()
ax1 = fig.add_subplot(1, 1, 1)

account_sid = "**********************************"
auth_token  = "**********************************"
client = Client(account_sid, auth_token)

def animate(i):
    graphdata = []
    xcoordinate = []
    textsend = True
    sumof = 0
    calibration = 0
    data = urllib.urlopen("http://findfamily.herokuapp.com/quakes").read()
    output1 = json.loads(data)
    output = output1['quakes']
    x = len(output)

    for i in range (1, x):
        graphelement = output1["quakes"][i]["magnitude"]
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

        graphdata.append(graphelement)
        xcoordinate.append(i)

    ax1.clear()
    ax1.plot(xcoordinate, graphdata, color="blue")
    ax1.fill_between(xcoordinate, graphdata, color="lightblue")
    mpld3.show(fig=None, ip='127.0.0.1', port=8888, n_retries=50, local=True, open_browser=True, http_server=None)[source]
    time.sleep(2)

ani = animation.FuncAnimation(fig, animate, interval=1000)
plt.show()
