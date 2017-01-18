-include x86.mk

LD_CFLAGS = -g -Wall
CFLAGS:= $(ARCH_FLAGS) -I../inc $(LD_CFLAGS)
#LDFLAGS:= -L../lib -ladapt -llog -lparam -lmisc -lpthread -lrt $(EX_LDFLAGS)
LDFLAGS:= -L../lib/X86  -lx86param -lx86log -lx86misc -lpthread -lrt $(EX_LDFLAGS)

all: mkupgrade defparam mkmac upgrade_tool
clean:
	rm -rf *.o mkupgrade defparam  mkmac  upgrade_tool

mkupgrade: mkupgrade.o adapt_param.o
	$(CC) -o $@ $^ $(LDFLAGS)
	cp -arvf mkupgrade ../bin/$(ARCH_CPU)-mkupgrade
defparam: defparam.o
	$(CC) -o $@ $^ $(LDFLAGS) 
	cp -arvf defparam ../bin/$(ARCH_CPU)-defparam
mkmac: mkrandmac.o
	$(CC) -o $@ $^ $(LDFLAGS)
	cp -arvf mkmac ../bin/$(ARCH_CPU)-mkmac
	
upgrade_tool: upgrade_tool.o
	$(CC) -o $@ $^ $(LDFLAGS)
	cp -arvf mkmac ../bin/$(ARCH_CPU)-upgrade_tool
	

