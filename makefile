# SPDX-License-Identifier: GPL-2.0-only

model = ite-829x

$(model) : $(model).c
	$(CC) -std=c99 -Wall -Wextra -Wpedantic $(CPPFLAGS) $(CFLAGS) -lhidapi-libusb $(LDFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm $(model)
