for f in ../../../libbmp/* ; \
do sn=`basename $f | sed -e "s/\./_/"` && ../../../../util/bin/bin2c.exe $sn < $f > $sn.c 2> $sn.h \
; done
