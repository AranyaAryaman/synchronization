import subprocess
import matplotlib.pyplot as plt
import numpy as np

def run_experiment(executable, num_threads):
    command = f'./{executable} {num_threads}'
    try:
        result = subprocess.run(command, shell=True, capture_output=True, text=True)
        output = result.stdout.split('\n')
        insert_time = float(output[0].split()[-2])
        retrieve_time = float(output[-2].split()[-2])
        return insert_time + retrieve_time
    except (IndexError, ValueError):
        print(f"Error running {executable} with {num_threads} threads.")
        return float('inf')  # Return a high value to indicate failure

thread_counts = [1, 2, 4, 8, 16]
original_times = []
mutex_times = []
spinlock_times = []
mutex_opt_times = []

for threads in thread_counts:
    original_time = run_experiment('parallel_hashtable', threads)
    mutex_time = run_experiment('parallel_mutex', threads)
    spinlock_time = run_experiment('parallel_spin', threads)
    mutex_opt_time = run_experiment('parallel_mutex_opt', threads)
    original_times.append(original_time)
    mutex_times.append(mutex_time)
    spinlock_times.append(spinlock_time)
    mutex_opt_times.append(mutex_opt_time)
    print(f'Threads: {threads}, Original: {original_time:.3f}s, Mutex: {mutex_time:.3f}s, Spinlock: {spinlock_time:.3f}s, Mutex Opt: {mutex_opt_time:.3f}s')

# Calculate overheads
mutex_overheads = [(mutex - orig) / orig * 100 for mutex, orig in zip(mutex_times, original_times) if orig != float('inf') and mutex != float('inf')]
spinlock_overheads = [(spin - orig) / orig * 100 for spin, orig in zip(spinlock_times, original_times) if orig != float('inf') and spin != float('inf')]
mutex_opt_overheads = [(mutex_opt - orig) / orig * 100 for mutex_opt, orig in zip(mutex_opt_times, original_times) if orig != float('inf') and mutex_opt != float('inf')]

# Calculate average overheads
avg_mutex_overhead = np.mean(mutex_overheads) if mutex_overheads else float('inf')
avg_spinlock_overhead = np.mean(spinlock_overheads) if spinlock_overheads else float('inf')
avg_mutex_opt_overhead = np.mean(mutex_opt_overheads) if mutex_opt_overheads else float('inf')

# Print overheads
print(f'\nMutex Overhead:')
for threads, overhead in zip(thread_counts, mutex_overheads):
    print(f'{threads} threads: {overhead:.2f}%')
print(f'Average Mutex Overhead: {avg_mutex_overhead:.2f}%')

print(f'\nSpinlock Overhead:')
for threads, overhead in zip(thread_counts, spinlock_overheads):
    print(f'{threads} threads: {overhead:.2f}%')
print(f'Average Spinlock Overhead: {avg_spinlock_overhead:.2f}%')

print(f'\nMutex Opt Overhead:')
for threads, overhead in zip(thread_counts, mutex_opt_overheads):
    print(f'{threads} threads: {overhead:.2f}%')
print(f'Average Mutex Opt Overhead: {avg_mutex_opt_overhead:.2f}%')

# Plot comparison
plt.figure(figsize=(10, 6))
plt.plot(thread_counts, original_times, marker='o', label='Original (Unsafe)')
plt.plot(thread_counts, mutex_times, marker='s', label='Mutex')
plt.plot(thread_counts, spinlock_times, marker='^', label='Spinlock')
plt.plot(thread_counts, mutex_opt_times, marker='d', label='Mutex Opt')
plt.xlabel('Number of Threads')
plt.ylabel('Execution Time (s)')
plt.title('Hash Table Performance: Original vs Mutex vs Spinlock vs Mutex Opt')
plt.legend()
plt.grid(True)
plt.savefig('hash_table_performance_comparison.png')
plt.close()

print('\nResults saved to hash_table_performance_comparison.png')