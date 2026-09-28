#!/bin/bash
# usage: racc_auto.sh speed/test/testvectors/stack m4/ref
#
# MUPQ_ITERATIONS is chosen automatically per variant:
#   stack: 2 iterations for every benchmarked variant (m4 and ref)
#   speed / m4 : MEM_OPT (from crypto_sign/raccoon-xxx/m4/param_list.h) == 2 -> 2 iterations
#        otherwise                                                    -> 100 iterations
#   speed / ref: only Raccoon-128 (d<=8), Raccoon-192 (d<=4), Raccoon-256 (d<=2) are run,
#        always with 100 iterations; the other variants are skipped.
# Set RACC_SKIP_DONE=1 to resume: variants whose log in RACC/ is already complete are skipped.
mode=$1
impl=$2

if [ -z "$mode" ] || [ -z "$impl" ]; then
	echo "usage: $0 speed/test/testvectors/stack m4/ref"
	exit 1
fi

# print MEM_OPT of the given variant (e.g. RACCOON_128_32) from the m4 param_list.h
get_mem_opt() {
	local dut=$1
	local lvl=${dut:8:3}
	awk -v dut="$dut" '
		$0 ~ "^#if defined\\(" dut "\\)" { inblk=1 }
		inblk && /^#define MEM_OPT/ { print $3; exit }
		inblk && /^#endif/ { inblk=0 }
	' crypto_sign/raccoon-${lvl}/${impl}/param_list.h
}

# decide MUPQ_ITERATIONS for a variant; print nothing if the variant must be skipped
#   stack mode: 2 iterations for every benchmarked variant (m4 and ref)
#   speed mode: see the rules at the top of this file
get_iterations() {
	local dut=$1
	local lvl=${dut:8:3}
	local d=${dut:12}
	local n=100
	[ "$mode" = "stack" ] && n=2
	if [ "$impl" = "ref" ]; then
		case "$lvl" in
			128) [ "$d" -le 8 ] && echo $n ;;
			192) [ "$d" -le 4 ] && echo $n ;;
			256) [ "$d" -le 2 ] && echo $n ;;
		esac
	elif [ "$mode" = "stack" ]; then
		echo $n
	else
		local mem_opt
		mem_opt=$(get_mem_opt "$dut")
		if [ -z "$mem_opt" ]; then
			echo "[WARN] MEM_OPT not found for $dut, defaulting to 100 iterations" >&2
			echo 100
		elif [ "$mem_opt" -eq 2 ]; then
			echo 2
		else
			echo 100
		fi
	fi
}

for dut in \
	RACCOON_128_1	RACCOON_128_2	RACCOON_128_4	\
	RACCOON_128_8	RACCOON_128_16	RACCOON_128_32	\
	RACCOON_192_1	RACCOON_192_2	RACCOON_192_4	\
	RACCOON_192_8	RACCOON_192_16	RACCOON_192_32	\
	RACCOON_256_1	RACCOON_256_2	RACCOON_256_4	\
	RACCOON_256_8	RACCOON_256_16	RACCOON_256_32
do
	iters=$(get_iterations "$dut")
	if [ -z "$iters" ]; then
		echo "[INFO] skipping $dut for $impl"
		continue
	fi

	mkdir -p RACC
	logf=RACC/${mode}_${dut}_${impl}.txt
	path="raccoon-${dut:8:3}_${impl}"
	# RACC_SKIP_DONE=1: skip a variant whose log already holds a complete run
	# (end marker '#' and at least $iters keypair entries), useful to resume an interrupted run
	if [ "${RACC_SKIP_DONE:-0}" = "1" ] && [ -f "$logf" ] && grep -q '#' "$logf" \
	   && [ "$(grep -c '^keypair' "$logf")" -ge "$iters" ]; then
		echo "[INFO] $logf already complete, skipping (RACC_SKIP_DONE=1)"
		continue
	fi
	echo "${path} (MUPQ_ITERATIONS=${iters})"
	make clean
	make bin/crypto_sign_${path}_${mode}.hex PLATFORM=nucleo-l4r5zi RACCF="-D"$dut"" MUPQ_ITERATIONS=${iters}
	echo === $logf ===
	echo "-D"$dut""
	interrupted=0
	trap 'interrupted=1' INT

	# flash, listen until the end marker '#' arrives; if the serial listener dies before
	# that (e.g. another process opened the port), re-flash and measure the variant again
	attempt=1
	while true; do
		# remove a stale log from a previous run *before* starting the listener, otherwise the
		# wait loop below may see the old '#' before the listener has truncated the file
		rm -f "$logf"
		openocd -f st_nucleo_l4r5.cfg -c "program bin/crypto_sign_${path}_${mode}.hex verify reset exit"
		python3 hostside/host_unidirectional.py > $logf & py_pid=$!
		sleep 0.5

		echo "[INFO] listening... (pid=$py_pid, attempt $attempt)"

		status=""
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
				status=done
				break
			fi
			if ! kill -0 "$py_pid" 2>/dev/null; then
				echo "[WARN] serial listener exited before '#' (attempt $attempt) for $logf"
				status=retry
				break
			fi
			sleep 0.05
		done
		[ "$status" = "done" ] && break
		attempt=$((attempt+1))
		if [ $attempt -gt 3 ]; then
			echo "[ERROR] giving up on $dut after 3 attempts"
			break
		fi
		sleep 5
	done

done
