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

The `hidapi-libusb` library is required by the program.

    make    # creates the ite-829x executable

# Using

    sudo ./ite-829x       # turns off LEDs
    sudo ./ite-829x 0     # turns off LEDs
    sudo ./ite-829x 2     # Clevo Control Center, Brightness 1
    sudo ./ite-829x 4     # Clevo Control Center, Brightness 2
    sudo ./ite-829x 6     # Clevo Control Center, Brightness 3
    sudo ./ite-829x 10    # Clevo Control Center, Brightness 4
