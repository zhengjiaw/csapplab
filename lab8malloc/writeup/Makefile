LABNAME = malloclab

all:
	latex $(LABNAME).tex
	latex $(LABNAME).tex # again to resolve references
	dvips -o $(LABNAME).ps $(LABNAME).dvi
	ps2pdf $(LABNAME).ps

clean:
	rm -f *.aux *.ps *.pdf *.dvi *.log *~

