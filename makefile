# SPDX-License-Identifier: GPL-2.0-only

CPPFLAGS ?= -Wall -Wextra -Wpedantic -Wno-unused-parameter

model = ite-829x

$(model) : $(model).c cmd.c
	$(CC) -std=c99 $(CPPFLAGS) $(CFLAGS) -lhidapi-libusb $(LDFLAGS) -o $@ $<

.PHONY: clean all
.DEFAULT_GOAL = all
all: $(model)
clean:
	rm -f $(model)
