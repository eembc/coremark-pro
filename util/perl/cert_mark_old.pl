#!/usr/bin/perl

# File: util/perl/cert_median.pl
#	Description:
#	Calculate median of performance runs during certification
use strict;

my $toolchain_info="";

if ($ARGV[0] =~ /toolchain_info=(.*)/) {
	$toolchain_info="$1";
	shift @ARGV;
}


my $id=0;
my $iid;
my $ires;
my $iname;
my $itime;
my $icon;
my $iwrk;
my $ifail;
my %all_fields;
my %field_id;
my %name;
my %res;
my %timed;
my $lastuid;
my $lastline;
my $perf_runs=1;
my $debug=0;
my %errfiles;
my $sep=",";

#	mark1/markopt are hashes where each entry contains an array of results for the mark with the same key
my %mark1; 
my %markopt; 
my %Nmark1; 
my %Nmarkopt; 
my %hasmark1; 
my %mark_tracker; 
my %hasmarkopt; 
my @mark_groups;
my @group_name;
my %workers_group;
my %both_group;
my %name2id;
my $curgid=0;
my $curname;
my %group_factor;
my %mark_factor;
my %name_prefix;
my %name_postfix;
my %invalid_mark;
$group_name[$curgid] = "1. MultiMark";
$curname = $group_name[$curgid];
$group_factor{$curname} = 10;
$mark_groups[$curgid] = {
	"rgbcmyk-5x12M1workers" => 	"1. MultiMark", 
	"ipres-72M1worker" => 		"1. MultiMark", 
	"ippktcheck-64M-1Worker" => "1. MultiMark", 
	"md5-32M1worker" => 	"1. MultiMark", 
	"rotate-16x4Ms1w1" => 	"1. MultiMark", 
	"rotate-16x4Ms64w1" => 	"1. MultiMark", 
	"64M-x264-1worker" => 	"1. MultiMark"
	};
$curgid++;
$group_name[$curgid] = "2. ParallelMark";
$curname = $group_name[$curgid];
$group_factor{$curname} = 10;
$mark_groups[$curgid] = {
	"rgbcmyk-5x12M" => "2. ParallelMark", 
	"ipres-72M" => "2. ParallelMark", 
	"rotate-color-4M-90deg" => "2. ParallelMark", 
	"md5-32M" => "2. ParallelMark", 
	"rotate-16x4Ms1" => "2. ParallelMark", 
	"rotate-16x4Ms64" => "2. ParallelMark", 
	"64M-x264" => "2. ParallelMark" 
	};
foreach my $key (keys %{$mark_groups[$curgid]}) {
	$workers_group{$key}=1;
}
$curgid++;
$group_name[$curgid] = "3. MixMark";
$curname = $group_name[$curgid];
$group_factor{$curname} = 10;
$mark_groups[$curgid] = {
	"64M-check-reassembly-tcp" => "3. MixMark",     
	"64M-check-reassembly-tcp-cmykw2-rotatew2" => "3. MixMark",
	"64M-check-reassembly-tcp-h264w2" => "3. MixMark",
	"64M-check-reassembly" => "3. MixMark",         
	"64M-cmykw2-rotatew2" => "3. MixMark",          
	"64M-rotatew2" => "3. MixMark",                 
	"64M-cmykw2" => "3. MixMark",                   
	"64M-tcp-mixed" => "3. MixMark",                
	"64M-x264-2workers" => "3. MixMark",            
	"ipres-72M1worker" => "3. MixMark",             
	"ippktcheck-64M-1Worker" => "3. MixMark"    
	};
foreach my $key (keys %{$mark_groups[$curgid]}) {
	$both_group{$key}=1;
}
$curgid++;
$group_name[$curgid] = "4. MultiMark";
$curname = $group_name[$curgid];
$group_factor{$curname} = 10;
$mark_groups[$curgid] = {
	"rgbcmyk-4Mw1" => 	"4. MultiMark", 
	"ipres-4Mw1" => 		"4. MultiMark", 
	"ippktcheck-4Mw1" => "4. MultiMark", 
	"md5-4Mw1" => 	"4. MultiMark", 
	"rotate-4Ms1w1" => 	"4. MultiMark", 
	"rotate-4Ms64w1" => 	"4. MultiMark", 
	"x264-4Mqw1" => 	"4. MultiMark",
	"iDCT-4Mw1" => "4. MultiMark", 
	};
$curgid++;	
$group_name[$curgid] = "5. ParallelMark";
$curname = $group_name[$curgid];
$group_factor{$curname} = 10;
$mark_groups[$curgid] = {
	"rgbcmyk-4M" => 	"5. ParallelMark", 
	"ipres-4M" => 		"5. ParallelMark", 
	"ippktcheck-4M" => "5. ParallelMark", 
	"md5-4M" => 	"5. ParallelMark", 
	"rotate-4Ms1" => 	"5. ParallelMark", 
	"rotate-4Ms64" => 	"5. ParallelMark", 
	"x264-4Mq" => 	"5. ParallelMark",
	"iDCT-4M" => 		"5. ParallelMark"
	};
foreach my $key (keys %{$mark_groups[$curgid]}) {
	$workers_group{$key}=1;
}
$curgid++;
$group_name[$curgid] = "6. MixMark";
$curname = $group_name[$curgid];
$group_factor{$curname} = 10;
$mark_groups[$curgid] = {
	"4M-check-reassembly-tcp" => "6. MixMark",
	"4M-check-reassembly-tcp-cmykw2-rotatew2" => "6. MixMark",
	"4M-check-reassembly-tcp-x264w2" => "6. MixMark",
	"4M-check-reassembly" => "6. MixMark",
	"4M-cmykw2-rotatew2" => "6. MixMark",
	"4M-rotatew2" => "6. MixMark",
	"4M-cmykw2" => "6. MixMark",
	"4M-tcp-mixed" => "6. MixMark",
	"4M-x264w2" => "6. MixMark",
	"4M-reassembly" => "6. MixMark",
	"4M-check" => "6. MixMark"
	};
foreach my $key (keys %{$mark_groups[$curgid]}) {
	$both_group{$key}=1;
}
$curgid++;
$curname="FPv1.0. Double Precision Small Dataset";
$group_factor{$curname} = 1;
$name_prefix{$curname} = "DpS";
$name_postfix{$curname} = " FPMarks";
$group_name[$curgid] = $curname;
$mark_groups[$curgid] = {
	"blacks-sml-n500v20" 	=> $curname,
	"horner-sml-1k" 		=> $curname,
	"linear_alg-sml-50x50" 	=> $curname,
	"lu-sml-20x2_50" 		=> $curname,
	"xp1px-sml-c100n20" 	=> $curname,
	"radix2-sml-2k" 		=> $curname,
	"nnet_data1" 			=> $curname,
	"atan-1k" 				=> $curname,
	"loops-all-tiny" 		=> $curname,
	"inner-product-sml-1k"	=> $curname,
	"ray-64x48at4s" 		=> $curname
	};
	
$curgid++;
$curname="FPv1.1. Double Precision Medium Dataset";
$name_prefix{$curname} = "DpM";
$name_postfix{$curname} = " FPMarks";
$group_name[$curgid] = $curname;
$group_factor{$curname} = 1;
$mark_groups[$curgid] = {
	"blacks-mid-n1000v40" 		=> $curname,
	"horner-mid-10k" 			=> $curname,
	"linear_alg-mid-100x100" 	=> $curname,
	"loops-all-mid-10k" 		=> $curname,
	"inner-product-mid-10k" 	=> $curname,
	"lu-mid-200x2_50" 			=> $curname,
	"radix2-mid-8k" 			=> $curname,
	"xp1px-mid-c1000n200" 		=> $curname,
	"ray-320x240at8s" 			=> $curname,
	"atan-64k" 					=> $curname,
	"nnet_test" 				=> $curname
	};
	
$curgid++;
$curname="FPv1.2. Double Precision Big Dataset";
$name_prefix{$curname} = "DpL";
$name_postfix{$curname} = " FPMarks";
$group_factor{$curname} = 1;
$group_name[$curgid] = $curname;
$mark_groups[$curgid] = {
	"blacks-big-n5000v200" 		=> $curname,
	"horner-big-100k" 			=> $curname,
	"linear_alg-big-1000x1000" 	=> $curname,
	"loops-all-big-100k" 		=> $curname,
	"inner-product-big-100k" 	=> $curname,
	"lu-big-2000x2_50" 			=> $curname,
	"radix2-big-64k" 			=> $curname,
	"xp1px-big-c10000n2000" 	=> $curname,
	"ray-1024x768at24s" 		=> $curname,
	"atan-1M" 					=> $curname
	};

$curgid++;
$curname="FPv1.D. Double Precision Mark";
my $FPDPgid=$curgid;
$name_prefix{$curname} = "Dp";
$name_postfix{$curname} = " FPMarks";
$group_factor{$curname} = 1;
$group_name[$curgid] = $curname;
foreach my $k (keys %{$mark_groups[$curgid-1]}) {
	$mark_groups[$curgid]{$k}=$curname;
}
foreach my $k (keys %{$mark_groups[$curgid-2]}) {
	$mark_groups[$curgid]{$k}=$curname;
}
foreach my $k (keys %{$mark_groups[$curgid-3]}) {
	$mark_groups[$curgid]{$k}=$curname;
}
	
$curgid++;
$curname="FPv1.3. SP Small Dataset";
my $FPSPSgid=$curgid;
$name_prefix{$curname} = "SpS";
$name_postfix{$curname} = " FPMarks";
$group_name[$curgid] = $curname;
$group_factor{$curname} = 1;
$mark_groups[$curgid] = {
	"blacks-sml-n500v20-sp" => $curname,
	"horner-sml-1k-sp" => $curname,
	"linear_alg-sml-50x50-sp" => $curname,
	"lu-sml-20x2_50-sp" => $curname,
	"loops-all-tiny-sp" => $curname,
	"inner-product-sml-1k-sp" => $curname,
	"atan-1k-sp" => $curname,
	"nnet-data1-sp" => $curname
	};

$curgid++;
$curname="FPv1.4. SP Medium Dataset";
$name_prefix{$curname} = "SpM";
$name_postfix{$curname} = " FPMarks";
$group_name[$curgid] = $curname;
$group_factor{$curname} = 1;
$mark_groups[$curgid] = {
	"blacks-mid-n1000v40-sp" => $curname,
	"horner-mid-10k-sp" => $curname,
	"linear_alg-mid-100x100-sp" => $curname,
	"loops-all-mid-10k-sp" => $curname,
	"inner-product-mid-10k-sp" => $curname,
	"lu-mid-200x2_50-sp" => $curname,
	"atan-64k-sp" => $curname,
	"nnet_test-sp" => $curname
	};


$curgid++;
$curname="FPv1.5. SP Big Dataset";
$name_prefix{$curname} = "SpL";
$name_postfix{$curname} = " FPMarks";
$group_factor{$curname} = 1;
$group_name[$curgid] = $curname;
$mark_groups[$curgid] = {
	"blacks-big-n5000v200-sp" => $curname,
	"horner-big-100k-sp" => $curname,
	"linear_alg-big-1000x1000-sp" => $curname,
	"loops-all-big-100k-sp" => $curname,
	"inner-product-big-100k-sp" => $curname,
	"lu-big-2000x2_50-sp" => $curname,	
	"atan-1M-sp" => $curname
	};

$curgid++;
$curname="FPv1.S. SP Mark";
my $FPSPgid=$curgid;
$name_prefix{$curname} = "Sp";
$name_postfix{$curname} = " FPMarks";
$group_factor{$curname} = 1;
$group_name[$curgid] = $curname;
foreach my $k (keys %{$mark_groups[$curgid-1]}) {
	$mark_groups[$curgid]{$k}=$curname;
}
foreach my $k (keys %{$mark_groups[$curgid-2]}) {
	$mark_groups[$curgid]{$k}=$curname;
}
foreach my $k (keys %{$mark_groups[$curgid-3]}) {
	$mark_groups[$curgid]{$k}=$curname;
}

$curgid++;
$curname="FPMark";
$name_prefix{$curname} = "";
$name_postfix{$curname} = "";
$group_factor{$curname} = 100;
$group_name[$curgid] = $curname;
foreach my $k (keys %{$mark_groups[$FPSPgid]}) {
	$mark_groups[$curgid]{$k}=$curname;
}
foreach my $k (keys %{$mark_groups[$FPDPgid]}) {
	$mark_groups[$curgid]{$k}=$curname;
}

$curgid++;
$curname="MicroFPMark";
$name_prefix{$curname} = "";
$name_postfix{$curname} = "";
$group_factor{$curname} = 1;
$group_name[$curgid] = $curname;
foreach my $k (keys %{$mark_groups[$FPSPSgid]}) {
	$mark_groups[$curgid]{$k}=$curname;
}

$curgid++;
$curname="CoreMark-PRO";
$name_prefix{$curname} = "";
$name_postfix{$curname} = " PROMarks".$toolchain_info;
$group_factor{$curname} = 1000;
$group_name[$curgid] = $curname;
$mark_groups[$curgid] = {
	"cjpeg-rose7-preset" => $curname,
	"linear_alg-mid-100x100-sp" => $curname,
	"loops-all-mid-10k-sp" => $curname,
	"nnet_test" => $curname,
	"parser-125k" => $curname,
	"radix2-big-64k" => $curname,
	"sha-test" => $curname,
	"zip-test" => $curname,
	"core" => $curname
	};
$mark_factor{$curname} = {
	"cjpeg-rose7-preset" => 40.3438,
	"linear_alg-mid-100x100-sp" => 38.5624,
	"loops-all-mid-10k-sp" => 0.87959,
	"nnet_test" => 1.45853 ,
	"parser-125k" => 4.81116,
	"radix2-big-64k" => 99.6587,
	"sha-test" => 48.5201,
	"zip-test" => 21.3618,
	"core" => 2855/10000
	};
	
#extract all results from the log file
my $line=1;
my $numres=0;
while (<>) {
	chomp;
	$line++;
	$lastline=$_;
	my @fields = split;
	my $nfields=scalar @fields;
	#collect field indexes
	if ($id==0) {
		foreach my $f (@fields) {
			$field_id{"$f"}=$id;
			$id++;
		}
		$id--;
		$iid  =$field_id{"#UID"};
		$ires =$field_id{"Iter/s"};
		$iname=$field_id{"Name"};
		$itime=$field_id{"t(s)"};
		$icon =$field_id{"Ctx"};
		$iwrk =$field_id{"Wrk"};
		$ifail =$field_id{"Fails"};
		next;
	}
	#scan for start of performance runs
	if (/performance runs/) {
		$perf_runs=1;
	}
	if (/verification run/ || /final result/) {
		$perf_runs=0;
	}
	next if (!$perf_runs);
	#now save all results
	next if ($nfields < $ires);
	$lastuid=$fields[$iid];
	if ($fields[$ifail]>0) {
		if (!defined($errfiles{$fields[$iname]})) {
			my $name=$fields[$iname];
			$name=~ s/.*\///;
			$name=~ s/\.run\.log//;
			print "*** Error detected for $name [ $fields[$ifail] ]\n";
			#print STDERR "*** Error detected for $name at $line\n";
		}
		$errfiles{$fields[$iname]}++;
	}
	$all_fields{$lastuid}=\@fields;
	if (defined($name{$lastuid})) {
		if ($fields[$iname] ne $name{$lastuid}) {
			print "Two workloads with the same UID!\n$fields[$iname] / $name{$lastuid}\n";
			die;
		}
	} else {
		$name{$lastuid}=$fields[$iname];
		$name2id{$name{$lastuid}}=$lastuid;
	}
	my $cont=$fields[$icon];
	if ($workers_group{$name{$lastuid}}) {
		$cont=$fields[$iwrk];
	}
	$cont=1 if ($cont =~ /default/i || $cont == 0);
	if ($both_group{$name{$lastuid}}) {
		my $contw=$fields[$iwrk];
		$contw=1 if ($contw =~ /default/i || $contw == 0);
		$cont*=$contw;
	}
	push @{$res{$lastuid}{$cont}}, $fields[$ires];
	push @{$timed{$lastuid}}, $fields[$itime];
	$numres++;
}
print "Processed $line lines. Found $numres individual results.\n" if ($debug==1);

#for each workload, calculate or get median, and divide based on contexts preparatory to calculating the mark
foreach $lastuid (sort {$name{$a} cmp $name{$b}} keys %res) {
	foreach my $cont (keys %{$res{$lastuid}}) {
		my $numres=scalar @{$res{$lastuid}{$cont}};
		splice @{$res{$lastuid}{$cont}}, 0, 1 if (($numres % 2) == 0);
		my $med=median($res{$lastuid}{$cont});
		my $foundme=0;
		foreach my $markgrouptype (@mark_groups) {
			if (defined($markgrouptype->{$name{$lastuid}})) {
				print "Add $name{$lastuid} [$med] to $markgrouptype->{$name{$lastuid}}\n" if ($debug>1);
				$foundme=1;
				my $gmed=$med;
				my $gname=$markgrouptype->{$name{$lastuid}};
				if (defined($mark_factor{$gname})) {
					$gmed /= $mark_factor{$gname}{$name{$lastuid}};
					printf "Scale $name{$lastuid} by $mark_factor{$gname}{$name{$lastuid}} to [$gmed]\n" if ($debug>0);
				} 
				if ($cont==1) {
					push @{$mark1{$gname}}, $gmed; 
					$mark_tracker{$gname}{$lastuid}=$gmed;
					push @{$Nmark1{$gname}}, $name{$lastuid}; 
					$hasmark1{$lastuid}=$gmed;
				} else {
					push @{$markopt{$gname}}, $gmed; 
					push @{$Nmarkopt{$gname}}, $name{$lastuid}; 
					$hasmarkopt{$lastuid}=$gmed;
				}
			}
		}
		if (!$foundme) {
			print "Could not find mark type for $name{$lastuid}\n" if ($debug==2);
		}
	}
}

my %markoptnone;
#it is possible that optimal performance for a workload is achieved with 1 context. In that case, use the value from 1 context.
foreach $lastuid (sort keys %hasmark1) {
	if (!defined($hasmarkopt{$lastuid})) {
		foreach my $markgrouptype (@mark_groups) {
			if (defined($markgrouptype->{$name{$lastuid}})) {
				my $gname=$markgrouptype->{$name{$lastuid}};
				push @{$markopt{$gname}}, $mark_tracker{$gname}{$lastuid}; 
				push @{$Nmarkopt{$gname}}, $name{$lastuid}; 
				$markoptnone{$gname}++;
				$hasmarkopt{$lastuid}=$mark_tracker{$gname}{$lastuid};
			}
		}
	}
}

foreach my $marktype (sort keys %markopt) {
	#count number of participating scores
	my $i;
	my $valid=test_num_scores($marktype,$markopt{$marktype},"Performance");
	#ok, now special handling - calculate the mark geomean with any special handling required
	my $markval=1;
	$markval=mark_geomean($markopt{$marktype},$Nmarkopt{$marktype},$marktype) if ($valid);
	#And now set the actual mark instead of an array of mark values.
	if ($valid) {
		$markopt{$marktype}=$group_factor{$marktype}*$markval ;
		print "GOT $markval for $marktype!\n" if $debug;
	} else {
		$invalid_mark{$marktype}++;
		print "Invalid mark for $marktype!\n" if $debug;
	}
}

foreach my $marktype (sort keys %mark1) {
	print "computing for $marktype\n" if $debug > 1;
	#count number of participating scores
	my $i;
	next if (defined($invalid_mark{$marktype}));
	my $valid=test_num_scores($marktype,$mark1{$marktype},"Scale");
	print "valid = $valid\n" if $debug > 1;
	#ok, now special handling - calculate the mark geomean with any special handling required
	my $gm=1;
	$gm=mark_geomean($mark1{$marktype},$Nmark1{$marktype},$marktype) if ($valid);
	print "gm = $gm\n" if $debug > 1;
	if ($gm == 0) {
		print "Error: Singleitem value for $marktype is 0!\n";
		next;
	}
	my $markval=($markopt{$marktype}/$group_factor{$marktype}) / $gm;
	print "markval = $markval\n" if $debug > 1;
	$mark1{$marktype}=$markval;
}

my $formatString = "%-25s %15s %10s %-37s\n";

#print "Mark${sep}Performance${sep}Scale${sep}Comments\n";
printf $formatString, "Mark", "Performance", "Scale", "Comments";
printf $formatString, '-' x 25, '-' x 15, '-' x 10, '-' x 37;

foreach my $marktype (sort keys %markopt) {
	#output performance mark
	next if (defined($invalid_mark{$marktype}));
	my $comment="";
	my $pref="";
	my $post="";
	if (defined $markoptnone{$marktype}) {
		$comment .= "Optimal performance at one item for $markoptnone{$marktype} results"; 
	}
	if (defined($name_prefix{$marktype})) { $pref=$name_prefix{$marktype}; }
	if (defined($name_postfix{$marktype})) { $post=$name_postfix{$marktype}; }

	print "$marktype -> $markopt{$marktype} v. $mark1{$marktype}\n" if $debug;
	my $v0=sprintf("%.6g",$markopt{$marktype});
	# PJT WTF is this - is  range op .. if ($v0 =~ /^[+-e\d\.]+$/) { $v0 = $pref.$v0.$post; }
	my $v1=$mark1{$marktype};


	# PJT it is very awkward to start a list of marks with 4., 5., ... 6...
	# PJT TODO just change names, don't regex FFS
	$marktype =~ s/^\d\. //;
	# PJT Clean up decimals
	$v0 = sprintf("%.3f", $v0);
	$v1 = sprintf("%.3f", $v1);
	printf $formatString, $marktype, $v0, $v1, $comment;

	#print "$marktype$sep$v0$sep$v1$sep$comment\n";
}

exit;

sub geomean {
    my $valref = shift;
    my @values = @$valref;
	my $power = scalar(@values);
	my $tot=0;
	foreach (@values) {
		$tot += log($_) if ($_!=0);
	#	print "-\t$_\n";
	}
	$tot=$tot/$power;
	my $res = exp($tot);
	#print "\n-\tGEO: $res\n";
	return $res;
}

sub median {
    my $rpole = shift;
    my @pole = @$rpole;

    my $ret;
    my @spole=sort {$a <=> $b} @pole;
    if( (@spole % 2) == 1 ) {
        $ret = $spole[((@spole+1) / 2)-1];
    } else {
        $ret = ($spole[(@spole / 2)-1] + $spole[@spole / 2]) / 2;
    }
	my $i;
    return $ret;
}

sub mean {
  my $result;
  foreach (@_) { $result += $_ }
  return $result / @_;
}

sub std_dev {
  my $mean = mean(@_);
  my @elem_squared;
  foreach (@_) {
    push (@elem_squared, ($_ **2));
  }
  return sqrt( mean(@elem_squared) - ($mean ** 2));
}

sub variance
{
    my $array_ref = shift;
    my $n = scalar(@{$array_ref});
    my $result = 0;
    my $item;
    my $sum = 0;
    my $sum_sq = 0;
    foreach $item (@{$array_ref})
    {
        $sum += $item;
        $sum_sq += $item*$item;
    }
    if ($n > 1)
    {
        $result = (($n * $sum_sq) - $sum**2)/($n*($n-1));
    }

    return $result;
}

sub splitparts 
{
	my ($geo1,$geo2,$nums,$names) = @_;
	my $i;
	for ($i=0 ; $i < scalar(@{$nums}) ; $i++) {
		if ($names->[$i] =~ /rotate\-/) {
			push @$geo2,$nums->[$i];
		} else {
			push @$geo1,$nums->[$i];
		}
	}
}

sub test_num_scores
{
	my ($marktype,$nums,$msg) = @_;
	my $numbench=0;
	my $groupi=0;
	my $i;
	foreach ($i=0; $i<@group_name ;$i++) {
		if ($group_name[$i] eq $marktype) {
			$numbench=scalar(keys %{$mark_groups[$i]});
			$groupi=$i;
		}
	}
	#make sure there are scores for a valid mark
	if (scalar(@{$nums}) < $numbench) {
		if ($debug>1) {
			print "Not enough scores to calculate $msg mark for $marktype!!\n\n";
			print "Need:\n";
			foreach my $key ( sort keys %{$mark_groups[$groupi]} ) {
				my $id=$name2id{$key};
				my $m1=$hasmark1{$id};
				my $m2=$hasmarkopt{$id};
				print "\t$key [$m1/$m2]\n";
			}
			print "Found only ".scalar(@{$nums})." scores\n";
		}
		return 0;
	} 
	return 1;
	#print "$marktype has $numbench, should have ".scalar(@{$nums})."\n";
}

sub mark_geomean 
{
	my ($nums,$names,$type) = @_;
	my @geo1;
	my @geo2;
	my $i;
	if ($debug) {
		print "Median $type:\n" ;
		for ($i=0 ; $i < scalar(@{$nums}) ; $i++) {
			print "\t$names->[$i]\t$nums->[$i]\n";
		}
	}
	splitparts(\@geo1,\@geo2,$nums,$names);
	if (@geo2) {
		my $val2=geomean(\@geo2);
		print "submark=$val2\n" if ($debug);
		push @geo1,$val2;
	}
	#calc geomean on array
	return geomean(\@geo1);
}
