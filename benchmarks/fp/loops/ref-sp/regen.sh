
i=0 fn=1k.c n=0x400 s=0x4645ca3 l=0x64 t=1 && \
~/dev/smp/builds/linux64/icc/bin/loops-all-mid-10k-sp.exe -D="-g1 -i$i" | \
perl ~/dev/smp/util/perl/copy_dataset.pl -i $i > $fn  
i=1 fn=10k.c && \
~/dev/smp/builds/linux64/icc/bin/loops-all-mid-10k-sp.exe -D="-g1 -i$i" | \
perl ~/dev/smp/util/perl/copy_dataset.pl -i $i > $fn  
i=2 fn=100k.c && \
~/dev/smp/builds/linux64/icc/bin/loops-all-mid-10k-sp.exe -D="-g1 -i$i" | \
perl ~/dev/smp/util/perl/copy_dataset.pl -i $i > $fn 
i=3 fn=100.c n=100 x=0x0fe8e785 l=0x64 t=1 && \
~/dev/smp/builds/linux64/icc/bin/loops-all-mid-10k-sp.exe -D="-g1 -i$i -x$x -n$n" | \
perl ~/dev/smp/util/perl/copy_dataset.pl -i $i > $fn  
i=4 fn=32.c n=32 x=0x0fe8e785 l=0x64 t=1 && \
~/dev/smp/builds/linux64/icc/bin/loops-all-mid-10k-sp.exe -D="-g1 -i$i -x$x -n$n" | \
perl ~/dev/smp/util/perl/copy_dataset.pl -i $i > $fn  
i=5 fn=10kdot.c n=0x2710 s=0x4645ca3 l=0x64 t=1 x=8 b=14 && \
~/dev/smp/builds/linux64/icc/bin/loops-all-mid-10k-sp.exe -D="-r1 -g1 -INT1 -i$i -x$x -n$n -s$s -l$l -t$t -b$b" | \
perl ~/dev/smp/util/perl/copy_dataset.pl -i $i > $fn  
i=6 fn=1kdot.c n=0x400 s=0x4645ca3 l=0x64 t=1 x=8 b=14 && \
~/dev/smp/builds/linux64/icc/bin/loops-all-mid-10k-sp.exe -D="-r1 -g1 -INT1 -i$i -x$x -n$n -s$s -l$l -t$t -b$b" | \
perl ~/dev/smp/util/perl/copy_dataset.pl -i $i > $fn  
i=7 fn=100kdot.c n=0x186a0 s=0x4645ca3 l=0x64 t=1 x=8 b=14 && \
~/dev/smp/builds/linux64/icc/bin/loops-all-mid-10k-sp.exe -D="-r1 -g1 -INT1 -i$i -x$x -n$n -s$s -l$l -t$t -b$b" | \
perl ~/dev/smp/util/perl/copy_dataset.pl -i $i > $fn  

