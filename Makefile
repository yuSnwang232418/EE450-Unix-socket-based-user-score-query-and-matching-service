all: central.cpp serverT.cpp serverS.cpp serverP.cpp clientA.cpp clientB.cpp
	g++ -std=c++11 -o serverC central.cpp
	g++ -std=c++11 -o serverT serverT.cpp
	g++ -std=c++11 -o serverS serverS.cpp
	g++ -std=c++11 -o serverP serverP.cpp
	g++ -std=c++11 -o clientA clientA.cpp
	g++ -std=c++11 -o clientB clientB.cpp
	
.PHONY: serverC
serverC:
	./serverC
.PHONY: serverT
serverT:
	./serverT
.PHONY: serverS
serverS:
	./serverS
.PHONY: serverP
serverP:
	./serverP
.PHONY: clientA
clientA:
	./clientA 
.PHONY: clientB
clientB:
	./clientB 

