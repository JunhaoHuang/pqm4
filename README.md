# Efficient and Compact High-order Masked Raccoon on Memory-Constrained Devices

This artifact contains the optimized Cortex-M4 implementation of the masked Raccoon signature scheme described in the paper, together with the scripts needed to reproduce the reported benchmarks. The implementation is built on top of the [pqm4](https://github.com/mupq/pqm4) benchmarking framework. This README documents only the Raccoon-specific parts; for installing and configuring the pqm4 toolchain, board support, and Python dependencies, please refer to the original pqm4 documentation in [PQM4_README.md](./PQM4_README.md).

The artifact provides two implementations for each parameter set:

- `crypto_sign/raccoon-{128,192,256}/m4`: the optimized implementation proposed in the paper.
- `crypto_sign/raccoon-{128,192,256}/ref`: the reference implementation, included for comparison.

Author: Junhao Huang (jhhuang_nuaa@126.com)

## Platform and Environment

All results in the paper were obtained on the following board and software environment:

| Component | Version |
| --- | --- |
| Board | Nucleo-L4R5ZI (STM32L4R5ZI, Cortex-M4) |
| Operating system | Ubuntu 24.04.2 LTS |
| Compiler | arm-none-eabi-gcc 13.3.1 |
| Debugger / flasher | OpenOCD 0.12.0 |
| Python | 3.12.3 |

## Usage

We provide shell scripts that automatically build, flash, and benchmark every masking order of Raccoon on the Nucleo-L4R5ZI board. All intermediate logs are written to the `RACC/` directory, which the scripts create on demand.

### Speed and stack usage (Table 4)

The cycle counts and stack usage reported in Table 4 are reproduced with the following four commands:

```
./racc_auto.sh speed m4
./racc_auto.sh speed ref
./racc_auto.sh stack m4
./racc_auto.sh stack ref
```

The script `racc_auto.sh` selects the number of benchmark iterations (`MUPQ_ITERATIONS`) automatically for each parameter set, following the methodology of the paper:

- **m4.** The script reads the `MEM_OPT` value of the selected parameter set from `crypto_sign/raccoon-xxx/m4/param_list.h`. Variants with `MEM_OPT == 2` (Raccoon-128 with $d = 32$, Raccoon-192 with $d \ge 16$, and Raccoon-256 with $d \ge 16$) are averaged over 2 executions; all other variants are averaged over 100 executions.
- **ref.** Only the variants that fit into the memory of the board are benchmarked, namely Raccoon-128 with $d \le 8$, Raccoon-192 with $d \le 4$, and Raccoon-256 with $d \le 2$, each averaged over 100 executions. The remaining variants are skipped, and the script reports `[INFO] skipping ...` for each of them.

The `MEM_OPT` macro selects the memory optimizations applied to a given parameter set:

| `MEM_OPT` | Description |
| --- | --- |
| 0 | No memory optimization; the memory layout of the reference implementation is reused. |
| 1 | Streaming and on-the-fly serialization (Sections 4.1 and 4.2). |
| 2 | `MEM_OPT = 1` plus the mask compression technique. |

In stack mode, every benchmarked variant (m4 and ref) is run for 2 iterations and the maximum stack usage is reported.

**Expected running time.** Before the first variant is built, `racc_auto.sh` prints the list of variants it is going to run together with an approximate running time for each of them and for the whole run, and it repeats the per-variant estimate when the variant starts. The estimates are derived from the cycle counts measured on the Nucleo-L4R5ZI and were within about 10% of the measured wall-clock time. Approximate running times of the four commands are:

| Command | Variants | Running time |
| --- | --- | --- |
| `./racc_auto.sh speed m4` | 18 | about 8 hours |
| `./racc_auto.sh speed ref` | 9 | about 2.7 hours |
| `./racc_auto.sh stack m4` | 18 | about 1.7 hours (2 iterations) |
| `./racc_auto.sh stack ref` | 9 | about 4 minutes (2 iterations) |

The slowest entries are Raccoon-256 with $d = 8$ (about 78 minutes) and Raccoon-256 with $d = 32$ (about 84 minutes for two executions) in speed mode, and Raccoon-256 with $d = 32$ (about 90 minutes) in stack mode.

### Polynomial arithmetic and masking gadgets (Figure 1, Tables 1, 2, and 3)

The benchmarks of polynomial sampling, polynomial arithmetic, SHAKE256, and the masking gadgets reported in Figure 1 and Tables 1 to 3 are obtained with:

```
./racc_poly_speed.sh poly_speed
```

The script copies [poly_speed.c](./poly_speed.c) to `mupq/crypto_sign/` if it is not already present, and then benchmarks Raccoon-128 with $d \in \{1, 2, 4, 8, 16\}$ for both the m4 and the ref implementation. The results of Tables 1 and 2 can be read directly from the log file `RACC/poly_speed_RACCOON_128_1_m4.txt`.

### Generating the LaTeX tables and Figure 1

The aggregated results are written into two self-contained LaTeX files, so that the tables in the paper can be regenerated from the raw logs in `RACC/`:

- [racc_table3.tex](./racc_table3.tex): Table 3, the masking gadgets, `SKEncode`, and matrix generation of Raccoon-128 with $d = 1$.
- [racc_table4.tex](./racc_table4.tex): Table 4, the cycle counts and stack usage of `keygen`, `sign`, and `verify` for all parameter sets.

Each file contains the table environment together with a block of `\newcommand` definitions that hold the measured values. The Python scripts below aggregate the logs in `RACC/` (cycle counts are averaged over the iterations, stack usage is the maximum over the iterations), print the resulting `\newcommand` lines, and replace the values of the corresponding `\newcommand` lines in the tex file. Only the values are replaced: the table layout and any macro that is not produced by the run are left untouched, and a macro that does not exist in the tex file is reported and skipped.

```
# Table 3 (masking gadgets): RACC/poly_speed_*.txt -> racc_table3.tex
python3 average_poly.py poly_speed

# Table 4 (speed and stack): RACC/speed_*.txt and RACC/stack_*.txt -> racc_table4.tex
python3 average.py speed
python3 average.py stack

# Figure 1 (incremental SHAKE256 absorb/squeeze): RACC/poly_speed_RACCOON_128_1_m4.txt -> figure1.png
python3 draw_figure1.py
```

An optional second argument selects a different tex file, for example `python3 average.py speed my_table.tex`. Passing `-` as the second argument only prints the `\newcommand` lines without modifying any file.

Both tex files compile stand-alone with `pdflatex racc_table3.tex` and `pdflatex racc_table4.tex`; they require the `multirow`, `graphicx`, `xspace`, and `geometry` packages. To include a table in the paper, copy the `\newcommand` block into the macro file of the paper and the `table` environment into the text.

## Leakage Assessment

The side-channel leakage assessment of Section 5.5 (Figures 2 and 3) is not part of this artifact. It was conducted on a different platform (CW308T-STM32F4 target board with a PicoScope oscilloscope) and requires additional countermeasures against (micro-)architectural overwriting effects that are specific to that measurement setup. The assessment was therefore carried out separately and cannot be reproduced with the Nucleo-L4R5ZI board used here.

## Licenses

Both the ref and the m4 implementations of Raccoon are released under the MIT license. Each implementation directory contains a copy of the license.
