DESTDIR=
INSTALLDIR=opt/liveamp
all: liveamp

liveamp: main.o NPulse.o NShaderManager.o NTexture.o
	g++ main.o NPulse.o NShaderManager.o NTexture.o -o liveamp -lpulse-simple -lpulse -lglfw -lGLEW -lGL -lpng -lX11 -lXext
	
main.o: main.cpp
	g++ -Wall -c main.cpp
	
NPulse.o: NPulse.h NPulse.cpp
	g++ -Wall -c NPulse.cpp
	
NShaderManager.o: NShaderManager.h NShaderManager.cpp
	g++ -Wall -c NShaderManager.cpp

NTexture.o: NTexture.h NTexture.cpp
	g++ -Wall -c NTexture.cpp
	
install: shaders/flat.frag shaders/flat.vert textures/desktop textures/desktop.png liveamp
	echo "Installing to $(DESTDIR)/$(INSTALLDIR)..."
	mkdir -p $(DESTDIR)/$(INSTALLDIR)
	mkdir -p $(DESTDIR)/$(INSTALLDIR)/shaders
	mkdir -p $(DESTDIR)/$(INSTALLDIR)/textures
	cp shaders/flat.frag $(DESTDIR)/$(INSTALLDIR)/shaders
	cp shaders/flat.vert $(DESTDIR)/$(INSTALLDIR)/shaders
	cp textures/desktop $(DESTDIR)/$(INSTALLDIR)/textures
	cp textures/desktop.png $(DESTDIR)/$(INSTALLDIR)/textures
	cp liveamp $(DESTDIR)/$(INSTALLDIR)
	mkdir -p $(DESTDIR)/usr/bin
	ln -T $(DESTDIR)/$(INSTALLDIR)/liveamp $(DESTDIR)/usr/bin/liveamp
	echo "Done!"
