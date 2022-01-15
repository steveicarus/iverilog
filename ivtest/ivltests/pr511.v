/*
 * This test is derived from bug report PR#511. Mostly what it is
 * doing is checking the behavior of === and wait.
 */

`timescale 1 ps / 1 ps

module I54;

 initial begin
 #500000
 $display( "FAILED." );
 $finish;
 end

 parameter I148 = 4096;

 integer I20;
 integer I57;
 integer I58;

 integer I137;

 time I106;
 time I95;
 time I97;

 time I122;
 time I142;
 time I67;
 time I25;
 time I83;
 time I128;
 time I10;
 time I12;
 time I73;
 time I159;

 reg I77;
 reg I108;
 reg I2;
 reg I81;
 reg I60;
 reg I114;
 reg I124;
 reg I41;

 reg I151;
 reg I43;
 reg [1:0] I68;
 reg [4*8-1:0] I138;
 reg [3:0] I71;
 reg [3:0] I149;
 reg [3:0] I6;
 reg [3:0] I76;
 reg I48;
 reg I61;
 reg I62;
 reg I49;
 reg I158;
 reg I85;
 reg [2:0] I84;
 reg [2:0] I30;
 reg I94;

 wire I69;
 wire I98;

 reg I115;
 reg I101;
 reg I14;
 reg I78;
 reg I131;
 reg I24;
 reg I52;
 reg I13;
 reg I132;
 reg I139;
 reg I107;

 wire I93;
 wire [3:0] I120;
 wire [3:0] I82;
 wire I38;
 wire I22;
 wire [1:0] I46;
 wire [2:0] I1;
 wire [4*8-1:0] I4;
 wire [3:0] I125;
 wire [3:0] I36;
 wire [3:0] I91;
 wire [3:0] I3;
 wire [3:0] I143;
 wire [1:0] I72;
 wire I34;
 wire I150;
 wire I40;
 wire [1:0] I51;

 reg [1:0] I39;
 reg [4-1:0] I70;

 wire I103;
 wire I29;

 wire I74;
 wire I144;

 reg I63;
 reg I15;
 reg I146;
 reg I110;
 reg I152;
 reg I129;
 reg I19;
 reg I112;
 reg I102;
 reg I156;
 reg I121;

 wire I23;
 wire [3:0] I118;
 wire [3:0] I86;
 wire I28;
 wire I65;
 wire [1:0] I79;
 wire [2:0] I17;
 wire [4*8-1:0] I133;
 wire [3:0] I27;
 wire [3:0] I99;
 wire [3:0] I135;
 wire [3:0] I90;
 wire [3:0] I35;
 wire [1:0] I55;
 wire I8;
 wire I126;
 wire I5;
 wire [1:0] I140;

 reg [1:0] I116;
 reg [4-1:0] I45;

 wire I105;
 wire I33;

 wire I75;
 wire I145;

 reg I64;
 reg I16;
 reg I147;
 reg I111;
 reg I153;
 reg I130;
 reg I21;
 reg I113;
 reg I104;
 reg I157;
 reg I123;

 wire I26;
 wire [3:0] I119;
 wire [3:0] I87;
 wire I32;
 wire I66;
 wire [1:0] I80;
 wire [2:0] I18;
 wire [4*8-1:0] I134;
 wire [3:0] I31;
 wire [3:0] I100;
 wire [3:0] I136;
 wire [3:0] I92;
 wire [3:0] I37;
 wire [1:0] I56;
 wire I9;
 wire I127;
 wire I7;
 wire [1:0] I141;

 reg [1:0] I117;
 reg [4-1:0] I47;

 wire [256:0] I59;
 wire [256:0] I50;
 wire [256:0] I88;
 wire [256:0] I154;
 wire [256:0] I89;
 wire [256:0] I155;

 assign I69 = I59[I20];
 assign I98 = I50[I20];
 assign I103 = I88[I57];
 assign I29 = I154[I57];
 assign I105 = I89[I58];
 assign I33 = I155[I58];

 wire I109 = ( ~ I94 ) & ( ~ I151 );
 wire I96 = ( ~ I94 ) & ( I151 );
 wire I42 = ( I94 ) & ( ~ I151 );
 wire I44 = ( I94 ) & ( I151 );

 always @( I114 ) begin
 I132 <= #I106 I114;
 I102 <= #I95 I114;
 I104 <= #I97 I114;
 end

 always @( I124 ) begin
 I139 <= #I106 I124;
 I156 <= #I95 I124;
 I157 <= #I97 I124;
 end

 initial begin
 I77 = 1'b0;
 #0 ;
 #(I142) I77 = 1'b1;
 forever #(I122/2) I77 = ~ I77;
 end

 initial begin
 I108 = 1'b0;
 #0 ;
 #(I25) I108 = 1'b1;
 forever #(I67/2) I108 = ~ I108;
 end

 initial begin
 I2 = 1'b0;
 #0 ;
 #(I128) I2 = 1'b1;
 forever #(I83/2) I2 = ~ I2;
 end

 initial begin
 I81 = 1'b0;
 #0 ;
 #(I12) I81 = 1'b1;
 forever #(I10/2) I81 = ~ I81;
 end

 initial begin
 I60 = 1'b0;
 #0 ;
 #(I159) I60 = 1'b1;
 forever #(I73/2) I60 = ~ I60;
 end

 initial begin
 I114 = 1'b1;
 #300000 ;
 @( posedge I81 ) ;
 @( posedge I81 ) ;
 #50 I114 = 1'b0;
 end

 initial begin
 I124 = 1'b1;
 #300000 ;
 @( posedge I60 ) ;
 @( posedge I60 ) ;
 #50 I124 = 1'b0;
 end

 initial begin : I53

 I137 = 1600;
 I20 = 40;
 I57 = 0;
 I58 = 80;
 I106 = 150;
 I95 = 300;
 I97 = 0;
 I151 = 1'b0;
 I94 = 1'b0;
 I122 = 6400;
 I142 = 0;
 I67 = 6270;
 I25 = 0;
 I83 = 6400;
 I128 = 1000;
 I10 = 6270;
 I12 = 0;
 I73 = 6400;
 I159 = 1000;

 wait ( I114 === 1'b1 );
 wait ( I114 === 1'b0 );
 wait ( I124 === 1'b0 );

 $display( "PASSED" );
 $finish;

 end

endmodule
