$wname=$ARGV[0];
$wname=~ s/.*\///;
$wname=~ s/.run.log//;
print "$wname\n";

while (<>) {
	if (/Max allocated memory:\s*([\d\.]+)([^\n]*)/) {
		$maxmem=$1.$2;
	}

	next if ($_ !~ /Total allocated memory/);
	#print;
	if (/signal start.*:\s*([\d\.]*)([^\n]*)/) {
		$s1=$1;
		$s1u=$2;
	}
	if (/before init.*:\s*([\d\.]*)([^\n]*)/) {
		$s2=$1;
		$s2u=$2;
	}
	if (/after init.*:\s*([\d\.]*)([^\n]*)/) {
		$s3=$1;
		$s3u=$2;
	}
	if (/before fini.*:\s*([\d\.]*)([^\n]*)/) {
		$s4=$1;
		$s4u=$2;
	}
	if (/after fini.*:\s*([\d\.]*)([^\n]*)/) {
		$s5=$1;
		$s5u=$2;
	}
	if (/signal finish.*:\s*([\d\.]*)([^\n]*)/) {
		$s6=$1;
		$s6u=$2;
	}
}

print "*** Memory leak start to finish!\n" if ($s1 != $s6);
print "*** Memory leak init to fini ($s2$s2u $s5$s5u!\n" if ($s2 != $s5);
if (($s3u eq "MB") && ($s2u eq "KB")) {
	$s3*=1024.0;
	$s3u=$s2u;
}
print "*** Need to handle units!\n" if ($s2u ne $s3u);
$sini=$s3-$s2 if ($s2u eq $s3u);
print "\tSetup memory required: $s2 $s2u\n";
print "\tFor each item: $sini $s3u\n";
print "\tMaximum allocation for one context: $maxmem\n";
