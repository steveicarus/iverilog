`begin_keywords "1364-2005"
// -- test force/release of:
//      a wire assigned to a reg
//      a wire with no assignment
//      a whole bus (assigned to a reg),
//      a single bit of a bus (assigned to a reg)
// -- make sure the force/release is passed into the hierarchy
//
// -- run with
//           iverilog -Wall tt.v
//           vvp a.out
// -- to see debug display statements, use
//           iverilog -Wall -DDISPLAY tt.v
//
module top ();

  reg [31:0]	ii;
  reg		bitfail, bitnafail, busfail, busbitfail;
  reg		a;
  reg [1:0]	b;
  wire		bit = a;
  wire		bitna;
  wire [1:0]	bus = b;
  wire [1:0]	ibus = b;
  wire		subfail, subfailna;

  // call in a lower level module
  subtop U1 (
    .subbit(bit),
    .subbus(bus),
    .subfail(subfail)
    );
  subtop U2 (
    .subbit(bitna),
    .subbus(bus),
    .subfail(subfailna)
    );

  initial begin
    a = 1'b1;
    b = 2'b01;
    #5; b = 2'b10;
    #10; b = 2'b11;
  end

  initial  begin
    #2;
    force bit = 0;
    force bitna = 0;
    force bus = 0;
    //$display("\n ****** force/release to ibus[0] commented; expect bit[0] failure ******* ");
    force ibus[0] = 0;
    #10;
    release bit;
    force bitna = 1;
    release bus;
    #5;
    release ibus[0];
  end

  initial begin
    bitfail = 0; bitnafail = 0; busfail = 0; busbitfail = 0;
    `ifdef DISPLAY
      $display("");
      $display("expecting bit, bus,ibus to be 1 at T=1");
      $display("then changing to 0 at T=2");
      $display("then bit and bus are 0 from T=3 to T=11, while");
      $display("ibus changes to 2 at T=5 and remains 2 through to T=16");
      $display("bit changes to 1 at T=12 and remains 1 from then on.");
      $display("bus changes to 2 at T=12");
      $display("then 2 from T=13 to T=14");
      $display("then changing to 3 at T=15");
      $display("then 3 from T=16 on");
      $display("ibus changes to 3 at T=17 and remains 3 from then on");
      $display("");
    `endif
    for(ii = 0; ii < 20; ii = ii + 1) begin
      // bit
      if((ii == 1) && (bit !== 1)) bitfail = 1;
      if((ii > 2) && (ii < 12) && (bit !== 0)) bitfail = 1;
      if((ii > 12) && (bit !== 1)) bitfail = 1;
      // bitna
      if((ii == 1) && (bitna !== 1'bz)) bitnafail = 1;
      if((ii > 2) && (ii < 12) && (bitna !== 0)) bitnafail = 1;
      if((ii > 12) && (bitna !== 1)) bitnafail = 1;
      // bus
      if((ii == 1) && (bus !== 1)) busfail = 1;
      if((ii > 2) && (ii < 12) && (bus !== 0)) busfail = 1;
      if((ii > 12) && (ii < 14) && (bus !== 2'b10)) busfail = 1;
      if((ii > 15) && (bus !== 2'b11)) busfail = 1;
      // ibus
      if((ii == 1) && (ibus !== 2'b01)) busbitfail = 1;
      if((ii > 2) && (ii < 4) && (ibus !== 2'b00)) busbitfail = 1;
      if((ii > 5) && (ii < 17) && (ibus !== 2'b10)) busbitfail = 1;
      if((ii > 17) && (ibus !== 2'b11)) busbitfail = 1;
      `ifdef DISPLAY
	$display("time: %0t, a: %b, bit: %b, bitna %b,  b: %b, bus: %b, ibus: %b",$time,a,bit,bitna,b,bus,ibus);
      `endif
      #1;
    end
    if(bitfail || bitnafail || busfail || busbitfail || subfail || subfailna) begin
      $display("\n\t--------- force test failed ---------");
      if(bitfail) $display("force to single wire assigned to a reg failed");
      if(bitnafail) $display("force to single unassigned wire failed");
      if(busfail) $display("force to whole of 2-bit bus failed");
      if(busbitfail) $display("force to bit[0] of 2-bit bus failed");
      if(!bitfail && !bitnafail && !busfail && !busbitfail) begin
	if(subfail) $display("force did not affect U1 hierarchy");
	if(subfailna) $display("force did not affect U2 hierarchy");
      end
      $display("\n");

    end else $display("PASSED");
  end

endmodule

module subtop(
  subbit,
  subbus,
  subfail
  );

  input       subbit;
  input [1:0] subbus;
  output      subfail;

  reg subfail;
  reg [31:0] ii;

  initial begin
    subfail = 0;
    for(ii = 0; ii < 20; ii = ii + 1) begin
      // subbit
      if((ii == 1) && (subbit !== 1) && (subbit !== 1'bz)) subfail = 1;
      if((ii > 2) && (ii < 12) && (subbit !== 0)) subfail = 1;
      if((ii > 12) && (subbit !== 1)) subfail = 1;
      // subbus
      if((ii == 1) && (subbus !== 1)) subfail = 1;
      if((ii > 2) && (ii < 12) && (subbus !== 0)) subfail = 1;
      if((ii > 12) && (ii < 14) && (subbus !== 2'b10)) subfail = 1;
      if((ii > 15) && (subbus !== 2'b11)) subfail = 1;
      `ifdef DISPLAY
	$display("\t\t\t\t\ttime: %0t, subbit: %b, subbus: %b",$time,subbit,subbus);
      `endif
      #1;
    end
  end
endmodule
`end_keywords
