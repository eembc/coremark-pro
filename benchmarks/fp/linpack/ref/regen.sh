rm -f refall32.log
rm -f refall64.log
for i in 0 1 2 3 4 5 ; do echo "/* dataset $i */" >> refall64.log && ~/dev/smp/builds/linux64/icc/bin/linear_alg-sml-50x50.exe -D="-g1 -i$i" | perl ~/dev/smp/util/perl/copy_dataset.pl -i $i  | perl single_to_multiple.pl >> refall64.log ; done
for i in 0 1 2 3 4 5 ; do echo "/* dataset $i */" >> refall32.log && ~/dev/smp/builds/linux64/icc/bin/linear_alg-sml-50x50-sp.exe -D="-g1 -i$i" | perl ~/dev/smp/util/perl/copy_dataset.pl -i $i  | perl single_to_multiple.pl >> refall32.log ; done
