LATEX	= latex -shell-escape
BIBTEX	= bibtex
DVIPS	= dvips
DVIPDF  = dvipdft
XDVI	= xdvi -gamma 4
GH		= gv

SRC	   := $(shell egrep -l '^[^%]*\\begin\{document\}' *.tex)
TRG		= $(SRC:%.tex=%.dvi)
PSF		= $(SRC:%.tex=%.ps)
PDF		= $(SRC:%.tex=%.pdf)

pdf: $(PDF)
	rm -f *.ps *.dvi *.out *.log *.aux *.bbl *.blg *.pyg *.toc *.c.tex

ps: $(PSF)

$(TRG): %.dvi: %.tex fork.c createprocess.c
	#one way of including source code is to use pygments
	pygmentize -f latex -o __fork.c.tex fork.c
	pygmentize -f latex -o __createprocess.c.tex createprocess.c
	#requires that you \include{pygments.tex} in your preamble

	$(LATEX) $<
	$(BIBTEX) $(<:%.tex=%)
	$(LATEX) $<
	$(LATEX) $<
	#remove the pygmentized output to avoid cluttering up the directory
	#rm __${SRC}.tex


$(PSF):%.ps: %.dvi
	$(DVIPS) -R -Poutline -t letter $< -o $@

$(PDF): %.pdf: %.ps
	ps2pdf $<

show: $(TRG)
	@for i in $(TRG) ; do $(XDVI) $$i & done

showps: $(PSF)
	@for i in $(PSF) ; do $(GH) $$i & done

all: pdf

clean:
	rm -f *.pdf *.ps *.dvi *.out *.log *.aux *.bbl *.blg *.pyg *.toc *.c.tex

.PHONY: all show clean ps pdf showps
