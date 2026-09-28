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

def update_tex(tex_path, tex_lines):
    """Write every \\newcommand in tex_lines into tex_path.

    Lines of the form \\newcommand{\\NAME}{value} that already exist in the tex
    file get their value replaced in place. A macro that does not exist yet is
    appended right after the last \\newcommand line of the file, so that the
    table body can start using it. The rest of the file (table outline,
    preamble, ...) is left untouched.
    """
    import re
    with open(tex_path, 'r') as f:
        content = f.read()
    updated, added = 0, []
    for tex in tex_lines:
        m = re.match(r'\\newcommand\{(\\[A-Za-z]+)\}\{(.*)\}$', tex)
        if not m:
            continue
        name, value = m.group(1), m.group(2)
        pattern = re.compile(r'^(\\newcommand\{' + re.escape(name) + r'\})\{[^}]*\}', re.M)
        content, n = pattern.subn(lambda mm: mm.group(1) + '{' + value + '}', content)
        if n:
            updated += n
        else:
            added.append(tex)
    if added:
        # insert after the last existing \newcommand line (or at the top if there is none)
        last = None
        for mm in re.finditer(r'^\\newcommand\{[^\n]*$', content, re.M):
            last = mm
        block = '\n'.join(added)
        if last is not None:
            content = content[:last.end()] + '\n' + block + content[last.end():]
        else:
            content = block + '\n' + content
    with open(tex_path, 'w') as f:
        f.write(content)
    print(f"% {tex_path}: updated {updated} \\newcommand values, added {len(added)}", file=sys.stderr)
    for tex in added:
        print(f"% added {tex}", file=sys.stderr)

def main(directory, type, tex_path):
    files = search_files_in_directory(directory, keyword=type)

    if not files:
        print(f"No files found in {directory} containing '{type}' in their name.")
        return

    all_tex_lines = []
    for file_path in files:
        # print(f"Processing file: {file_path}")
        results, len = process_clock_cycles(file_path)
        
        # for function, stats in results.items():
        #     print(f"{function} {stats['average']/1000:.0f}k")
        # if len==1:
        print(f"% Results for {file_path} with {len} elements")
        tex_lines = res_to_tex(file_path, results)
        for tex in tex_lines:
            print(tex)
        all_tex_lines += tex_lines

    if tex_path:
        update_tex(tex_path, all_tex_lines)
    

directory_path = 'RACC/'
if len(sys.argv) < 2:
     print("Usage: python3 average_poly.py poly_speed [tex_file]")
     sys.exit(1)
type = sys.argv[1]
# tex file whose \newcommand values are replaced with the averaged results
# (pass "-" as the second argument to only print the results)
tex_path = sys.argv[2] if len(sys.argv) > 2 else 'racc_table3.tex'
if tex_path == '-':
    tex_path = None
main(directory_path, type, tex_path)
