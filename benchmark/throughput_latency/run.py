# Run from the top level directory:
#     python3 benchmark/throughput_latency/run.py

import matplotlib.pyplot as plt
import subprocess

OUTPUT_FILE = 'out.txt'

def parse_output():
    throughput = 0
    average_latency = 0.0

    with open(OUTPUT_FILE, 'r') as f:
        for line in f:
            # Remove timestamp and location info.
            split_line = line.split(' ')
            line = ' '.join(split_line[4:])
            if line.startswith('throughput'):
                throughput = int(line.split(' ')[1])
            elif line.startswith('average latency'):
                # Chop off decimal portion of latency, as it is small compared
                # to the overall latency.
                average_latency = int(float(line.split(' ')[2]))

    return throughput, average_latency


def run_once(points, num_clients, requests_per_client):
    subprocess.call(['./benchmark/throughput_latency/run_once.sh', str(num_clients), str(requests_per_client), OUTPUT_FILE])
    throughput, average_latency = parse_output()
    print(f'average latency: {average_latency} microseconds, throughput: {throughput} req/s')
    print('=============================')
    points += [(throughput, average_latency)]


if __name__ == "__main__":
    points = []

    run_once(points, 1, 10000)
    run_once(points, 5, 2000)
    run_once(points, 10, 1000)
    run_once(points, 50, 200)
    run_once(points, 100, 100)
    run_once(points, 250, 40)
    # run_once(points, 500, 20)
    # run_once(points, 1000, 10)

    x = [x[0] for x in points]
    y = [x[1] for x in points]
    print(points, x, y)

    plt.plot(x, y)
    plt.plot(x, y, 'or')
    plt.xlabel('throughput (req/s)')
    plt.ylabel('latency (microseconds)')
    plt.show()
