import numpy as np
import os
import sys
import glob
def process_clock_cycles(file_path):
    data = {
        'keypair cycles:': [],
        'sign cycles:': [],
        'verify cycles:': [],
        'keypair stack usage:': [],
        'sign stack usage:': [],
        'verify stack usage:': []
    }

    valid_functions = {
        'keypair cycles:', 
        'sign cycles:', 
        'verify cycles:',
        'keypair stack usage:',
        'sign stack usage:',
        'verify stack usage:'
        }

    with open(file_path, 'r') as f:
        lines = f.readlines()

    i=0
    while i + 2 < len(lines):
        function = lines[i].strip()
        if function in valid_functions:
            try:
                cycles = int(lines[i + 1].strip())  
                data[function].append(cycles)
                i+=2
            except ValueError:
                print(f"Invalid cycle value for function {function}: {lines[i + 1].strip()}")
                continue
        else:
            i+=1
            # print(f"Invalid function name: {function}")

    results = {}
    for function, cycles_list in data.items():
        if cycles_list: 
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
    if 'stack' in file_path:
        return results, len(data['keypair stack usage:'])
    else:
        return results, len(data['keypair cycles:'])

def search_files_in_directory(directory, keyword):
    pattern = os.path.join(directory, f"{keyword}*.txt")
    files = glob.glob(pattern)
    return files


def res_to_tex(file_path, results):
    tex_lines = []
    line = "\\newcommand{"
    if 'RACCOON_128' in file_path:
        line+= "\\RACCI"
    elif 'RACCOON_192' in file_path:
        line+= "\\RACCII"
    elif 'RACCOON_256' in file_path:
        line+= "\\RACCIII"

    if '_1_' in file_path:
        line+= "A"
    elif '_2_' in file_path:
        line+= "B"
    elif '_4_' in file_path:
        line+= "C"
    elif '_8_' in file_path:
        line+= "D"
    elif '_16_' in file_path:
        line+= "E"
    elif '_32_' in file_path:
        line+= "F"
    
    if '_m4' in file_path:
        line+= "opt"
    elif '_ref' in file_path:
        line+= "ref"

    for function, stats in results.items():
        if "speed" in file_path:
            if function == 'sign cycles:':
                sign=line + "sign}{"
                sign+= f"{stats['average'] / 1000:.0f}k}}"
                tex_lines.append(sign)
            if function == 'verify cycles:':
                verify=line + "verify}{"
                verify+= f"{stats['average'] / 1000:.0f}k}}"
                tex_lines.append(verify)
            if function == 'keypair cycles:':
                keygen=line + "keygen}{"
                keygen+= f"{stats['average'] / 1000:.0f}k}}"
                tex_lines.append(keygen)
        elif "stack" in file_path:
            if function == 'sign stack usage:':
                sign=line + "signStack}{"
                sign+= f"{stats['average']:.0f}}}"
                tex_lines.append(sign)
            if function == 'verify stack usage:':
                verify=line + "verifyStack}{"
                verify+= f"{stats['average']:.0f}}}"
                tex_lines.append(verify)
            if function == 'keypair stack usage:':
                keygen=line + "keygenStack}{"
                keygen+= f"{stats['average']:.0f}}}"
                tex_lines.append(keygen)
    return tex_lines

def main(directory, type):
    files = search_files_in_directory(directory, keyword=type)

    if not files:
        print(f"No files found in {directory} containing '{type}' in their name.")
        return

    for file_path in files:
        # print(f"Processing file: {file_path}")
        results, len = process_clock_cycles(file_path)
        
        # for function, stats in results.items():
        #     print(f"{function} {stats['average']/1000:.0f}k")
        # if len==1:
        print(f"% Results for {file_path} with {len} elements")
        for tex in res_to_tex(file_path, results):
            print(tex)
    

directory_path = 'RACC/' 
if len(sys.argv) < 2:
     print("Usage: python3 average.py <type>")
     sys.exit(1)
type = sys.argv[1]
main(directory_path, type)
