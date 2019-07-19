#!/usr/bin/perl

# 
# This is a re-write of cert_mark.pl that is more robust,
# better commented, and much smaller.
#
# Differences
# - It does not re-create "best" by deciding which score is higher
# - It does not check to make sure the correct # of runs completed
# - Prints BOTH single and best
# - Relies on the median line to specify (median|best) becuase 
#   using XMCD='-c1 -w1' is ambiguous and breaks things. (cannot
#   infer from context)
#

use warnings;
use strict;
use Data::Dumper;
use Getopt::Long;
use File::Basename;

$0 = basename $0; # I like to start all messages with $0

# The global score database; currently just holds ips
# {$name}{[single|best]}{'ips'} = iterations/second
my %g_scores;

# The global mark definition
#
# Notes:
# - the __SUBSET_x__ value defines sub-geomeans
# - the value of the test name is the scale factor
# - 'factor' is the group scale factor
# - the score is: "factor * geomean(tests, geommean(SUBSETS))"
#
my %g_mark_definitions = (

	# The scale factors for CoreMark-PRO were defined by the workgroup
	# and are based on the scores of the reference platform.
	'CoreMark-PRO' => {
		'factor' => 1000,
		'geomean' => {
			'cjpeg-rose7-preset' => (1 / 40.3438),
			'linear_alg-mid-100x100-sp' => (1 / 38.5624),
			'loops-all-mid-10k-sp' => (1 / 0.87959),
			'nnet_test' => (1 / 1.45853 ),
			'parser-125k' => (1 / 4.81116),
			'radix2-big-64k' => (1 / 99.6587),
			'sha-test' => (1 / 48.5201),
			'zip-test' => (1 / 21.3618),
			'core' => (10000 / 2855)
		}
	},

	'MultiBench, 1. MultiMark' => {
		'factor' => 10,
		'geomean' => {
			'iDCT-4Mw1' => 1,
			'ippktcheck-4Mw1' => 1,
			'ipres-4Mw1' => 1,
			'md5-4Mw1' => 1,
			'rgbcmyk-4Mw1' => 1,
			'__SUBSET_1__' => {
				'rotate-4Ms1w1' => 1,
				'rotate-4Ms64w1' => 1
			},
			'x264-4Mqw1' => 1
		}
	},

	'MultiBench, 2. ParallelMark' => {
		'factor' => 10,
		'geomean' => {
			'iDCT-4M' => 1,
			'ippktcheck-4M' => 1,
			'ipres-4M' => 1,
			'md5-4M' => 1,
			'rgbcmyk-4M' => 1,
			'__SUBSET_1__' => {
				'rotate-4Ms1' => 1,
				'rotate-4Ms64' => 1
			},
			'x264-4Mq' => 1
		}
	},

	'MultiBench, 3. MixMark' => {
		'factor' => 10,
		'geomean' => {
			'4M-check' => 1,
			'4M-tcp-mixed' => 1,
			'4M-cmykw2' => 1,
			'4M-reassembly' => 1,
			'4M-rotatew2' => 1,
			'4M-x264w2' => 1,
			'4M-check-reassembly' => 1,
			'4M-check-reassembly-tcp' => 1,
			'4M-check-reassembly-tcp-cmykw2-rotatew2' => 1,
			'4M-check-reassembly-tcp-x264w2' => 1,
			'4M-cmykw2-rotatew2' => 1
		}
	},

	'FPv1.0. DP Small Dataset' => {
		'factor' => 1,
		'geomean' => {
			'blacks-sml-n500v20' => 1,
			'horner-sml-1k' => 1,
			'linear_alg-sml-50x50'=> 1,
			'lu-sml-20x2_50' => 1,
			'xp1px-sml-c100n20' => 1,
			'radix2-sml-2k' => 1,
			'nnet_data1' => 1,
			'atan-1k' => 1,
			'loops-all-tiny' => 1,
			'inner-product-sml-1k'=> 1,
			'ray-64x48at4s' => 1
		}
	},
	
	'FPv1.1. DP Medium Dataset' => {
		'factor' => 1,
		'geomean' => {
			'blacks-mid-n1000v40' => 1,
			'horner-mid-10k' => 1,
			'linear_alg-mid-100x100' => 1,
			'loops-all-mid-10k' => 1,
			'inner-product-mid-10k' => 1,
			'lu-mid-200x2_50' => 1,
			'radix2-mid-8k' => 1,
			'xp1px-mid-c1000n200' => 1,
			'ray-320x240at8s' => 1,
			'atan-64k' => 1,
			'nnet_test' => 1
		}
	},
	
	'FPv1.2. DP Big Dataset' => {
		'factor' => 1,
		'geomean' => {
			'blacks-big-n5000v200' => 1,
			'horner-big-100k' => 1,
			'linear_alg-big-1000x1000' => 1,
			'loops-all-big-100k' => 1,
			'inner-product-big-100k' => 1,
			'lu-big-2000x2_50' => 1,
			'radix2-big-64k' => 1,
			'xp1px-big-c10000n2000' => 1,
			'ray-1024x768at24s' => 1,
			'atan-1M' => 1
		}
	},

	'FPv1.3. SP Small Dataset' => {
		'factor' => 1,
		'geomean' => {
			'blacks-sml-n500v20-sp' => 1,
			'horner-sml-1k-sp' => 1,
			'linear_alg-sml-50x50-sp' => 1,
			'lu-sml-20x2_50-sp' => 1,
			'loops-all-tiny-sp' => 1,
			'inner-product-sml-1k-sp' => 1,
			'atan-1k-sp' => 1,
			'nnet-data1-sp' => 1
		}
	},

	'FPv1.4. SP Medium Dataset' => {
		'factor' => 1,
		'geomean' => {
			'blacks-mid-n1000v40-sp' => 1,
			'horner-mid-10k-sp' => 1,
			'linear_alg-mid-100x100-sp' => 1,
			'loops-all-mid-10k-sp' => 1,
			'inner-product-mid-10k-sp' => 1,
			'lu-mid-200x2_50-sp' => 1,
			'atan-64k-sp' => 1,
			'nnet_test-sp' => 1
		}
	},

	'FPv1.5. SP Big Dataset' => {
		'factor' => 1,
		'geomean' => {
			'blacks-big-n5000v200-sp' => 1,
			'horner-big-100k-sp' => 1,
			'linear_alg-big-1000x1000-sp' => 1,
			'loops-all-big-100k-sp' => 1,
			'inner-product-big-100k-sp' => 1,
			'lu-big-2000x2_50-sp' => 1,	
			'atan-1M-sp' => 1
		}
	},

	'FPv1.D. DP Mark' => {
		'factor' => 1,
#		'geomean' => {} # aggregate, filled in below
	},

	'FPv1.S. SP Mark' => {
		'factor' => 1,
#		'geomean' => {} # aggregate, filled in below
	},

	'FPMark' => {
		'factor' => 100,
#		'geomean' => {} # aggregate, filled in below
	},

	'MicroFPMark' => {
		'factor' => 1,
#		'geomean' => {} # aggregate, filled in below
	}

);

#
# Construct aggregate marks programatically to limit human error. :)
#
foreach my $mark (
	'FPv1.0. DP Small Dataset',
	'FPv1.1. DP Medium Dataset',
	'FPv1.2. DP Big Dataset') {
	foreach my $workload (keys %{$g_mark_definitions{$mark}{'geomean'}}) {
		$g_mark_definitions{'FPv1.D. DP Mark'}{'geomean'}{$workload} = 1;
	}
}

foreach my $mark (
	'FPv1.3. SP Small Dataset',
	'FPv1.4. SP Medium Dataset',
	'FPv1.5. SP Big Dataset') {
	foreach my $workload (keys %{$g_mark_definitions{$mark}{'geomean'}}) {
		$g_mark_definitions{'FPv1.S. SP Mark'}{'geomean'}{$workload} = 1;
	}
}

foreach my $mark (
	'FPv1.S. SP Mark',
	'FPv1.D. DP Mark') {
	foreach my $workload (keys %{$g_mark_definitions{$mark}{'geomean'}}) {
		$g_mark_definitions{'FPMark'}{'geomean'}{$workload} = 1;
	}
}

foreach my $mark (
	'FPv1.3. SP Small Dataset') {
	foreach my $workload (keys %{$g_mark_definitions{$mark}{'geomean'}}) {
		$g_mark_definitions{'MicroFPMark'}{'geomean'}{$workload} = 1;
	}
}

# In order to provide feedback about failures, the script MUST have
# an indication of what kind of mark it is computing. In the past this
# was autodetected, but at the expense of providing the suers with
# feedback if a test failed. By manually stating the suite we are computing
# the script can tell the user what tests are missing. (Previously it would
# generate misleading results of this error was turned on because some tests
# that exist in CoreMark-PRO also exist in FPMark, creating lots of bogus
# errors). Note that the group names MUST match the workload sets because
# the makefile calls this script with --suite <suite>
my %g_suite_groups = (
	'multibench' => [
		'MultiBench, 1. MultiMark',
		'MultiBench, 2. ParallelMark',
		'MultiBench, 3. MixMark'
	],
	'fpmark' => [
		'FPMark',
		'FPv1.0. DP Small Dataset',
		'FPv1.1. DP Medium Dataset',
		'FPv1.2. DP Big Dataset',
		'FPv1.3. SP Small Dataset',
		'FPv1.4. SP Medium Dataset',
		'FPv1.5. SP Big Dataset',
		'FPv1.D. DP Mark',
		'FPv1.S. SP Mark',
		'MicroFPMark'
	],
	'autobench' => [
	],
	'coremarkpro' => [
		'CoreMark-PRO'
	]
);

# Command-line options
my %g_opts;

# MAIN ROUTINE
{
	&init;
	&parse_log;
	&print_scaletable;
	&print_multimarks;
	exit 0;
}

sub print_scaletable {
	print "\n";
	print "WORKLOAD RESULTS TABLE\n";
	print "\n";
	printf "%-47s %10s %10s %10s\n", "", "MultiCore", "SingleCore", "";
	printf "%-47s %10s %10s %10s\n", "Workload Name", "(iter/s)", "(iter/s)", "Scaling";
	printf "%-47s %10s %10s %10s\n", '-' x 47, '-' x 10, '-' x 10, '-' x 10;
	foreach my $w (sort keys %g_scores) {
		next if $w =~ /empty-wld/; # this is needed but shouldn't be in the table
		my $single = $g_scores{$w}{'single'}{'ips'};
		my $best = $g_scores{$w}{'best'}{'ips'};
		my $scale;
		if (defined $single && defined $best && $single != 0) {
			$scale = $best / $single;
		}
		printf "%-47s %10s %10s %10s\n",
			$w,
			defined $best ? sprintf("%10.2f", $best) : "n/a",
			defined $single ? sprintf("%10.2f", $single) : "n/a",
			defined $scale ? sprintf("%10.2f", $scale) : "n/a"
		;
	}
}

sub print_multimarks {
	print "\n";
	print "MARK RESULTS TABLE\n";
	print "\n";
	printf "%-47s %10s %10s %10s\n", "Mark Name", "MultiCore", "SingleCore", "Scaling";
	printf "%-47s %10s %10s %10s\n", '-' x 47, '-' x 10, '-' x 10, '-' x 10;
	# Check to make sure we have all the data first
	foreach my $mark (sort @{$g_suite_groups{$g_opts{'suite'}}}) {
		my $factor = $g_mark_definitions{$mark}{'factor'};
		my $single = $factor * recurse_geomean($mark, 'single', $g_mark_definitions{$mark}{'geomean'});
		my $best = $factor * recurse_geomean($mark, 'best', $g_mark_definitions{$mark}{'geomean'});
		# the computation return returns -1 if a score is missing
		if ($single < 0 || $best < 0) {
			print "$0: ERROR: Could not compute score for '$mark'\n";
		} else {
			printf "%-47s %10.2f %10.2f %10.2f\n", $mark, $best, $single, $best / $single
		}
	}
}

# Some of the marks require that sets of the scores be computed and
# then added to list of scores to be geomean'd. E.g., GM(a, b, GM(d, e), f)
# This is accomplished by checking for array references inside the list
# of scores in the definition. Missing scores cause a '-1' to trickle up
# through the computation.
sub recurse_geomean {
	my ($dbg_mark, $mode, $ptr) = @_;
	my @res = ();
	my $errors = 0;
	for my $v (keys %$ptr) {
		my $ips;
		if ($v =~ m/^__SUBSET_\d+__$/) {
			$ips = recurse_geomean($dbg_mark, $mode, $ptr->{$v});
			if ($ips < 0) {
				$errors = 1;
			} else {
				push @res, $ips;
			}
		} else {
			$ips = $g_scores{$v}{$mode}{'ips'};
			my $factor = $ptr->{$v};
			# Make sure the run actually completed
			if (! defined $ips) {
				print "$0: ERROR: Missing score for $v ($mode)\n";
				$errors = 1;
			} elsif ($g_scores{$v}{$mode}{'errors'} != 0) {
				print "$0: ERROR: Errors encountered in test $v, $mode run\n";
				$errors = 1;
			} else {
				my $score = $ips * $factor;
				$dbg_mark =~ s/\s+//g;
				printf("$0: DEBUG: IPS: %-30s %-30s %10s %14.4f %14.4f %14.4f\n",
					$dbg_mark,
					$v,
					$mode,
					$factor,
					$ips,
					$score
				) if $g_opts{'debug'};
				push @res, $score;
			}
		}
	}
	return $errors ? -1 : geomean(@res);
}

# Standard issue geometric mean computation.
sub geomean {
    my @values = @_;
	my $power = scalar @values;
	my $tot = 1;
	foreach (@values) {
		$tot *= $_;
	}
	return $tot ** (1 / $power);
}

# Flattens the lists of tests for an entry in the definitions hash
sub flatten_lists {
	my $lptr = shift;
	my @list;
	foreach my $l (keys %$lptr) {
		if ($l =~ /^__SUBSET_\d+__$/) {
			push @list, flatten_lists($lptr->{$l});
		} else {
			push @list, $l;
		}
	}
	return @list;
}

sub parse_log {

	my $fn = $g_opts{'input'};
	my $fp;
	open($fp, $fn) or die "$0: ERROR: Failed to open $fn ($!)\n";
	my $mode = 'init'; # determine the parsing context of the logfile
	my $ln = 0; # linenumber
	my %most_recent_error; # a workaround to track any verf errors
	while (<$fp>)  {
		++$ln;
		# Determine the context of the line because lines are independent,
		# you need to know what you've seen to understand what the data
		# means.
		if (/UID/) { # header line
			next;
		}
		if (/Results for (verification|performance)/) {
			$mode = $1;
			next;
		}
		if (/Median for final result/) {
			$mode = 'median';
			next;
		}
		next if /^\s*#/;

		# We only use the fields up to 'itertaions per second'
		my ($UID,
		    $suite,
		    $name,
		    $contexts,
		    $workers,
		    $fails,
		    $secs,
		    $iterations,
		    $ips) = split(/\s+/);

		# since cert_median.pl doesn't aggregate failures...
		# capture any failures we see in verf mode...
		if ($mode eq "verification") {
			$most_recent_error{$name} = $fails;
		}

		# Skip anything that isn't a 'median' score
		if (!/^#/ && $mode ne 'median') {
			next;
		}

		# determine if this is best or single run (the cert_median.pl script prints this)
		m/median\s+(best|single)/;
		my $scoring = $1;
		die "$0: RT-ERROR: Score mode (best|single) did not appear on median line, $fn : $ln\n"
			unless defined $scoring;

		# This shouldn't ever happen, but just make sure
		die "$0: RT-ERROR: Median score already exists for $name / $scoring, $fn : $ln\n"
			if defined $g_scores{$name}{$scoring};

		# TODO still need to understand why MultiBench needs this according to org. author
		next if $name eq "empty-wld";

		if ($ips =~ /inf/i) {
			printf "$0: WARNING: Score for $name ($scoring) is invalid\n";
		} else {
			$g_scores{$name}{$scoring}{'ips'} = $ips;
			# This should always exist, and at least be 0 if no errors,
			# if not, it means the verification run failed.
			my $errors = $most_recent_error{$name};
			die "$0: RT-ERROR: Missing validation run for $name, $fn : $ln\n"
				if not defined $errors;
			$g_scores{$name}{$scoring}{'errors'} = $errors;
			undef %most_recent_error; # eliminate any chance of stragglers!
		}
	}
}

sub init {
	die <DATA> if "" eq GetOptions(\%g_opts, 
		'input=s',
		'suite=s',
		'help',
		'debug'
	);


	die <DATA> if defined $g_opts{'help'};

	die "$0: ERROR: Please specify a valid --input log\n"
		unless defined $g_opts{'input'}  && -f $g_opts{'input'};

	my @suites = sort keys %g_suite_groups;
	die "$0: ERROR: Please specify a valide --suite [".join('|', @suites)."]\n"
		if ! defined $g_opts{'suite'} || !defined $g_suite_groups{$g_opts{'suite'}};
}

__DATA__

Usage:

	% cert_mark.pl --input|-i <cert log file> --suite|-s

Description:

	Summarizes the log file created by the certify step found under
	./builds/TARGET/TOOLCHAIN/cert/TIMSTAMP/TARGET.TOOLCHAIN.log

	It first produces a list of all successful runs it found and their
	best/single performance. It then attempts to compute a mark based
	on the suite, and will fail if any of the workloads are missing
	or have failures in them.

