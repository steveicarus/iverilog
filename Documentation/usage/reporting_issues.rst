
Reporting Issues
================

The developers of and contributers to Icarus Verilog use github to track
issues and to create patches for the product. If you believe you have found a
problem, use the Issues tracker at the
`Icarus Verilog github page <https://github.com/steveicarus/iverilog>`_.

You may browse the bugs database for existing
bugs that may be related to yours. You might find that your bug has
already been fixed in a later release or snapshot. If that's the case,
then you are set.

On the main page, you will find a row of selections near the top. Click the
`Issues <https://github.com/steveicarus/iverilog/issues>`_ link to get to the
list of issues, open and closed. You will find a friendly green button where
you can create a new issue. You will be asked to create a title for your
issue, and to write a detailed description of your issue. Please include
enough information that anyone who sees your issue can understand and
reproduce it.

Good Issue Reporting
--------------------

Before an error can be fixed, one needs to understand what the problem
is. Try to explain what is wrong and why you think it is wrong. Please
try to include sample code that demonstrates the problem.

One key characteristic of a well reported issue is a small sample program that
demonstrates the issue. The smaller the better. No developer wants to wade
through hundreds of lines of working Verilog to find the few lines that cause
trouble, so if you can get it down to a 10 line sample program, then your
issue will be far more likely to be addressed.

Also, include the command line you use to invoke the compiler. For
example::

	iverilog -o foo.out -tvvp foo.v
	iverilog foo.vl -s starthere

Be prepared to have a conversation about your issue. More often then you would
expect, the issue turns out to be a bug in your program, and the person
looking into your issue may point out a bug in your code. You learn something,
and we all win. We are not always correct, though, so if we are incorrect,
help us see our error, if that's appropriate. If we don't understand what your
issue is, we will label your issue with a "Need info" label, and if we never
hear from you again, your issue may be closed summarily.

If you can submit a complete, working program that we can use in the
regression test suite, then that is the best. Check out the existing tests in
the regression test suite to see how they are structured. If you have a
complete test that can go into the test suite, then that saves everyone a lot
of grief, and again you increase the odds that your issue will be addressed.

How To Create A Pull Request
----------------------------

Bug reports with patches/PRs are very welcome. Please also add a new test case in the regression test suite to prevent the bug from reappearing.

If you are editing the source, you should be using the latest
version from git. Please see the developer documentation for more
detailed instructions -- :doc:`Getting Started as a Contributer <getting_started>` .

COPYRIGHT ISSUES

Icarus Verilog is Copyright (c) 1998-2018 Stephen Williams except
where otherwise noted. Minor patches are covered as derivative works
(or editorial comment or whatever the appropriate legal term is) and
folded into the rest of ivl. However, if a submission can reasonably
be considered independently copyrightable, it's yours and I encourage
you to claim it with appropriate copyright notices. This submission
then falls under the "otherwise noted" category.

I must insist that any copyright material submitted for inclusion
include the GPL license notice as shown in the rest of the source.
