from time import sleep
from libs.c_tcp_lib.tcp_client import TcpClient

from serial import Serial

while True:
    try:
        tcp_conn = TcpClient('localhost', 44440)
        break
    except Exception as exception:
        sleep(0.1)


def connect_serial(port: str) -> Serial:
    while True:
        try:
            return Serial(port)
        except Exception as exception:
            sleep(0.1)


wheel_conn = connect_serial("wheel")
radar_conn = connect_serial("radar")

try:
    while True:
        command = tcp_conn.Receive(1000)
        if len(command) == 0:
            continue

        arduino = command[0]

        message = '\n'.join(command[1:])
        (wheel_conn if arduino == "0" else radar_conn).write(message.encode('utf-8'))

        if arduino == "0":
            step_num = int(command[1])
            while True:
                step = int(wheel_conn.readline().decode('utf-8'))
                tcp_conn.Send(step)
                if step == step_num:
                    break
        else:
            while True:
                data = radar_conn.readline().decode('utf-8')
                tcp_conn.Send(data)
                if data.strip().isnumeric():
                    break


except Exception as exception:
    print(exception)
