Designed for the Flinders & Hackerspace at Tonsley (FH@T) badge.

It is based on the PIC32MX250F128D and uses UECIDE to supply an Arduino
like environment.

Original code was supplied by Damian Kleiss who also designed the board.

More information: http://hackerspace-adelaide.org.au/wiki/FH@T_Badge

Note that UECIDE *requires* that the directory has the same name as
the ino file. If the directory name differs you will see a Java null
pointer exception when trying to build. This means that you will have
to rename the directory if you get the zip file, or specify it
explicitly when cloning.

For the zip file:
Download https://github.com/DanielO/FH-TBadge/archive/master.zip (will
end up as FH-TBadge-master.zip)
Unzip it (will create a directory called FH-TBadge-master)
Rename FH-TBadge-master to FH@TBadge

For git clone:
git clone https://github.com/DanielO/FH-TBadge.git FH@TBadge

(or rename the FH-TBadge to FH@TBadge after cloning)
