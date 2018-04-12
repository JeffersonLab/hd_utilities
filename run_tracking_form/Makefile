RUN_ARG = $(shell echo $(RUN_START)+20\*\($*-1\) | bc)

all: clean checklist_run

checklist_run: checklist_run_1.pdf checklist_run_2.pdf checklist_run_3.pdf \
	checklist_run_4.pdf checklist_run_5.pdf checklist_run_6.pdf \
	checklist_run_7.pdf checklist_run_8.pdf checklist_run_9.pdf

checklist_run_%.tex: checklist_run.tex incr_runs.perl
	incr_runs.perl $(RUN_ARG) > $@

%.pdf : %.tex
	pdflatex $*

clean:
	$(RM) *.pdf checklist_run_*.tex
