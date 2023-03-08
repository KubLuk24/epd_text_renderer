# About
This is a work-in-progress text renderer for WBR (WhiteBlackRed) E-Paper displays from Waveshare using SPI.

The code is far from finished an I will be updating it soon.

Planned things are:

- [x] Basic text render support
- [ ] Support for multi-line renders
- [ ] Text scaling option
- [ ] Color rendering

# How it works

Waveshare displays work on a simple basis. You have a set width and a set height, these are the borders of your display.
The display begins setting pixel values at 0, 0. This means in the bottom right corner if you orient your display vertically (by it's height parameter).

After initializing the display and sending a command to start accepting data, you send in 8 bits of data every time, this allows you to set the color values of 8 pixels at a time. We send them as hexadecimal values instead of pure binary, so:

`0x00`: 0000 0000

`0xFF`: 1111 1111

Where `0` is a colored pixel and `1` is a white pixel.

We have to fill in all values before starting the display refresh.

## Why is this important?

Arduino has a very limited amount of RAM, so every byte counts. This code is made to be as RAM efficient as my smooth brain can make it.
If you feel like you can improve this code, feel free to open a pull request.
I myself am an amateur when it comes to C++ programming so any help is appreciated.
