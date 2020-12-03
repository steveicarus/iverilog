# MSYS2 build recipe

This subdir contains a [PKGBUILD](https://wiki.archlinux.org/index.php/PKGBUILD) recipe for building iverilog on [MSYS2](https://www.msys2.org/). MSYS2 is a collection of tools and libraries for Windows, which is closely based on the packaging approach in [Arch Linux](https://www.archlinux.org/). Precisely, the package manager in MSYS2 is a port of [pacman](https://wiki.archlinux.org/index.php/pacman). Therefore, the structure of PKGBUILD recipes in MSYS2 is very similar to packages in Arch Linux and the build script (`makepkg-mingw`) is a port of [makepkg](https://wiki.archlinux.org/index.php/makepkg).

Other than that, PKGBUILD files are shell scripts containing some specific functions (build, package, check, etc.) and metadata (variables). The build system takes care of dependencies, creating temporary directories, generating a tarball, etc. Therefore, the recommended approach for building iverilog on Windows is the following:

```sh
# Install the toolchain
pacman -S mingw-w64-x86_64-toolchain
# and/or mingw-w64-i686-toolchain

# Retrieve iverilog sources. Optionally, retrieve a tarball, or an specific branch/version.
git clone https://github.com/steveicarus/iverilog
cd iverilog

# Call makepkg-mingw from subdir 'msys2'. It will build, check and package iverilog.
cd msys2
MINGW_INSTALLS=mingw64 makepkg-mingw --noconfirm --noprogressbar -sCLf
# or, set the envvar to 'mingw32', or unset it for building both packages at the same time

# Optionally, install the tarball(s)/package(s). Or just distribute it/them.
pacman -U --noconfirm *.zst
```

NOTE: the continuous integration workflow in [github.com/steveicarus](https://github.com/steveicarus) uses the procedure above for building iverilog on MINGW32 and MINGW64 each time a commit is pushed or a Pull Request is updated. The two generated packages are uploaded as artifacts. Hence, users willing to test *nightly* builds or specific features, can download and install the tarballs from the corresponding CI run.

Nevertheless, the content of functions `build` and `check` in the PKGBUILD file should be familiar for any user willing to build iverilog *manually*. Those can be executed in a shell (ignoring makepkg), as long as the few envvars are properly defined.

HINT: this document explains the most straightforward and automatec solution for building iverilog on Windows. However, intermediate and advanced users might want to check [iverilog.fandom.com/wiki/Installation_using_MSYS2](https://iverilog.fandom.com/wiki/Installation_using_MSYS2) for some specific tweaks, such as using a custom prefix for iverilog executables, or using MSYS2 packages/tarballs outside of MSYS2.
