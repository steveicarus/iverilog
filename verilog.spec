Summary: Icarus Verilog 0.8
Name: verilog08
Version: 0.8.7
Release: 0
License: GPL
Group: Applications/Engineering
Source: ftp://icarus.com/pub/eda/verilog/v0.8/verilog-0.8.7.tar.gz
URL: http://www.icarus.com/eda/verilog/index.html
Packager: Stephen Williams <steve@icarus.com>

BuildRoot: %{_tmppath}/%{name}-%{version}-%{release}-root

BuildRequires: 	gcc-c++, zlib-devel, bison, flex, gperf, termcap
BuildRequires:	libbz2-devel, bzip2, readline-devel
%ifarch x86_64
BuildRequires:	glibc-devel-32bit, libbz2-1-32bit, zlib-devel-32bit, glibc-32bit
BuildRequires:	termcap-32bit readline-devel-32bit
%endif

# This provides tag allows me to use a more specific name for things
# that actually depend on me, Icarus Verilog.
Provides: iverilog

%description
Icarus Verilog is a Verilog compiler that generates a variety of
engineering formats, including simulation. It strives to be true
to the IEEE-1364 standard.

%prep
%setup -n verilog-0.8.7

%build
%ifarch x86_64
./configure --prefix=/usr --mandir='$(prefix)/share/man' libdir64='$(prefix)/lib64' vpidir1=vpi64 vpidir2=. --enable-vvp32 --enable-suffix
%else
./configure --prefix=/usr --mandir='$(prefix)/share/man' --enable-suffix
%endif
make CXXFLAGS=-O

%install
make prefix=$RPM_BUILD_ROOT/usr install

%files

%attr(-,root,root) /usr/share/man/man1/iverilog-0.8.1.gz
%attr(-,root,root) /usr/share/man/man1/iverilog-fpga-0.8.1.gz
%attr(-,root,root) /usr/share/man/man1/iverilog-vpi-0.8.1.gz
%attr(-,root,root) /usr/share/man/man1/vvp-0.8.1.gz

%attr(-,root,root) /usr/bin/iverilog-0.8
%attr(-,root,root) /usr/bin/iverilog-vpi-0.8
%attr(-,root,root) /usr/bin/vvp-0.8
%attr(-,root,root) /usr/lib/ivl-0.8/ivl
%attr(-,root,root) /usr/lib/ivl-0.8/ivlpp
%attr(-,root,root) /usr/lib/ivl-0.8/null.tgt
%attr(-,root,root) /usr/lib/ivl-0.8/null.conf
%attr(-,root,root) /usr/lib/ivl-0.8/null-s.conf
%attr(-,root,root) /usr/lib/ivl-0.8/vvp.tgt
%attr(-,root,root) /usr/lib/ivl-0.8/vvp.conf
%attr(-,root,root) /usr/lib/ivl-0.8/vvp-s.conf
%attr(-,root,root) /usr/lib/ivl-0.8/fpga.tgt
%attr(-,root,root) /usr/lib/ivl-0.8/fpga.conf
%attr(-,root,root) /usr/lib/ivl-0.8/fpga-s.conf
%attr(-,root,root) /usr/lib/ivl-0.8/edif.tgt
%attr(-,root,root) /usr/lib/ivl-0.8/edif.conf
%attr(-,root,root) /usr/lib/ivl-0.8/edif-s.conf
%attr(-,root,root) /usr/lib/ivl-0.8/xnf.conf
%attr(-,root,root) /usr/lib/ivl-0.8/xnf-s.conf
%ifarch x86_64
%attr(-,root,root) /usr/bin/vvp32-0.8
%attr(-,root,root) /usr/lib/ivl-0.8/vpi64/system.vpi
%attr(-,root,root) /usr/lib/ivl-0.8/vpi64/cadpli.vpl
%attr(-,root,root) /usr/lib64/libvpi-0.8.a
%attr(-,root,root) /usr/lib64/libveriuser-0.8.a
%endif
%attr(-,root,root) /usr/lib/ivl-0.8/system.sft
%attr(-,root,root) /usr/lib/ivl-0.8/system.vpi
%attr(-,root,root) /usr/lib/ivl-0.8/cadpli.vpl
%attr(-,root,root) /usr/lib/libvpi-0.8.a
%attr(-,root,root) /usr/lib/libveriuser-0.8.a
%attr(-,root,root) /usr/include/iverilog-0.8/ivl_target.h
%attr(-,root,root) /usr/include/iverilog-0.8/vpi_user.h
%attr(-,root,root) /usr/include/iverilog-0.8/acc_user.h
%attr(-,root,root) /usr/include/iverilog-0.8/veriuser.h
%attr(-,root,root) /usr/include/iverilog-0.8/_pli_types.h
