# Run from the top level directory:
#     python3 benchmark/throughput_latency/run.py

import subprocess

OUTPUT_FILE = 'out.txt'

def parse_output():
    average_latency = 0.0
    throughput = 0

    with open(OUTPUT_FILE, 'r') as f:
        for line in f:
            # Remove timestamp and location info.
            split_line = line.split(' ')
            line = ' '.join(split_line[4:])
            if line.startswith('average latency'):
                average_latency = line.split(' ')[2]
            elif line.startswith('throughput'):
                throughput = line.split(' ')[1]

    return average_latency, throughput


def run_once(num_clients, requests_per_client):
    subprocess.call(['./benchmark/throughput_latency/run_once.sh', str(num_clients), str(requests_per_client), OUTPUT_FILE])
    average_latency, throughput = parse_output()
    print(f'average latency: {average_latency} microseconds, throughput: {throughput} req/s')
    print('=============================')


if __name__ == "__main__":
    run_once(1, 10000)
    run_once(10, 1000)
    run_once(100, 100)
    run_once(1000, 10)
