#!/bin/bash
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
	make bin/crypto_sign_${path}_${1}.hex PLATFORM=nucleo-l4r5zi RACCF="-D"$dut"" MUPQ_ITERATIONS=100
	echo === $logf ===
	echo "-D"$dut""
	openocd -f st_nucleo_l4r5.cfg -c "program bin/crypto_sign_${path}_${1}.hex verify reset exit"
	python3 hostside/host_unidirectional.py > $logf & py_pid=$!

    echo "[INFO] listening... (pid=$py_pid)"

	interrupted=0
	trap 'interrupted=1' INT

	# testing # in logf
	while true; do
		if [ "$interrupted" -eq 1 ]; then
			echo "[INFO] Ctrl+C detected, stopping listener"
			kill "$py_pid" 2>/dev/null
			wait "$py_pid" 2>/dev/null
			exit
		fi
		if grep -q '#' "$logf"; then
			echo "[INFO] '#' detected in $logf, stopping listener"
			kill "$py_pid" 2>/dev/null
			wait "$py_pid" 2>/dev/null
			break
		fi
		sleep 0.05
	done

done