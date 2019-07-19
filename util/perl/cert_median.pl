#!/usr/bin/perl

# File: util/perl/cert_median.pl
#	Description:
#	Calculate median of performance runs during certification
use strict;

my $id=0;
my $iid;
my $ires;
my $iname;
my $itime;
my %all_fields;
my %field_id;
my %name;
my %res;
my %timed;
my $lastuid;
my $lastline;
my $perf_runs=0;

#hardcode column ids
$iid = 0;
$ires = 8;
$iname = 2;
$itime = 6;

my ($fn, $contype) = @ARGV;
die "Usage: $0 filename currencytype(best|single)\n" 
	if ! defined $contype || ! -f $fn;

open FP, $fn or die "$0: Failed to open file $fn : $!\n";

#extract all results from the log file
while (<FP>) {
	chomp;
	$lastline=$_;
	my @fields = split /\s+/;
	my $nfields=scalar @fields;

#	#collect field indexes
#	if ($id==0) {
#		foreach my $f (@fields) {
#			$field_id{"$f"}=$id;
#			$id++;
#		}
#		$id--;
#		$iid=$field_id{"UID"};
#		$ires=$field_id{"i/s"};
#		$iname=$field_id{"Name"};
#		$itime=$field_id{"t(s)"};
#		next;
#	}

	#scan for start of performance runs
	if (/performance runs/) {
		$perf_runs=1;
	}
	if (/verification run/) {
		$perf_runs=0;
	}
	next if (!$perf_runs);
	#now save all results
	next if ($nfields < $ires);
	next if ($fields[$iid] eq "UID");
	next if $fields[$iid] =~ /^#/;

	$lastuid=$fields[$iid];
	$all_fields{$lastuid}=\@fields;
	$name{$lastuid}=$fields[$iname];
	push @{$res{$lastuid}}, $fields[$ires];
	push @{$timed{$lastuid}}, $fields[$itime];
}
close FP;

#for each workload, calculate and print the median as an extra result line
foreach $lastuid (sort keys %res) {
	my $numres=scalar @{$res{$lastuid}};
	if ($numres<3) {
		print "Could not find 3 valid results for $name{$lastuid} [$lastuid]\n";
		next;
	}
	splice @{$res{$lastuid}}, 0, $numres-3;
	my $med=median($res{$lastuid});
	$all_fields{$lastuid}->[$ires]=$med;
	$med=median($timed{$lastuid});
	$all_fields{$lastuid}->[$itime]=$med;

	printf("%-15d %5s %-40s %3d %3d %5d %10.3f %10d %10.2f %9d %10d median $contype\n",
		@{$all_fields{$lastuid}},
		#variance($res{$lastuid}),
		#std_dev($res{$lastuid})
	);

#	foreach my $f (@{$all_fields{$lastuid}}) {
#		print "$f\t";
#	}
#	#add variance and standard deviation at the end.
#	print variance($res{$lastuid});
#	print "\t";
#	print std_dev($res{$lastuid});
#	print "\n";

}

exit;

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
    my $n = scalar @{$array_ref};
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
