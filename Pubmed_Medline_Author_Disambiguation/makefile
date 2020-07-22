#CXX=icpc
#CC=icc
#CPPFLAGS=-O3 -g -xsse4.1 -ipo -static-intel -Wall
#CPPFLAGS=-O0 -g -Wall -fno-inline -I/usr/local/include -L/usr/local/lib
CXX=g++
CC=gcc
CPPFLAGS=-O0 -g -Wall -fno-inline -I/usr/local/include -L/usr/local/lib
CPPFLAGS=-O3 -g -Wall -I/usr/local/include -L/usr/local/lib -std=c++11
CFLAGS=-O3 -g -Wall -I/usr/local/include -L/usr/local/lib
OOQPINCLUDEDIR=/usr/local/include/ooqp
ILOGINSTALLDIR=/Users/RobertTeresi/Documents/CPLEX

CPLEXINCLUDE=$(ILOGINSTALLDIR)/cplex/include
CONCERTINCLUDE=$(ILOGINSTALLDIR)/concert/include
CPLEXLIB=$(ILOGINSTALLDIR)/cplex/lib/x86-64_osx/static_pic
CONCERTLIB=$(ILOGINSTALLDIR)/concert/lib/x86-64_osx/static_pic

all:exedisambig txt2sqlite3

exedisambig: Disambigmain.o DisambigDefs.o DisambigRatios.o DisambigEngine.o DisambigFileOper.o strcmp95.o DisambigComp.o DisambigTraining.o Threading.o DisambigCluster.o DisambigRatioSmoothing.o DisambigNewCluster.o DisambigCustomizedDefs.o DisambigPostProcess.o DisambigUtilities.o
	$(CXX) -o $@ $? $(CPPFLAGS) -L$(CPLEXLIB) -lilocplex -lcplex -L$(CONCERTLIB) -lconcert -lm -lpthread

Disambigmain.o: Disambigmain.cpp
	$(CXX) -c $? $(CPPFLAGS)

DisambigDefs.o:	DisambigDefs.cpp DisambigDefs.h
	$(CXX) -c $? $(CPPFLAGS)

DisambigRatios.o: DisambigRatios.cpp DisambigRatios.h
	$(CXX) -c $? $(CPPFLAGS)

DisambigEngine.o: DisambigEngine.cpp DisambigEngine.h
	$(CXX) -c $? $(CPPFLAGS)

DisambigFileOper.o: DisambigFileOper.cpp DisambigFileOper.h
	$(CXX) -c $? $(CPPFLAGS)	

DisambigComp.o: DisambigComp.cpp DisambigComp.h
	$(CXX) -c $? $(CPPFLAGS)

DisambigTraining.o: DisambigTraining.h DisambigTraining.cpp
	$(CXX) -c $? $(CPPFLAGS)

strcmp95.o: strcmp95.c strcmp95.h
	$(CC) -c $? $(CFLAGS)

Threading.o: Threading.cpp Threading.h
	$(CXX) -c $? $(CPPFLAGS)

DisambigCluster.o: DisambigCluster.cpp DisambigCluster.h
	$(CXX) -c $? $(CPPFLAGS)

DisambigRatioSmoothing.o: DisambigRatioSmoothing.cpp
	$(CXX) -c $? $(CPPFLAGS) -I$(OOQPINCLUDEDIR) -I$(CPLEXINCLUDE) -I$(CONCERTINCLUDE) -DIL_STD -DNDEBUG -w

DisambigNewCluster.o: DisambigNewCluster.h DisambigNewCluster.cpp
	$(CXX) -c $? $(CPPFLAGS)

DisambigCustomizedDefs.o: DisambigCustomizedDefs.h DisambigCustomizedDefs.cpp
	$(CXX) -c $? $(CPPFLAGS)

DisambigPostProcess.o: DisambigPostProcess.h DisambigPostProcess.cpp
	$(CXX) -c $? $(CPPFLAGS)

DisambigUtilities.o: DisambigUtilities.h DisambigUtilities.cpp
	$(CXX) -c $? $(CPPFLAGS)

clean:
	rm *.o *.gch

txt2sqlite3: txt2sqlite3.h txt2sqlite3.cpp
	$(CXX) -o $@ $? $(CPPFLAGS) -lsqlite3
