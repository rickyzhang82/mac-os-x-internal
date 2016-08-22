#! /usr/bin/perl -w
   
my $FOUR_KB  = "4" x 4096;
my $BINDIR   = "/usr/local/bin";
my $HFSDEBUG = "$BINDIR/hfsdebug";
my $HFS_CHANGE_NEXT_ALLOCATION = "$BINDIR/hfs_change_next_allocation";
   
sub
usage()
{
    die "usage: $0 <volume>\n\twhere <volume> must not be the root volume\n";
}
   
(-x $HFSDEBUG && -x $HFS_CHANGE_NEXT_ALLOCATION) or die "$0: missing tools\n";
($#ARGV == 0) or usage();
my $volume = $ARGV[0];
my @sb = stat($volume);
((-d $volume) && @sb && ($sb[0] != (stat("/"))[0])) or usage();
my $file = "$volume/fragmented.$$";
(! -e $file) or die "$0: file $file already exists\n";
   
`echo -n $FOUR_KB > "$file"`; # create a file
(-e "$file") or die "$0: failed to create file ($file)\n";
   
WHILE_LOOP: while (1) {
   
    my @out = `$HFSDEBUG "$file" | grep -B 1 'allocation blocks'`;
   
    $out[0] =~ /^\s+([^\s]+)\s+([^\s]+)..*$/;
    my $lastStartBlock = $1; # starting block of the file's last extent
    my $lastBlockCount = $2; # number of blocks in the last extent
   
    $out[1] =~ /[\s*\d+] allocation blocks in (\d+) extents total.*/;
    my $nExtents = $1;       # number of extents the file currently has
    if ($nExtents >= 8) {    # do we already have 8 or more extents?
        print "\ncreated $file with $nExtents extents\n";
        last WHILE_LOOP;
    }
   
    # set volume's next allocation pointer to the block right after our file
    my $conflict = sprintf("0x%x", hex($lastStartBlock) + hex($lastBlockCount));
    `$HFS_CHANGE_NEXT_ALLOCATION $volume $conflict`;
   
    print "start=$lastStartBlock count=$lastBlockCount extents=$nExtents ".
          "conflict=$conflict\n";
   
    `echo hello > "$volume/dummy.txt"`; # create dummy file to consume space
    `echo -n $FOUR_KB >> "$file"`;      # extend our file to cause discontiguity
    `rm "$volume/dummy.txt"`;           # remove the dummy file
} # WHILE_LOOP
   
exit(0);
