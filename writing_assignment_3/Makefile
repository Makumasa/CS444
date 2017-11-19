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
	rm -rf *.ps *.dvi *.out *.log *.aux *.bbl *.blg *.pyg *.toc *.cpp.tex _minted*

ps: $(PSF)

$(TRG): %.dvi: %.tex  snippet1.cpp
	#one way of including source code is to use pygments
	pygmentize -f latex -o __snippet1.cpp.tex snippet1.cpp
	#requires that you \include{pygments.tex} in your preamble

	$(LATEX) $<
	$(BIBTEX) $(<:%.tex=%)
	$(LATEX) $<
	$(LATEX) $<


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
	rm -rf *.pdf *.ps *.dvi *.out *.log *.aux *.bbl *.blg *.pyg *.toc *.cpp.tex _minted*

.PHONY: all show clean ps pdf showps
