model = ite-829x

$(model) : $(model).c
	gcc -std=c99 -Wall -Wextra -Wpedantic -lhidapi-libusb -o $@ $<
