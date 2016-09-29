#!/bin/sh
set -ex
mkdir -p m4
autoreconf --force --install --symlink
