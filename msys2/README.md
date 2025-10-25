# MSYS2 build recipe

This directory contains a [PKGBUILD](https://wiki.archlinux.org/index.php/PKGBUILD)
recipe for building Icarus Verilog in [MSYS2](https://www.msys2.org/). MSYS2
is a collection of tools and libraries for Windows that is closely based on
the packaging approach in [Arch Linux](https://www.archlinux.org/). The package
manager in MSYS2 is a port of [pacman](https://wiki.archlinux.org/index.php/pacman).
Therefore the structure of PKGBUILD recipes in MSYS2 is very similar to that in
Arch Linux and the build script (`makepkg-mingw`) is a port of
[makepkg](https://wiki.archlinux.org/index.php/makepkg).

Other than that, PKGBUILD files are shell scripts containing some specific
functions (build, package, check, etc.) and metadata (variables). The build
system takes care of dependencies, creating temporary directories, generating
a tarball, etc. Therefore, the recommended approach for building Icarus Verilog
on Windows is the following:

```sh
# Install the base development tools
pacman -S base-devel

# Retrieve the Icarus Verilog sources. Optionally, retrieve a tarball, or a
# specific branch/version.
git clone https://github.com/steveicarus/iverilog
cd iverilog

# Call makepkg-mingw from the directory 'msys2'. It will install dependencies,
# build, check, and package Icarus Verilog.
cd msys2
makepkg-mingw --noconfirm --noprogressbar -sCLf

# Optionally, install the tarball(s)/package(s). Or just distribute it/them.
pacman -U --noconfirm *.zst
```

Additional configuration options can be passed to the configuration step
by setting the environment variable `IVL_CONFIG_OPTIONS` before calling
`makepkg-mingw`, e.g.
```sh
export IVL_CONFIG_OPTIONS=-"-enable-suffix=-devel --enable-libvvp"
```

NOTE: the continuous integration workflow in [github.com/steveicarus/iverilog]
(https://github.com/steveicarus/iverilog) uses the above procedure for building
Icarus Verilog for Windows each time a commit is pushed or a pull request is
updated. The generated packages are uploaded as artifacts. Hence, users willing
to test *development* builds or specific features, can download and install the
tarballs from the corresponding CI run.

Nevertheless, the content of functions `build` and `check` in the PKGBUILD file
should be familiar to any user willing to build iverilog *manually*. Those can
be executed in a shell (ignoring makepkg), as long as the few environment
variables are properly defined.
