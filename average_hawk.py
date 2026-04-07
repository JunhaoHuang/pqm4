import re

# Read the file
with open('/home/hjh/Documents/project/pqm4/RACC/hawk_1024_new.txt', 'r') as f:
	content = f.read()

# Extract all values
keypair_values = [int(x) for x in re.findall(r'keypair cycles:\s*(\d+)', content)]
sign_values = [int(x) for x in re.findall(r'sign cycles:\s*(\d+)', content)]
verify_values = [int(x) for x in re.findall(r'verify cycles:\s*(\d+)', content)]

# Calculate averages
keypair_avg = sum(keypair_values) / len(keypair_values)
sign_avg = sum(sign_values) / len(sign_values)
verify_avg = sum(verify_values) / len(verify_values)

print(f"keypair average cycles: {keypair_avg:.2f}")
print(f"sign average cycles: {sign_avg:.2f}")
print(f"verify average cycles: {verify_avg:.2f}")