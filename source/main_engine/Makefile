
MAIN_TARGET=Mainengine

.PHONY: all clean

all:
	echo -ne "\n\n\n\n"; cd grain_codec;   	 								make && cd ..;
#	echo -ne "\n\n\n\n"; cd tutk;  											make && cd ..;
	echo -ne "\n\n\n\n"; cd danale;  										make && cd ..;
	echo -ne "\n\n\n\n"; cd adapt;   	 									make && cd ..;
	echo -ne "\n\n\n\n"; cd network; 										make && cd ..;		
	echo -ne "\n\n\n\n"; cd storage; 										make && cd ..;		
	echo -ne "\n\n\n\n"; cd main;   	 									make && cd ..;		
		
clean:
	echo -ne "\n\n\n\n"; cd grain_codec;   	 								make clean && cd ..;
	echo -ne "\n\n\n\n"; cd network; 										make clean && cd ..;
#	echo -ne "\n\n\n\n"; cd tutk; 											make clean && cd ..;
	echo -ne "\n\n\n\n"; cd danale;  									    make clean && cd ..;
	echo -ne "\n\n\n\n"; cd storage;  									    make clean && cd ..;
	echo -ne "\n\n\n\n"; cd adapt;   	 									make clean && cd ..;
	echo -ne "\n\n\n\n"; cd main;   	 									make clean && cd ..;	
	rm -rf bin/$(MAIN_TARGET)
	rm -rf lib/$(MAIN_TARGET)
	
