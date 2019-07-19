#!/usr/bin/perl

# File: util/perl/mith_auto.pl
#	Small utility to aid in converting new kernels to MITH infrastructure.

use FindBin qw($Bin);	#To get our base script directory. All finds relative to this.
use File::Find::Match qw( :constants ); #Recursively search benchmark dirs for xml defs
use File::Find::Match::Util qw( filename );
use File::Basename;
use Data::Dumper; #debug

my %all_defs;
my $debug=1;

my $def_file="$Bin/my_defs.transform";
read_defs($def_file,\%all_defs);
convert_defs(\%all_defs,".");
if ($debug) {
	print "Usage:\n" ;
	print Dumper($all_defs{USE}) ;
	print "Not Using:\n";
	foreach my $key (keys %all_defs) {
		next if ($key eq "USE");
		my $val=$all_defs{$key};
		if (!defined($all_defs{USE}->{$key}) &&
			!defined($all_defs{USE}->{$val}) ) {
			print "\t$key/$val\n";
		}
	}
}
1;

sub read_defs {
	my ($file,$all_defs_ref) = @_;
	my $fn='[\w_]+'; #function name pattern
	print "Process $file\n" if $debug;
	open (F,"<$file") or die "Could not process $file\n";
	while (<F>) {
		chomp;
		$line = $_;
		if ($line =~ /convert ($fn) (th_$fn)/) {
			$all_defs_ref->{$1}=$2;
		}
	}
	print Dumper($all_defs_ref) if $debug;
	close (F);
}

sub convert_defs {
	my ($all_defs_ref,$dirname)=@_;
    my $finder = new File::Find::Match(
        filename('.svn') => sub { IGNORE },
        qr/\.[ch]$/ => sub {
            convert_defs_file($_[0],$all_defs_ref);
            MATCH;
        },
        default => sub {
           # print "Default handler $_[0].\n";
            MATCH;
        },
	);
	$finder->find($dirname);
}

sub convert_defs_file {
	my ($file,$all_defs_ref) = @_;
	my $tmp="tmpfile.txt";
	my $has_non_th=0;
	print "Process $file:" if $debug;
	open (F,"<$file") or die "Could not process $file\n";
	open (OUT,">$tmp") or die "Could not process $tmp\n";
	my $all_found="\t";
	while (<F>) {
		$line =$_;
		#skip irrelevant lines
		if ((/free software/) || (/^\s*\/\*/)) {
			print OUT $line;
			next;
		}
		#find actuals
		foreach my $key (keys %$all_defs_ref) {
			my $val=$all_defs_ref->{$key};
			if ($line =~ /[=\s\(\)\-\+\/\*\!]$key(\s*[\(\)\,\s;])/) {
				my $fpat=quotemeta $1;
				$line =~ s/$key($fpat)/$val\1/g;
				$has_non_th++;
				$all_defs_ref->{USE}->{$key}++;
				$all_found.="$key,";
			}
			if ($line =~ /$val/) {
				$all_defs_ref->{USE}->{$val}++;
			}
		}
		print OUT $line;
	}
	close (F);
	close (OUT);
	if ($has_non_th > 0) {
		print "found $has_non_th calls to standard functions with th equivalents\n" if $debug;
		print "$all_found\n" if ($debug && ($all_found =~ /\w/));
		rename ($file,$file.".non_th_defs") or die "Could not rename $file\n";
		rename ($tmp,$file) or die "Could not rename $tmp\n";
	} else {
		print "No standard calls or no th equivalents\n" if $debug;
		unlink ($tmp);
	}
}
