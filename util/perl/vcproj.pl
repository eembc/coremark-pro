#!/usr/bin/perl

# File : util/perl/vcproj.pl
#	Small utility to create VS8 project files for new workloads or work items.

	use Data::GUID;
	use Data::Dumper; #debug
	use FindBin qw($Bin);	#To get our base script directory. All finds relative to this.
	use File::Find::Match qw( :constants ); #Recursively search benchmark dirs for xml defs
	use File::Find::Match::Util qw( filename );
	use File::Basename;
    use Switch;
	
	my $guidnum = Data::GUID->new;
	my $guid = $guidnum->as_string; # or "$guid"
	my $file_template="\t\t\t<File\n\t\t\t\tRelativePath=\"..\\..\\FPATH\"\n\t\t\t\t>\n\t\t\t</File>\n";

	my @hfiles;
	my @cfiles;
	my $path=$ARGV[0];
	my $projname=basename($path);
	print "$projname=basename($path)\n";
	read_bench_items($path,\@hfiles,\@cfiles);

	my $proj_template;
	if (defined($ARGV[1])) {
		#multiple args, assume workload project create, executed from workloads folder
		#first arg could be proj or sln
		if ($ARGV[1] eq "proj") {
			$proj_template="$Bin/template-wld.vcproj";
		}
	} else {
		#only one arg, assume work item project create, executed from benchmarks folder
		$proj_template="$Bin/template.vcproj";
	}
	
	open (IN,"$proj_template") or die "No in\n";
	open (OUT,">projects/vs8/$projname.vcproj") or die "could not open projects/vs8/$projname.vcproj\n";
	while (<IN>) {
		$line=$_;
		if (/IN\.(\w+)/) {
			my $typ=$1;
			$line =~ s/IN\.//;
			switch ($typ) {
				case "name" { $line =~ s/name/$projname/; }
				case "guid" { $line =~ s/guid/{$guid}/; }
				case "cfiles" { 
					$line="";
					foreach my $f (@cfiles) {
						my $fnt=$file_template;
						$f =~ s/\/\//\\/g;
						$f =~ s/\//\\/g;
						$fnt =~ s/FPATH/$f/;
						$line .= $fnt;
					}
				}
				case "hfiles" { 
					$line="";
					foreach my $f (@hfiles) {
						my $fnt=$file_template;
						$f =~ s/\/\//\\/g;
						$f =~ s/\//\\/g;
						$fnt =~ s/FPATH/$f/;
						$line .= $fnt;
					}
				}
			}			
		}
		print OUT $line;
	}
	close IN;
	close OUT;
	
	exit 1;

sub read_bench_items {
	my ($path,$hfiles,$cfiles)=@_;
    my $finder = new File::Find::Match(
        filename('.svn') => sub { IGNORE },
        qr/\.c$/ => sub {
            push @$cfiles,$_[0];
            MATCH;
        },
        qr/\.h$/ => sub {
            push @$hfiles,$_[0];
            MATCH;
        },
        default => sub {
           # print "Default handler $_[0].\n";
            MATCH;
        },
	);
	$finder->find($path);

}

