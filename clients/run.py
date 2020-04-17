import os
import time

N_SENSORS = 5
N_SINK = 0

IP = "192.168.178.20"


def main():
    a = 0
    while True:
        start_time = time.time()

        for i in range(0, N_SENSORS):
            command = f'docker run --init -it --rm --ip=“192.168.178.176” efrecon/mqtt-client pub -h {IP} -t "test" -m "Hi from client {i}"'
            os.system(command)

        print(f"flow {a}")

        time.sleep(5.0 - ((time.time() - start_time) % 60.0))
        a += 1


if __name__ == "__main__":
    main()
