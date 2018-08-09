# frabenu - Framebuffer Menu

frabenu is a simple linux program that shows a graphical menu and returns the choice in a script friendly way.

## Motivation

After preparing my Raspberry Pi with Kodi and RetroPie, I wished to have a menu to choose between
Desktop, Kodi, RetroPie or just the shell.
I found a shell script that works, but I had a graphical menu in my mind
that can be controlled by keyboard and gamepad.
Nothing I found matched my wishes, so frabenu was born.

## Implementation

frabenu is mostly based on fbida (https://www.kraxel.org/cgit/fbida/) (GPL-2.0+).

## Build

You need cmake and some libraries at minimum:

    sudo apt-get install cmake libpixman-1-dev

If you want to support all file formats you may enter:

    sudo apt-get install libjpeg-dev libexif-dev libpng-dev libtiff-dev

After download or clone frabenu you can create a build directory and run cmake and make like:

    mkdir build
    cd build
    cmake ..
    make

## Usage

The only build result you need is `frabenu`. Copy it wherever you want.
Create your menu images using your favoured tools.
You will need one picture for every menu item and all filenames must be identically except of some numbers.
Be sure to start numbering with 1.

Some Examples:

A menu with one row or one column of 4 items:

 * `MyMenu_1.png`
 * `MyMenu_2.png`
 * `MyMenu_3.png`
 * `MyMenu_4.png`

A menu with 2 x 3 items: 

 * `MyMenu_1_1.png`
 * `MyMenu_2_1.png`
 * `MyMenu_3_1.png`
 * `MyMenu_1_2.png`
 * `MyMenu_2_2.png`
 * `MyMenu_3_2.png`

Now you can call `fabenu`.

A menu with one row of 4 items:

    frabenu 4x1 MyMenu_%x.png

A menu with one column of 4 items:

    frabenu 1x4 MyMenu_%y.png

A menu with 2 rows and 3 colums:

    frabenu 3x2 MyMenu_%x_%y.png

A menu with 3 rows and 2 colums:

    frabenu 2x3 MyMenu_%y_%x.png

The minimum layout is 1x1 (just an image viewer).
The maximum layout is 9x9.

There are four scrolling modes (1-4). Use the `-s` option to specify it, default is 1.

    frabenu -s4 2x3 MyMenu_%y_%x.png

Just try it to understand the modes.

There is also an [example script](example/menu.sh) to show you who to use frabenu.

## License

frabenu is distributed under [GPLv2 or later](COPYING)