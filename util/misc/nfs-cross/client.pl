use strict;
use IO::Socket::INET;

my $target_ipaddress = "xx.xx.xx.xx" ;
my $option= @ARGV[0];


my $sock_port= 9000;



if ($option eq "-load") 
{
#can't do much here, because can't open connection twice, so do it while running
=cut;
	my $file = @ARGV[1]; 
	shift (@ARGV);
	my $exec = shift (@ARGV);
	$exec =~ s/.*\/(.*\.exe)/$1/;
	$exec = "./".$exec;
	print "$exec \n";     	
#	my $test_flag= send($sock, $exec ,0);
    print "\nLoad Request sent for $file \n" ;#if (defined $test_flag);
    my $ack=undef;
 #   $test_flag=recv($sock, $ack , 32, 0);
    print "Target replies: $ack  \n" ;#if (defined $test_flag );
=cut;

}
elsif($option eq "-run")
{
	
	my $host= $target_ipaddress;
	my $sock = IO::Socket::INET->new(PeerAddr => "$host",
                         PeerPort => "$sock_port",                 
                         Proto    => 'tcp') || die " socket not created";



	shift (@ARGV); #load/run
	my $exec = shift (@ARGV);
	$exec =~ s/.*\/(.*\.exe)/$1/;
	$exec =~ s/.*\/(.*\.exe)/$1/;
	my $cmd_options = join (" ",@ARGV );
	$exec = "./".$exec;
	$cmd_options = $exec." ".$cmd_options."\0" ; #on target we will be using system command
	

	my $test_flag= send($sock, $cmd_options , 0 );
    print "\nRun Request sent with cmd line: $cmd_options \n" if (defined $test_flag) ;
		
    my $ack=undef;
    $test_flag=recv($sock, $ack , 32, 0);
    print "Target replies for run request: $ack  \n" if (defined $test_flag );
	close $sock;
	
	#now get the output from temporary run log created by target and dump it in actual runlog 
	print "-------------- Actual output of the prgram follows ----------\n";
	
	my $temp_runlog= $exec;
	$temp_runlog =~s/.\/(.*).exe/..\/logs\/$1.run1.log/;
	
	open (FILE1, "$temp_runlog") or die("could not open $temp_runlog\n");		
	
	while (<FILE1>)
	{
	
		print   "$_";
	}
	
	close (FILE1);
	unlink ($temp_runlog);
    
}else
{
	print "Illegal option for client script \n";
}
