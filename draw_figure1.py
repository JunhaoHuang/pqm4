import matplotlib.pyplot as plt

# Incremental size

import re

with open('RACC/poly_speed_RACCOON_128_1_m4.txt', 'r') as f:
	content = f.read()

inc_absorb = re.findall(r'SHAKE256 Inc Absorb cycles:\s*(\d+)', content)
inc_squeeze = re.findall(r'SHAKE256 Inc Squeeze cycles:\s*(\d+)', content)

inc_absorb = [int(x) for x in inc_absorb]
inc_squeeze = [int(x) for x in inc_squeeze]

x = list(range(1, len(inc_absorb) + 1))

print("inc_absorb:", inc_absorb)
print("inc_squeeze:", inc_squeeze)

# Baseline cycles
base_absorb = re.findall(r'SHAKE256 Absorb cycles:\s*(\d+)', content)
base_squeeze = re.findall(r'SHAKE256 Squeeze cycles:\s*(\d+)', content)

base_absorb = int(base_absorb[0]) if base_absorb else None
base_squeeze = int(base_squeeze[0]) if base_squeeze else None

# =======================
# 单位转换：cycles → 1000 cycles
# =======================
scale_factor = 10000
inc_absorb_k = [v / scale_factor for v in inc_absorb]
inc_squeeze_k = [v / scale_factor for v in inc_squeeze]
base_absorb_k = base_absorb / scale_factor
base_squeeze_k = base_squeeze / scale_factor

plt.figure(figsize=(10, 6))

plt.plot(x, inc_absorb_k, color='orange', label='SHAKE256 Inc Absorb', linewidth=2)
plt.plot(x, inc_squeeze_k, label='SHAKE256 Inc Squeeze', linewidth=2)

plt.axhline(base_absorb_k, linestyle='--', color='orange', label='SHAKE256 One-shot Absorb')
plt.axhline(base_squeeze_k, linestyle='--', label='SHAKE256 One-shot Squeeze')

plt.xlabel('Chunk Size (Bytes)')
plt.ylabel(r'Cycles ($\times 10^5$)')
plt.title('SHAKE256 Absorb & Squeeze Cycles at Different Chunk Sizes')
plt.legend()
plt.grid(True)

plt.tight_layout()
plt.axvline(136, linestyle='--', color='grey', label='SHAKE256 Rate $r=136$')
plt.legend()

plt.savefig('figure1.png', dpi=300, bbox_inches='tight')
plt.show()