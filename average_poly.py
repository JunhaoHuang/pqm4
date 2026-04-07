import numpy as np
import os
import sys
import glob
def process_clock_cycles(file_path):
    data = {
        'gen_matrix cycles:': [],
        'racc_encode_sk cycles:': [],
        'racc_decode_sk cycles:': [],
        'zero_encoding cycles:': [],
        'racc_refresh cycles:': [],
        'racc_ntt_refresh cycles:': [],
        'racc_decode cycles:': [],
        'add_rep_noise cycles:': []
    }

    valid_functions = {
        'gen_matrix cycles:',
        'racc_encode_sk cycles:',
        'racc_decode_sk cycles:',
        'zero_encoding cycles:',
        'racc_refresh cycles:',
        'racc_ntt_refresh cycles:',
        'racc_decode cycles:',
        'add_rep_noise cycles:'
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
    return results, len(data['gen_matrix cycles:'])

def search_files_in_directory(directory, keyword):
    pattern = os.path.join(directory, f"{keyword}*.txt")
    files = glob.glob(pattern)
    return files


def res_to_tex(file_path, results):
    tex_lines = []
    line = "\\newcommand{\\"

    if '_1_' in file_path:
        order = "A"
    elif '_2_' in file_path:
        order = "B"
    elif '_4_' in file_path:
        order = "C"
    elif '_8_' in file_path:
        order = "D"
    elif '_16_' in file_path:
        order = "E"
    elif '_32_' in file_path:
        order = "F"
    
    if '_m4' in file_path:
        impl = "opt"
    elif '_ref' in file_path:
        impl = "ref"

    for function, stats in results.items():
        if function == 'gen_matrix cycles:':
            sign=line + "MG" + order + impl + "}{"
            sign+= f"{stats['average'] / 1000:.0f}k}}"
            tex_lines.append(sign)
        elif function == 'racc_encode_sk cycles:':
            sign=line + "SKE" + order + impl + "}{"
            sign+= f"{stats['average'] / 1000:.0f}k}}"
            tex_lines.append(sign)
        elif function == 'racc_decode_sk cycles:':
            sign=line + "SKD" + order + impl + "}{"
            sign+= f"{stats['average']:.0f}}}"
            tex_lines.append(sign)
        elif function == 'zero_encoding cycles:':
            sign=line + "ZE" + order + impl + "}{"
            sign+= f"{stats['average']:.0f}}}"
            tex_lines.append(sign)
        elif function == 'racc_refresh cycles:':
            sign=line + "RE" + order + impl + "}{"
            sign+= f"{stats['average']:.0f}}}"
            tex_lines.append(sign)
        elif function == 'racc_ntt_refresh cycles:':
            sign=line + "RENTT" + order + impl + "}{"
            sign+= f"{stats['average']:.0f}}}"
            tex_lines.append(sign)
        elif function == 'racc_decode cycles:':
            sign=line + "DE" + order + impl + "}{"
            sign+= f"{stats['average']:.0f}}}"
            tex_lines.append(sign)
        elif function == 'add_rep_noise cycles:':
            sign=line + "ARN" + order + impl + "}{"
            sign+= f"{stats['average'] / 1000:.0f}k}}"
            tex_lines.append(sign)
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
     print("Usage: python3 average_poly.py poly_speed")
     sys.exit(1)
type = sys.argv[1]
main(directory_path, type)
