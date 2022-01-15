//
// Copyright (c) 1999 Thomas Coonan (tcoonan@mindspring.com)
//
//    This source code is free software; you can redistribute it
//    and/or modify it in source code form under the terms of the GNU
//    General Public License as published by the Free Software
//    Foundation; either version 2 of the License, or (at your option)
//    any later version.
//
//    This program is distributed in the hope that it will be useful,
//    but WITHOUT ANY WARRANTY; without even the implied warranty of
//    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
//    GNU General Public License for more details.
//
//    You should have received a copy of the GNU General Public License
//    along with this program; if not, write to the Free Software
//    Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA
//
//
// SYNTHETIC PIC 2.0                                          4/23/98
//
//    This is a synthesizable Microchip 16C57 compatible
//    microcontroller.  This core is not intended as a high fidelity model of
//    the PIC, but simply a way to offer a simple processor core to people
//    familiar with the PIC who also have PIC tools.
//
//    pictest.v  -   top-level testbench (NOT SYNTHESIZABLE)
//    piccpu.v   -   top-level synthesizable module
//    picregs.v  -   register file instantiated under piccpu
//    picalu.v   -   ALU instantiated under piccpu
//    picidec.v  -   Instruction Decoder instantiated under piccpu
//    hex2rom.c  -   C program used to translate MPLAB's INTEL HEX output
//                   into the Verilog $readmemh compatible file
//    test*.asm  -   (note the wildcard..) Several test programs used
//                   to help debug the verilog.  I used MPLAB and the simulator
//                   to develop these programs and get the expected results.
//                   Then, I ran them on Verilog-XL where they appeared to
//                   match.
//
//    Copyright, Tom Coonan, '97.
//    Use freely, but not for resale as is.  You may use this in your
//    own projects as desired.  Just don't try to sell it as is!
//
//
module picalu (
   op,
   a,
   b,
   y,
   cin,
   cout,
   zout
);

input  [3:0]	op;	// ALU Operation
input  [7:0]	a;	// 8-bit Input a
input  [7:0]	b;	// 8-bit Input b
output [7:0]	y;	// 8-bit Output
input		cin;
output		cout;
output		zout;

// Reg declarations for outputs
reg		cout;
reg		zout;
reg [7:0]	y;

// Internal declarations
reg		addercout; // Carry out straight from the adder itself.

parameter  ALUOP_ADD        = 4'b0000;
parameter  ALUOP_SUB        = 4'b1000;
parameter  ALUOP_AND        = 4'b0001;
parameter  ALUOP_OR         = 4'b0010;
parameter  ALUOP_XOR        = 4'b0011;
parameter  ALUOP_COM        = 4'b0100;
parameter  ALUOP_ROR        = 4'b0101;
parameter  ALUOP_ROL        = 4'b0110;
parameter  ALUOP_SWAP       = 4'b0111;


always @(a or b or cin or op) begin
   case (op) // synopsys full_case parallel_case
      ALUOP_ADD:  {addercout,  y}  <= a + b;
      ALUOP_SUB:  {addercout,  y}  <= a - b; // Carry out is really "borrow"
      ALUOP_AND:  {addercout,  y}  <= {1'b0, a & b};
      ALUOP_OR:   {addercout,  y}  <= {1'b0, a | b};
      ALUOP_XOR:  {addercout,  y}  <= {1'b0, a ^ b};
      ALUOP_COM:  {addercout,  y}  <= {1'b0, ~a};
      ALUOP_ROR:  {addercout,  y}  <= {a[0], cin, a[7:1]};
      ALUOP_ROL:  {addercout,  y}  <= {a[7], a[6:0], cin};
      ALUOP_SWAP: {addercout,  y}  <= {1'b0, a[3:0], a[7:4]};
      default:    {addercout,  y}  <= {1'b0, 8'h00};
   endcase
end

always @(y)
   zout <= (y == 8'h00);

always @(addercout or op)
   if (op == ALUOP_SUB) cout <= ~addercout; // Invert adder's carry to get borrow
   else                 cout <=  addercout;

endmodule
//
// SYNTHETIC PIC 2.0                                          4/23/98
//
//    This is a synthesizable Microchip 16C57 compatible
//    microcontroller.  This core is not intended as a high fidelity model of
//    the PIC, but simply a way to offer a simple processor core to people
//    familiar with the PIC who also have PIC tools.
//
//    pictest.v  -   top-level testbench (NOT SYNTHESIZABLE)
//    piccpu.v   -   top-level synthesizable module
//    picregs.v  -   register file instantiated under piccpu
//    picalu.v   -   ALU instantiated under piccpu
//    picidec.v  -   Instruction Decoder instantiated under piccpu
//    hex2rom.c  -   C program used to translate MPLAB's INTEL HEX output
//                   into the Verilog $readmemh compatible file
//    test*.asm  -   (note the wildcard..) Several test programs used
//                   to help debug the verilog.  I used MPLAB and the simulator
//                   to develop these programs and get the expected results.
//                   Then, I ran them on Verilog-XL where they appeared to
//                   match.
//
//    Copyright, Tom Coonan, '97.
//    Use freely, but not for resale as is.  You may use this in your
//    own projects as desired.  Just don't try to sell it as is!
//
//
module piccpu (
   clk,
   reset,
   paddr,
   pdata,
   portain,
   portbout,
   portcout,

   debugw,
   debugpc,
   debuginst,
   debugstatus
);

input		clk;
input		reset;
output [8:0]	paddr;
input  [11:0]	pdata;
input  [7:0]	portain;
output [7:0]	portbout;
output [7:0]	portcout;

output [7:0]	debugw;
output [8:0]	debugpc;
output [11:0]	debuginst;
output [7:0]	debugstatus;

// Register declarations for outputs
reg [8:0]	paddr;
reg [7:0]	portbout;
reg [7:0]	portcout;

// This should be set to the ROM location where our restart vector is.
// As set here, we have 512 words of program space.
//
parameter RESET_VECTOR = 9'h1FF;

parameter	INDF_ADDRESS	= 3'h0,
		TMR0_ADDRESS	= 3'h1,
		PCL_ADDRESS	= 3'h2,
		STATUS_ADDRESS	= 3'h3,
		FSR_ADDRESS	= 3'h4,
		PORTA_ADDRESS	= 3'h5,
		PORTB_ADDRESS	= 3'h6,
		PORTC_ADDRESS	= 3'h7;

// Experimental custom peripheral, "Lil Adder (a 4-bit adder)" is at this address.
//
parameter	EXPADDRESS_LILADDER = 7'h7F;

// *********  Special internal registers

// Instruction Register
reg  [11:0]	inst;

// Program Counter
reg  [8:0]	pc;
reg  [8:0]	pcplus1; // Output of the pc incrementer.

// Stack Registers and Stack "levels" register.
reg [ 1:0]	stacklevel;
reg [ 8:0]	stack1;
reg [ 8:0]	stack2;

// W Register
reg [ 7:0]	w;

// The STATUS register (#3) is 8 bits wide, however, we only currently use 2 bits
// of it; the C and Z bit.
//
// bit 0  -  C
// bit 2  -  Z
//
reg [ 7:0]	status;

// The FSR register is the pointer register used for Indirect addressing (e.g. using INDF).
reg  [ 7:0]	fsr;

// Timer 0
reg  [ 7:0]	tmr0;
reg  [ 7:0]	prescaler;

// Option Register
reg [7:0]	option;

// Tristate Control registers. We do not neccessarily support bidirectional ports, but
//    will save a place for these registers and the TRIS instruction.  Use for debug.
reg [7:0]	trisa;
reg [7:0]	trisb;
reg [7:0]	trisc;

// I/O Port registers
//
reg [7:0]	porta;	// Input PORT
reg [7:0]	portb;	// Output PORT
reg [7:0]	portc;	// Output PORT

// ********** Instruction Related signals ******

reg		skip;  // When HI force a NOP (all zeros) into inst

reg  [2:0]	pcinsel;

// Derive special sub signals from inst register
wire [ 7:0]	k;
wire [ 4:0]	fsel;
wire [ 8:0]	longk;
wire		d;
wire [ 2:0]	b;

// ********** File Address ************
//
// This is the 7-bit Data Address that includes the lower 5-bit fsel, the
// FSR bits and any indirect addressing.
// Use this bus to address the Register File as well as all special registers, etc.
//
reg [6:0]	fileaddr;

// Address Selects
reg		specialsel;
reg		regfilesel;
reg		expsel;

// ******  Instruction Decoder Outputs **************

// Write enable for the actual ZERO and CARRY bits within the status register.
// Generated by the Instruction Decoder.
//
wire [1:0]	aluasel;
wire [1:0]	alubsel;
wire [3:0]	aluop;

wire		zwe;
wire		cwe;

wire		isoption;
wire		istris;

wire		fwe;	// High if any "register" is being written to at all.
wire		wwe;	// Write Enable for the W register.  Produced by Instruction Decoder.

// *************  Internal Busses, mux connections, etc.  ********************

// Bit decoder bits.
reg [7:0]	bd;	// Final decoder value after any polarity inversion.
reg [7:0]	bdec;	// e.g. "Bit Decoded"

// Data in and out of the and out of the register file
//
reg [7:0]	regfilein;	// Input into Register File, is connected to the dbus.
wire [7:0]	regfileout;	// Path leaving the register file, goes to SBUS Mux
reg		regfilewe;	// Write Enable
reg		regfilere;	// Read Enable

//
// The dbus (Destination Bus) comes out of the ALU.  It is available to all
// writable registers.
//
// The sbus (Source Bus) comes from all readable registers as well as the output
// of the Register File.  It is one of the primary inputs into the ALU muxes.
//
// The ebus (Expansion Bus) comes from any of our custom modules.  They must
// all coordinate to place whoever's data onto ebus.
//
reg  [7:0]	dbus;
reg  [7:0]	sbus;
reg  [7:0]	ebus;


// ALU Signals
//
reg  [7:0]	alua;
reg  [7:0]	alub;
wire [7:0]	aluout;
wire		alucin;
wire		alucout;
wire		aluz;

// ALU A and B mux selects.
//
parameter	ALUASEL_W	= 2'b00,
		ALUASEL_SBUS	= 2'b01,
		ALUASEL_K	= 2'b10,
		ALUASEL_BD	= 2'b11;

parameter	ALUBSEL_W	= 2'b00,
		ALUBSEL_SBUS	= 2'b01,
		ALUBSEL_K	= 2'b10,
		ALUBSEL_1	= 2'b11;

// ALU Operation codes.
//
parameter   ALUOP_ADD  = 4'b0000;
parameter   ALUOP_SUB  = 4'b1000;
parameter   ALUOP_AND  = 4'b0001;
parameter   ALUOP_OR   = 4'b0010;
parameter   ALUOP_XOR  = 4'b0011;
parameter   ALUOP_COM  = 4'b0100;
parameter   ALUOP_ROR  = 4'b0101;
parameter   ALUOP_ROL  = 4'b0110;
parameter   ALUOP_SWAP = 4'b0111;


// Instantiate each of our subcomponents
//
picregs  regs (
   .clk		(clk),
   .reset	(reset),
   .we		(regfilewe),
   .re		(regfilere),
   .bank	(fileaddr[6:5]),
   .location	(fileaddr[4:0]),
   .din		(regfilein),
   .dout	(regfileout)
);

// Instatiate the ALU.
//
picalu   alu  (
   .op         (aluop),
   .a          (alua),
   .b          (alub),
   .y          (aluout),
   .cin        (status[0]),
   .cout       (alucout),
   .zout       (aluz)
);

// Instantiate the Instruction Decoder.  This is really just a lookup table.
// Given the instruction, generate all the signals we need.
//
// For example, each instruction implies a specific ALU operation.  Some of
// these are obvious such as the ADDW uses an ADD alu op.  Less obvious is
// that a mov instruction uses an OR op which lets us do a simple copy.
//
// Data has to funnel through the ALU, which sometimes makes for contrived
// ALU ops.
//
picidec  idec (
   .inst     (inst),
   .aluasel  (aluasel),
   .alubsel  (alubsel),
   .aluop    (aluop),
   .wwe      (wwe),
   .fwe      (fwe),
   .zwe      (zwe),
   .cwe      (cwe),
   .bdpol    (bdpol),
   .option   (isoption),
   .tris     (istris)
);

// *********** Debug ****************
assign	debugw = w;
assign	debugpc = pc;
assign	debuginst = inst;
assign	debugstatus = status;

// *********** REGISTER FILE Addressing ****************
//
// We implement the following:
//    - The 5-bit fsel address is within a "BANK" which is 32 bytes.
//    - The FSR bits 6:5 are the BANK select, so there are 4 BANKS, a
//      total of 128 bytes.  Minus the 8 special registers, that's
//      really 120 bytes.
//    - The INDF register is for indirect addressing.  Referencing INDF
//      uses FSR as the pointer.  Therefore, using INDF/FSR you address
//      7-bits of memory.
// We DO NOT currently implement the PAGE for program (e.g. STATUS register
// bits 6:5)
//
// The fsel address *may* be zero in which case, we are to do indirect
// addressing, using FSR register as the 8-bit pointer.
//
// Otherwise, use the 5-bits of FSEL (from the Instruction itself) and
// 2 bank bits from the FSR register (bits 6:5).
//
always @(fsel or fsr) begin
   if (fsel == INDF_ADDRESS) begin
      // The INDEX register is addressed.  There really is no INDEX register.
      // Use the FSR as an index, e.g. the FSR contents are the fsel.
      //
      fileaddr <= fsr[6:0];
   end
   else begin
      // Use FSEL field and status bank select bits
      //
      fileaddr <= {fsr[6:5], fsel};
   end
end

// Write Enable to Register File.
// Assert this when the general fwe (write enable to *any* register) is true AND Register File
//    address range is specified.
//
always @(regfilesel or fwe)
   regfilewe <= regfilesel & fwe;

// Read Enable (this if nothing else, helps in debug.)
// Assert if Register File address range is specified AND the ALU is actually using some
//    data off the SBUS.
//
always @(regfilesel or aluasel or alubsel)
   regfilere <= regfilesel & ((aluasel == ALUASEL_SBUS) | (alubsel == ALUBSEL_SBUS));

// *********** Address Decodes **************
//
// Generate 3 selects: specialsel, regfilesel and expsel
always @(fileaddr) begin
   casex (fileaddr)
      7'bXX00XXX: // The SPECIAL Registers are lower 8 addresses, in ALL BANKS
         begin
            specialsel	<= 1'b1;
            regfilesel	<= 1'b0;
            expsel	<= 1'b0;
         end
      7'b1111111: // EXPANSION Registers are the top (1) addresses
         begin
            specialsel	<= 1'b0;
            regfilesel	<= 1'b0;
            expsel	<= 1'b1;
         end
      default:  // Anything else must be in the Register File
         begin
            specialsel	<= 1'b0;
            regfilesel	<= 1'b1;
            expsel	<= 1'b0;
         end
   endcase
end

// *********** SBUS **************
// The sbus (Source Bus) is the output of a multiplexor that takes
// inputs from the Register File, and all other special registers
// and input ports.  The Source Bus then, one of the inputs to the ALU


// First MUX selects from all the special regsiters
//
always @(fsel or fsr or tmr0 or pc or status
         or porta or portb or portc or regfileout or ebus
         or specialsel or regfilesel or expsel) begin

   // For our current mix of registers and peripherals, only the first 8 addresses
   // are "special" registers (e.g. not in the Register File).  As more peripheral
   // registers are added, they must be muxed into this MUX as well.
   //
   // We currently prohibit tristates.
   //
   //
   if (specialsel) begin
      // Special register
      case (fsel[2:0]) // synopsys parallel_case full_case
         3'h0:	sbus <= fsr;
         3'h1:	sbus <= tmr0;
         3'h2:	sbus <= pc[7:0];
         3'h3:	sbus <= status;
         3'h4:	sbus <= fsr;
         3'h5:	sbus <= porta; // PORTA is an input-only port
         3'h6:	sbus <= portb; // PORTB is an output-only port
         3'h7:	sbus <= portc; // PORTC is an output-only port
      endcase
   end
   else begin
      //
      // Put whatever address equation is neccessary here.  Remember to remove unnecessary
      // memory elements from Register File (picregs.v).  It'll still work, but they'd be
      // wasted flip-flops.
      //
      if (expsel) begin
         sbus <= ebus;
      end
      else begin
         if (regfilesel) begin
            // Final Priority is Choose the register file
            sbus <= regfileout;
         end
         else begin
            sbus <= 8'h00;
         end
      end
   end
end

// ************** DBUS ******
//  The Destination bus is just the output of the ALU.
//
always @(aluout)
   dbus <= aluout;

always @(dbus)
   regfilein <= dbus;

// Drive the ROM address bus straight from the PC
//
always @(pc)
   paddr = pc;


// Define sub-signals out of inst
//
assign k =     inst[7:0];
assign fsel  = inst[4:0];
assign longk = inst[8:0];
assign d     = inst[5];
assign b     = inst[7:5];

// Bit Decoder.
//
// Simply take the 3-bit b field in the PIC instruction and create the
// expanded 8-bit decoder field, which is used as a mask.
//


always @(b) begin
   case (b)
      3'b000: bdec <= 8'b00000001;
      3'b001: bdec <= 8'b00000010;
      3'b010: bdec <= 8'b00000100;
      3'b011: bdec <= 8'b00001000;
      3'b100: bdec <= 8'b00010000;
      3'b101: bdec <= 8'b00100000;
      3'b110: bdec <= 8'b01000000;
      3'b111: bdec <= 8'b10000000;
   endcase
end

always @(bdec or bdpol)
   bd <= bdec ^ bdpol;

// Instruction regsiter usually get the ROM data as its input, but
// sometimes for branching, the skip signal must cause a NOP.
//
always @(posedge clk) begin
   if (reset) begin
      inst <= 12'h000;
   end
   else begin
      if (skip == 1'b1) begin
         inst <= 12'b000000000000; // FORCE NOP
      end
      else begin
         inst <= pdata;
      end
   end
end

// SKIP signal.
//
// We want to insert the NOP instruction for the following conditions:
//    GOTO,CALL and RETLW instructions (All have inst[11:10] = 2'b10
//    BTFSS instruction when aluz is HI  (
//    BTFSC instruction when aluz is LO
//
always @(inst or aluz) begin
   casex ({inst, aluz})
      13'b10??_????_????_?:    // A GOTO, CALL or RETLW instructions
         skip <= 1'b1;

      13'b0110_????_????_1:    // BTFSC instruction and aluz == 1
         skip <= 1'b1;

      13'b0111_????_????_0:    // BTFSS instruction and aluz == 0
         skip <= 1'b1;

      13'b0010_11??_????_1:    // DECFSZ instruction and aluz == 1
         skip <= 1'b1;

      13'b0011_11??_????_1:    // INCFSZ instruction and aluz == 1
         skip <= 1'b1;

      default:
         skip <= 1'b0;
   endcase
end

// 4:1 Data MUX into alua
//
//
always @(aluasel or w or sbus or k or bd) begin
   case (aluasel)
      2'b00: alua <= w;
      2'b01: alua <= sbus;
      2'b10: alua <= k;
      2'b11: alua <= bd;
   endcase
end

// 4:1 Data MUX into alub
//
//
always @(alubsel or w or sbus or k) begin
   case (alubsel)
      2'b00: alub <= w;
      2'b01: alub <= sbus;
      2'b10: alub <= k;
      2'b11: alub <= 8'b00000001;
   endcase
end

// W Register
always @(posedge clk) begin
   if (reset) begin
      w <= 8'h00;
   end
   else begin
      if (wwe) begin
         w <= dbus;
      end
   end
end

// ************ Writes to various Special Registers (fsel between 0 and 7)

// INDF Register (Register #0)
//
//    Not a real register.  This is the Indirect Addressing mode register.
//    See the regfileaddr logic.

// TMR0 Register (Register #1)
//
//    Timer0 is currently only a free-running timer clocked by the main system clock.
//
always @(posedge clk) begin
   if (reset) begin
      tmr0 <= 8'h00;
   end
   else begin
      // See if the status register is actually being written to
      if (fwe & specialsel & (fsel == TMR0_ADDRESS)) begin
         // Yes, so just update the register from the dbus
         tmr0 <= dbus;
      end
      else begin
         // Mask off the prescaler value based on desired divide ratio.
         // Whenever this is zero, then that is our divided pulse.  Increment
         // the final timer value when it's zero.
         //
         case (option[2:0]) // synopsys parallel_case full_case
            3'b000: if (~|(prescaler & 8'b00000001)) tmr0 <= tmr0 + 1;
            3'b001: if (~|(prescaler & 8'b00000011)) tmr0 <= tmr0 + 1;
            3'b010: if (~|(prescaler & 8'b00000111)) tmr0 <= tmr0 + 1;
            3'b011: if (~|(prescaler & 8'b00001111)) tmr0 <= tmr0 + 1;
            3'b100: if (~|(prescaler & 8'b00011111)) tmr0 <= tmr0 + 1;
            3'b101: if (~|(prescaler & 8'b00111111)) tmr0 <= tmr0 + 1;
            3'b110: if (~|(prescaler & 8'b01111111)) tmr0 <= tmr0 + 1;
            3'b111: if (~|(prescaler & 8'b11111111)) tmr0 <= tmr0 + 1;
         endcase
      end
   end
end

// The prescaler is always counting from 00 to FF
always @(posedge clk) begin
   if (reset) begin
      prescaler <= 8'h00;
   end
   else begin
      // See if the status register is actually being written to
      prescaler <= prescaler + 1;
   end
end


// PCL Register (Register #2)
//
//    PC Lower 8 bits.  This is handled in the PC section below...


// STATUS Register (Register #3)
//
always @(posedge clk) begin
   if (reset) begin
      status <= 8'h00;
   end
   else begin
      // See if the status register is actually being written to
      if (fwe & specialsel & (fsel == STATUS_ADDRESS)) begin
         // Yes, so just update the register from the dbus
         status <= dbus;
      end
      else begin
         // Update status register on a bit-by-bit basis.
         //
         // For the carry and zero flags, each instruction has its own rule as
         // to whether to update this flag or not.  The instruction decoder is
         // providing us with an enable for C and Z.  Use this to decide whether
         // to retain the existing value, or update with the new alu status output.
         //
         status <= {
            status[7],			// BIT 7: Undefined.. (maybe use for debugging)
            status[6],			// BIT 6: Program Page, HI bit
            status[5],			// BIT 5: Program Page, LO bit
            status[4],			// BIT 4: Time Out bit (not implemented at this time)
            status[3],			// BIT 3: Power Down bit (not implemented at this time)
            (zwe) ? aluz : status[2],	// BIT 2: Z
            status[1],			// BIT 1: DC
            (cwe) ? alucout : status[0]	// BIT 0: C
         };
      end
   end
end

// FSR Register  (Register #4)
//
always @(posedge clk) begin
   if (reset) begin
      fsr <= 8'h00;
   end
   else begin
      // See if the status register is actually being written to
      if (fwe & specialsel & (fsel == FSR_ADDRESS)) begin
         fsr <= dbus;
      end
   end
end

// OPTION Register
//
// The special OPTION instruction should move W into the OPTION register.
always @(posedge clk) begin
   if (reset) begin
      option <= 8'h00;
   end
   else begin
      if (isoption)
         option <= dbus;
   end
end

// PORTA Input Port   (Register #5)
//
// Register anything on the module's porta input on every single clock.
//
always @(posedge clk) begin
   if (reset) begin
      porta <= 8'h00;
   end
   else begin
      porta <= portain;
   end
end

// PORTB Output Port  (Register #6)
always @(posedge clk) begin
   if (reset) begin
      portb <= 8'h00;
   end
   else begin
      if (fwe & specialsel & (fsel == PORTB_ADDRESS) & ~istris) begin
         portb <= dbus;
      end
   end
end

// Connect the output ports to the register output.
always @(portb)
   portbout <= portb;

// PORTC Output Port  (Register #7)
always @(posedge clk) begin
   if (reset) begin
      portc <= 8'h00;
   end
   else begin
      if (fwe & specialsel & (fsel == PORTC_ADDRESS) & ~istris) begin
         portc <= dbus;
      end
   end
end

// Connect the output ports to the register output.
always @(portc)
   portcout <= portc;

// TRIS Registers
always @(posedge clk) begin
   if (reset) begin
      trisa <= 8'hff; // Default is to tristate them
   end
   else begin
      if (fwe & specialsel & (fsel == PORTA_ADDRESS) & istris) begin
         trisa <= dbus;
      end
   end
end

always @(posedge clk) begin
   if (reset) begin
      trisb <= 8'hff; // Default is to tristate them
   end
   else begin
      if (fwe & specialsel & (fsel == PORTB_ADDRESS) & istris) begin
         trisb <= dbus;
      end
   end
end

always @(posedge clk) begin
   if (reset) begin
      trisc <= 8'hff; // Default is to tristate them
   end
   else begin
      if (fwe & specialsel & (fsel == PORTC_ADDRESS) & istris) begin
         trisc <= dbus;
      end
   end
end


// ********** PC AND STACK *************************
//
// There are 4 ways to change the PC.  They are:
//    GOTO  101k_kkkk_kkkk
//    CALL  1001_kkkk_kkkk
//    RETLW 1000_kkkk_kkkk
//    MOVF  0010_0010_0010  (e.g. a write to reg #2)
//
// Remember that the skip instructions work by inserting
// a NOP instruction or not into program stream and don't
// change the PC.
//

// We need pc + 1 in several places, so lets define this incrementor and
// its output signal it in one place so that we never get redundant adders.
//
always @(pc)
   pcplus1 <= pc + 1;

parameter	PC_SELECT_PCPLUS1	= 3'b000,
		PC_SELECT_K             = 3'b001,
		PC_SELECT_STACK1        = 3'b010,
		PC_SELECT_STACK2        = 3'b011,
		PC_SELECT_DBUS          = 3'b100,
		PC_SELECT_RESET_VECTOR  = 3'b101;

// 8:1 Data MUX into PC
always @(posedge clk) begin
   case (pcinsel) // synopsys parallel_case full_case
      3'b000:  pc <= pcplus1;
      3'b001:  pc <= k;
      3'b010:  pc <= stack1;
      3'b011:  pc <= stack2;
      3'b100:  pc <= dbus;
      3'b101:  pc <= RESET_VECTOR;

      // Don't really carry about these...
      3'b110:  pc <= pc;
      3'b111:  pc <= pc;
   endcase
end

// Select for the MUX going into the PC.
//
//
always @(inst or stacklevel or reset) begin
   if (reset == 1'b1) begin
      pcinsel <= PC_SELECT_RESET_VECTOR;
   end
   else begin
      casex ({inst, stacklevel})
         14'b101?_????_????_??: pcinsel <= PC_SELECT_K;		// GOTO
         14'b1001_????_????_??: pcinsel <= PC_SELECT_K;		// CALL
         14'b1000_????_????_00: pcinsel <= PC_SELECT_STACK1;	// RETLW
         14'b1000_????_????_01: pcinsel <= PC_SELECT_STACK1;	// RETLW
         14'b1000_????_????_10: pcinsel <= PC_SELECT_STACK2;	// RETLW
         14'b1000_????_????_11: pcinsel <= PC_SELECT_STACK2;	// RETLW
         14'b0010_0010_0010_??: pcinsel <= PC_SELECT_DBUS;	// MOVF where f=PC
         default:
            pcinsel <= PC_SELECT_PCPLUS1;
      endcase
   end
end


// Implement STACK1 and STACK2 registers
//
// The Stack registers are only fed from the PC itself!
//
always @(posedge clk) begin
   if (reset) begin
      stack1 <= 9'h000;
   end
   else begin
      // CALL instruction
      if (inst[11:8] == 4'b1001) begin
         case (stacklevel)
            2'b00:
               // No previous CALLs
               begin
                  stack1 <= pc;
                  $display ("Write to STACK1: %0h", pc);
               end
            2'b01:
               // ONE previous CALL
               begin
                  stack2 <= pc;
                  $display ("Write to STACK2: %0h", pc);
               end
            2'b10:
               // TWO previous CALLs -- This is illegal on the 16C5X!
               begin
                  $display ("Too many CALLs!!");
               end
            2'b11:
               begin
                  $display ("Too many CALLs!!");
               end
         endcase
      end
   end
end

// Write to stacklevel
//
// The stacklevel register keeps track of the current stack depth.  On this
// puny processor, there are only 2 levels (you could fiddle with this and
// increase the stack depth).  There are two stack registers, stack1 and stack2.
// The stack1 register is used first (e.g. the first time a call is performed),
// then stack2.  As CALLs are done, stacklevel increments.  Conversely, as
// RETs are done, stacklevel goes down.

always @(posedge clk) begin
   if (reset == 1'b1) begin
      stacklevel <= 2'b00;  // On reset, there should be no CALLs in progress
   end
   else begin
      casex ({inst, stacklevel})
         // Call instructions
         14'b1001_????_????_00: stacklevel <= 2'b01;  // Record 1st CALL
         14'b1001_????_????_01: stacklevel <= 2'b10;  // Record 2nd CALL
         14'b1001_????_????_10: stacklevel <= 2'b10;  // Already 2! Ignore
         14'b1001_????_????_11: stacklevel <= 2'b00;  // {shouldn't happen}

         // Return instructions
         14'b1000_????_????_00: stacklevel <= 2'b00;  // {shouldn't happen}
         14'b1000_????_????_01: stacklevel <= 2'b00;  // Go back to no CALL in progress
         14'b1000_????_????_10: stacklevel <= 2'b01;  // Go back to 1 CALL in progress
         14'b1000_????_????_11: stacklevel <= 2'b10;  // {shouldn't happen} sort of like, Go back to 2 calls in progress
         default:
            stacklevel <= stacklevel;
      endcase
   end
end



// ************  EXPANSION  *************************
//
// The following is an example of customization.
//
// Example:  Create a read/write port located at address 7F.  It'll be 8-bits, where
//           the upper 4 bits are outputs and the lower 4 bits are inputs.
//           Use indirect addressing to access it (INDF/FSR).  Just for fun, let's
//           attach a special loop-back circuit between the outputs and inputs.
//           Let's attach... say... a 4-bit adder.
//

reg [3:0]	special_peripheral_writeable_bits;
reg [3:0]	special_peripheral_readeable_bits;

// Implement the writeable bits.
//
always @(posedge clk) begin
   if (reset) begin
      special_peripheral_writeable_bits <= 4'b0000;
   end
   else begin
      if (fwe & expsel & (fileaddr == EXPADDRESS_LILADDER)) begin
         special_peripheral_writeable_bits <= dbus;
      end
   end
end

// Implement the special peripheral function (the 4-bit adder for this example).
always @(special_peripheral_writeable_bits) begin
   special_peripheral_readeable_bits <= special_peripheral_writeable_bits + 1;
end

// Drive the ebus.  With only one custom address, no more muxing needs to be
// done, but if there are multiple custom circuits, everyone needs to cooperate
// and drive ebus properly.
//
always @(fileaddr or special_peripheral_readeable_bits) begin
   if (fileaddr == EXPADDRESS_LILADDER)
      ebus <= special_peripheral_readeable_bits;
   else
      ebus <= 8'hff;
end

endmodule
//
// SYNTHETIC PIC 2.0                                          4/23/98
//
//    This is a synthesizable Microchip 16C57 compatible
//    microcontroller.  This core is not intended as a high fidelity model of
//    the PIC, but simply a way to offer a simple processor core to people
//    familiar with the PIC who also have PIC tools.
//
//    pictest.v  -   top-level testbench (NOT SYNTHESIZABLE)
//    piccpu.v   -   top-level synthesizable module
//    picregs.v  -   register file instantiated under piccpu
//    picalu.v   -   ALU instantiated under piccpu
//    picidec.v  -   Instruction Decoder instantiated under piccpu
//    hex2rom.c  -   C program used to translate MPLAB's INTEL HEX output
//                   into the Verilog $readmemh compatible file
//    test*.asm  -   (note the wildcard..) Several test programs used
//                   to help debug the verilog.  I used MPLAB and the simulator
//                   to develop these programs and get the expected results.
//                   Then, I ran them on Verilog-XL where they appeared to
//                   match.
//
//    Copyright, Tom Coonan, '97.
//    Use freely, but not for resale as is.  You may use this in your
//    own projects as desired.  Just don't try to sell it as is!
//
//
// This is the PIC Instruction Decoder.
//
// The 12-bit PIC instruction must result in a set of control
// signals to the ALU, register write enables and other wires.
// This is purely combinatorial.  This can physically be
// implemented as a ROM, or, in this implementation a Verilog
// casex statement is used to directly synthesize the signals.
// This approach is more portable, and hopefully much reduction
// occurs in the equations.
//
// The Synthetic PIC Manual contains a table that better shows
// all the required signals per instruction.  I basically
// took that table and created the Verilog implementation below.
//

module picidec (
	inst,
	aluasel,
	alubsel,
	aluop,
	wwe,
	fwe,
	zwe,
	cwe,
	bdpol,
	option,
	tris
);

input  [11:0]	inst;

output [1:0]	aluasel;
output [1:0]	alubsel;
output [3:0]	aluop;
output		wwe;
output		fwe;
output		zwe;
output		cwe;
output		bdpol;
output		option;
output		tris;

reg [14:0] decodes;

// For reference, the ALU Op codes are:
//
//   ADD  0000
//   SUB  1000
//   AND  0001
//   OR   0010
//   XOR  0011
//   COM  0100
//   ROR  0101
//   ROL  0110
//   SWAP 0111

assign {	aluasel,	// Select source for ALU A input. 00=W, 01=SBUS, 10=K, 11=BD
		alubsel,	// Select source for ALU B input. 00=W, 01=SBUS, 10=K, 11="1"
		aluop,		// ALU Operation (see comments above for these codes)
		wwe,		// W register Write Enable
		fwe,		// File Register Write Enable
		zwe,		// Status register Z bit update
		cwe,		// Status register Z bit update
		bdpol,		// Polarity on bit decode vector (0=no inversion, 1=invert)
		tris,		// Instruction is an TRIS instruction
		option		// Instruction is an OPTION instruction
	} = decodes;

// This is a large combinatorial decoder.
// I use the casex statement.

always @(inst) begin
	casex (inst)
		// *** Byte-Oriented File Register Operations
		//
		//                                 A  A  ALU  W F Z C B T O
		//                                 L  L   O   W W W W D R P
		//                                 U  U   P   E E E E P I T
		//                                 A  B               O S
		//                                                    L
		12'b0000_0000_0000: decodes <= 15'b00_00_0000_0_0_0_0_0_0_0; // NOP
		12'b0000_001X_XXXX: decodes <= 15'b00_00_0010_0_1_0_0_0_0_0; // MOVWF
		12'b0000_0100_0000: decodes <= 15'b00_00_0011_1_0_1_0_0_0_0; // CLRW
		12'b0000_011X_XXXX: decodes <= 15'b00_00_0011_0_1_1_0_0_0_0; // CLRF
		12'b0000_100X_XXXX: decodes <= 15'b01_00_1000_1_0_1_1_0_0_0; // SUBWF (d=0)
		12'b0000_101X_XXXX: decodes <= 15'b01_00_1000_0_1_1_1_0_0_0; // SUBWF (d=1)
		12'b0000_110X_XXXX: decodes <= 15'b01_11_1000_1_0_1_0_0_0_0; // DECF  (d=0)
		12'b0000_111X_XXXX: decodes <= 15'b01_11_1000_0_1_1_0_0_0_0; // DECF  (d=1)
		12'b0001_000X_XXXX: decodes <= 15'b00_01_0010_1_0_1_0_0_0_0; // IORWF (d=0)
		12'b0001_001X_XXXX: decodes <= 15'b00_01_0010_0_1_1_0_0_0_0; // IORWF (d=1)
		12'b0001_010X_XXXX: decodes <= 15'b00_01_0001_1_0_1_0_0_0_0; // ANDWF (d=0)
		12'b0001_011X_XXXX: decodes <= 15'b00_01_0001_0_1_1_0_0_0_0; // ANDWF (d=1)
		12'b0001_100X_XXXX: decodes <= 15'b00_01_0011_1_0_1_0_0_0_0; // XORWF (d=0)
		12'b0001_101X_XXXX: decodes <= 15'b00_01_0011_0_1_1_0_0_0_0; // XORWF (d=1)
		12'b0001_110X_XXXX: decodes <= 15'b00_01_0000_1_0_1_1_0_0_0; // ADDWF (d=0)
		12'b0001_111X_XXXX: decodes <= 15'b00_01_0000_0_1_1_1_0_0_0; // ADDWF (d=1)
		12'b0010_000X_XXXX: decodes <= 15'b01_01_0010_1_0_1_0_0_0_0; // MOVF  (d=0)
		12'b0010_001X_XXXX: decodes <= 15'b01_01_0010_0_1_1_0_0_0_0; // MOVF  (d=1)
		12'b0010_010X_XXXX: decodes <= 15'b01_01_0100_1_0_1_0_0_0_0; // COMF  (d=0)
		12'b0010_011X_XXXX: decodes <= 15'b01_01_0100_0_1_1_0_0_0_0; // COMF  (d=1)
		12'b0010_100X_XXXX: decodes <= 15'b01_11_0000_1_0_1_0_0_0_0; // INCF  (d=0)
		12'b0010_101X_XXXX: decodes <= 15'b01_11_0000_0_1_1_0_0_0_0; // INCF  (d=1)
		12'b0010_110X_XXXX: decodes <= 15'b01_11_1000_1_0_0_0_0_0_0; // DECFSZ(d=0)
		12'b0010_111X_XXXX: decodes <= 15'b01_11_1000_0_1_0_0_0_0_0; // DECFSZ(d=1)
		12'b0011_000X_XXXX: decodes <= 15'b01_01_0101_1_0_0_1_0_0_0; // RRF   (d=0)
		12'b0011_001X_XXXX: decodes <= 15'b01_01_0101_0_1_0_1_0_0_0; // RRF   (d=1)
		12'b0011_010X_XXXX: decodes <= 15'b01_01_0110_1_0_0_1_0_0_0; // RLF   (d=0)
		12'b0011_011X_XXXX: decodes <= 15'b01_01_0110_0_1_0_1_0_0_0; // RLF   (d=1)
		12'b0011_100X_XXXX: decodes <= 15'b01_01_0111_1_0_0_0_0_0_0; // SWAPF (d=0)
		12'b0011_101X_XXXX: decodes <= 15'b01_01_0111_0_1_0_0_0_0_0; // SWAPF (d=1)
		12'b0011_110X_XXXX: decodes <= 15'b01_11_0000_1_0_0_0_0_0_0; // INCFSZ(d=0)
		12'b0011_111X_XXXX: decodes <= 15'b01_11_0000_0_1_0_0_0_0_0; // INCFSZ(d=1)

		// *** Bit-Oriented File Register Operations
                //
		//                                 A  A  ALU  W F Z C B T O
		//                                 L  L   O   W W W W D R P
		//                                 U  U   P   E E E E P I T
		//                                 A  B               O S
		//                                                    L
		12'b0100_XXXX_XXXX: decodes <= 15'b11_01_0001_0_1_0_0_1_0_0; // BCF
		12'b0101_XXXX_XXXX: decodes <= 15'b11_01_0010_0_1_0_0_0_0_0; // BSF
		12'b0110_XXXX_XXXX: decodes <= 15'b11_01_0001_0_0_0_0_0_0_0; // BTFSC
		12'b0111_XXXX_XXXX: decodes <= 15'b11_01_0001_0_0_0_0_0_0_0; // BTFSS

		// *** Literal and Control Operations
                //
		//                                 A  A  ALU  W F Z C B T O
		//                                 L  L   O   W W W W D R P
		//                                 U  U   P   E E E E P I T
		//                                 A  B               O S
		//                                                    L
		12'b0000_0000_0010: decodes <= 15'b00_00_0010_0_1_0_0_0_0_1; // OPTION
		12'b0000_0000_0011: decodes <= 15'b00_00_0000_0_0_0_0_0_0_0; // SLEEP
		12'b0000_0000_0100: decodes <= 15'b00_00_0000_0_0_0_0_0_0_0; // CLRWDT
		12'b0000_0000_0101: decodes <= 15'b00_00_0000_0_1_0_0_0_1_0; // TRIS 5
		12'b0000_0000_0110: decodes <= 15'b00_00_0010_0_1_0_0_0_1_0; // TRIS 6
		12'b0000_0000_0111: decodes <= 15'b00_00_0010_0_1_0_0_0_1_0; // TRIS 7
                //
		//                                 A  A  ALU  W F Z C B T O
		//                                 L  L   O   W W W W D R P
		//                                 U  U   P   E E E E P I T
		//                                 A  B               O S
		//                                                    L
		12'b1000_XXXX_XXXX: decodes <= 15'b10_10_0010_1_0_0_0_0_0_0; // RETLW
		12'b1001_XXXX_XXXX: decodes <= 15'b10_10_0010_0_0_0_0_0_0_0; // CALL
		12'b101X_XXXX_XXXX: decodes <= 15'b10_10_0010_0_0_0_0_0_0_0; // GOTO
		12'b1100_XXXX_XXXX: decodes <= 15'b10_10_0010_1_0_0_0_0_0_0; // MOVLW
		12'b1101_XXXX_XXXX: decodes <= 15'b00_10_0010_1_0_1_0_0_0_0; // IORLW
		12'b1110_XXXX_XXXX: decodes <= 15'b00_10_0001_1_0_1_0_0_0_0; // ANDLW
		12'b1111_XXXX_XXXX: decodes <= 15'b00_10_0011_1_0_1_0_0_0_0; // XORLW

		default:
			decodes <= 15'b00_00_0000_0_0_0_0_0_0_0;
	endcase
end

endmodule


//
// SYNTHETIC PIC 2.0                                          4/23/98
//
//    This is a synthesizable Microchip 16C57 compatible
//    microcontroller.  This core is not intended as a high fidelity model of
//    the PIC, but simply a way to offer a simple processor core to people
//    familiar with the PIC who also have PIC tools.
//
//    pictest.v  -   top-level testbench (NOT SYNTHESIZABLE)
//    piccpu.v   -   top-level synthesizable module
//    picregs.v  -   register file instantiated under piccpu
//    picalu.v   -   ALU instantiated under piccpu
//    picidec.v  -   Instruction Decoder instantiated under piccpu
//    hex2rom.c  -   C program used to translate MPLAB's INTEL HEX output
//                   into the Verilog $readmemh compatible file
//    test*.asm  -   (note the wildcard..) Several test programs used
//                   to help debug the verilog.  I used MPLAB and the simulator
//                   to develop these programs and get the expected results.
//                   Then, I ran them on Verilog-XL where they appeared to
//                   match.
//
//    Copyright, Tom Coonan, '97.
//    Use freely, but not for resale as is.  You may use this in your
//    own projects as desired.  Just don't try to sell it as is!
//
//

//`define DEBUG_SHOWREADS
//`define DEBUG_SHOWWRITES

// Memory Map:
//
// PIC Data Memory addressing is complicated.  See the Data Book for full explanation..
//
// Basically, each BANK contains 32 locations.  The lowest 16 locations in ALL Banks
// are really all mapped to the same bank (bank #0).  The first 8 locations are the Special
// registers like the STATUS and PC registers.  The upper 16 words in each bank, really are
// unique to each bank.  The smallest PIC (16C54) only has the one bank #0.
//
// So, as a programmer, what you get is this.  No matter what bank you are in (FSR[6:5])
// you always have access to your special registers and also to registers 8-15.  You can
// change to a 1 of 4 banks by setting FSR[6:5] and get 4 different sets of registers
// 16-31.
//
// For numbering purposes, I've numbered the registers as if they are one linear memory
// space, just like in the Data Book (see Figure 4-15 "Direct/Indirect Addressing").
// So, the unique 16 registers in bank #1 are named r48 - r63 (I use decimal).  The
// unique registers in bank #3 are therefore r112 - r127.  There is no r111 because,
// remember, the lower 16 registers each each bank are all reall the same registers 0-15.
//
// Confused?!  The Data Book explains it better than I can.
//
//   bank location
//     XX 00rrr  -  The special registers are not implemented in this register file.
//     XX 01rrr  -  The 8 common words, just above the Special Regs, same for all Banks
//     00 1rrrr  -  The 16 words unique to Bank #0
//     01 1rrrr  -  The 16 words unique to Bank #1
//     10 1rrrr  -  The 16 words unique to Bank #2
//     11 1rrrr  -  The 16 words unique to Bank #3
//
//  So,
//     Special Regs are location[4:3] == 00
//     Common Regs are  location[4:3] == 01
//     Words in banks   location[4]   == 1
//
//
//  I had problems trying to use simple register file declarations that
//  would always, always work right, were synthesizable and allowed
//  me to easily remove locations from the memory space.  SOOOooo... I
//  did the brute force thing and enumerated all the locations..
//
//  Much larger spaces will probably need a RAM and whatever I do would need
//  custom hacking anyway..  I don't see an obvious solution to all this, but
//  welcome suggestions..
//
module picregs (clk, reset, we, re, bank, location, din, dout);

input		clk;
input		reset;
input		we;
input		re;
input  [1:0]	bank;		// Bank 0,1,2,3
input  [4:0]	location;	// Location
input  [7:0]	din;		// Input
output [7:0]	dout;		// Output

//parameter	MAX_ADDRESS = 127;

reg [7:0]	dout;

integer index;

// Declare the major busses in and out of each block.
//
reg [7:0]	commonblockout;	// Data Memory common across all banks
reg [7:0]	highblock0out;	// Upper 16 bytes in BANK #0
reg [7:0]	highblock1out;	// Upper 16 bytes in BANK #1
reg [7:0]	highblock2out;	// Upper 16 bytes in BANK #2
reg [7:0]	highblock3out;	// Upper 16 bytes in BANK #3

reg [7:0]	commonblockin;	// Data Memory common across all banks
reg [7:0]	highblock0in;	// Upper 16 bytes in BANK #0
reg [7:0]	highblock1in;	// Upper 16 bytes in BANK #1
reg [7:0]	highblock2in;	// Upper 16 bytes in BANK #2
reg [7:0]	highblock3in;	// Upper 16 bytes in BANK #3

reg		commonblocksel;	// Select
reg		highblock0sel;	// Select
reg		highblock1sel;	// Select
reg		highblock2sel;	// Select
reg		highblock3sel;	// Select
// synopsys translate_off
integer cycle_counter;
initial cycle_counter = 0;
always @(negedge clk) begin
   if (re) begin
`ifdef DEBUG_SHOWREADS
      $display ("[%0d] Read:  data = %0h(hex), from bank #%0d(dec) location %0h", cycle_counter, dout, bank, location);
`endif
   end
   if (we) begin
`ifdef DEBUG_SHOWWRITES
      $display ("[%0d] Write: data = %0h(hex), to   bank #%0d(dec) location %0h", cycle_counter, din, bank, location);
`endif
   end
   if (~reset) cycle_counter = cycle_counter + 1;
end
// synopsys translate_on

// READ the Register File
//
always @(bank or location or re
		or commonblockout
		or highblock0out
		or highblock1out
		or highblock2out
		or highblock3out) begin
   if (re) begin
      if (location[4:3] == 2'b01) begin
         // This is the lower 8 words, common to all banks, just above special registers
         dout <= commonblockout;	// Access to first 8 locations past Special Registers
      end
      else begin
         if (location[4]) begin
            // Address is in the upper 16 words on one of the 4 banks
            case (bank) // synopsys full_case parallel_case
               2'b00:  dout <= highblock0out;	// Upper 16 words of Bank #0
               2'b01:  dout <= highblock1out;	// Upper 16 words of Bank #1
               2'b10:  dout <= highblock2out;	// Upper 16 words of Bank #2
               2'b11:  dout <= highblock3out;	// Upper 16 words of Bank #3
            endcase
         end
         else begin
            dout <= 8'hff;
         end
      end
   end
   else begin
      dout <= 8'hff;
   end
end

// Initial Write logic.
//
// Generate the specific write enables based on the PIC's bank/location rules.
// The individual memory blocks will do the actual synchronous write.
//
always @(we or bank or location or reset) begin
   if (reset) begin
      commonblocksel <= 1'b0;
      highblock0sel  <= 1'b0;
      highblock1sel  <= 1'b0;
      highblock2sel  <= 1'b0;
      highblock3sel  <= 1'b0;
   end
   else begin
      if (we) begin
         if (location[4:3] == 2'b01) begin
            // This is the lower 8 words, common to all banks, just above special registers
            commonblocksel <= 1'b1;
            highblock0sel  <= 1'b0;
            highblock1sel  <= 1'b0;
            highblock2sel  <= 1'b0;
            highblock3sel  <= 1'b0;
         end
         else begin
            if (location[4]) begin
               // Address is in the upper 16 words on one of the 4 banks
               commonblocksel <= 1'b0;
               case (bank) // synopsys full_case parallel_case
                  2'b00:  {highblock0sel, highblock1sel, highblock2sel, highblock3sel} <= 4'b1000; // Upper 16 words of Bank #0
                  2'b01:  {highblock0sel, highblock1sel, highblock2sel, highblock3sel} <= 4'b0100; // Upper 16 words of Bank #1
                  2'b10:  {highblock0sel, highblock1sel, highblock2sel, highblock3sel} <= 4'b0010; // Upper 16 words of Bank #2
                  2'b11:  {highblock0sel, highblock1sel, highblock2sel, highblock3sel} <= 4'b0001; // Upper 16 words of Bank #3
               endcase
            end
            else begin
               commonblocksel <= 1'b0;
               highblock0sel  <= 1'b0;
               highblock1sel  <= 1'b0;
               highblock2sel  <= 1'b0;
               highblock3sel  <= 1'b0;
            end
         end
      end
      else begin
         commonblocksel <= 1'b0;
         highblock0sel  <= 1'b0;
         highblock1sel  <= 1'b0;
         highblock2sel  <= 1'b0;
         highblock3sel  <= 1'b0;
      end
   end
end

// *** Buses feeding the memory blocks are driven directly from din.

always @(din)
   commonblockin <= din;

always @(din)
   highblock0in <= din;

always @(din)
   highblock1in <= din;

always @(din)
   highblock2in <= din;

always @(din)
   highblock3in <= din;

// ****************** Common Block *************

reg [7:0]	r8, r9, r10, r11, r12, r13, r14, r15;

// Read from common block
always @(location or
		r8 or r9 or r10 or r11 or r12 or r13 or r14 or r15) begin
   case (location[2:0])
      3'h0: commonblockout <= r8;
      3'h1: commonblockout <= r9;
      3'h2: commonblockout <= r10;
      3'h3: commonblockout <= r11;
      3'h4: commonblockout <= r12;
      3'h5: commonblockout <= r13;
      3'h6: commonblockout <= r14;
      3'h7: commonblockout <= r15;
   endcase
end

// Write to common block
always @(posedge clk) begin
   if (we & commonblocksel) begin
      case (location[2:0])
         3'h0: r8  <= commonblockin;
         3'h1: r9  <= commonblockin;
         3'h2: r10 <= commonblockin;
         3'h3: r11 <= commonblockin;
         3'h4: r12 <= commonblockin;
         3'h5: r13 <= commonblockin;
         3'h6: r14 <= commonblockin;
         3'h7: r15 <= commonblockin;
      endcase
   end
end

// **************** Highblock0 ****************

reg [7:0]	r16, r17, r18, r19, r20, r21, r22, r23;
reg [7:0]	r24, r25, r26, r27, r28, r29, r30, r31;

// Read from high block bank0
always @(location or
		r16 or r17 or r18 or r19 or r20 or r21 or r22 or r23 or
		r24 or r25 or r26 or r27 or r28 or r29 or r30 or r31
) begin
   case (location[3:0])
      4'h0: highblock0out <= r16;
      4'h1: highblock0out <= r17;
      4'h2: highblock0out <= r18;
      4'h3: highblock0out <= r19;
      4'h4: highblock0out <= r20;
      4'h5: highblock0out <= r21;
      4'h6: highblock0out <= r22;
      4'h7: highblock0out <= r23;
      4'h8: highblock0out <= r24;
      4'h9: highblock0out <= r25;
      4'hA: highblock0out <= r26;
      4'hB: highblock0out <= r27;
      4'hC: highblock0out <= r28;
      4'hD: highblock0out <= r29;
      4'hE: highblock0out <= r30;
      4'hF: highblock0out <= r31;
   endcase
end

// Write to high block bank 0
always @(posedge clk) begin
   if (we & highblock0sel) begin
      case (location[3:0])
         4'h0: r16 <= highblock0in;
         4'h1: r17 <= highblock0in;
         4'h2: r18 <= highblock0in;
         4'h3: r19 <= highblock0in;
         4'h4: r20 <= highblock0in;
         4'h5: r21 <= highblock0in;
         4'h6: r22 <= highblock0in;
         4'h7: r23 <= highblock0in;
         4'h8: r24 <= highblock0in;
         4'h9: r25 <= highblock0in;
         4'hA: r26 <= highblock0in;
         4'hB: r27 <= highblock0in;
         4'hC: r28 <= highblock0in;
         4'hD: r29 <= highblock0in;
         4'hE: r30 <= highblock0in;
         4'hF: r31 <= highblock0in;
      endcase
   end
end

// **************** Highblock1 ****************

reg [7:0]	r48, r49, r50, r51, r52, r53, r54, r55;
reg [7:0]	r56, r57, r58, r59, r60, r61, r62, r63;

// Read
always @(location or
		r48 or r49 or r50 or r51 or r52 or r53 or r54 or r55 or
		r56 or r57 or r58 or r59 or r60 or r61 or r62 or r63
) begin
   case (location[3:0])
      4'h0: highblock1out <= r48;
      4'h1: highblock1out <= r49;
      4'h2: highblock1out <= r50;
      4'h3: highblock1out <= r51;
      4'h4: highblock1out <= r52;
      4'h5: highblock1out <= r53;
      4'h6: highblock1out <= r54;
      4'h7: highblock1out <= r55;
      4'h8: highblock1out <= r56;
      4'h9: highblock1out <= r57;
      4'hA: highblock1out <= r58;
      4'hB: highblock1out <= r59;
      4'hC: highblock1out <= r60;
      4'hD: highblock1out <= r61;
      4'hE: highblock1out <= r62;
      4'hF: highblock1out <= r63;
   endcase
end

// Write
always @(posedge clk) begin
   if (we & highblock1sel) begin
      case (location[3:0])
         4'h0: r48 <= highblock1in;
         4'h1: r49 <= highblock1in;
         4'h2: r50 <= highblock1in;
         4'h3: r51 <= highblock1in;
         4'h4: r52 <= highblock1in;
         4'h5: r53 <= highblock1in;
         4'h6: r54 <= highblock1in;
         4'h7: r55 <= highblock1in;
         4'h8: r56 <= highblock1in;
         4'h9: r57 <= highblock1in;
         4'hA: r58 <= highblock1in;
         4'hB: r59 <= highblock1in;
         4'hC: r60 <= highblock1in;
         4'hD: r61 <= highblock1in;
         4'hE: r62 <= highblock1in;
         4'hF: r63 <= highblock1in;
      endcase
   end
end


// **************** Highblock2 ****************

reg [7:0]	r80, r81, r82, r83, r84, r85, r86, r87;
reg [7:0]	r88, r89, r90, r91, r92, r93, r94, r95;

// Read
always @(location or
		r80 or r81 or r82 or r83 or r84 or r85 or r86 or r87 or
		r88 or r89 or r90 or r91 or r92 or r93 or r94 or r95
) begin
   case (location[3:0])
      4'h0: highblock2out <= r80;
      4'h1: highblock2out <= r81;
      4'h2: highblock2out <= r82;
      4'h3: highblock2out <= r83;
      4'h4: highblock2out <= r84;
      4'h5: highblock2out <= r85;
      4'h6: highblock2out <= r86;
      4'h7: highblock2out <= r87;
      4'h8: highblock2out <= r88;
      4'h9: highblock2out <= r89;
      4'hA: highblock2out <= r90;
      4'hB: highblock2out <= r91;
      4'hC: highblock2out <= r92;
      4'hD: highblock2out <= r93;
      4'hE: highblock2out <= r94;
      4'hF: highblock2out <= r95;
   endcase
end

// Write
always @(posedge clk) begin
   if (we & highblock2sel) begin
      case (location[3:0])
         4'h0: r80 <= highblock2in;
         4'h1: r81 <= highblock2in;
         4'h2: r82 <= highblock2in;
         4'h3: r83 <= highblock2in;
         4'h4: r84 <= highblock2in;
         4'h5: r85 <= highblock2in;
         4'h6: r86 <= highblock2in;
         4'h7: r87 <= highblock2in;
         4'h8: r88 <= highblock2in;
         4'h9: r89 <= highblock2in;
         4'hA: r90 <= highblock2in;
         4'hB: r91 <= highblock2in;
         4'hC: r92 <= highblock2in;
         4'hD: r93 <= highblock2in;
         4'hE: r94 <= highblock2in;
         4'hF: r95 <= highblock2in;
      endcase
   end
end

// **************** Highblock3 ****************

// *** The Following Registers are removed because of CUSTOM Hardware (see piccpu.v) **
//
//    r129 (or 7E)
//
// **********
reg [7:0]	r112, r113, r114, r115, r116, r117, r118, r119;
reg [7:0]	r120, r121, r122, r123, r124, r125, r126 /*, r127*/ ;

// Read
always @(location or
		r112 or r113 or r114 or r115 or r116 or r117 or r118 or r119 or
		r120 or r121 or r122 or r123 or r124 or r125 or r126 /* or r127 */
) begin
   case (location[3:0])
      4'h0: highblock3out <= r112;
      4'h1: highblock3out <= r113;
      4'h2: highblock3out <= r114;
      4'h3: highblock3out <= r115;
      4'h4: highblock3out <= r116;
      4'h5: highblock3out <= r117;
      4'h6: highblock3out <= r118;
      4'h7: highblock3out <= r119;
      4'h8: highblock3out <= r120;
      4'h9: highblock3out <= r121;
      4'hA: highblock3out <= r122;
      4'hB: highblock3out <= r123;
      4'hC: highblock3out <= r124;
      4'hD: highblock3out <= r125;
      4'hE: highblock3out <= r126;
      4'hF: highblock3out <= 8'hff /* r127*/ ;
   endcase
end

// Write
always @(posedge clk) begin
   if (we & highblock3sel) begin
      case (location[3:0])
         4'h0: r112 <= highblock3in;
         4'h1: r113 <= highblock3in;
         4'h2: r114 <= highblock3in;
         4'h3: r115 <= highblock3in;
         4'h4: r116 <= highblock3in;
         4'h5: r117 <= highblock3in;
         4'h6: r118 <= highblock3in;
         4'h7: r119 <= highblock3in;
         4'h8: r120 <= highblock3in;
         4'h9: r121 <= highblock3in;
         4'hA: r122 <= highblock3in;
         4'hB: r123 <= highblock3in;
         4'hC: r124 <= highblock3in;
         4'hD: r125 <= highblock3in;
         4'hE: r126 <= highblock3in;
         4'hF: /* r127 <= highblock3in */;
      endcase
   end
end

// synopsys translate_off
`define CLEAR_MEMORY
`ifdef CLEAR_MEMORY
initial
begin
   $display ("Clearing SRAM.");
   clear_memory;
end
task clear_memory;
begin
   // Common registers
   r8  = 0;
   r9  = 0;
   r10 = 0;
   r11 = 0;
   r12 = 0;
   r13 = 0;
   r14 = 0;
   r15 = 0;

   // Bank #0
   r16 = 0;
   r17 = 0;
   r18 = 0;
   r19 = 0;
   r20 = 0;
   r21 = 0;
   r22 = 0;
   r23 = 0;
   r24 = 0;
   r25 = 0;
   r26 = 0;
   r27 = 0;
   r28 = 0;
   r29 = 0;
   r30 = 0;
   r31 = 0;

   // Bank #1
   r48 = 0;
   r49 = 0;
   r50 = 0;
   r51 = 0;
   r52 = 0;
   r53 = 0;
   r54 = 0;
   r55 = 0;
   r56 = 0;
   r57 = 0;
   r58 = 0;
   r59 = 0;
   r60 = 0;
   r61 = 0;
   r62 = 0;
   r63 = 0;

   // Bank #2
   r80 = 0;
   r94 = 0;

   // Bank #3
   r112 = 0;
   r126 = 0;

end
endtask
`endif
// synopsys translate_on
endmodule
//
// SYNTHETIC PIC 2.0                                          4/23/98
//
//    This is a synthesizable Microchip 16C57 compatible
//    microcontroller.  This core is not intended as a high fidelity model of
//    the PIC, but simply a way to offer a simple processor core to people
//    familiar with the PIC who also have PIC tools.
//
//    pictest.v  -   top-level testbench (NOT SYNTHESIZABLE)
//    piccpu.v   -   top-level synthesizable module
//    picregs.v  -   register file instantiated under piccpu
//    picalu.v   -   ALU instantiated under piccpu
//    picidec.v  -   Instruction Decoder instantiated under piccpu
//    hex2rom.c  -   C program used to translate MPLAB's INTEL HEX output
//                   into the Verilog $readmemh compatible file
//    test*.asm  -   (note the wildcard..) Several test programs used
//                   to help debug the verilog.  I used MPLAB and the simulator
//                   to develop these programs and get the expected results.
//                   Then, I ran them on Verilog-XL where they appeared to
//                   match.
//
//    Copyright, Tom Coonan, '97.
//    Use freely, but not for resale as is.  You may use this in your
//    own projects as desired.  Just don't try to sell it as is!
//
//

module pictest;

// Select which test to run HERE..
parameter	TEST_NUMBER = 9;

// *** Testing variables
// Debug flags.
integer		dbg_showporta;	// Are set in an 'initial' for default values,
integer		dbg_showportb;	//    override in specific tests...
integer		dbg_showportc;	// Setting to 1 will cause variable to be displayed.
integer		dbg_showinst;
integer		dbg_showrom;
integer		dbg_showw;
integer		dbg_showpc;

// cycles counter variables
integer		dbg_showcycles;	// Set to 1 to see cycles
integer		dbg_limitcycles;// Set to one to enable maxcycles check
integer		dbg_maxcycles;	// Limit simulation to some number of cycles.
integer		cycles;		// Cycles counter.



// *** Interface to the PICCPU
reg		clk;
reg		reset;
reg  [7:0]	porta;
wire [7:0]	portb;
wire [7:0]	portc;

reg  [11:0]	rom[0:511];
wire [8:0]	romaddr;
reg  [11:0]	romdata;

// ROM Interface
always @(romaddr) begin
   romdata = rom[romaddr];
end

reg [7:0]	last_debugw;
reg [8:0]	last_debugpc;
reg [11:0]	last_debuginst;
reg [7:0]	last_debugstatus;
wire [7:0]	debugw;
wire [8:0]	debugpc;
wire [11:0]	debuginst;
wire [7:0]	debugstatus;

// Instantiate one PICCPU to be tested.
piccpu piccpu_inst (
   .clk		(clk),
   .reset	(reset),
   .paddr	(romaddr),
   .pdata	(romdata),
   .portain	(porta),
   .portbout	(portb),
   .portcout	(portc),
   .debugw	(debugw),
   .debugpc	(debugpc),
   .debuginst	(debuginst),
   .debugstatus	(debugstatus)
);


// Reset
initial begin
//	$dumpfile("pic.vcd");
//	$dumpvars(0,pictest);
   reset = 1;
   #200;
   reset = 0;
end


// Drive the clock input
initial begin
   clk  = 0;
   forever begin
      #50 clk = 1;
      #50 clk = 0;
   end
end

// Debug defaults.  Override in individual test tasks.
//
initial begin
   dbg_showporta  = 0;
   dbg_showportb  = 0;
   dbg_showportc  = 0;
   dbg_showinst   = 0;
   dbg_showrom    = 0;
   dbg_showw      = 0;
   dbg_showpc     = 0;
   dbg_showcycles = 0;

   dbg_limitcycles = 1;
   dbg_maxcycles   = 50000;
end

// Call the appropriate test task based on the TEST_NUMBER parameter set at top.
//
initial begin
   case (TEST_NUMBER)
      1: test1;
      2: test2;
      3: test3;
      4: test4;
      5: test5;
      6: test6;
      7: test7;
      8: test8;
      9: test9;
      default:
         begin
            $display ("ERROR: Unknown Test Number: %0d", TEST_NUMBER);
            $finish;
         end
   endcase
end

task test1;
begin
   $display ("SYNTHETIC PIC 2.0.  This is TEST #1");
   #1;

   // Only Watch Port B
   dbg_showportb  = 1;
   dbg_showcycles = 1;

   $readmemh ("TEST1.ROM",rom);
   dbg_limitcycles = 1;
   dbg_maxcycles   = 500;
end
endtask

task test2;
begin
   $display ("SYNTHETIC PIC 2.0.  This is TEST #2");
   #1;

   // Only Watch Port B
   dbg_showportb = 1;

   $readmemh ("TEST2.ROM", rom);
   dbg_limitcycles = 1;
   dbg_maxcycles = 500;
end
endtask

task test3;
begin
   $display ("SYNTHETIC PIC 2.0.  This is TEST #3");
   #1;

   // Only Watch Port B
   dbg_showportb = 1;

   $readmemh ("TEST3.ROM", rom);
   dbg_limitcycles = 1;
   dbg_maxcycles = 500;
end
endtask

task test4;
begin
   $display ("SYNTHETIC PIC 2.0.  This is TEST #4");
   #1;

   // Only Watch Port B
   dbg_showportb = 1;

   $readmemh ("TEST4.ROM", rom);
   dbg_limitcycles = 1;
   dbg_maxcycles = 500;
end
endtask

task test5;
begin
   $display ("SYNTHETIC PIC 2.0.  This is TEST #5");
   #1;

   // Only Watch Port B
   dbg_showportb = 1;

   $readmemh ("TEST5.ROM", rom);
   dbg_limitcycles = 1;
   dbg_maxcycles = 500;
end
endtask

task test6;
begin
   $display ("SYNTHETIC PIC 2.0.  This is TEST #6");
   #1;

   // Watch Port B and C
   dbg_showportb = 1;
   dbg_showportc = 1;
   dbg_limitcycles = 0;

   $readmemh ("TEST6.ROM", rom);
   #200;

   repeat (20) begin
      porta = $random;
      #10000;
   end

   $finish;
end
endtask

task test7;
begin
   $display ("SYNTHETIC PIC 2.0.  This is TEST #7");
   #1;

   // Only Watch Port B
   dbg_showportb = 1;

   $readmemh ("TEST7.ROM", rom);
   dbg_limitcycles = 1;
   dbg_maxcycles = 500;
end
endtask

task test8;
begin
   $display ("SYNTHETIC PIC 2.0.  This is TEST #8");
   #1;

   // Watch All ports
   dbg_showporta = 1;
   dbg_showportb = 1;
   dbg_showportc = 1;

   $readmemh ("TEST8.ROM", rom);
   dbg_limitcycles = 1;
   dbg_maxcycles = 500;
end
endtask

task test9;
begin
   $display ("SYNTHETIC PIC 2.0.  This is TEST #9");
   #1;

   // Watch All ports
   dbg_showportb = 1;
   dbg_showportc = 1;

   $readmemh ("contrib/TEST9.ROM", rom);
   dbg_limitcycles = 1;
   dbg_maxcycles = 2000;
end
endtask

// ******** END Of TEST TASKS

// Cycles counter
//
initial begin
   cycles = 0;
   #1;
   // Don't start counting until after reset.
   @(negedge reset);

   forever begin
      @(posedge clk);
      cycles = cycles + 1;
      if ((cycles % 256) == 0) begin
         if (dbg_showcycles) begin
            $display ("#Cycles = %0d", cycles);
         end
      end

      if (dbg_limitcycles) begin
         if (cycles > dbg_maxcycles) begin
            $display ("Maximum cycles (%0d) Exceeded.  Halting simulation.", dbg_maxcycles);
            $finish(0);
         end
      end
   end
end

always @(romaddr) begin
   if (dbg_showrom)
      $display ("ROM Address = %h, Data = %h", romaddr, romdata);
end

always @(porta) begin
   if (dbg_showporta)
      $display ("%d: porta changes to: %h", $time, porta);
end

always @(portb) begin
   if (dbg_showportb)
      $display ("%d: portb changes to: %h", $time, portb);
end

always @(portc) begin
   if (dbg_showportc)
      $display ("%d: portc changes to: %h", $time, portc);
end

initial begin
   if (dbg_showw) begin
      forever begin
         @(negedge clk);
         if (debugw != last_debugw) begin
            $display ("W = %0h", debugw);
         end
         last_debugw = debugw;
      end
   end
end

initial begin
   if (dbg_showpc) begin
      forever begin
         @(negedge clk);
         $display ("PC = %0h", debugpc);
      end
   end
end



reg [11:0] last_pc;

always @(posedge clk) begin
   last_pc = debugpc;
end

initial begin
   if (dbg_showinst) begin
      forever begin
        @(negedge clk);

	if (debuginst[11:0] == 12'b0000_0000_0000) begin
		$display ("%h NOP", last_pc);
	end
	else if (debuginst[11:5] == 7'b0000_001) begin
		$display ("%h MOVWF  f=0x%0h", last_pc, debuginst[4:0]);
	end
	else if (debuginst == 12'b0000_0100_0000) begin
		$display ("%h CLRW", last_pc);
	end
	else if (debuginst[11:5] == 7'b0000_011) begin
		$display ("%h CLRF  f=0x%0h", last_pc, debuginst[4:0]);
	end
	else if (debuginst[11:6] == 7'b0000_10) begin
		if (piccpu_inst.d == 0)	$display ("%h SUBWF  f=0x%0h, W", last_pc, debuginst[4:0]);
		else		$display ("%h SUBWF  f=0x%0h, f", last_pc, debuginst[4:0]);
	end

	else if (debuginst[11:6] == 7'b0000_11) begin
		if (piccpu_inst.d == 0)	$display ("%h DECF  f=0x%0h, W", last_pc, debuginst[4:0]);
		else		$display ("%h DECF  f=0x%0h, f", last_pc, debuginst[4:0]);
	end
	else if (debuginst[11:6] == 7'b0001_00) begin
		if (piccpu_inst.d == 0)	$display ("%h IORWF  f=0x%0h, W", last_pc, debuginst[4:0]);
		else		$display ("%h IORWF  f=0x%0h, f", last_pc, debuginst[4:0]);
	end
	else if (debuginst[11:6] == 7'b0001_01) begin
		if (piccpu_inst.d == 0)	$display ("%h ANDWF  f=0x%0h, W", last_pc, debuginst[4:0]);
		else		$display ("%h ANDWF  f=0x%0h, f", last_pc, debuginst[4:0]);
	end
	else if (debuginst[11:6] == 7'b0001_10) begin
		if (piccpu_inst.d == 0)	$display ("XORWF  f=0x%0h, W", last_pc, debuginst[4:0]);
		else		$display ("%h XORWF  f=0x%0h, f", last_pc, debuginst[4:0]);
	end
	else if (debuginst[11:6] == 7'b0001_11) begin
		if (piccpu_inst.d == 0)	$display ("%h ADDWF  f=0x%0h, W", last_pc, debuginst[4:0]);
		else		$display ("%h ADDWF  f=0x%0h, f", last_pc, debuginst[4:0]);
	end
	else if (debuginst[11:6] == 7'b0010_00) begin
		if (piccpu_inst.d == 0)	$display ("%h MOVF  f=0x%0h, W", last_pc, debuginst[4:0]);
		else		$display ("%h MOVF  f=0x%0h, f", last_pc, debuginst[4:0]);
	end
	else if (debuginst[11:6] == 7'b0010_01) begin
		if (piccpu_inst.d == 0)	$display ("%h COMF  f=0x%0h, W", last_pc, debuginst[4:0]);
		else		$display ("%h COMF  f=0x%0h, f", last_pc, debuginst[4:0]);
	end
	else if (debuginst[11:6] == 7'b0010_10) begin
		if (piccpu_inst.d == 0)	$display ("%h INCF  f=0x%0h, W", last_pc, debuginst[4:0]);
		else		$display ("%h INCF  f=0x%0h, f", last_pc, debuginst[4:0]);
	end
	else if (debuginst[11:6] == 7'b0010_11) begin
		if (piccpu_inst.d == 0)	$display ("%h DECFSZ  f=0x%0h, W", last_pc, debuginst[4:0]);
		else		$display ("%h DECFSZ  f=0x%0h, f", last_pc, debuginst[4:0]);
	end
	else if (debuginst[11:6] == 7'b0011_00) begin
		if (piccpu_inst.d == 0)	$display ("%h RRF  f=0x%0h, W", last_pc, debuginst[4:0]);
		else		$display ("%h RRF  f=0x%0h, f", last_pc, debuginst[4:0]);
	end
	else if (debuginst[11:6] == 7'b0011_01) begin
		if (piccpu_inst.d == 0)	$display ("%h RLF  f=0x%0h, W", last_pc, debuginst[4:0]);
		else		$display ("%h RLF  f=0x%0h, f", last_pc, debuginst[4:0]);
	end
	else if (debuginst[11:6] == 7'b0011_10) begin
		if (piccpu_inst.d == 0)	$display ("%h SWAPF  f=0x%0h, W", last_pc, debuginst[4:0]);
		else		$display ("%h SWAPF  f=0x%0h, f", last_pc, debuginst[4:0]);
	end
	else if (debuginst[11:6] == 7'b0011_11) begin
		if (piccpu_inst.d == 0)	$display ("%h INCFSZ  f=0x%0h, W", last_pc, debuginst[4:0]);
		else		$display ("%h INCFSZ  f=0x%0h, f", last_pc, debuginst[4:0]);
	end

	// Bit-Oriented File Register Operations
	else if (debuginst[11:8] == 4'b0100) begin
		$display ("%h BCF  f=0x%0h, bit=%0d", last_pc, debuginst[4:0], piccpu_inst.b);
	end
	else if (debuginst[11:8] == 4'b0101) begin
		$display ("%h BCF  f=0x%0h, bit=%0d", last_pc, debuginst[4:0], piccpu_inst.b);
	end
	else if (debuginst[11:8] == 4'b0110) begin
		if (piccpu_inst.skip) $display ("%h BTFSC  f=0x%0h, bit=%0d  {Will Skip..}", last_pc, debuginst[4:0], piccpu_inst.b);
		else      $display ("%h BTFSC  f=0x%0h, bit=%0d  {Will NOT Skip..}", last_pc, debuginst[4:0], piccpu_inst.b);
	end
	else if (debuginst[11:8] == 4'b0111) begin
		if (piccpu_inst.skip) $display ("%h BTFSS  f=0x%0h, bit=%0d  {Will Skip..}", last_pc, debuginst[4:0], piccpu_inst.b);
		else      $display ("%h BTFSS  f=0x%0h, bit=%0d  {Will NOT Skip..}", last_pc, debuginst[4:0], piccpu_inst.b);
	end

	// Literal and Control Operations
	else if (debuginst[11:0] == 16'b0000_0000_0010) begin
		$display ("%h OPTION", last_pc);
	end
	else if (debuginst[11:0] == 16'b0000_0000_0011) begin
		$display ("%h SLEEP", last_pc);
	end
	else if (debuginst[11:0] == 16'b0000_0000_0100) begin
		$display ("%h CLRWDT", last_pc);
	end
	else if (debuginst[11:3] == 13'b0000_0000_0) begin
		$display ("%h TRIS,  f=0x%0h", last_pc, debuginst[2:0]);
	end
	else if (debuginst[11:8] == 4'b1000) begin
		$display ("%h RETLW,  k=0x%0h", last_pc, debuginst[7:0]);
	end
	else if (debuginst[11:8] == 4'b1001) begin
		$display ("%h CALL,  k=0x%0h", last_pc, debuginst[7:0]);
	end
	else if (debuginst[11:9] == 3'b101) begin
		$display ("%h GOTO,  k=0x%0h", last_pc, debuginst[8:0]);
	end
	else if (debuginst[11:8] == 4'b1100) begin
		$display ("%h MOVLW,  k=0x%0h", last_pc, debuginst[7:0]);
	end
	else if (debuginst[11:8] == 4'b1101) begin
		$display ("%h IORLW,  k=0x%0h", last_pc, debuginst[7:0]);
	end
	else if (debuginst[11:8] == 4'b1110) begin
		$display ("%h ANDLW,  k=0x%0h", last_pc, debuginst[7:0]);
	end
	else if (debuginst[11:8] == 4'b1111) begin
		$display ("%h XORLW,  k=0x%0h", last_pc, debuginst[7:0]);
	end
	else begin
		$display ("Hmmm!  instruction not recognized?! %0h", debuginst);
	end
      end
   end
end


endmodule
