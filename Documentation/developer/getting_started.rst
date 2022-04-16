
Getting Started as a Contributer
================================

Icarus Verilog development is centered around the github repository at
`github.com/steveicarus/iverilog <http://github.com/steveicarus/iverilog>`_.
Contributing to Icarus Verilog requires a basic knowledge of git and github,
so see the github documentation for more information. The sections below will
step you through the basics of getting the source code from github, making a
branch, and submitting a pull request for review.

Getting Icarus Verilog
----------------------

To start, you will need to clone the code. It is preferred that you use the
"ssh" method, and the ssh based clone with the command:

.. code-block:: console

  % git clone git@github.com:steveicarus/iverilog.git

This assumes that you have a github account (accounts are free) and you have
set up your ssh authentication keys. See the
`Authentication Guides here <https://docs.github.com/en/authentication>`_.

The "git clone" command will get you all the source:

.. code-block:: console

  % git clone git@github.com:steveicarus/iverilog.git
  Cloning into 'iverilog'...
  remote: Enumerating objects: 66234, done.
  remote: Counting objects: 100% (6472/6472), done.
  remote: Compressing objects: 100% (4123/4123), done.
  remote: Total 66234 (delta 2412), reused 6039 (delta 2190), pack-reused 59762
  Receiving objects: 100% (66234/66234), 27.98 MiB | 2.53 MiB/s, done.
  Resolving deltas: 100% (50234/50234), done.
  % cd iverilog/

Normally, this is enough as you are now pointing at the most current
development code, and you have implicitly created a branch "master" that
tracks the development head. However, If you want to actually be working on a
specific version, say for example version 11, the v11-branch, you checkout
that branch with the command:

.. code-block:: console

  % git checkout --track -b v11-branch origin/v11-branch

This creates a local branch that tracks the v11-branch in the repository, and
switches you over to your new v11-branch. The tracking is important as it
causes pulls from the repository to re-merge your local branch with the remote
v11-branch. You always work on a local branch, then merge only when you
push/pull from the remote repository.

Now that you've cloned the repository and optionally selected the branch you
want to work on, your local source tree may later be synced up with the
development source by using the git command:

.. code-block:: console

  % git pull
  Already up to date.

Finally, configuration files are built by the extra step:

.. code-block:: console

  % sh autoconf.sh
  Autoconf in root...
  Precompiling lexor_keyword.gperf
  Precompiling vhdlpp/lexor_keyword.gperf

You will need autoconf and gperf installed in order for the script to work.
If you get errors such as:

.. code-block:: console

  % sh autoconf.sh
  Autoconf in root...
  autoconf.sh: 10: autoconf: not found
  Precompiling lexor_keyword.gperf
  autoconf.sh: 13: gperf: not found.

You will need to install download and install the autoconf and gperf tools.

Now you are ready to configure and compile the source.

Icarus Specific Configuration Options
-------------------------------------

Icarus takes many of the standard configuration options and those will not be
described here. The following are specific to Icarus Verilog:

.. code-block:: none

  --enable-suffix[=suffix]

This option allows the user to build Icarus with a default suffix or when
provided a user defined suffix. All programs or directories are tagged with
this suffix. e.g.(iverilog-0.8, vvp-0.8, etc.). The output of iverilog will
reference the correct run time files and directories. The run time will check
that it is running a file with a compatible version e.g.(you can not run a
V0.9 file with the V0.8 run time).

A debug options is:

.. code-block:: none

  --with-valgrind

This option adds extra memory cleanup code and pool management code to allow
better memory leak checking when valgrind is available. This option is not
need when checking for basic errors with valgrind.

Compiling on Linux
------------------

(Note: You will need to install bison, flex, g++ and gcc) This is probably the
easiest step. Given that you have the source tree from the above instructions,
the compile and install is generally as simple as:

.. code-block:: console

  % ./configure
  configure: loading site script /usr/share/site/x86_64-unknown-linux-gnu
  checking build system type... x86_64-unknown-linux-gnu
  checking host system type... x86_64-unknown-linux-gnu
  checking for gcc... gcc
  checking whether the C compiler works... yes
  checking for C compiler default output file name... a.out
  checking for suffix of executables...
  [...and so on...]

  % make
  mkdir dep
  Using git-describe for VERSION_TAG
  g++ -DHAVE_CONFIG_H -I. -Ilibmisc  -Wall -Wextra -Wshadow   -g -O2 -MD -c main.cc -o main.o
  mv main.d dep/main.d
  g++ -DHAVE_CONFIG_H -I. -Ilibmisc  -Wall -Wextra -Wshadow   -g -O2 -MD -c async.cc -o async.o
  mv async.d dep/async.d
  g++ -DHAVE_CONFIG_H -I. -Ilibmisc  -Wall -Wextra -Wshadow   -g -O2 -MD -c design_dump.cc -o design_dump.o
  mv design_dump.d dep/design_dump.d
  g++ -DHAVE_CONFIG_H -I. -Ilibmisc  -Wall -Wextra -Wshadow   -g -O2 -MD -c discipline.cc -o discipline.o
  [...and so on...]

The end result is a complete build of Icarus Verilog. You can install your
compiled version with a command like this:

.. code-block:: console

  % sudo make install

Regression Tests
----------------

Icarus Verilog comes with a fairly extensive regression test suite. As of
2022, that test suite is included with the source in the "ivtest"
directory. Contained in that directory are a couple driver scripts that run
all the regression tests on the installed version of Icarus Verilog. So for
example:

.. code-block:: console

  % cd ivtest
  % ./vvp_reg.pl --strict

will run all the regression tests for the simulation engine. (This is what
most people will want to do.) You should rerun this test before submitting
patches to the developers. Also, if you are adding a new feature, you should
add test programs to the regression test suite to validate your new feature
(or bug fix.)

Note that pull requests will be required to pass these regression tests before
being merged.

Forks, Branches and Pull Requests
---------------------------------

Currently, the preferred way to submit patches to Icarus Verilog is via pull
requests.
`Pull requests <https://docs.github.com/en/github-ae@latest/pull-requests>`_
can be created from the main repository if you have write access (very few
people have write access) or more commonly from a fork, so the first step is
to create a fork that you can work with. It is easy enough to create a fork,
just go to the
`github.com/steveicarus/iverilog <http://github.com/steveicarus/iverilog>`_
page and use the "fork" button in the upper right corner. This will create
a new repository that you can clone instead of the steveicarus/iverilog
repository. You then use your local repository to create feature branches,
then submit them for inclusion in the main repository as pull
requests. Remember to `synchronize your fork
<https://docs.github.com/en/github-ae@latest/pull-requests/collaborating-with-pull-requests/working-with-forks/syncing-a-fork>`_
periodically with the main repository. This will make sure your work is based
on the latest upstream and avoid merge conflicts.

Create your patch by first creating a branch that contains your commits:

.. code-block:: console

  % git checkout -b my-github-id/branch-name

We are encouraging using this scheme for naming your branches that are
destined for pull requests. Use your github id in the branch name. So for
example:

.. code-block:: console

  % git checkout -b steveicarus/foo-feature

Do your work in this branch, then when you are ready to create a pull request,
first push the branch up to github:

.. code-block:: console

  % git push -u origin my-github-id/branch-name

Then go to github.com to create your pull request. `Create your pull request
against the "master" branch of the upstream repository
<https://docs.github.com/en/pull-requests/collaborating-with-pull-requests/proposing-changes-to-your-work-with-pull-requests/creating-a-pull-request-from-a-fork>`_,
or the version branch that you are working on. Your pull reuqest will be run
through continuous integration, and reviewed by one of the main
authors. Feedback may be offered to your PR, and once accepted, an approved
individual will merge it for you. Then you are done.

