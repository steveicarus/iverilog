#
#    This source code is free software; you can redistribute it
#    and/or modify it in source code form under the terms of the GNU
#    Library General Public License as published by the Free Software
#    Foundation; either version 2 of the License, or (at your option)
#    any later version.
#
#    This program is distributed in the hope that it will be useful,
#    but WITHOUT ANY WARRANTY; without even the implied warranty of
#    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#    GNU Library General Public License for more details.
#
#    You should have received a copy of the GNU Library General Public
#    License along with this program; if not, write to the Free
#    Software Foundation, Inc.,
#    59 Temple Place - Suite 330
#    Boston, MA 02111-1307, USA
#
#ident "$Id: enable_vvp32.mk,v 1.1 2003/10/09 01:07:20 steve Exp $"
#

all32: bin32 bin32/cadpli.vpl

bin32:
	mkdir bin32

bin32/%.o: %.c
	$(CC) -m32 -Wall -I$(srcdir) -I$(srcdir)/.. $(CPPFLAGS) $(CFLAGS) -MD -c $< -o $@

bin32/cadpli.vpl: $(addprefix bin32/,$O) ../vvp/bin32/libvpi.a ../libveriuser/bin32/libveriuser.o
	$(CC) $(SHARED) -m32 -o $@ $(addprefix bin32/,$O) ../libveriuser/bin32/libveriuser.o ../vvp/bin32/libvpi.a

install32: all32 $(vpidir32)/cadpli.vpl

$(vpidir32)/cadpli.vpl: bin32/cadpli.vpl
	$(INSTALL_PROGRAM) bin32/cadpli.vpl $(vpidir32)/cadpli.vpl

uninstall32:
	rm -f $(vpidir32)/cadpli.vpl
