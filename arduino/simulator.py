from time import sleep
from libs.c_tcp_lib.tcp_client import TcpClient

radar_width = 1.8


def get_radar_input(angle: float) -> int:
    print("Angle: %0.2f" % angle)
    print("Enter result of scan in mm (2000 if distance > 2000 mm:")
    return int(input())


def get_step(curr_step: int) -> None:
    print("Current step: %d" % curr_step)
    print("Enter something if virtual motor made step:")
    input()


while True:
    try:
        tcp_conn = TcpClient('localhost', 44440)
        break
    except Exception as exception:
        sleep(0.1)

try:
    while True:
        command = tcp_conn.Receive(1000)
        if len(command) == 0:
            continue

        arduino = command[0]

        message = '\n'.join(command[1:])

        if arduino == "0":  # wheel move
            step_num = int(command[1])
            move_dir = int(command[2])
            speed = int(command[3])

            if move_dir == 0:
                move_dir = "left"
            elif move_dir == 1:
                move_dir = "forward"
            else:
                move_dir = "right"

            print("Computer want car to move")
            print("\tSteps: %d" % step_num)
            print("\tDirection: %s" % move_dir)
            print("\tSpeed: 1/%d" % speed)

            for i in range(step_num):
                get_step(i + 1)
                tcp_conn.Send(i + 1)

            print("Move done")
        else:
            print("Computer want car to make scan")

            for i in range(200):
                dist = get_radar_input(i * radar_width)
                tcp_conn.Send(i * radar_width, dist)

            tcp_conn.Send(0)
            print("Scan done")

except Exception as exception:
    print(exception)
