# SPDX-License-Identifier: GPL-2.0-only

CPPFLAGS ?= -Wall -Wextra -Wpedantic

model = ite-829x

$(model) : $(model).c
	$(CC) -std=c99 $(CPPFLAGS) $(CFLAGS) -lhidapi-libusb $(LDFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm $(model)
