while (<>) {
	if (/(.*)};/) { print "$1},\n"; $indat=0; }
	print if ($indat); 
	if (/={(.*)/) { print "\t{$1"; $indat=1; }
}
