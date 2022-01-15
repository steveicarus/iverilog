module naughty_module(
	input clk,
	input [71:0] pzw,
	input [95:0] xy_d,
	input [23:0] pbit,
	output [95:0] xas,
	output [95:0] yas
);

initial $display("PASSED");

function [3:0] xa;
	input pbit;
	input [1:0] ix_in;
	input [1:0] iy_in;
	input [3:0] pzw;
begin
	xa =
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h0, 3'h0} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h0, 3'h1} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h0, 3'h2} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h0, 3'h3} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h0, 3'h4} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h0, 3'h5} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h0, 3'h6} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h0, 3'h7} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h1, 3'h0} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h1, 3'h1} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h1, 3'h2} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h1, 3'h3} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h1, 3'h4} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h1, 3'h5} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h1, 3'h6} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h1, 3'h7} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h2, 3'h0} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h2, 3'h1} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h2, 3'h2} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h2, 3'h3} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h2, 3'h4} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h2, 3'h5} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h2, 3'h6} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h2, 3'h7} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h3, 3'h0} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h3, 3'h1} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h3, 3'h2} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h3, 3'h3} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h3, 3'h4} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h3, 3'h5} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h3, 3'h6} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h3, 3'h7} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h0, 3'h0} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h0, 3'h1} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h0, 3'h2} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h0, 3'h3} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h0, 3'h4} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h0, 3'h5} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h0, 3'h6} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h0, 3'h7} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h1, 3'h0} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h1, 3'h1} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h1, 3'h2} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h1, 3'h3} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h1, 3'h4} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h1, 3'h5} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h1, 3'h6} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h1, 3'h7} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h2, 3'h0} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h2, 3'h1} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h2, 3'h2} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h2, 3'h3} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h2, 3'h4} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h2, 3'h5} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h2, 3'h6} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h2, 3'h7} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h3, 3'h0} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h3, 3'h1} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h3, 3'h2} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h3, 3'h3} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h3, 3'h4} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h3, 3'h5} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h3, 3'h6} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h3, 3'h7} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h0, 3'h0} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h0, 3'h1} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h0, 3'h2} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h0, 3'h3} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h0, 3'h4} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h0, 3'h5} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h0, 3'h6} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h0, 3'h7} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h1, 3'h0} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h1, 3'h1} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h1, 3'h2} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h1, 3'h3} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h1, 3'h4} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h1, 3'h5} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h1, 3'h6} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h1, 3'h7} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h2, 3'h0} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h2, 3'h1} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h2, 3'h2} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h2, 3'h3} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h2, 3'h4} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h2, 3'h5} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h2, 3'h6} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h2, 3'h7} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h3, 3'h0} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h3, 3'h1} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h3, 3'h2} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h3, 3'h3} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h3, 3'h4} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h3, 3'h5} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h3, 3'h6} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h3, 3'h7} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h0, 3'h0} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h0, 3'h1} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h0, 3'h2} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h0, 3'h3} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h0, 3'h4} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h0, 3'h5} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h0, 3'h6} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h0, 3'h7} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h1, 3'h0} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h1, 3'h1} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h1, 3'h2} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h1, 3'h3} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h1, 3'h4} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h1, 3'h5} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h1, 3'h6} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h1, 3'h7} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h2, 3'h0} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h2, 3'h1} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h2, 3'h2} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h2, 3'h3} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h2, 3'h4} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h2, 3'h5} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h2, 3'h6} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h2, 3'h7} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h3, 3'h0} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h3, 3'h1} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h3, 3'h2} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h3, 3'h3} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h3, 3'h4} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h3, 3'h5} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h3, 3'h6} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h3, 3'h7} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h0, 3'h0} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h0, 3'h1} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h0, 3'h2} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h0, 3'h3} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h0, 3'h4} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h0, 3'h5} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h0, 3'h6} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h0, 3'h7} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h1, 3'h0} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h1, 3'h1} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h1, 3'h2} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h1, 3'h3} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h1, 3'h4} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h1, 3'h5} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h1, 3'h6} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h1, 3'h7} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h2, 3'h0} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h2, 3'h1} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h2, 3'h2} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h2, 3'h3} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h2, 3'h4} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h2, 3'h5} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h2, 3'h6} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h2, 3'h7} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h3, 3'h0} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h3, 3'h1} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h3, 3'h2} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h3, 3'h3} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h3, 3'h4} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h3, 3'h5} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h3, 3'h6} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h3, 3'h7} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h0, 3'h0} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h0, 3'h1} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h0, 3'h2} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h0, 3'h3} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h0, 3'h4} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h0, 3'h5} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h0, 3'h6} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h0, 3'h7} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h1, 3'h0} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h1, 3'h1} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h1, 3'h2} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h1, 3'h3} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h1, 3'h4} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h1, 3'h5} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h1, 3'h6} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h1, 3'h7} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h2, 3'h0} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h2, 3'h1} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h2, 3'h2} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h2, 3'h3} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h2, 3'h4} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h2, 3'h5} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h2, 3'h6} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h2, 3'h7} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h3, 3'h0} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h3, 3'h1} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h3, 3'h2} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h3, 3'h3} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h3, 3'h4} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h3, 3'h5} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h3, 3'h6} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h3, 3'h7} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h0, 3'h0} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h0, 3'h1} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h0, 3'h2} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h0, 3'h3} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h0, 3'h4} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h0, 3'h5} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h0, 3'h6} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h0, 3'h7} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h1, 3'h0} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h1, 3'h1} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h1, 3'h2} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h1, 3'h3} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h1, 3'h4} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h1, 3'h5} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h1, 3'h6} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h1, 3'h7} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h2, 3'h0} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h2, 3'h1} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h2, 3'h2} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h2, 3'h3} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h2, 3'h4} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h2, 3'h5} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h2, 3'h6} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h2, 3'h7} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h3, 3'h0} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h3, 3'h1} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h3, 3'h2} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h3, 3'h3} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h3, 3'h4} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h3, 3'h5} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h3, 3'h6} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h3, 3'h7} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h0, 3'h0} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h0, 3'h1} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h0, 3'h2} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h0, 3'h3} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h0, 3'h4} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h0, 3'h5} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h0, 3'h6} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h0, 3'h7} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h1, 3'h0} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h1, 3'h1} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h1, 3'h2} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h1, 3'h3} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h1, 3'h4} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h1, 3'h5} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h1, 3'h6} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h1, 3'h7} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h2, 3'h0} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h2, 3'h1} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h2, 3'h2} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h2, 3'h3} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h2, 3'h4} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h2, 3'h5} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h2, 3'h6} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h2, 3'h7} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h3, 3'h0} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h3, 3'h1} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h3, 3'h2} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h3, 3'h3} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h3, 3'h4} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h3, 3'h5} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h3, 3'h6} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h3, 3'h7} ? 4'h3 :
	4'h0;
end
endfunction

function [3:0] ya;
	input pbit;
	input [1:0] ix_in;
	input [1:0] iy_in;
	input [3:0] pzw;
begin
	ya =
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h0, 3'h0} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h0, 3'h1} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h0, 3'h2} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h0, 3'h3} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h0, 3'h4} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h0, 3'h5} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h0, 3'h6} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h0, 3'h7} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h1, 3'h0} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h1, 3'h1} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h1, 3'h2} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h1, 3'h3} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h1, 3'h4} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h1, 3'h5} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h1, 3'h6} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h1, 3'h7} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h2, 3'h0} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h2, 3'h1} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h2, 3'h2} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h2, 3'h3} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h2, 3'h4} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h2, 3'h5} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h2, 3'h6} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h2, 3'h7} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h3, 3'h0} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h3, 3'h1} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h3, 3'h2} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h3, 3'h3} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h3, 3'h4} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h3, 3'h5} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h3, 3'h6} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h0, 2'h3, 3'h7} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h0, 3'h0} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h0, 3'h1} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h0, 3'h2} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h0, 3'h3} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h0, 3'h4} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h0, 3'h5} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h0, 3'h6} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h0, 3'h7} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h1, 3'h0} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h1, 3'h1} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h1, 3'h2} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h1, 3'h3} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h1, 3'h4} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h1, 3'h5} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h1, 3'h6} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h1, 3'h7} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h2, 3'h0} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h2, 3'h1} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h2, 3'h2} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h2, 3'h3} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h2, 3'h4} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h2, 3'h5} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h2, 3'h6} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h2, 3'h7} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h3, 3'h0} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h3, 3'h1} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h3, 3'h2} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h3, 3'h3} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h3, 3'h4} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h3, 3'h5} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h3, 3'h6} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h1, 2'h3, 3'h7} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h0, 3'h0} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h0, 3'h1} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h0, 3'h2} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h0, 3'h3} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h0, 3'h4} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h0, 3'h5} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h0, 3'h6} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h0, 3'h7} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h1, 3'h0} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h1, 3'h1} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h1, 3'h2} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h1, 3'h3} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h1, 3'h4} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h1, 3'h5} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h1, 3'h6} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h1, 3'h7} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h2, 3'h0} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h2, 3'h1} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h2, 3'h2} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h2, 3'h3} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h2, 3'h4} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h2, 3'h5} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h2, 3'h6} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h2, 3'h7} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h3, 3'h0} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h3, 3'h1} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h3, 3'h2} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h3, 3'h3} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h3, 3'h4} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h3, 3'h5} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h3, 3'h6} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h2, 2'h3, 3'h7} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h0, 3'h0} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h0, 3'h1} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h0, 3'h2} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h0, 3'h3} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h0, 3'h4} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h0, 3'h5} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h0, 3'h6} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h0, 3'h7} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h1, 3'h0} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h1, 3'h1} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h1, 3'h2} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h1, 3'h3} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h1, 3'h4} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h1, 3'h5} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h1, 3'h6} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h1, 3'h7} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h2, 3'h0} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h2, 3'h1} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h2, 3'h2} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h2, 3'h3} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h2, 3'h4} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h2, 3'h5} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h2, 3'h6} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h2, 3'h7} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h3, 3'h0} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h3, 3'h1} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h3, 3'h2} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h3, 3'h3} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h3, 3'h4} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h3, 3'h5} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h3, 3'h6} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h0, 2'h3, 2'h3, 3'h7} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h0, 3'h0} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h0, 3'h1} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h0, 3'h2} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h0, 3'h3} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h0, 3'h4} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h0, 3'h5} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h0, 3'h6} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h0, 3'h7} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h1, 3'h0} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h1, 3'h1} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h1, 3'h2} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h1, 3'h3} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h1, 3'h4} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h1, 3'h5} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h1, 3'h6} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h1, 3'h7} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h2, 3'h0} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h2, 3'h1} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h2, 3'h2} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h2, 3'h3} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h2, 3'h4} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h2, 3'h5} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h2, 3'h6} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h2, 3'h7} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h3, 3'h0} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h3, 3'h1} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h3, 3'h2} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h3, 3'h3} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h3, 3'h4} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h3, 3'h5} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h3, 3'h6} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h0, 2'h3, 3'h7} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h0, 3'h0} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h0, 3'h1} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h0, 3'h2} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h0, 3'h3} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h0, 3'h4} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h0, 3'h5} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h0, 3'h6} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h0, 3'h7} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h1, 3'h0} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h1, 3'h1} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h1, 3'h2} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h1, 3'h3} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h1, 3'h4} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h1, 3'h5} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h1, 3'h6} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h1, 3'h7} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h2, 3'h0} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h2, 3'h1} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h2, 3'h2} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h2, 3'h3} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h2, 3'h4} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h2, 3'h5} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h2, 3'h6} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h2, 3'h7} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h3, 3'h0} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h3, 3'h1} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h3, 3'h2} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h3, 3'h3} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h3, 3'h4} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h3, 3'h5} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h3, 3'h6} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h1, 2'h3, 3'h7} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h0, 3'h0} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h0, 3'h1} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h0, 3'h2} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h0, 3'h3} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h0, 3'h4} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h0, 3'h5} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h0, 3'h6} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h0, 3'h7} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h1, 3'h0} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h1, 3'h1} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h1, 3'h2} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h1, 3'h3} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h1, 3'h4} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h1, 3'h5} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h1, 3'h6} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h1, 3'h7} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h2, 3'h0} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h2, 3'h1} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h2, 3'h2} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h2, 3'h3} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h2, 3'h4} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h2, 3'h5} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h2, 3'h6} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h2, 3'h7} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h3, 3'h0} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h3, 3'h1} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h3, 3'h2} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h3, 3'h3} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h3, 3'h4} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h3, 3'h5} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h3, 3'h6} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h2, 2'h3, 3'h7} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h0, 3'h0} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h0, 3'h1} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h0, 3'h2} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h0, 3'h3} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h0, 3'h4} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h0, 3'h5} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h0, 3'h6} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h0, 3'h7} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h1, 3'h0} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h1, 3'h1} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h1, 3'h2} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h1, 3'h3} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h1, 3'h4} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h1, 3'h5} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h1, 3'h6} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h1, 3'h7} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h2, 3'h0} ? 4'h5 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h2, 3'h1} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h2, 3'h2} ? 4'h8 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h2, 3'h3} ? 4'h7 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h2, 3'h4} ? 4'h4 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h2, 3'h5} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h2, 3'h6} ? 4'h1 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h2, 3'h7} ? 4'h2 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h3, 3'h0} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h3, 3'h1} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h3, 3'h2} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h3, 3'h3} ? 4'h9 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h3, 3'h4} ? 4'h6 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h3, 3'h5} ? 4'h3 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h3, 3'h6} ? 4'h0 :
	{pbit, ix_in, iy_in, pzw} == {1'h1, 2'h3, 2'h3, 3'h7} ? 4'h0 :
	4'h0;
end
endfunction


	reg [95:0] r_xa;
	reg [95:0] r_ya;


	always @(posedge clk)
	begin
		r_xa[ 3:  0] <= xa(pbit[ 0], xy_d[ 1: 0], xy_d[ 3: 2], pzw[ 2: 0]);
		r_xa[ 7:  4] <= xa(pbit[ 1], xy_d[ 5: 4], xy_d[ 7: 6], pzw[ 5: 3]);
		r_xa[11:  8] <= xa(pbit[ 2], xy_d[ 9: 8], xy_d[11:10], pzw[ 8: 6]);
		r_xa[15: 12] <= xa(pbit[ 3], xy_d[13:12], xy_d[15:14], pzw[11: 9]);
		r_xa[19: 16] <= xa(pbit[ 4], xy_d[17:16], xy_d[19:18], pzw[14:12]);
		r_xa[23: 20] <= xa(pbit[ 5], xy_d[21:20], xy_d[23:22], pzw[17:15]);
		r_xa[27: 24] <= xa(pbit[ 6], xy_d[25:24], xy_d[27:26], pzw[20:18]);
		r_xa[31: 28] <= xa(pbit[ 7], xy_d[29:28], xy_d[31:30], pzw[23:21]);
		r_xa[35: 32] <= xa(pbit[ 8], xy_d[33:32], xy_d[35:34], pzw[26:24]);
		r_xa[39: 36] <= xa(pbit[ 9], xy_d[37:36], xy_d[39:38], pzw[29:27]);
		r_xa[43: 40] <= xa(pbit[10], xy_d[41:40], xy_d[43:42], pzw[32:30]);
		r_xa[47: 44] <= xa(pbit[11], xy_d[45:44], xy_d[47:46], pzw[35:33]);
		r_xa[51: 48] <= xa(pbit[12], xy_d[49:48], xy_d[51:50], pzw[38:36]);
		r_xa[55: 52] <= xa(pbit[13], xy_d[53:52], xy_d[55:54], pzw[41:39]);
		r_xa[59: 56] <= xa(pbit[14], xy_d[57:56], xy_d[59:58], pzw[44:42]);
		r_xa[63: 60] <= xa(pbit[15], xy_d[61:60], xy_d[63:62], pzw[47:45]);
		r_xa[67: 64] <= xa(pbit[16], xy_d[65:64], xy_d[67:66], pzw[50:48]);
		r_xa[71: 68] <= xa(pbit[17], xy_d[69:68], xy_d[71:70], pzw[53:51]);
		r_xa[75: 72] <= xa(pbit[18], xy_d[73:72], xy_d[75:74], pzw[56:54]);
		r_xa[79: 76] <= xa(pbit[19], xy_d[77:76], xy_d[79:78], pzw[59:57]);
		r_xa[83: 80] <= xa(pbit[20], xy_d[81:80], xy_d[83:82], pzw[62:60]);
		r_xa[87: 84] <= xa(pbit[21], xy_d[85:84], xy_d[87:86], pzw[65:63]);
		r_xa[91: 88] <= xa(pbit[22], xy_d[89:88], xy_d[91:90], pzw[68:66]);
		r_xa[95: 92] <= xa(pbit[23], xy_d[93:92], xy_d[95:94], pzw[71:69]);

		r_ya[ 3:  0] <= ya(pbit[ 0], xy_d[ 1: 0], xy_d[ 3: 2], pzw[ 2: 0]);
		r_ya[ 7:  4] <= ya(pbit[ 1], xy_d[ 5: 4], xy_d[ 7: 6], pzw[ 5: 3]);
		r_ya[11:  8] <= ya(pbit[ 2], xy_d[ 9: 8], xy_d[11:10], pzw[ 8: 6]);
		r_ya[15: 12] <= ya(pbit[ 3], xy_d[13:12], xy_d[15:14], pzw[11: 9]);
		r_ya[19: 16] <= ya(pbit[ 4], xy_d[17:16], xy_d[19:18], pzw[14:12]);
		r_ya[23: 20] <= ya(pbit[ 5], xy_d[21:20], xy_d[23:22], pzw[17:15]);
		r_ya[27: 24] <= ya(pbit[ 6], xy_d[25:24], xy_d[27:26], pzw[20:18]);
		r_ya[31: 28] <= ya(pbit[ 7], xy_d[29:28], xy_d[31:30], pzw[23:21]);
		r_ya[35: 32] <= ya(pbit[ 8], xy_d[33:32], xy_d[35:34], pzw[26:24]);
		r_ya[39: 36] <= ya(pbit[ 9], xy_d[37:36], xy_d[39:38], pzw[29:27]);
		r_ya[43: 40] <= ya(pbit[10], xy_d[41:40], xy_d[43:42], pzw[32:30]);
		r_ya[47: 44] <= ya(pbit[11], xy_d[45:44], xy_d[47:46], pzw[35:33]);
		r_ya[51: 48] <= ya(pbit[12], xy_d[49:48], xy_d[51:50], pzw[38:36]);
		r_ya[55: 52] <= ya(pbit[13], xy_d[53:52], xy_d[55:54], pzw[41:39]);
		r_ya[59: 56] <= ya(pbit[14], xy_d[57:56], xy_d[59:58], pzw[44:42]);
		r_ya[63: 60] <= ya(pbit[15], xy_d[61:60], xy_d[63:62], pzw[47:45]);
		r_ya[67: 64] <= ya(pbit[16], xy_d[65:64], xy_d[67:66], pzw[50:48]);
		r_ya[71: 68] <= ya(pbit[17], xy_d[69:68], xy_d[71:70], pzw[53:51]);
		r_ya[75: 72] <= ya(pbit[18], xy_d[73:72], xy_d[75:74], pzw[56:54]);
		r_ya[79: 76] <= ya(pbit[19], xy_d[77:76], xy_d[79:78], pzw[59:57]);
		r_ya[83: 80] <= ya(pbit[20], xy_d[81:80], xy_d[83:82], pzw[62:60]);
		r_ya[87: 84] <= ya(pbit[21], xy_d[85:84], xy_d[87:86], pzw[65:63]);
		r_ya[91: 88] <= ya(pbit[22], xy_d[89:88], xy_d[91:90], pzw[68:66]);
		r_ya[95: 92] <= ya(pbit[23], xy_d[93:92], xy_d[95:94], pzw[71:69]);
	end

	assign xas = r_xa;
	assign yas = r_ya;

endmodule
