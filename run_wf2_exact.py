import subprocess

def run_wf2_exact(num_threads=1, dataset="data.01.csv"):
    bin_path = "./build/wf2_exact"
    dataset_opt = f"-f graphs/{dataset}"
    num_threads_opt = f"-t {num_threads}"
    const_time_str = "Graph construction time:"
    match_time_str = "Pattern matching time:"
    const_time = 0
    match_time = 0

    cmd = f"{bin_path} {dataset_opt} {num_threads_opt}"
    output = subprocess.Popen(cmd, 
            shell=True, 
            stdout=subprocess.PIPE, 
            stderr=subprocess.DEVNULL).stdout
    for line in output.readlines():
        line = line.decode("utf-8")
        t1 = line.find(const_time_str)
        t2 = line.find(match_time_str)
        if t1 != -1:
            t1 += len(const_time_str)
            const_time = float(line[t1:])
        elif t2 != -1:
            t2 += len(match_time_str)
            match_time = float(line[t2:])

    return (const_time, match_time)


def main():
    print("How many runs per setup? ", end="")
    runs = int(input())
    print("Discard how many initial runs? ", end="")
    discard = int(input())
    num_threads_range = [1, 4, 8, 16, 32, 64]

    runtimes = dict()
    counted_runs = runs - discard
    for num_threads in num_threads_range:
        print(f"Running setup (num_threads = {num_threads})...")
        runtimes[num_threads] = (0, 0)
        for i in range(runs):
            t1, t2 = run_wf2_exact(num_threads=num_threads)
            if i >= discard:
                t1_sum, t2_sum = runtimes[num_threads]
                runtimes[num_threads] = (t1 + t1_sum, t2 + t2_sum)
            discarded_str = "\t (discarded)" if i < discard else ""
            print(f"\tGraph Construction: {t1}, Pattern Matching: {t2} {discarded_str}")

        t1, t2 = runtimes[num_threads]
        t1, t2 = t1 / counted_runs, t2 / counted_runs
        print(f"\tGraph Construction Avg: {t1:.3f}, Pattern Matching Avg: {t2:.3f}, Total avg: {(t1 + t2):.3f}")
        runtimes[num_threads] = (t1, t2)

    print("\nResults:")
    for num_threads in runtimes:
        t1, t2 = runtimes[num_threads]
        print(f"\"{num_threads} threads\", {t1:.3f}, {t2:.3f}, {(t1 + t2):.3f}")


if __name__ == "__main__":
    main()
