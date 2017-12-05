LATEX	= latex -shell-escape
BIBTEX	= bibtex
DVIPS	= dvips
DVIPDF  = dvipdft
XDVI	= xdvi -gamma 4
GH		= gv

SHELL  := /bin/bash
SRC	   := $(shell egrep -l '^[^%]*\\begin\{document\}' *.tex)
TRG		= $(SRC:%.tex=%.dvi)
PSF		= $(SRC:%.tex=%.ps)
PDF		= $(SRC:%.tex=%.pdf)

pdf: $(PDF)
	rm -rf *.ps *.dvi *.out *.log *.aux *.bbl *.blg *.pyg *.toc *.cpp.tex *.c.tex

ps: $(PSF)

$(TRG): %.dvi: %.tex  fork.c createprocess.c seclvl.c example.c snippet1.cpp spinlock.c
	(unset PYTHONPATH && unset PYTHONHOME && pygmentize -f latex -o __fork.c.tex fork.c)
	(unset PYTHONPATH && unset PYTHONHOME && pygmentize -f latex -o __createprocess.c.tex createprocess.c)
	(unset PYTHONPATH && unset PYTHONHOME && pygmentize -f latex -o __seclvl.c.tex seclvl.c)
	(unset PYTHONPATH && unset PYTHONHOME && pygmentize -f latex -o __example.c.tex example.c)
	(unset PYTHONPATH && unset PYTHONHOME && pygmentize -f latex -o __snippet1.cpp.tex snippet1.cpp)
	(unset PYTHONPATH && unset PYTHONHOME && pygmentize -f latex -o __spinlock.c.tex spinlock.c)
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
	rm -rf *.pdf *.ps *.dvi *.out *.log *.aux *.bbl *.blg *.pyg *.toc *.cpp.tex *.c.tex

.PHONY: all show clean ps pdf showps
