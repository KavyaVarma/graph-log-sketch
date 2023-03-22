import subprocess

def run_wf2_exact(gmt_workers=1, gmt_uthreads=2, dataset="data.01.csv"):
    bin_path = "./build/wf2_exact"
    dataset_path = f"./graphs/{dataset}"
    num_threads_arg = "-t"
    time_str = "Graph_construction_time"
    time = 0

    cmd = f"{bin_path} {num_threads_arg} {gmt_workers} {uthreads_arg} {gmt_uthreads} {dataset_path}"
    output = subprocess.Popen(cmd, shell=True, stdout=subprocess.PIPE).stdout
    for line in output.readlines():
        line = line.decode("utf-8")
        k1 = line.find(k1_str)
        k3 = line.find(k3_str)
        if k1 != -1:
            k1 += len(k1_str)
            k1_time = float(line[k1:])
        elif k3 != -1:
            k3 += len(k3_str)
            k3_time = float(line[k3:])

    return (k1_time, k3_time)


def main():
    print("How many runs per setup? ", end="")
    runs = int(input())
    print("Discard how many initial runs? ", end="")
    discard = int(input())
    workers_range = [64, 128, 256]
    uthreads_range = [16, 32, 64, 128, 256]

    runtimes = dict()
    counted_runs = runs - discard
    for workers in workers_range:
        for uthreads in uthreads_range:
            total_uthreads = workers * uthreads
            if total_uthreads < 2**12 or total_uthreads >= 2**15:
                continue

            print(f"Running setup (workers = {workers}, uthreads_per_worker = {uthreads})...")
            runtimes[(workers, uthreads)] = (0, 0)
            for i in range(runs):
                k1, k3 = run_wf2_exact(gmt_workers=workers, gmt_uthreads=uthreads)
                if i >= discard:
                    k1_sum, k3_sum = runtimes[(workers, uthreads)]
                    runtimes[(workers, uthreads)] = (k1 + k1_sum, k3 + k3_sum)
                discarded_str = "\t (discarded)" if i < discard else ""
                print(f"\tKernel 1: {k1}, Kernel 3: {k3} {discarded_str}")

            k1, k3 = runtimes[(workers, uthreads)]
            k1, k3 = k1 / counted_runs, k3 / counted_runs
            print(f"\tKernel 1 avg: {k1:.6f}, Kernel 3 avg: {k3:.6f}, Total avg: {(k1 + k3):.6f}")
            runtimes[(workers, uthreads)] = (k1, k3)

    print("\nResults:")
    for setup in runtimes:
        workers, uthreads = setup
        k1, k3 = runtimes[setup]
        print(f"\"({workers}, {uthreads})\", {k1:.6f}, {k3:.6f}, {(k1 + k3):.6f}")


if __name__ == "__main__":
    main()
