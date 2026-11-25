#!/bin/sh
# Generate the build system (configure, Makefile.in, ...) from configure.ac.
# Needs autoconf, automake and libtool.
set -e
cd "$(dirname "$0")"
mkdir -p m4
autoreconf --force --install
echo "Now run ./configure (see README.md for options) and make."
