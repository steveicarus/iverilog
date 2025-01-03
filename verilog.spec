#norootforbuild
#
%define major 13
%define minor 0
%define rev_date 20250103
# Normally, the suff-ix is %nil, meaning the suffix is to not be used.
# But if the builder wants to make a suffixed package, he may set this
# to a value (i.e. -test) to cause suffixes to be put in all the right
# places.
%define suff %nil
#
#
Summary: Icarus Verilog
Name: verilog%{suff}
Version: %{major}.%{minor}.%{rev_date}
Release: 0
License: GPL
Group: Productivity/Scientific/Electronics
Source: verilog%{suff}-%{rev_date}.tar.gz
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
%setup -n verilog%{suff}-%{rev_date}

%build
if test X%{suff} != X
then
    %{configure} --enable-suffix=%{suff}
else
    %{configure}
fi
make CXXFLAGS=-O

%install
%if 0%{?suse_version}
%{makeinstall}
%else
make DESTDIR=$RPM_BUILD_ROOT install
%endif

%clean
rm -rf $RPM_BUILD_ROOT

%files

%attr(-,root,root) %doc COPYING README.txt BUGS.txt QUICK_START.txt ieee1364-notes.txt mingw.txt swift.txt netlist.txt t-dll.txt vpi.txt cadpli/cadpli.txt
%attr(-,root,root) %doc examples/*

%attr(-,root,root) %{_mandir}/man1/iverilog%{suff}.1.gz
%attr(-,root,root) %{_mandir}/man1/iverilog-vpi%{suff}.1.gz
%attr(-,root,root) %{_mandir}/man1/vvp%{suff}.1.gz

%attr(-,root,root) %{_bindir}/iverilog%{suff}
%attr(-,root,root) %{_bindir}/iverilog-vpi%{suff}
%attr(-,root,root) %{_bindir}/vvp%{suff}
%attr(-,root,root) %{_libdir}/ivl%{suff}/ivl
%attr(-,root,root) %{_libdir}/ivl%{suff}/ivlpp
%attr(-,root,root) %{_libdir}/ivl%{suff}/vhdlpp
%attr(-,root,root) %{_libdir}/ivl%{suff}/blif.tgt
%attr(-,root,root) %{_libdir}/ivl%{suff}/blif.conf
%attr(-,root,root) %{_libdir}/ivl%{suff}/blif-s.conf
%attr(-,root,root) %{_libdir}/ivl%{suff}/null.tgt
%attr(-,root,root) %{_libdir}/ivl%{suff}/null.conf
%attr(-,root,root) %{_libdir}/ivl%{suff}/null-s.conf
%attr(-,root,root) %{_libdir}/ivl%{suff}/sizer.tgt
%attr(-,root,root) %{_libdir}/ivl%{suff}/sizer.conf
%attr(-,root,root) %{_libdir}/ivl%{suff}/sizer-s.conf
%attr(-,root,root) %{_libdir}/ivl%{suff}/stub.tgt
%attr(-,root,root) %{_libdir}/ivl%{suff}/stub.conf
%attr(-,root,root) %{_libdir}/ivl%{suff}/stub-s.conf
%attr(-,root,root) %{_libdir}/ivl%{suff}/vvp.tgt
%attr(-,root,root) %{_libdir}/ivl%{suff}/vvp.conf
%attr(-,root,root) %{_libdir}/ivl%{suff}/vvp-s.conf
%attr(-,root,root) %{_libdir}/ivl%{suff}/vhdl.tgt
%attr(-,root,root) %{_libdir}/ivl%{suff}/vhdl.conf
%attr(-,root,root) %{_libdir}/ivl%{suff}/vhdl-s.conf
%attr(-,root,root) %{_libdir}/ivl%{suff}/vlog95.tgt
%attr(-,root,root) %{_libdir}/ivl%{suff}/vlog95.conf
%attr(-,root,root) %{_libdir}/ivl%{suff}/vlog95-s.conf
%attr(-,root,root) %{_libdir}/ivl%{suff}/pcb.tgt
%attr(-,root,root) %{_libdir}/ivl%{suff}/pcb.conf
%attr(-,root,root) %{_libdir}/ivl%{suff}/pcb-s.conf
%attr(-,root,root) %{_libdir}/ivl%{suff}/system.sft
%attr(-,root,root) %{_libdir}/ivl%{suff}/system.vpi
%attr(-,root,root) %{_libdir}/ivl%{suff}/va_math.sft
%attr(-,root,root) %{_libdir}/ivl%{suff}/va_math.vpi
%attr(-,root,root) %{_libdir}/ivl%{suff}/v2005_math.sft
%attr(-,root,root) %{_libdir}/ivl%{suff}/v2005_math.vpi
%attr(-,root,root) %{_libdir}/ivl%{suff}/v2009.sft
%attr(-,root,root) %{_libdir}/ivl%{suff}/v2009.vpi
%attr(-,root,root) %{_libdir}/ivl%{suff}/vhdl_sys.sft
%attr(-,root,root) %{_libdir}/ivl%{suff}/vhdl_sys.vpi
%attr(-,root,root) %{_libdir}/ivl%{suff}/vhdl_textio.sft
%attr(-,root,root) %{_libdir}/ivl%{suff}/vhdl_textio.vpi
%attr(-,root,root) %{_libdir}/ivl%{suff}/vpi_debug.vpi
%attr(-,root,root) %{_libdir}/ivl%{suff}/cadpli.vpl
%attr(-,root,root) %{_libdir}/libvpi%{suff}.a
%attr(-,root,root) %{_libdir}/libveriuser%{suff}.a
%attr(-,root,root) %{_libdir}/ivl%{suff}/include/constants.vams
%attr(-,root,root) %{_libdir}/ivl%{suff}/include/disciplines.vams
%attr(-,root,root) /usr/include/iverilog%{suff}/ivl_target.h
%attr(-,root,root) /usr/include/iverilog%{suff}/vpi_user.h
%attr(-,root,root) /usr/include/iverilog%{suff}/sv_vpi_user.h
%attr(-,root,root) /usr/include/iverilog%{suff}/acc_user.h
%attr(-,root,root) /usr/include/iverilog%{suff}/veriuser.h
%attr(-,root,root) /usr/include/iverilog%{suff}/_pli_types.h

%changelog -n verilog
* Thu Jul 25 2013 - steve@icarus.com
- Add blif code generator files.

* Wed Feb 25 2009 - steve@icarus.com
- Handle a package suffix if desired.

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
