# Copyright (C) 2010 The Chromium OS Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

OUT = $(CURDIR)
$(shell mkdir -p $(OUT))

all: $(OUT)/rootdev $(OUT)/librootdev.so.1.0

$(OUT)/rootdev: rootdev.c main.c
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ -O2 -Wall

$(OUT)/librootdev.so.1.0: rootdev.c
	$(CC) $(CFLAGS) $(LDFLAGS) -shared -fPIC \
		-Wl,-soname,librootdev.so.1 $< -o $@
	ln -s $(@F) $(OUT)/librootdev.so.1
	ln -s $(@F) $(OUT)/librootdev.so

clean:
	rm -f $(OUT)/rootdev $(OUT)/librootdev.so*

.PHONY: clean
