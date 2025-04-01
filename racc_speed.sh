#!/bin/bash
# usage: racc_speed.sh speed/stack
for dut in \
	RACCOON_128_4
do
	make clean
	logf=RACC/${1}_${dut}_m4.txt
	path="raccoon-${dut:8:3}_m4"
	echo ${path}
	make bin/crypto_sign_${path}_${1}.hex PLATFORM=nucleo-l4r5zi RACCF="-D"$dut"" MUPQ_ITERATIONS=1
	echo === $logf ===
	echo "-D"$dut""
	openocd -f st_nucleo_l4r5.cfg -c "program bin/crypto_sign_${path}_${1}.hex verify reset exit"
	python3 hostside/host_unidirectional.py > $logf

	############### ref ##############
	# logf=RACC/${1}_${dut}_ref.txt
	# path="raccoon-${dut:8:3}_ref"
	# echo ${path}
	# make bin/crypto_sign_${path}_${1}.hex PLATFORM=nucleo-l4r5zi RACCF="-D"$dut"" MUPQ_ITERATIONS=1
	# echo === $logf ===
	# echo "-D"$dut""
	# openocd -f st_nucleo_l4r5.cfg -c "program bin/crypto_sign_${path}_${1}.hex verify reset exit"
	# python3 hostside/host_unidirectional.py > $logf
done