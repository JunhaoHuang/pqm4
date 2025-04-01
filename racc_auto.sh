#!/bin/bash

#	Disable frequency scaling until the next boot. Intel:
#		echo 1 > /sys/devices/system/cpu/intel_pstate/no_turbo
#	AMD:
#		echo 0 > /sys/devices/system/cpu/cpufreq/boost

# for dut in \
# 	RACCOON_128_1	RACCOON_128_2	RACCOON_128_4	\
# 	RACCOON_128_8	RACCOON_128_16	RACCOON_128_32	\
# 	RACCOON_192_1	RACCOON_192_2	RACCOON_192_4	\
# 	RACCOON_192_8	RACCOON_192_16	RACCOON_192_32	\
# 	RACCOON_256_1	RACCOON_256_2	RACCOON_256_4	\
# 	RACCOON_256_8	RACCOON_256_16	RACCOON_256_32
# do
# 	logf=bench_$dut.txt
# 	echo === $logf ===
# 	make clean
# 	make RACCF="-D"$dut" -DBENCH_TIMEOUT=1000"

# 	./xtest | tee $logf
# 	grep -e '_core_keygen' -e '_core_sign' -e '_core_verify' racc_core.su >> $logf
# done


# usage: racc_auto.sh speed/test/testvectors/stack m4/ref
for dut in \
	RACCOON_128_1	RACCOON_128_2	RACCOON_128_4	\
	RACCOON_128_8	RACCOON_128_16	RACCOON_128_32	\
	RACCOON_192_1	RACCOON_192_2	RACCOON_192_4	\
	RACCOON_192_8	RACCOON_192_16	RACCOON_192_32	\
	RACCOON_256_1	RACCOON_256_2	RACCOON_256_4	\
	RACCOON_256_8	RACCOON_256_16	RACCOON_256_32
do
	logf=RACC/${1}_${dut}_${2}.txt
	path="raccoon-${dut:8:3}_${2}"
	echo ${path}
	make clean
	make bin/crypto_sign_${path}_${1}.hex PLATFORM=nucleo-l4r5zi # RACCF="-D"$dut""
	echo === $logf ===
	echo "-D"$dut""
	openocd -f st_nucleo_l4r5.cfg -c "program bin/crypto_sign_${path}_${1}.hex verify reset exit"
	# st-flash --reset write bin/crypto_sign_${path}_${1}.bin 0x8000000
	python3 hostside/host_unidirectional.py > $logf
done