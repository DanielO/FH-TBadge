#!/usr/bin/env python

import PIL
import sys

def extract(infile, outfile, name):
    im = PIL.Image.open(infile)
    s = PIL.ImageSequence.Iterator(im)

    outfile.write("/* This file is autogenerated - any changes will be lost */\n\n")
    outfile.write("static const uint8_t PROGMEM %s[] = {\n" % (name))

    framedurs = []

    framenum = 1
    for f in s:
        framedurs.append(f.info['duration'])
        outfile.write("\t")
        frame = f.tobytes()
        assert len(frame) == 64 # Expecting 8x8 1 byte per pixel
        for row in range(8):
            data = 0
            for col in range(8):
                if frame[row * 8 + col] == '\x00': # Colour 0 = on
                    data |= 1 << col
            outfile.write("0x%02x, " % (data))
        outfile.write("// Frame %d\n" % (framenum))
        framenum += 1
    outfile.write("};\n")
    outfile.write("static const uint8_t PROGMEM %s_durs[] = {\n" % (name))
    n = 12
    for i in range(0, len(framedurs), n):
        sub = framedurs[i:i + n]
        outfile.write("\t");
        for j in sub:
            outfile.write("%d, " % (j))
        outfile.write("\n");
    outfile.write("};\n");

def main():
    if len(sys.argv) != 4:
        print "Bad usage"
        print "gifgen.py source.gif dest.h varname"
        print ""
        print "Source GIF must be 8x8, colour 0 pixels will be on"
        print "For Fh@t badge source, use.."
        print "gifgen.py source.gif anim.h anim"
        sys.exit(1)
    infn = sys.argv[1]
    outfn = sys.argv[2]
    name = sys.argv[3]
    if infn == '-':
        infile = sys.stdin
    else:
        infile = file(infn)
    if outfn == '-':
        outfile = sys.stdout
    else:
        outfile = file(outfn, 'w')
    extract(infile, outfile, name)

if __name__ == '__main__':
    main()