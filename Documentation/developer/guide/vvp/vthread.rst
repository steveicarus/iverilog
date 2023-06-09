
Thread Details
==============

Thread objects in vvp are created by `.thread` statements in the
input source file.

A thread object includes a program counter and private bit
registers. The program counter is used to step the processor through
the code space as it executes instructions. The bit registers each
hold Verilog-style 4-value bits and are for use by the arithmetic
operators as they operate.

The program counter normally increments by one instruction after the
instruction is fetched. If the instruction is a branching instruction,
then the execution of the instruction sets a new value for the pc.

Instructions that use the bit registers have as an operand a <bit>
value. There is usually space in the instruction for 2 <bit>
operands. Instructions that work on vectors pull the vector values
from the bit registers starting with the LSB and up.

The bit addresses 0, 1, 2 and 3 are special constant bits 0, 1, x and
z, and are used as read-only immediate values. If the instruction
takes a single bit operand, then the appropriate value is simply read
out. If the instruction expects a vector, then a vector of the
expected width is created by replicating the constant value.

Bits 4, 5, 6 and 7 are read/write bits but are reserved by many
instructions for special purposes. Comparison operators, for example,
use these as comparison flag bits.

The remaining 64K-8 possible <bit> values are read-write bit registers
that can be accessed singly or as vectors. This obviously implies that
a bit address is 16 bits.

Threads also contain 16 numeric registers. These registers can hold a
real value or a 64bit integer, and can be used in certain cases where
numeric values are needed. The thread instruction set includes
%ix/* instructions to manipulate these registers. The instructions
that use these registers document which register is used, and what the
numeric value is used for. Registers 0-3 are often given fixed
meanings to instructions that need an integer value.

::

    /*
     * Copyright (c) 2001 Stephen Williams (steve@icarus.com)
     *
     *    This source code is free software; you can redistribute it
     *    and/or modify it in source code form under the terms of the GNU
     *    General Public License as published by the Free Software
     *    Foundation; either version 2 of the License, or (at your option)
     *    any later version.
     *
     *    This program is distributed in the hope that it will be useful,
     *    but WITHOUT ANY WARRANTY; without even the implied warranty of
     *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
     *    GNU General Public License for more details.
     *
     *    You should have received a copy of the GNU General Public License
     *    along with this program; if not, write to the Free Software
     *    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
     */
