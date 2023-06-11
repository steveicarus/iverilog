
Verilog-A math library
======================

License.
--------

  Verilog-A math library built for Icarus Verilog
  https://github.com/steveicarus/iverilog/

  Copyright (C) 2007-2010  Cary R. (cygcary@yahoo.com)

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License along
  with this program; if not, write to the Free Software Foundation, Inc.,
  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

Standard Verilog-A Mathematical Functions.
------------------------------------------

The va_math VPI module implements all the standard math functions provided
by Verilog-A as Verilog-D system functions. The names are the same except
like all Verilog-D system functions the name must be prefixed with a '$'.
For reference the functions are::

  $ln(x)       --  Natural logarithm
  $log10(x)    --  Decimal logarithm
  $exp(x)      --  Exponential
  $sqrt(x)     --  Square root
  $min(x,y)    --  Minimum
  $max(x,y)    --  Maximum
  $abs(x)      --  Absolute value
  $floor(x)    --  Floor
  $ceil(x)     --  Ceiling
  $pow(x,y)    --  Power (x**y)
  $sin(x)      --  Sine
  $cos(x)      --  Cosine
  $tan(x)      --  Tangent
  $asin(x)     --  Arc-sine
  $acos(x)     --  Arc-cosine
  $atan(x)     --  Arc-tangent
  $atan2(y,x)  --  Arc-tangent of y/x
  $hypot(x,y)  --  Hypotenuse (sqrt(x**2 + y**2))
  $sinh(x)     --  Hyperbolic sine
  $cosh(x)     --  Hyperbolic cosine
  $tanh(x)     --  Hyperbolic tangent
  $asinh(x)    --  Arc-hyperbolic sine
  $acosh(x)    --  Arc-hyperbolic cosine
  $atanh(x)    --  Arc-hyperbolic tangent

The only limit placed on the x and y arguments by the library is that they
must be numbers (not constant strings). The underlying C library controls
any other limits placed on the arguments. Most libraries return +-Inf or
NaN for results that cannot be represented with real numbers. All functions
return a real result.

Standard Verilog-A Mathematical Constants.
------------------------------------------

The Verilog-A mathematical constants can be accessed by including the
"constants.vams" header file. It is located in the standard include
directory. Recent version of Icarus Verilog (0.9.devel) automatically
add this directory to the end of the list used to find include files.
For reference the mathematical constants are::

  `M_PI        --  Pi
  `M_TWO_PI    --  2*Pi
  `M_PI_2      --  Pi/2
  `M_PI_4      --  Pi/4
  `M_1_PI      --  1/Pi
  `M_2_PI      --  2/Pi
  `M_2_SQRTPI  --  2/sqrt(Pi)
  `M_E         --  e
  `M_LOG2E     --  log base 2 of e
  `M_LOG10E    --  log base 10 of e
  `M_LN2       --  log base e of 2
  `M_LN10      --  log base e of 10
  `M_SQRT2     --  sqrt(2)
  `M_SQRT1_2   --  1/sqrt(2)

Using the Library.
------------------

Just add "-m va_math" to your iverilog command line/command file and
\`include the "constants.vams" file as needed.

Thanks
------

I would like to thank Larry Doolittle for his suggestions and
Stephen Williams for developing Icarus Verilog.
