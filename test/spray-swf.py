##########################################################################
####   Felipe Andres Manzano     *   felipe.andres.manzano@gmail.com  ####
####   http://twitter.com/feliam *   http://wordpress.com/feliam      ####
##########################################################################
from miniPDF import *
import zlib,sys,md5

def _zipEmbeddeFile(doc, f,minimal=False):
    fileStr = file(f).read()
    embedded = PDFStream(fileStr)
    if not minimal:
        embedded.add('Type', PDFName('EmbeddedFile'))
#        embedded.add('Subtype',PDFName('application#2Fpdf'))
        embedded.add('Params',PDFDict({'Size': PDFNum(len(fileStr)),
                                 'CheckSum': PDFOctalString(md5.new(fileStr).digest())}) )
        embedded.add('DL', ' %d '%len(fileStr))
    embedded.appendFilter(FlateDecode())
    doc.add(embedded)

    #embedded list
    embeddedlst = PDFDict()
    embeddedlst.add('F',PDFRef(embedded))

    #fileSpec
    filespec = PDFDict()
    filespec.add('Type',PDFName('Filespec'))
    filespec.add('F',PDFString(f))
    filespec.add('EF', embeddedlst)
    doc.add(filespec)
    return filespec

#The document
doc = PDFDoc()

#The withe page
#contents
contents = PDFStream('')
doc.add(contents)

#page
page = PDFDict()
page.add("Type", PDFName("Page"))
page.add("Contents", PDFRef(contents))
doc.add(page)

#pages
pages = PDFDict()
pages.add("Type", PDFName("Pages"))
pages.add("Kids", PDFArray([PDFRef(page)]))
pages.add("Count", PDFNum(1))
doc.add(pages)

#add parent reference in page
page.add("Parent", PDFRef(pages))

#catalog
catalog = PDFDict()
catalog.add("Type", PDFName("Catalog"))
catalog.add("Pages", PDFRef(pages))
doc.add(catalog)
doc.setRoot(catalog)


#SWF PART BEGINS...
#A name tree of embedded file specification dictionaries as detailed in 
#Section 3.10.2 of the PDF Reference, sixth edition.
assets = PDFDict()
swfname, efref = PDFString(sys.argv[1]),PDFRef(_zipEmbeddeFile(doc,sys.argv[1]))
assets.add('Names',PDFArray([swfname, efref]))
doc.add(assets)

#Astream with flash variables
#flashVars = PDFStream(file(sys.argv[2]).read())
#doc.add(flashVars)

#stream with flash variables
auxiliar = PDFStream('''''')
doc.add(auxiliar)

#TABLE 9.51c Entries in a RichMediaParams dictionary
RMParams = PDFDict()
RMParams.add('Type', PDFName('RichMediaParams'))
RMParams.add('FlashVars', PDFString(file(sys.argv[2]).read()))
RMParams.add('Binding', PDFName('Background'))
RMParams.add('Settings', PDFRef(auxiliar))
doc.add(RMParams)

#The instances list
instances = []
#TABLE 9.51b Entries in a RichMediaInstance dictionary
RMI = PDFDict()
RMI.add('Type',PDFName('RichMediaInstance'))
RMI.add('Subype',PDFName('Flash'))
RMI.add('Params',PDFRef(RMParams))
RMI.add('Asset',efref)
doc.add(RMI)
instances.append(PDFRef(RMI))

#the configuration list
configurations = []
#TABLE 9.51a Entries in a RichMediaConfiguration dictionary
RMCfg = PDFDict()
RMCfg.add('Type',PDFName('RichMediaConfiguration'))

#Default value: If no value is specified, the run time determines the
#scene type by referring to the type of asset file specified by the first
#element in the Instances array.
RMCfg.add('Subtype',PDFName('Flash'))
RMCfg.add('Name',PDFString('ElFlash'))
RMCfg.add('Instances', PDFArray(instances))
doc.add(RMCfg)
configurations.append(PDFRef(RMCfg))



#TABLE 9.50a Entries in a RichMediaDeactivation dictionary
activation = PDFDict()
activation.add('Type', PDFName('RichMediaActivation'))
activation.add('Condition', PDFName('PO'))
#Default value: The first element within the Configurations
#array specified in the RichMediaContent dictionary.
#activation.add('Configuration', PDFRef(...))

#An array of indirect object 
#references to file specification dictionaries, each of which
#describe a JavaScript file that shall be present in the Assets
#name tree of the RichMediaContent dictionary.
#Default value: If the array has no elements, no script is
#executed.
#activation.add('Scripts',PDFArray([PDFRef(jsfile)]))
doc.add(activation)

#TABLE 9.50b Entries in a RichMediaDeactivation dictionary
deactivation = PDFDict()
deactivation.add('Type', PDFName('RichMediaDeactivation'))
deactivation.add('Condition', PDFName('XD'))
doc.add(deactivation)

#RichMediaCfg
#TABLE 9.50 Entries in a RichMediaSettings dictionary
RMS = PDFDict()
RMS.add('Type',PDFName('RichMediaSettings'))
RMS.add('Subtype',PDFName('Flash'))
RMS.add('Activation', PDFRef(activation))
RMS.add('Deactivation', PDFRef(deactivation))
doc.add(RMS)

#TABLE 9.51 Entries in a RichMediaContent dictionary
RMC = PDFDict()
RMC.add('Type', PDFName('RichMediaContent'))
RMC.add('Assets', PDFRef(assets))
RMC.add('Configurations',PDFArray(configurations))
doc.add(RMC)


#RichMedia annotation
#Section 9.5 of the PDF Reference
annot = PDFDict()
annot.add('Type',PDFName('Annot'))
annot.add('Subtype',PDFName('RichMedia'))
annot.add('Rect','[ 266 116 430 204 ]')
annot.add('NM',PDFString(sys.argv[1]))
#9.6.1 RichMedia Annotations
annot.add('RichMediaSettings', PDFRef(RMS))
annot.add('RichMediaContent', PDFRef(RMC))
doc.add(annot)

#add the RichMedia annotation to the 1st page
page.add("Annots", PDFArray([PDFRef(annot)]))
print doc
