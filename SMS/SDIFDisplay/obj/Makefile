include ../makeinclude

X_EXTRA_LIBS = 

OBJECTS = \
        $(addsuffix .o , $(basename $(notdir $(SOURCES))))
 
%.o: ../src/%.c
	$(CC) -c $(CFLAGS) $< -o $@

%.o: ../src/%.cxx
	$(CXX) -c $(CXXFLAGS) $< -o $@
 
include ../sources 

all: ../bin/SDIFDisplay

clean:
	rm -f *.o core .depend

../bin/SDIFDisplay: $(OBJECTS)
	$(CXX) $(OBJECTS) \
	$(LDLIBS) \
	-o $@

../bin/SDIFDisplay-static: $(OBJECTS) 
	$(CXX) $(OBJECTS) $(LDLIBS_FLTK_STATIC) -o $@ 
	strip $@


.depend: 
	$(MAKEDEPEND) $(INCLDIRS) $(DEFINES) $(addprefix ../, $(SOURCES)) > .depend
	
depend:
	$(MAKEDEPEND) $(INCLDIRS) $(DEFINES) $(addprefix ../, $(SOURCES)) > .depend

include .depend
