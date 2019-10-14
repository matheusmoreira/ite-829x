# ite-829x

This program controls the LEDs present on the ITE Device(829x) keyboard
fabricated by Integrated Technology Express, Inc.
This keyboard is used on the Clevo PA70ES.
The Avell C73 is based on this model.

USB data:

    idVendor           0x048d Integrated Technology Express, Inc.
    idProduct          0x8910
    bcdDevice            0.01
    iManufacturer           1 ITE Tech. Inc.
    iProduct                2 ITE Device(829x)
    iSerial                 0

# Building

[![Build Status](https://travis-ci.com/matheusmoreira/ite-829x.svg?branch=master)](https://travis-ci.com/matheusmoreira/ite-829x)

The `hidapi-libusb` library is required by the program.

    make    # creates the ite-829x executable

# Using

    sudo ./ite-829x << LEDs
        reset
        brightness+speed 4 0
        effects 4
    LEDs
