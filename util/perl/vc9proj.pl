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
	
	my $sln=0;
	
	my $guidnum = Data::GUID->new;
	my $guid = $guidnum->as_string; # or "$guid"
	my %iguid;
	my $file_template="\t\t\t<File\n\t\t\t\tRelativePath=\"..\\..\\FPATH\"\n\t\t\t\t>\n\t\t\t</File>\n";

	my @hfiles;
	my @cfiles;
	my $depguid="";
	my $projdef="";
	my $configbase=<<END;
		{guid}.Debug|Win32.ActiveCfg = Debug|Win32
		{guid}.Debug|Win32.Build.0 = Debug|Win32
		{guid}.demo-release|Win32.ActiveCfg = Debug|Win32
		{guid}.demo-release|Win32.Build.0 = Debug|Win32
		{guid}.Release|Win32.ActiveCfg = Release|Win32
		{guid}.Release|Win32.Build.0 = Release|Win32
END
	my $wconfigs=$configbase;
	my $iconfigs="";
	my $path=$ARGV[0];
	my $projname=basename($path);
	my $outext="vcproj";
	print "$projname=basename($path)\n";
	read_bench_items($path,\@hfiles,\@cfiles);

	my $proj_template;
	if (defined($ARGV[1])) {
		#multiple args, assume workload project create, executed from workloads folder
		#first arg could be proj or sln
		if ($ARGV[1] eq "proj") {
			$proj_template="$Bin/template-wld9.vcproj";
			$user_defs=1;
			if (defined($ARGV[2])) {
				$projname = $ARGV[2];
			}
		}
		if ($ARGV[1] eq "sln") {
			$proj_template="$Bin/template9.sln";
			$sln=1;
		}
	} else {
		#only one arg, assume work item project create, executed from benchmarks folder
		$proj_template="$Bin/template9.vcproj";
	}
	
	if ($sln) {
		my @items=@ARGV;
		shift @items;
		shift @items;
		if (scalar(@items) < 1) {
			die "No items for solution $projname!\n";
		}
		print "Creating solution for ".scalar(@items)." items\n";
		$outext="sln";
		# Get guids and setup for each work item
		my $head="Project\(\"\{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942\}\"\) =";
		foreach my $item (@items) {
			$guidnum = Data::GUID->new;
			my $bpath="..\\..\\..\\benchmarks\\projects\\vs9";
			$iguid{$item} = getguid("../benchmarks/projects/vs9/$item\.vcproj"); 
			my $g="\{$iguid{$item}\}";
			$depguid .= "\t\t$g = $g\n";
			$projdef .= "$head \"$item\", \"$bpath\\$item\.vcproj\", \"$g\"\nEndProject\n";
			my $iconfig=$configbase;
			$iconfig=~s/guid/$iguid{$item}/g;
			$iconfigs.=$iconfig;
			
		}
		#get guid for the workload
		$guid=getguid("projects/vs9/$projname.vcproj");
		$wconfigs=~s/guid/$guid/g;
		#chomp all strings for nice sln file
		chomp $iconfigs;
		chomp $wconfigs;
		chomp $projdef;
		chomp $depguid;
	}
	
	open (IN,"$proj_template") or die "No in\n";
	open (OUT,">projects/vs9/$projname.$outext") or die "could not open projects/vs9/$projname.vcproj\n";

	while (<IN>) {
		$line=$_;
		while ($line =~ /IN\.(\w+[\.]*\w+)/) {
			my $typ=$1;
			$line =~ s/IN\.//;
			switch ($typ) {
				case "name" { $line =~ s/name/$projname/; }
				case "w.name" { $line =~ s/w.name/$projname/; }
				case "guid" { $line =~ s/guid/{$guid}/; }
				case "w.guid" { $line =~ s/w.guid/$guid/; }
				case "i.depguid" { $line =~ s/i.depguid/$depguid/; }
				case "i.projdef" { $line =~ s/i.projdef/$projdef/; }
				case "i.configs" { $line =~ s/i.configs/$iconfigs/; }
				case "w.configs" { $line =~ s/w.configs/$wconfigs/; }
				case "cfiles" { 
					$line="";
					foreach my $f (@cfiles) {
						my $fnt=$file_template;
						$f =~ s/\/\//\\/g;
						$f =~ s/\//\\/g;
						print "$f\n";
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
	if ($user_defs) {
		my $in="$Bin/template9.vcproj.ETQ.shay.user";
		my $out="projects/vs9/$projname.vcproj.ETQ.shay.user";
		system("cp $in $out");
	}
	
	exit 0;

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

sub getguid {
	my $fn=shift;
	my $guidline = `grep ProjectGUID $fn`;
	$guidline =~ s/[^{]*{([^}]*).*/\1/;
	chomp $guidline;
	return $guidline;
}
