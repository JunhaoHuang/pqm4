# Efficient and Compact High-order Masking Raccoon on Memory Constrained Devices
Since the proposed Raccoon implementation is built upon the pqm4 repository, we use a different README.md to demonstrate the usage of the proposed Raccoon implementation. For the installation and configuration of the environment, we refer to the [README.md](./README.md) of pqm4. The optimized implementation locates in crypto_sign/raccoon-xxx/m4. We also provide reference implementation of Raccoon in crypto_sign/raccoon-xxx/ref for comparison.

Author: Junhao Huang jhhuang_nuaa@126.com

## Platform & Environment
The results in the paper are obtained in the following board and environment:
- The Nucleo-L4R5ZI board
- Ubuntu 24.04.2 LTS
- arm-none-eabi-gcc 13.2.1
- OpenOCD 0.12.0
- Python3 3.12.3

## Usage
We provide scripts to automatically switch to different masking orders of Raccoon for benchmarking on Nucleo-L4R5ZI board. The intermediate results of these scripts will be generated under `RACC/` directory, which needs to be manually created before running the scripts. 

One can directly use the following command for reproducing results in Table 4:

```
mkdir -p RACC

./racc_auto.sh speed m4
./racc_auto.sh speed ref
./racc_auto.sh stack m4
./racc_auto.sh stack ref
```

The results for polynomial sampling, arithmetic, and masking gadgets in Figure 1, Table 1, Table 2, and Table 3 can be obtained with the following command (Note that one needs to copy the [poly_speed.c](./poly_speed.c) to `/mupq/crypto_sign/` before running the script):

```
./racc_poly_speed.sh poly_speed
```

The output will be temporarily stored in `RACC/` directory. To analyse the results for polynomial arithmetic, stack and speed, we provide Python scripts to get the averaged results and translate to LaTeX command so that we can easily integrate our results in the paper.

```
# polynomial sampling, arithmetic, and masking gadgets (Table 1,2,3). Results for Table 1,2 can be interpreted from the corresponding intermediate result in RACC/poly_speed_RACCOON_128_1_m4.txt.
python3 average_poly.py poly_speed 

# Generating Figure 1.
python3 draw_figure1.py 

# Generating speed or stack results of the schemes (Table 4).
python3 average.py speed
python3 average.py stack
```

## Licenses
The ref and m4 implementations of Raccoon are both licensed under the MIT license. Each directory contains a copy of the license. 