#!/usr/bin/perl

# File: util/perl/results_parser.pl
#	Description:
#	Utility to parse the execution log of a workload and extract summary information
use File::Basename;
#Fields handled are in the order :
#UID	Suite	Name	Contexts	Item Fails	Time(secs)	Iterations	It/s	Codesize	Datasize
my %runlog;
my %sizelog;
my %name2uid;
my %erglog;
my $oname="";
my $with_energy=0;

# Turn this on to cut and paste a new header into heading.txt
if (0) {
	#        uid   s   n    c    w   f   t    i   i/s   cs   ds   v   std
	printf("%-15s %5s %-40s %3s %3s %5s %10s %10s %10s %10s %10s %10s %10s %10s\n",
		"#UID",
		"Suite",
		"Name",
		"Ctx",
		"Wrk",
		"Fails",
		"t(s)",
		"Iter",
		"i/s",
		"Codesize",
		"Datasize",
		"Variance",
		"StdDev"
	);
	exit 0;
}

foreach $arg (@ARGV) {
	my $uid;
	if ($oname eq "") {
		if (!-f $arg) {
			die "First argument must be the output filename\n";
		}
		open(OUT,">>$arg") || die "Could not open output $arg\n";
		$oname=$arg;
		next;
	}
	if (-f $arg) {
		$uid=parse_run_log($arg);

    #ptorelli -- this is a bug if the word /run/ appears in the path
    #$arg =~ s/run/size/;
    #slightly less buggy way of changing a filename
    use File::Basename;
    my $dir = dirname $arg;
    my $file = basename $arg;
    $file =~ s/run/size/g;
    # but still uses a non-portable path delimeter
    $arg = $dir . "/" . $file;

		if ($uid>0) {
			parse_size_log($arg,$uid);
		}
		foreach $item (sort keys %runlog) {
			print OUT $runlog{$item};
			if (defined($sizelog{$item})) {
				print OUT $sizelog{$item};
			} else {
				print OUT "\t?" x 2;
			}
			print OUT "\t\t"; # for performance variance and stddev
			if ($with_energy) {
				if (defined($erglog{$item})) {
					print OUT $erglog{$item};
				} else {
					print OUT "\t?" x 12;
				}
			}
			print OUT "\n";
		}
	} else {
		print OUT "ERROR: Missing logfile $arg\n";
		die "$arg is not a valid file\n";
	}
}

exit;

# Function: parse_run_log
#	Extract summary information from an execution log
sub parse_run_log {
	my ($name,$rest)=@_;
	my $wname="";
	my $uid=0;
	my $fails=0;
	my $its=0;
	my $ctxt=0;
	my $secs=0;
	my $wrkr="default";
	my $oname=$name;
	if (!open(LOG,"<$name")) {
		$runlog{0}="0\tMLT\t$name\t0\t1\t0\t0\t0";
		print "Could not open runlog $name\n";
		return 0;
	}
	while (<LOG>) {
		$_ =~ s/\r\n/\n/; #translate linefeeds if needed
		chomp;
		$line=$_;
		if ($line =~ /Workload:([^=]*)=(.*)/) {
			$oname=$1;
			$uid=$2;
			$fails=0;
			$wname=quotemeta $oname;
			$name2uid{lc $wname}=$uid;
		}
		next if ($wname eq "");
		if ($line =~ /$wname:contexts=(.*)/) {
			$ctxt=$1;
		}
		if ($line =~ /$wname:workers=(.*)/) {
			$wrkr=$1;
		}
		if ($line =~ /fails=(.*)/) {
			$fails+=$1;
		}
		if ($line =~ /$wname:time\(secs\)=(.*)/) {
			$secs=$1;
		}
		if ($line =~ /$wname:iterations=(.*)/) {
			$its=$1;
		}
		if ($line =~ /Done:$wname=$uid/) {
			$itps="INF";
			if ($secs>0) {
				$itps=$its/$secs;
			}

			# $runlog{$uid}="$uid\tMLT\t$oname\t$ctxt\t$wrkr\t$fails\t$secs\t$its\t".$itps;
			$runlog{$uid} = sprintf("%-15d %5s %-40s %3d %3d %5d %10.3f %10d %10.2f",
				$uid,
				"MLT",
				$oname,
				$ctxt,
				$wrkr,
				$fails,
				$secs,
				$its,
				$itps)
			;
		}
	}
	if ($uid==0) {
		$runlog{0}="0\tMLT\t$name\t0\t0\t1\t0\t0\t0";
		print "Invalid results for $name\n";
		return 0;
	}
	close(LOG);
	return $uid;
}

# Function: parse_size_log
#	Extract summary information from a size log
sub parse_size_log {
	my ($name,$uid)=@_;
	my $wname="";
	my $gcctype=0;
	my $iname = -1;
	my $oktype=0;
	open(LOG,"<$name") or die "Could not open sizelog $name\n";
	while (<LOG>) {
		$_ =~ s/\r\n/\n/; #translate linefeeds if needed
		chomp;
		$line=$_;
		#parse binutils standard output
		if ($line =~ /text/ && $line =~ /data/ && $line =~/bss/) {
			@fields=split /\t/,$line;
			for ($i=0; $i<@fields; $i++) {
				$itxt = $i if ($fields[$i] =~ /text/);
				$idat = $i if ($fields[$i] =~ /data/);
				$ibss = $i if ($fields[$i] =~ /bss/);
				$iname = $i if ($fields[$i] =~ /filename/);
			}
			$gcctype=1;
			$oktype=1;
			next;
		}
		next if (!$oktype);
		if ($gcctype) {
			@fields=split /\t/,$line;
			#remove extra spaces
			for ($i=0; $i<@fields; $i++) { $fields[$i]=~s/\s//g; }
			#get fields by index
			$txt=$fields[$itxt];
			$dat=$fields[$idat];
			$bss=$fields[$ibss];
			$alldat = $dat + $bss;
			if ($iname>=0) {
				#allow using a size file with multiple lines, if wldname==filename
				my $image=lc $fields[$iname];
				my($filename, $directories, $suffix) = fileparse($image);
				if (defined($name2uid{$filename})) {
					$uid = $name2uid{$filename};
				} 
			}
			#$sizelog{$uid}="\t$txt\t".$alldat;
			$sizelog{$uid} = sprintf("%10d %10d", $txt, $alldat);
		}
	}
	close(LOG);
}
