import socket
import sys
import netifaces as ni
import json
import matplotlib.pyplot as plt
import matplotlib.animation as animation
import math
import sys
import time

status = 1
excessing_hrm = 0
excessing_acc = 0

def recv_raw(connection):
    size = 96
    f_size = 0
    disp = []
    while f_size != size:
        disp += connection.recv(size - f_size)
        f_size = len(disp)
    return disp

def handle_close(evt):
    global status
    status = 0

def is_vitals_ok():
    return not (excessing_hrm or excessing_acc)

an_acc_last_time = 0
an_count_acc = 1
an_sum_acc = 0
an_acc_avg = 0
an_acc_spike_count = 0
def analytica_acc(acc):
    global an_acc_last_time
    global excessing_acc
    global an_count_acc
    global an_sum_acc
    global an_acc_avg
    global an_acc_spike_count

    if (an_acc_last_time == acc['time']):
        return;

    an_sum_acc += acc['acc'][0]
    an_sum_acc = 0
    an_acc_avg = an_sum_acc/an_count_acc

    if (abs(acc['acc'][0] - an_acc_avg) > 0.3):
        an_acc_spike_count += 1

    if (an_acc_spike_count > 3):
        excessing_acc = 1
        an_acc_spike_count = 0
        print "Your motion is extremely active"
    else:
        excessing_acc = 0

    an_acc_last_time = acc['time']

an_hrm_ltime = 0
an_count_hrm = 1
an_sum_hrm = 0
an_hrm_avg = 0
an_hrm_spike_count = 0
def analytica_hrm(hrm):
    global an_hrm_ltime
    global excessing_hrm
    global an_count_hrm
    global an_sum_hrm
    global an_hrm_avg
    global an_spike_count

    if (an_hrm_ltime == hrm['time']):
        return;

    an_sum_hrm += hrm['hrm']
    an_count_hrm += 1
    an_hrm_avg = an_sum_hrm / an_count_hrm
    print "Avg bpm ", an_hrm_avg

    an_hrm_ltime = hrm['time']

acc = {'time': 0, 'acc': [0, 0, 0]}
hrm = {'time': 0, 'hrm': 0}
init_time = 0

file_acc = "accel.csv"
file_hrm = "hrm.csv"

# Figure rendering
fig = plt.figure()
fig.canvas.mpl_connect('close_event', handle_close)
ax = fig.add_subplot(1, 1, 1)

fig2 = plt.figure()
fig2.canvas.mpl_connect('close_event', handle_close)
ax2 = fig2.add_subplot(1, 1, 1)

p_time_hrm = []
p_time_acc = []
p_hrm = []
p_acc = []

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
            hrm['time'] = (json_data['time'] - init_time)/1000.0
            hrm['hrm'] = json_data['heart_rate']
            f_hrm.write("%.2f,%d\n" % (hrm['time'], hrm['hrm']))

            p_time_hrm.append(acc['time'])
            p_hrm.append(hrm['hrm'])
            p_time_hrm = p_time_hrm[-100:]
            p_hrm = p_hrm[-100:]
            plt.figure(2)
            ax2.clear()
            ax2.plot(p_time_hrm, p_hrm)
            plt.ylim(50, 130)
            plt.title('Heart rate over the time')
            #plt.xticks(rotation=45, ha='right')
            plt.ylabel('Heart rate (bpm)')
            plt.xlabel('Time (sec)')

        if 'accX' in json_data:
            acc['time'] = (json_data['time'] - init_time)/1000.0
            acc['acc'][0] = json_data['accX']
            acc['acc'][1] = json_data['accY']
            acc['acc'][2] = json_data['accZ']
            f_acc.write("%.3f,%.3f,%.3f,%.3f\n" % (acc['time'], acc['acc'][0], acc['acc'][1], acc['acc'][2]))

            p_time_acc.append(acc['time'])
            p_acc.append(acc['acc'][0])
            p_time_acc = p_time_acc[-50:]
            p_acc = p_acc[-50:]
            plt.figure(1)
            ax.clear()
            ax.plot(p_time_acc, p_acc)
            plt.ylim(-1.5, 1.5)
            plt.title('Acceleration over the time')
            #plt.xticks(rotation=45, ha='right')
            plt.ylabel('Acceleration (m/sec^2)')
            plt.xlabel('Time (sec)')

        if 'init_time' in json_data:
            init_time = json_data['init_time']
            continue

        analytica_acc(acc)
        analytica_hrm(hrm)
        plt.pause(0.005)

        if not status:
            raise ValueError('Window is closed')

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
