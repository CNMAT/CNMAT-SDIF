all:  	bin/SDIFDisplay

doc:		.dummy
	make -C doc

# .dummy is used to force 
.dummy:


depend: 
	make -C obj depend

clean: 
	make -C obj clean
	rm -f bin/*
	
include VERSION
include makeinclude 
include sources

bin/SDIFDisplay: .dummy
	make -C obj	

static: bin/SDIFDisplay-static

bin/SDIFDisplay-static:
	make -C obj ../bin/SDIFDisplay-static	

install: bin/SDIFDisplay doc/SDIFDisplay.1
	cp doc/SDIFDisplay.1 ${prefix}/man/man1/
	cp bin/SDIFDisplay ${bindir}

dist: dist_bin dist_source

dist_bin: bin/SDIFDisplay bin/SDIFDisplay-static
	strip bin/SDIFDisplay 
	strip bin/SDIFDisplay-static
	rm -f doc/manpage*
	(cd .. ; tar zcvf SDIFDisplay-$(VERSION).i386.tgz	\
	SDIFDisplay/bin SDIFDisplay/README SDIFDisplay/doc \
	--exclude doc/SDIFDisplay.log \
	--exclude doc/SDIFDisplay.aux \
	--exclude doc/SDIFDisplay.tex \
	--exclude doc/SDIFDisplay.dvi \
	)

OBJECTS_EXCLUDE = \
	$(addprefix --exclude obj/, $(addsuffix .o , $(basename $(notdir $(SOURCES)))))

distro: dist_source dist_bin

dist_source:
	rm -f config.*
	rm -f doc/manpage*
	make -C doc
	(cd .. ; tar zcvf SDIFDisplay-$(VERSION).src.tgz SDIFDisplay \
	--exclude doc/SDIFDisplay.log \
	--exclude doc/SDIFDisplay.aux \
	--exclude doc/SDIFDisplay.tex \
	--exclude doc/SDIFDisplay.dvi \
	--exclude bin/SDIFDisplay \
	--exclude bin/SDIFDisplay-static \
	--exclude makeinclude \
	--exclude obj/.depend \
	$(OBJECTS_EXCLUDE) \
	) 
