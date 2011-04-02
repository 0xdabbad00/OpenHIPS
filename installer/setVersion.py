#! /usr/bin/python
from tempfile import mkstemp
from shutil import move
from os import remove, close
import re
import sys



def replace(file, pattern, subst):
	print (file)
	#Create temp file
	fh, abs_path = mkstemp()
	new_file = open(abs_path,'w')
	old_file = open(file)
	for line in old_file:
		new_file.write(re.sub(pattern, subst, line))
	#close temp file
	new_file.close()
	close(fh)
	old_file.close()
	#Remove original file
	remove(file)
	#Move new file
	move(abs_path, file)

# Main program
if (len(sys.argv) == 1):
	sys.exit("Specify the version. Example: %s 1.0.0.0\n" % sys.argv[0])

version = sys.argv[1]
versionnumber = re.sub("\.", ",", version)
print ("Setting the following files to use version %s\n" % version)

replace("../ohipsfs/firststage.rc", "#define VERSION_NUMBER [\d\,]*", "#define VERSION_NUMBER %s" % versionnumber);
replace("../ohipsfs/firststage.rc", "#define VERSION_STRING \"[\d\.]*\"", "#define VERSION_STRING \"%s\"" % version);


replace("../ohipsp/protector.rc", "#define VERSION_NUMBER [\d\,]*", "#define VERSION_NUMBER %s" % versionnumber);
replace("../ohipsp/protector.rc", "#define VERSION_STRING \"[\d\.]*\"", "#define VERSION_STRING \"%s\"" % version);


replace("../installer/installer.wxs", " Version=\"[\d\.]*\"", " Version=\"%s\"" % version);
replace("../ohipssvc/Properties/AssemblyInfo.cs", "\[assembly: AssemblyVersion(\"[\d\.]*\")\]", "\[assembly: AssemblyVersion(\"%s\")\]" % version);
replace("../ohipssvc/Properties/AssemblyInfo.cs", "\[assembly: AssemblyFileVersion(\"[\d\.]*\")\]", "\[assembly: AssemblyFileVersion(\"%s\")\]" % version);

replace("../ohipsui/Properties/AssemblyInfo.cs", "\[assembly: AssemblyVersion(\"[\d\.]*\")\]", "\[assembly: AssemblyVersion(\"%s\")\]" % version);
replace("../ohipsui/Properties/AssemblyInfo.cs", "\[assembly: AssemblyFileVersion(\"[\d\.]*\")\]", "\[assembly: AssemblyFileVersion(\"%s\")\]" % version);
replace("../ohipsui/TrayIcon.cs", "private string szVersion = \"[\d\.]*\";", "private string szVersion = \"%s\";" % version);
