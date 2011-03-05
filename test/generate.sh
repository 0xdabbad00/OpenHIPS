#!/bin/bash
echo - Generating spray-js.pdf
python spray-js.py > spray-js.pdf
echo - Generating spray-inline_image.pdf
python spray-inline_image.py > spray-inline_image.pdf
echo - Removing old SwfSpray.swf
rm SwfSpray.swf
echo - Generating SwfSpray.swf
# I know I should do this more cleanly
/cygdrive/c/Program\ Files\ \(x86\)/Motion-Twin/haxe/haxe SwfSpray -swf9 SwfSpray.swf
echo - Generating spray-swf.pdf
python spray-swf.py SwfSpray.swf swfspray_support.txt > spray-swf.pdf
