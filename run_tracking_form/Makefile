RUN_ARG = $(shell echo $(RUN_START)+30\*\($*-1\) | bc)

all: clean checklist_run

checklist_run: checklist_run_1.pdf checklist_run_2.pdf checklist_run_3.pdf \
	checklist_run_4.pdf

checklist_run_%.tex: checklist_run.tex incr_runs.perl
	incr_runs.perl $(RUN_ARG) > $@

%.pdf : %.tex
	pdflatex $*

clean:
	$(RM) *.pdf checklist_run_*.tex
