RUN_ARG = $(shell echo $(RUN_START)+25\*\($*-1\) | bc)

all: checklist_run checklist_shift.ps G1_checklist.ps E1_checklist.ps \
	EG1_checklist.ps

checklist_run: checklist_run_1.ps checklist_run_2.ps checklist_run_3.ps \
	checklist_run_4.ps

checklist_run_%.tex: checklist_run.tex incr_runs.perl
	incr_runs.perl $(RUN_ARG) > $@

%.ps : %.dvi
	dvips -o $*.ps $*

%.ps : %.ps.gz
	rm -f $*.ps
	gunzip -c $*.ps.gz > $*.ps

%.dvi : %.tex
	latex $*
	latex $*

clean:
	$(RM) *.ps *.dvi checklist_run_*.tex
