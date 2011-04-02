rule TestRule
{
	strings:
		$ = "unpack"
	condition:
		1 of them
}