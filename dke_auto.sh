#!/bin/bash
# usage: dke_auto.sh 128/256/512 speed/test/testvectors/stack m4/ref

path=DKE-${1}_${3}

logf=RACC/${2}_${path}.txt
echo ${path}

make bin/crypto_kem_${path}_${2}.hex PLATFORM=nucleo-l4r5zi MUPQ_ITERATIONS=100
echo === $logf ===
openocd -f st_nucleo_l4r5.cfg -c "program bin/crypto_kem_${path}_${2}.hex verify reset exit"
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
