export WMAX=64
export CMAX=64
export BLIST="ippktcheck-4M ipres-4M md5-4M rgbcmyk-4M rotate-4Ms1 rotate-4Ms64 rotate-color1Mp x264-4Mq iDCT-4M"
export MLIST="4M-check-reassembly 4M-check-reassembly-tcp 4M-check-reassembly-tcp-cmykw2-rotatew2 4M-check-reassembly-tcp-x264w2 4M-cmykw2-rotatew2 4M-rotatew2 4M-cmykw2 4M-tcp-mixed 4M-x264w2 4M-reassembly 4M-check"
export TARGET=linux
for b in $BLIST ; do for (( w=1 ; w<=$WMAX ; w++ )) ; do echo "w=$w" >> results_workers.log && make wrun-${b} -v0 -w$w > /dev/null && grep workloads builds/$TARGET/logs/$b.run.log >> results_workers.log ;done ;done
for b in $BLIST ; do for (( c=1 ; c<=$CMAX ; c++ )) ; do echo "c=$c" >> results_contexts.log && make wrun-${b}w1 -v0 -c$c > /dev/null && grep workloads builds/$TARGET/logs/$b.run.log >> results_contexts.log ;done ;done
for b in $MLIST ; do for (( c=1 ; c<=$CMAX ; c++ )) ; do for (( w=1 ; w<=$WMAX ; w++ )); do echo "c=$c w=$w" >> results_mix.log && make wrun-${b} -v0 -w$w -c$c > /dev/null && grep workloads builds/$TARGET/logs/$b.run.log >> results_mix.log ;done ;done;done

