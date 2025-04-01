import numpy as np
import os
import glob
def process_clock_cycles(file_path):
    # 使用字典存储每个函数的时钟周期数据
    data = {
        'keypair cycles:': [],
        'sign cycles:': [],
        'verify cycles:': []
    }

    # 定义有效的函数名集合
    valid_functions = {'keypair cycles:', 'sign cycles:', 'verify cycles:'}

    # 读取文件并提取时钟周期数据
    with open(file_path, 'r') as f:
        lines = f.readlines()

    # 遍历每一对函数名和时钟周期
    i=0
    while i < len(lines):
        function = lines[i].strip()
        # 仅处理有效的函数名
        if function in valid_functions:
            try:
                cycles = int(lines[i + 1].strip())  # 转换为整数
                data[function].append(cycles)
                i+=2
            except ValueError:
                # 如果时钟周期不是有效的整数，跳过该项
                print(f"Invalid cycle value for function {function}: {lines[i + 1].strip()}")
                continue
        else:
            i+=1
            # print(f"Invalid function name: {function}")

    # 计算每个函数的总时钟周期、平均值和中位数
    results = {}
    for function, cycles_list in data.items():
        if cycles_list:  # 仅计算存在数据的函数
            total_cycles = sum(cycles_list)
            average_cycles = np.mean(cycles_list)
            median_cycles = np.median(cycles_list)

            results[function] = {
                'total': total_cycles,
                'average': average_cycles,
                'median': median_cycles
            }
        else:
            results[function] = {
                'total': 0,
                'average': 0,
                'median': 0
            }

    return results, len(data['keypair cycles:'])

def search_files_in_directory(directory, keyword="speed"):
    # 使用glob搜索所有包含 "speed" 的文件，忽略子目录
    pattern = os.path.join(directory, f"*{keyword}*.txt")
    files = glob.glob(pattern)
    return files

def main(directory):
    # 搜索目录下所有包含 "speed" 的文件
    files = search_files_in_directory(directory)

    if not files:
        print(f"No files found in {directory} containing '{keyword}' in their name.")
        return

    # 遍历所有找到的文件进行统计
    for file_path in files:
        print(f"Processing file: {file_path}")
        results, len = process_clock_cycles(file_path)
        print(f"Results for {file_path} with {len} elements")
        for function, stats in results.items():
            print(f"{function} {stats['average']/1000:.0f}k")

# 使用示例
directory_path = 'RACC/'  # 当前目录，替换为你需要的目录路径
main(directory_path)
