all: SDIFDisplay.ps SDIFDisplay.1 SDIFDisplay.html ../README

clean:
	rm -f SDIFDisplay.html SDIFDisplay.tex SDIFDisplay.dvi SDIFDisplay.ps

../README: SDIFDisplay.html
	lynx -dump SDIFDisplay.html > ../README

SDIFDisplay.tex: SDIFDisplay.sgml
	jade -t tex -d /usr/lib/sgml/docbook/print/docbook.dsl SDIFDisplay.sgml

SDIFDisplay.dvi: SDIFDisplay.tex
	jadetex SDIFDisplay.tex

SDIFDisplay.ps: SDIFDisplay.dvi
	dvips -o SDIFDisplay.ps SDIFDisplay.dvi

SDIFDisplay.1: SDIFDisplay.sgml
	nsgmls SDIFDisplay.sgml | sgmlspl docbook2man-spec.pl 

SDIFDisplay.html: SDIFDisplay.sgml
	jade -t sgml -d /usr/lib/sgml/docbook/html/docbook.dsl SDIFDisplay.sgml
	mv r1.htm SDIFDisplay.html
