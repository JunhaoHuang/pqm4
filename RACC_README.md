## Efficient and Compact High-order Masking Raccoon on Memory Constrained Devices
Since the proposed Raccoon implementation is built upon the pqm4 repository, we use a different README.md to demonstrate the usage of the proposed Raccoon implementation. Our implementation locates in /crypto_sign/raccoon-xxx/m4. We also provide reference implementation of Raccoon in /crypto_sign/raccoon-xxx/ref for comparison.

## Platform & Environment
The results in the paper are obtained in the following board and enviroment:
- The Nucleo-L4R5ZI board
- Ubuntu 24.04.2 LTS
- arm-none-eabi-gcc 13.2.1

## Usage
We provide scripts to automatically switch to different masking order of Raccoon for benchmarking on Nucleo-L4R5ZI board. One can directly use the following command for testing/benchmarking:

```
./racc_auto.sh speed/test/testvectors/stack m4/ref
```

The results for polynomial arithmetic can use the following command:

```
./racc_poly_speed.sh poly_speed
```

The output will be temporarily stored in /RACC directory. To analyse the results for stack and speed, we also provide a python script to get the averaged results and translate to latex command so that we can easily integrate our results in the paper.

```
python3 average.py speed/stack
```
