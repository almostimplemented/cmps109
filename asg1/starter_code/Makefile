# Author: Andrew Edwards
# Email:  ancedwar@ucsc.edu
# ID:	  1253060
# Date:	  2015 Jan 18

MKFILE      = Makefile
DEPFILE     = ${MKFILE}.dep
NOINCL      = ci clean spotless
NEEDINCL    = ${filter ${NOINCL}, ${MAKECMDGOALS}}
GMAKE       = ${MAKE} --no-print-directory

COMPILECPP  = g++ -g -O0 -Wall -Wextra -rdynamic -std=gnu++11
MAKEDEPCPP  = g++ -MM

CPPSOURCE   = commands.cpp debug.cpp inode.cpp util.cpp main.cpp
CPPHEADER   = commands.h debug.h inode.h util.h
EXECBIN     = yshell
OBJECTS     = ${CPPSOURCE:.cpp=.o}
OTHERS      = ${MKFILE} README
ALLSOURCES  = ${CPPHEADER} ${CPPSOURCE} ${OTHERS}

all : ${EXECBIN}
	- checksource ${ALLSOURCES}

${EXECBIN} : ${OBJECTS}
	${COMPILECPP} -o $@ ${OBJECTS}

%.o : %.cpp
	${COMPILECPP} -c $<

ci : ${ALLSOURCES}
	cid + ${ALLSOURCES}
	- checksource ${ALLSOURCES}

clean :
	- rm ${OBJECTS} ${DEPFILE} *.ysh.err *.ysh.out *.ysh.status

spotless : clean
	- rm ${EXECBIN}

dep : ${CPPSOURCE} ${CPPHEADER}
	@ echo "# ${DEPFILE} created `LC_TIME=C date`" >${DEPFILE}
	${MAKEDEPCPP} ${CPPSOURCE} >>${DEPFILE}

${DEPFILE} : ${MKFILE}
	@ touch ${DEPFILE}
	${GMAKE} dep

#
# Subimt
#

submit :
	submit cmps109-wm.w15 asg1 ${ALLSOURCES}
	mkdir -p latest_submit
	cp ${ALLSOURCES} latest_submit

again :
	${GMAKE} spotless dep ci all lis

ifeq (${NEEDINCL}, )
include ${DEPFILE}
endif

