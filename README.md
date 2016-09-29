# seismograph -- tools for producing CoreOS images

## cgpt

A partitioning tool.

## rootdev

Performs operations to discover and annotate the root block device

Originally inspired by the old rdev utility from util-linux but has been
completely rewritten since then. git://git.debian.org/~lamont/util-linux.git /
717db2c8177203fe242ea35b31bc312abe9aa3c9

- Provides core functionality in a library: librootdev
- Walks sysfs to discover the block devices
- Supports resolving through to /sys/block/XXX/slaves/*/dev devices
- Will test and, optionally, symlink to the /dev entry for standard devices.
- Is testable.
