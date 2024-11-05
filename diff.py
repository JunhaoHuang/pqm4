import difflib
from colorama import init, Fore, Style

# Initialize colorama for Windows compatibility
init(autoreset=True)

def compare_files_with_colors(file1, file2):
    with open(file1, 'r', encoding='utf-8') as f1, open(file2, 'r', encoding='utf-8') as f2:
        file1_lines = f1.readlines()
        file2_lines = f2.readlines()

    diff = difflib.unified_diff(file1_lines, file2_lines, lineterm='', n=0)
    
    has_diff = False
    print("Differences between the two files:\n")
    
    for line in diff:
        has_diff = True
        if line.startswith('@@'):
            # Yellow color for line number difference
            print(Fore.YELLOW + f"\nAt lines {line.strip()}:")
        elif line.startswith('-'):
            # Red color for content removed from file1
            print(Fore.RED + f"File1: {line.strip()}")
        elif line.startswith('+'):
            # Green color for content added in file2
            print(Fore.GREEN + f"File2: {line.strip()}")

    if not has_diff:
        print("The two files are identical.")


# Usage example
file1 = 'out1.txt'
file2 = 'test.out'
compare_files_with_colors(file1, file2)