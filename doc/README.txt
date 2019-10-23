The default software license is the Boost license, BOOST_LICENSE.txt.  In particular, everything in the Zaimoni.STL subdirectory is Boost (agrees with Z.C++).

Iskandria's build system is a Makefile system taken from Vanilla Angband, which in turn took it from a now-presumed-dead semi-commercial website.  GNU make is assumed;

The minimum C++ standard for building Iskandria is C++11.

There is a dependency on the Simple and Fast Media Library (SFML< https://www.sfml-dev.org/).  Some notes on doing a local build:
* It's C++.   Thus, one build per compiler of interest.
** e.g.: /usr/local/include/SMFL for headers, /MingW32/lib for the static libraries
** for now, relevant DLLs go in the same directory as the executable.  The static build option isn't fully prototyped yet.
** SFML builds without warnings in C++98.  The only warnings when building in C++11 are from std::auto_ptr being deprecated.  C++14 hasn't been tested yet.
* Build system for SFML is CMake.  Unbreaking stock CMake for MingW with MingW's sh on path is trivial, but this is a won't-fix (documented on CMake mailing lists).
* The main library is zlib/png license.

There is a dependency on the Boost library.  We only need the headers.
* This project originated, and uses, the Boost License.

Files that are not Boost are to be itemized below:

fonts/unifont....ttf: GPLv3; master website https://savannah.gnu.org/projects/unifont/ .
Assumption is that the fonts are precompiled binaries.

