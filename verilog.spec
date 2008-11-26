#norootforbuild
#
%define rev_date 20081118
#
#
Summary: Icarus Verilog
Name: verilog
Version: 0.9.0.%{rev_date}
Release: 0
License: GPL
Group: Productivity/Scientific/Electronics
Source: verilog-%{rev_date}.tar.gz
URL: http://www.icarus.com/eda/verilog/index.html
Packager: Stephen Williams <steve@icarus.com>

BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

BuildRequires: gcc-c++, zlib-devel, bison, flex, gperf, readline-devel

# This provides tag allows me to use a more specific name for things
# that actually depend on me, Icarus Verilog.
Provides: iverilog

%description
Icarus Verilog is a Verilog compiler that generates a variety of
engineering formats, including simulation. It strives to be true
to the IEEE-1364 standard.

%prep
%setup -n verilog-%{rev_date}

%build
%{configure}
make CXXFLAGS=-O

%install
%{makeinstall}

%clean
rm -rf $RPM_BUILD_ROOT

%files

%attr(-,root,root) %doc COPYING README.txt BUGS.txt QUICK_START.txt ieee1364-notes.txt mingw.txt swift.txt netlist.txt t-dll.txt vpi.txt cadpli/cadpli.txt
%attr(-,root,root) %doc examples/*

%attr(-,root,root) %{_mandir}/man1/iverilog.1.gz
%attr(-,root,root) %{_mandir}/man1/iverilog-vpi.1.gz
%attr(-,root,root) %{_mandir}/man1/vvp.1.gz

%attr(-,root,root) %{_bindir}/iverilog
%attr(-,root,root) %{_bindir}/iverilog-vpi
%attr(-,root,root) %{_bindir}/vvp
%attr(-,root,root) %{_libdir}/ivl/ivl
%attr(-,root,root) %{_libdir}/ivl/ivlpp
%attr(-,root,root) %{_libdir}/ivl/null.tgt
%attr(-,root,root) %{_libdir}/ivl/null.conf
%attr(-,root,root) %{_libdir}/ivl/null-s.conf
%attr(-,root,root) %{_libdir}/ivl/stub.tgt
%attr(-,root,root) %{_libdir}/ivl/stub.conf
%attr(-,root,root) %{_libdir}/ivl/stub-s.conf
%attr(-,root,root) %{_libdir}/ivl/vvp.tgt
%attr(-,root,root) %{_libdir}/ivl/vvp.conf
%attr(-,root,root) %{_libdir}/ivl/vvp-s.conf
%attr(-,root,root) %{_libdir}/ivl/vhdl.tgt
%attr(-,root,root) %{_libdir}/ivl/vhdl.conf
%attr(-,root,root) %{_libdir}/ivl/vhdl-s.conf
%attr(-,root,root) %{_libdir}/ivl/system.sft
%attr(-,root,root) %{_libdir}/ivl/system.vpi
%attr(-,root,root) %{_libdir}/ivl/va_math.sft
%attr(-,root,root) %{_libdir}/ivl/va_math.vpi
%attr(-,root,root) %{_libdir}/ivl/v2005_math.sft
%attr(-,root,root) %{_libdir}/ivl/v2005_math.vpi
%attr(-,root,root) %{_libdir}/ivl/cadpli.vpl
%attr(-,root,root) %{_libdir}/libvpi.a
%attr(-,root,root) %{_libdir}/libveriuser.a
%attr(-,root,root) %{_libdir}/ivl/include/constants.vams
%attr(-,root,root) %{_libdir}/ivl/include/disciplines.vams
%attr(-,root,root) /usr/include/iverilog/ivl_target.h
%attr(-,root,root) /usr/include/iverilog/vpi_user.h
%attr(-,root,root) /usr/include/iverilog/acc_user.h
%attr(-,root,root) /usr/include/iverilog/veriuser.h
%attr(-,root,root) /usr/include/iverilog/_pli_types.h

%changelog -n verilog
* Tue Nov 25 2008 - steve@icarus.com
- Move header files frim /verilog/ to /iverilog/

* Tue Nov 18 2008 - steve@icarus.com
- New snapshot 20080905

* Fri Sep 03 2008 - steve@icarus.com
- New snapshot 20080905

* Sat Aug 30 2008 - steve@icarus.com
- Add vhdl target files
- Add V/AMS header files.

* Fri Jan 25 2008 - steve@icarus.com
- Removed vvp32 support for x86_64 build.

* Sun Feb 28 2007 - steve@icarus.com
- Added formatting suitable for openSUSE packaging.
