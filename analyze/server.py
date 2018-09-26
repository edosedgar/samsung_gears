import socket
import sys
import netifaces as ni
import json
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import math
import sys

acc = {'time': 0, 'acc': 0}
hrm = {'time': 0, 'hrm': 0}
init_time = 0

file_acc = "accel.csv"
file_hrm = "hrm.csv"

fig = plt.figure()
ax = fig.add_subplot(1, 1, 1)
xs = []
ys = []

def recv_raw(connection):
    size = 96
    f_size = 0
    disp = []
    while f_size != size:
        disp += connection.recv(size - f_size)
        f_size = len(disp)
    return disp

#===================================================================
def animate(i, connection, xs, ys):
    global init_time
    global acc
    global hrm
    global ax

    data = "".join(recv_raw(connection)).rstrip("\0")
    json_data = json.loads(data)
    if 'heart_rate' in json_data:
        hrm['time'] = json_data['time'] - init_time
        hrm['hrm'] = json_data['heart_rate']
    if 'accX' in json_data:
        acc['time'] = json_data['time'] - init_time
        acc['acc'] = math.sqrt(json_data['accX']**2 +
                               json_data['accY']**2 +
                               json_data['accZ']**2)
        xs.append(acc['time'])
        ys.append(acc['acc'])

    if 'init_time' in json_data:
        init_time = json_data['init_time']

    xs = xs[-100:]
    ys = ys[-100:]
    ax.clear()
    ax.plot(xs, ys)
    plt.title('Heart rate over time')
    plt.xticks(rotation=45, ha='right')
    plt.ylabel('Heart rate')
    plt.xlabel('Time (ms)')

#================================================================
# Figure out host IP address
ni.ifaddresses('wlp4s0')
ip = ni.ifaddresses('wlp4s0')[ni.AF_INET][0]['addr']
# Create a TCP/IP socket
sock = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_address = (ip, 14342)
print 'Starting up on %s port %s' % server_address
sock.bind(server_address)
# Listen for incoming connections
sock.listen(1)

while True:
    # Wait for a connection
    print 'Waiting for a connection'
    connection, client_address = sock.accept()
    print 'Got connection from', client_address

    data = connection.recv(16)
    if (data == "dbebf164ffefe13"):
        print 'Accepted, device handshake: ', data
        break
    else:
        print 'Handshake is not correct'
        connection.close()

print "Waiting for a data"

#ani = animation.FuncAnimation(fig, animate, fargs=(connection, xs, ys), interval=20)
#plt.show()

if (len(sys.argv) == 3):
    file_acc = sys.argv[2]
    file_hrm = sys.argv[1]

f_acc = open(file_acc, "w+")
f_hrm = open(file_hrm, "w+")

while True:
    try:
        data = "".join(recv_raw(connection)).rstrip("\0")
        json_data = json.loads(data)
        if 'heart_rate' in json_data:
            hrm['time'] = json_data['time'] - init_time
            hrm['hrm'] = json_data['heart_rate']
            f_hrm.write("%ld,%d\n" % (hrm['time'], hrm['hrm']))
        if 'accX' in json_data:
            acc['time'] = json_data['time'] - init_time
            acc['acc'] = math.sqrt(json_data['accX']**2 +
                                   json_data['accY']**2 +
                                   json_data['accZ']**2)
            print acc
            f_acc.write("%ld,%.3f\n" % (acc['time'], acc['acc']))
        if 'init_time' in json_data:
            init_time = json_data['init_time']

    except KeyboardInterrupt:
        connection.close()
        f_acc.close()
        f_hrm.close()
        break
    except ValueError:
        connection.close()
        f_acc.close()
        f_hrm.close()
        break
    except socket.error:
        connection.close()
        f_acc.close()
        f_hrm.close()
        break
