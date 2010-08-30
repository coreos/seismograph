# Copyright (C) 2010 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

all: rootdev librootdev.so.1.0

rootdev: rootdev.c main.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ -O2 -Wall

librootdev.so.1.0: rootdev.c
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -fPIC \
          -Wl,-soname,librootdev.so.1 $< -o $@
	ln -s $@ librootdev.so.1
	ln -s $@ librootdev.so

clean:
	rm -f rootdev librootdev.so*

.PHONY: clean
