
Xilinx Hint
===========

For those of you who wish to use Icarus Verilog, in combination with
the Xilinx back end (Foundation or Alliance), it can be done.  I have
run some admittedly simple (2300 equivalent gates) designs through this
setup, targeting a Spartan XCS10.

Verilog:
--------

Older versions of Icarus Verilog (like 19990814) couldn't synthesize
logic buried in procedural (flip-flop) assignment.  Newer versions
(like 20000120) don't have this limitation.

Procedural assignments have to be given one at a time, to be
"found" by xnfsyn.  Say

::

   always @ (posedge Clk) Y = newY;
   always @ (posedge Clk) Z = newZ;

rather than

::

   always @ (posedge Clk) begin
       Y = newY;
       Z = newZ;
   end

Steve's xnf.txt covers most buffer and pin constructs, but I had reason
to use a global clock net not connected to an input pin.  The standard
Verilog for a buffer, combined with a declaration to turn that into a
BUFG, is::

   buf BUFG( your_output_here, your_input_here );
   $attribute(BUFG,"XNF-LCA","BUFG:O,I")

I use post-processing on my .xnf files to add "FAST" attributes to
output pins.

Running ivl:
------------

The -F switches are important.  The following order seems to robustly
generate valid XNF files, and is used by "verilog -X"::

  -Fsynth -Fnodangle -Fxnfio

Generating .pcf files:
----------------------

The ngdbuild step seems to lose pin placement information that ivl
puts in the XNF file.  Use xnf2pcf to extract this information to
a .pcf file, which the Xilinx place-and-route software _will_ pay
attention to.  Steve says he now makes that information available
in an NCF file, with -fncf=<path>, but I haven't tested that.

Running the Xilinx back end:

You can presumably use the GUI, but that doesn't fit in Makefiles :-).
Here is the command sequence in pseudo-shell-script::

  ngdbuild -p $part $1.xnf $1.ngd
  map -p $part -o map.ncd $1.ngd
  xnf2pcf <$1.xnf >$1.pcf    # see above
  par -w -ol 2 -d 0 map.ncd $1.ncd $1.pcf
  bitgen_flags = -g ConfigRate:SLOW -g TdoPin:PULLNONE -g DonePin:PULLUP \
                 -g CRC:enable -g StartUpClk:CCLK -g SyncToDone:no \
                 -g DoneActive:C1 -g OutputsActive:C3 -g GSRInactive:C4 \
                 -g ReadClk:CCLK -g ReadCapture:enable -g ReadAbort:disable
  bitgen $1.ncd -l -w $bitgen_flags

The Xilinx software has diarrhea of the temp files (14, not including
.xnf, .pcf, .ngd, .ncd, and .bit), so this sequence is best done in a
dedicated directory.  Note in particular that map.ncd is a generic name.

I had reason to run this remotely (and transparently within a Makefile)
via ssh.  I use the gmake rule::

    %.bit : %.xnf
        ssh -x -a -o 'BatchMode yes' ${ALLIANCE_HOST} \
               remote_alliance ${REMOTE_DIR} $(basename $@) 2>&1 < $<
    scp ${ALLIANCE_HOST}:${REMOTE_DIR}/$@ .

and the remote_alliance script (on ${ALLIANCE_HOST})::

    /bin/csh
    cd $1
    cat >! $2.xnf
    xnf2pcf <$2.xnf >! $2.pcf
    ./backend $2

There is now a "Xilinx on Linux HOWTO" at http://www.polybus.com/xilinx_on_linux.html
I haven't tried this yet, it looks interesting.

Downloading:
------------

I use the XESS (http://www.xess.com/) XSP-10 development board, which
uses the PC parallel (printer) port for downloading and interaction
with the host.  They made an old version of their download program
public domain, posted it at http://www.xess.com/FPGA/xstools.zip ,
and now there is a Linux port at ftp://ftp.microux.com/pub/pilotscope/xstools.tar.gz .

The above hints are based on my experience with Foundation 1.5 on NT
(gack) and Alliance 2.1i on Solaris.  Your mileage may vary.  Good luck!

 - Larry Doolittle   <LRDoolittle@lbl.gov>   August 19, 1999
                                    updated February 1, 2000
