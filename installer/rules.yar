/*
YARA 1.5 rules OpenHIPS
Author: 0xdabbad00
Based on Metasploit shell code's
*/

rule shellcodeBindTcp
{
	meta:
		sourceOrg = "metasploit"
		sourcePath = "msf3\\modules\\payloads\\singles\\windows\\shell_bind_tcp.rb"
	strings:
	/* Offsets 	'LPORT'    => [ 201, 'n' ],
				'EXITFUNC' => [ 311, 'V' ]
	*/
		$string = { FC E8 89 00 00 00 60 89 E5 31 D2 64 8B 52 30 8B  52 0C 8B 52 14 8B 72 28 0F B7 4A 26 31 FF 31 C0    AC 3C 61 7C 02 2C 20 C1 CF 0D 01 C7 E2 F0 52 57    8B 52 10 8B 42 3C 01 D0 8B 40 78 85 C0 74 4A 01    D0 50 8B 48 18 8B 58 20 01 D3 E3 3C 49 8B 34 8B    01 D6 31 FF 31 C0 AC C1 CF 0D 01 C7 38 E0 75 F4    03 7D F8 3B 7D 24 75 E2 58 8B 58 24 01 D3 66 8B    0C 4B 8B 58 1C 01 D3 8B 04 8B 01 D0 89 44 24 24    5B 5B 61 59 5A 51 FF E0 58 5F 5A 8B 12 EB 86 5D    68 33 32 00 00 68 77 73 32 5F 54 68 4C 77 26 07    FF D5 B8 90 01 00 00 29 C4 54 50 68 29 80 6B 00    FF D5 50 50 50 50 40 50 40 50 68 EA 0F DF E0 FF    D5 89 C7 31 DB 53 68 02 00 ?? ?? 89 E6 6A 10 56    57 68 C2 DB 37 67 FF D5 53 57 68 B7 E9 38 FF FF    D5 53 53 57 68 74 EC 3B E1 FF D5 57 89 C7 68 75    6E 4D 61 FF D5 68 63 6D 64 00 89 E3 57 57 57 31    F6 6A 12 59 56 E2 FD 66 C7 44 24 3C 01 01 8D 44    24 10 C6 00 44 54 50 56 56 56 46 56 4E 56 56 53    56 68 79 CC 3F 86 FF D5 89 E0 4E 56 46 FF 30 68    08 87 1D 60 FF D5 BB ?? ?? ?? ?? 68 A6 95 BD 9D    FF D5 3C 06 7C 0A 80 FB E0 75 05 BB 47 13 72 6F    6A 00 53 FF D5}
		
	condition: $string
}

rule shellcodeBindTcpXpfw
{
	meta:
		sourceOrg = "metasploit"
		sourcePath = "msf3\\modules\\payloads\\singles\\windows\\shell_bind_tcp_xpfw.rb"
	strings:
	/* Offsets 	'LPORT'    => [ 387, 'n' ],
				'EXITFUNC' => [ 517, 'V' ],
	*/
		$string = { e8 56 00 00 00 53 55 56 57 8b 6c 24 18 8b 45 3c  8b 54 05 78 01 ea 8b 4a 18 8b 5a 20 01 eb e3 32  49 8b 34 8b 01 ee 31 ff fc 31 c0 ac 38 e0 74 07  c1 cf 0d 01 c7 eb f2 3b 7c 24 14 75 e1 8b 5a 24  01 eb 66 8b 0c 4b 8b 5a 1c 01 eb 8b 04 8b 01 e8  eb 02 31 c0 5f 5e 5d 5b c2 08 00 5e 6a 30 59 64  8b 19 8b 5b 0c 8b 5b 1c 8b 1b 8b 5b 08 53 68 8e  4e 0e ec ff d6 89 c7 81 ec 00 01 00 00 57 56 53  89 e5 e8 27 00 00 00 90 01 00 00 b6 19 18 e7 a4  19 70 e9 e5 49 86 49 a4 1a 70 c7 a4 ad 2e e9 d9  09 f5 ad cb ed fc 3b 57 53 32 5f 33 32 00 5b 8d  4b 20 51 ff d7 89 df 89 c3 8d 75 14 6a 07 59 51  53 ff 34 8f ff 55 04 59 89 04 8e e2 f2 2b 27 54  ff 37 ff 55 30 31 c0 50 50 50 50 40 50 40 50 ff  55 2c 89 c7 89 7d 0c e8 06 00 00 00 4f 4c 45 33  32 00 ff 55 08 89 c6 56 68 1b 06 c8 0d ff 55 04  6a 02 6a 00 ff d0 56 68 80 c8 26 6e ff 55 04 89  c7 e8 20 00 00 00 f5 8a 89 f7 c4 ca 32 46 a2 ec  da 06 e5 11 1a f2 42 e9 4c 30 39 6e d8 40 94 3a  b9 13 c4 0c 9c d4 58 50 8d 75 ec 56 50 6a 01 6a  00 83 c0 10 50 ff d7 8d 4d e0 51 8b 55 ec 8b 02  8b 4d ec 51 8b 50 1c ff d2 8d 45 f8 50 8b 4d e0  8b 11 8b 45 e0 50 8b 4a 1c ff d1 31 c0 50 8b 55  f8 8b 02 8b 4d f8 51 8b 50 24 ff d2 31 db 53 53  68 02 00 ?? ?? 89 e0 6a 10 50 8b 7d 0c 57 ff 55  24 53 57 ff 55 28 53 54 57 ff 55 20 89 c7 68 43  4d 44 00 89 e3 87 fa 31 c0 8d 7c 24 ac 6a 15 59  f3 ab 87 fa 83 ec 54 c6 44 24 10 44 66 c7 44 24  3c 01 01 89 7c 24 48 89 7c 24 4c 89 7c 24 50 8d  44 24 10 54 50 51 51 51 41 51 49 51 51 53 51 ff  75 00 68 72 fe b3 16 ff 55 04 ff d0 89 e6 ff 75  00 68 ad d9 05 ce ff 55 04 89 c3 6a ff ff 36 ff  d3 ff 75 00 68 ?? ?? ?? ?? ff 55 04 31 db 53 ff  d0}
	condition: $string
}

rule shellcodeReverseTcp
{
	meta:
		sourceOrg = "metasploit"
		sourcePath = "msf3\\modules\\payloads\\singles\\windows\\shell_reverse_tcp.rb"
	strings:
	/* Offsets 	'LPORT'    => [ 203, 'n'    ],
				'LHOST'    => [ 196, 'ADDR' ],
				'EXITFUNC' => [ 284, 'V'    ],
	*/
		$string = {FC E8 89 00 00 00 60 89 E5 31 D2 64 8B 52 30 8B  52 0C 8B 52 14 8B 72 28 0F B7 4A 26 31 FF 31 C0  AC 3C 61 7C 02 2C 20 C1 CF 0D 01 C7 E2 F0 52 57  8B 52 10 8B 42 3C 01 D0 8B 40 78 85 C0 74 4A 01  D0 50 8B 48 18 8B 58 20 01 D3 E3 3C 49 8B 34 8B  01 D6 31 FF 31 C0 AC C1 CF 0D 01 C7 38 E0 75 F4  03 7D F8 3B 7D 24 75 E2 58 8B 58 24 01 D3 66 8B  0C 4B 8B 58 1C 01 D3 8B 04 8B 01 D0 89 44 24 24  5B 5B 61 59 5A 51 FF E0 58 5F 5A 8B 12 EB 86 5D  68 33 32 00 00 68 77 73 32 5F 54 68 4C 77 26 07  FF D5 B8 90 01 00 00 29 C4 54 50 68 29 80 6B 00  FF D5 50 50 50 50 40 50 40 50 68 EA 0F DF E0 FF  D5 89 C7 68 ?? ?? ?? ?? 68 02 00 ?? ?? 89 E6 6A  10 56 57 68 99 A5 74 61 FF D5 68 63 6D 64 00 89  E3 57 57 57 31 F6 6A 12 59 56 E2 FD 66 C7 44 24  3C 01 01 8D 44 24 10 C6 00 44 54 50 56 56 56 46  56 4E 56 56 53 56 68 79 CC 3F 86 FF D5 89 E0 4E  56 46 FF 30 68 08 87 1D 60 FF D5 BB ?? ?? ?? ??  68 A6 95 BD 9D FF D5 3C 06 7C 0A 80 FB E0 75 05  BB 47 13 72 6F 6A 00 53 FF D5}
	condition: $string
}


rule shellcodeFindKernel32
{
	meta:
		sourceOrg = "metasploit"
		sourcePath = "msf3\\modules\\payloads\\singles\\windows\\messagebox.rb"
	strings:
		$string = {31 d2 b2 77 31 c9 64 8b 71 30 8b 76 0c 8b 76 1c 8b 46 08 8b 7e 20 8b 36 38 4f 18 75 f3 59 01 d1 ff e1 60 8b 6c 24 24 8b 45 3c 8b 54 28 78 01 ea 8b 4a 18 8b 5a 20 01 eb e3 34 49 8b 34 8b 01 ee 31 ff 31 c0 fc ac 84 c0 74 07 c1 cf 0d 01 c7 eb f4 3b 7c 24 28 75 e1 8b 5a 24 01 eb 66 8b 0c 4b 8b 5a 1c 01 eb 8b 04 8b 01 e8 89 44 24 1c 61 c3}
	condition: $string
}


