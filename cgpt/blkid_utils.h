// Copyright (c) 2014 CoreOS Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <blkid/blkid.h>

char * dev_to_wholedevname(blkid_dev dev);
int dev_to_partno(blkid_dev dev);
