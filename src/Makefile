APPNAME=horizon
APPCLIENT=$(APPNAME)
APPSERVER=$(APPNAME)_server

#CXXFLAGS= -ggdb3
CXXFLAGS?= -O3 -fomit-frame-pointer -ffast-math
override CXXFLAGS+= -Wall -fsigned-char -fno-exceptions -fno-rtti

PLATFORM= $(shell $(CC) -dumpmachine)

ifeq (,$(PLATFORM_BIN))
ifneq (,$(findstring arm,$(PLATFORM)))
PLATFORM_BIN=arm
else
ifneq (,$(findstring 64,$(PLATFORM)))
PLATFORM_BIN=amd64
else
PLATFORM_BIN=x86
endif
endif
endif

ifeq (,$(INSTDIR))
INSTDIR=../bin/$(PLATFORM_BIN)
endif

TOOLSET_PREFIX=
ifneq (,$(findstring cross,$(PLATFORM)))
ifneq (,$(findstring 64,$(PLATFORM)))
TOOLSET_PREFIX=x86_64-w64-mingw32-
else
TOOLSET_PREFIX=i686-w64-mingw32-
endif
CXX=g++
CC=gcc
endif

CXX_TEMP:=$(CXX)
CC_TEMP:=$(CC)
override CXX=$(TOOLSET_PREFIX)$(CXX_TEMP)
override CC=$(TOOLSET_PREFIX)$(CC_TEMP)

INCLUDES= -I. -Ishared -Iengine -Igame -Ienet/include

STRIP=
ifeq (,$(findstring -g,$(CXXFLAGS)))
ifeq (,$(findstring -pg,$(CXXFLAGS)))
	STRIP=strip
endif
endif
STRIP_TEMP:=$(STRIP)
override STRIP=$(TOOLSET_PREFIX)$(STRIP_TEMP)

MV=mv
CP=cp
MKDIR=mkdir -p

ifneq (,$(findstring mingw,$(PLATFORM)))
APPMODIFIER=_windows
BIN_SUFFIX=.exe
WINDRES=windres
WINDRES_TEMP:=$(WINDRES)
WINLIB=lib/$(PLATFORM_BIN)
WINDLL=../bin/$(PLATFORM_BIN)
override WINDRES=$(TOOLSET_PREFIX)$(WINDRES_TEMP)
ifneq (,$(findstring 64,$(PLATFORM)))
override WINDRES+= -F pe-x86-64
else
override WINDRES+= -F pe-i386
endif
STD_LIBS= -static-libgcc -static-libstdc++
CLIENT_INCLUDES= $(INCLUDES) -Iinclude
CLIENT_LIBS= -mwindows $(STD_LIBS) -L$(WINDLL) -L$(WINLIB) -lSDL2 -lSDL2_image -lSDL2_mixer -lzlib1 -lopengl32 -lenet -lws2_32 -lwinmm
else
APPMODIFIER=
ifneq (,$(findstring linux,$(PLATFORM)))
BIN_SUFFIX=_linux
else
ifneq (,$(findstring bsd,$(PLATFORM)))
BIN_SUFFIX=_bsd
else
BIN_SUFFIX=_native
endif
endif
CLIENT_INCLUDES= $(INCLUDES) -I/usr/X11R6/include `sdl2-config --cflags`
CLIENT_LIBS= -Lenet -lenet -L/usr/X11R6/lib -lX11 `sdl2-config --libs` -lSDL2_image -lSDL2_mixer -lz -lGL
endif
ifneq (,$(findstring linux,$(PLATFORM)))
CLIENT_LIBS+= -lrt
else
ifneq (,$(findstring gnu,$(PLATFORM)))
CLIENT_LIBS+= -lrt
endif
endif
CLIENT_OBJS= \
	shared/crypto.o \
	shared/geom.o \
	shared/glemu.o \
	shared/stream.o \
	shared/tools.o \
	shared/zip.o \
	engine/aa.o \
	engine/bih.o \
	engine/blend.o \
	engine/client.o	\
	engine/command.o \
	engine/console.o \
	engine/dynlight.o \
	engine/grass.o \
	engine/light.o \
	engine/main.o \
	engine/material.o \
	engine/menus.o \
	engine/movie.o \
	engine/normal.o	\
	engine/octa.o \
	engine/octaedit.o \
	engine/octarender.o \
	engine/physics.o \
	engine/pvs.o \
	engine/rendergl.o \
	engine/renderlights.o \
	engine/rendermodel.o \
	engine/renderparticles.o \
	engine/rendersky.o \
	engine/rendertext.o \
	engine/renderva.o \
	engine/server.o	\
	engine/serverbrowser.o \
	engine/shader.o \
	engine/sound.o \
	engine/stain.o \
	engine/texture.o \
	engine/ui.o \
	engine/water.o \
	engine/world.o \
	engine/worldio.o \
	game/ai.o \
	game/client.o \
	game/entities.o \
	game/game.o \
	game/render.o \
	game/server.o \
	game/waypoint.o \
	game/weapon.o

CLIENT_PCH= shared/cube.h.gch engine/engine.h.gch game/game.h.gch

ifneq (,$(findstring mingw,$(PLATFORM)))
SERVER_INCLUDES= -DSTANDALONE $(INCLUDES) -Iinclude
SERVER_LIBS= -mwindows $(STD_LIBS) -L$(WINDLL) -L$(WINLIB) -lzlib1 -lenet -lws2_32 -lwinmm
else
SERVER_INCLUDES= -DSTANDALONE $(INCLUDES)
SERVER_LIBS= -Lenet -lenet -lz
endif
SERVER_OBJS= \
	shared/crypto-standalone.o \
	shared/stream-standalone.o \
	shared/tools-standalone.o \
	engine/command-standalone.o \
	engine/server-standalone.o \
	engine/worldio-standalone.o \
	game/entities-standalone.o \
	game/server-standalone.o

LIBENET= enet/libenet.a

all:

default: all

clean-enet:
	$(MAKE) -C enet clean

clean-client:
	@rm -fv $(CLIENT_PCH) $(CLIENT_OBJS) $(APPCLIENT)$(BIN_SUFFIX)

clean-server:
	@rm -fv $(SERVER_OBJS) $(APPSERVER)$(BIN_SUFFIX)

clean: clean-enet clean-client clean-server

%.h.gch: %.h
	$(CXX) $(CXXFLAGS) -x c++-header -o $(subst .h.gch,.tmp.h.gch,$@) $(subst .h.gch,.h,$@)
	$(MV) $(subst .h.gch,.tmp.h.gch,$@) $@

%-standalone.o: %.cpp
	$(CXX) $(CXXFLAGS) -c -o $@ $(subst -standalone.o,.cpp,$@)

$(CLIENT_OBJS): CXXFLAGS += $(CLIENT_INCLUDES)
$(filter shared/%,$(CLIENT_OBJS)): $(filter shared/%,$(CLIENT_PCH))
$(filter engine/%,$(CLIENT_OBJS)): $(filter engine/%,$(CLIENT_PCH))
$(filter game/%,$(CLIENT_OBJS)): $(filter game/%,$(CLIENT_PCH))

$(SERVER_OBJS): CXXFLAGS += $(SERVER_INCLUDES)

$(APPCLIENT)_windows$(BIN_SUFFIX): $(CLIENT_OBJS)
	$(WINDRES) -i $(APPNAME).rc -J rc -o $(APPNAME).res -O coff
	$(CXX) $(CXXFLAGS) -o $@ $(APPNAME).res $(CLIENT_OBJS) $(CLIENT_LIBS)

$(APPSERVER)_windows$(BIN_SUFFIX): $(SERVER_OBJS)
	$(WINDRES) -i $(APPNAME).rc -J rc -o $(APPNAME).res -O coff
	$(CXX) $(CXXFLAGS) -o $@ $(APPNAME).res $(SERVER_OBJS) $(SERVER_LIBS)

$(APPCLIENT)$(BIN_SUFFIX): $(LIBENET) $(CLIENT_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(CLIENT_OBJS) $(CLIENT_LIBS)

$(APPSERVER)$(BIN_SUFFIX): $(LIBENET) $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -o $@ $(SERVER_OBJS) $(SERVER_LIBS)

client: $(APPCLIENT)$(APPMODIFIER)$(BIN_SUFFIX)

server: $(APPSERVER)$(APPMODIFIER)$(BIN_SUFFIX)

$(INSTDIR)/%$(BIN_SUFFIX): %$(APPMODIFIER)$(BIN_SUFFIX)
	$(MKDIR) $(INSTDIR)
	$(CP) $< $@
ifneq (,$(STRIP))
	$(STRIP) $@
endif

install-client: $(INSTDIR)/$(APPCLIENT)$(BIN_SUFFIX)

install-server: $(INSTDIR)/$(APPSERVER)$(BIN_SUFFIX)

install: install-client install-server

$(LIBENET):
	$(MAKE) -C enet

depend:
	makedepend -Y -I. -Ishared -Iengine -Igame $(subst .o,.cpp,$(CLIENT_OBJS))
	makedepend -a -o-standalone.o -Y -I. -Ishared -Iengine -Igame -DSTANDALONE $(subst -standalone.o,.cpp,$(SERVER_OBJS))
	makedepend -a -o.h.gch -Y -I. -Ishared -Iengine -Igame $(subst .h.gch,.h,$(CLIENT_PCH))

all: client server

#include system-install.mk
#include dist.mk

# DO NOT DELETE
engine/engine.h.gch: engine/version.h shared/cube.h.gch

shared/crypto.o: shared/cube.h shared/tools.h shared/geom.h shared/ents.h
shared/crypto.o: shared/command.h shared/glexts.h shared/glemu.h
shared/crypto.o: shared/iengine.h shared/igame.h
shared/geom.o: shared/cube.h shared/tools.h shared/geom.h shared/ents.h
shared/geom.o: shared/command.h shared/glexts.h shared/glemu.h
shared/geom.o: shared/iengine.h shared/igame.h
shared/glemu.o: shared/cube.h shared/tools.h shared/geom.h shared/ents.h
shared/glemu.o: shared/command.h shared/glexts.h shared/glemu.h
shared/glemu.o: shared/iengine.h shared/igame.h
shared/stream.o: shared/cube.h shared/tools.h shared/geom.h shared/ents.h
shared/stream.o: shared/command.h shared/glexts.h shared/glemu.h
shared/stream.o: shared/iengine.h shared/igame.h
shared/tools.o: shared/cube.h shared/tools.h shared/geom.h shared/ents.h
shared/tools.o: shared/command.h shared/glexts.h shared/glemu.h
shared/tools.o: shared/iengine.h shared/igame.h
shared/zip.o: shared/cube.h shared/tools.h shared/geom.h shared/ents.h
shared/zip.o: shared/command.h shared/glexts.h shared/glemu.h
shared/zip.o: shared/iengine.h shared/igame.h
engine/aa.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h shared/geom.h
engine/aa.o: shared/ents.h shared/command.h shared/glexts.h shared/glemu.h
engine/aa.o: shared/iengine.h shared/igame.h engine/world.h engine/octa.h
engine/aa.o: engine/light.h engine/texture.h engine/bih.h engine/model.h
engine/bih.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h shared/geom.h
engine/bih.o: shared/ents.h shared/command.h shared/glexts.h shared/glemu.h
engine/bih.o: shared/iengine.h shared/igame.h engine/world.h engine/octa.h
engine/bih.o: engine/light.h engine/texture.h engine/bih.h engine/model.h
engine/blend.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h shared/geom.h
engine/blend.o: shared/ents.h shared/command.h shared/glexts.h shared/glemu.h
engine/blend.o: shared/iengine.h shared/igame.h engine/world.h engine/octa.h
engine/blend.o: engine/light.h engine/texture.h engine/bih.h engine/model.h
engine/client.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h shared/geom.h
engine/client.o: shared/ents.h shared/command.h shared/glexts.h
engine/client.o: shared/glemu.h shared/iengine.h shared/igame.h
engine/client.o: engine/world.h engine/octa.h engine/light.h engine/texture.h
engine/client.o: engine/bih.h engine/model.h
engine/command.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h shared/geom.h
engine/command.o: shared/ents.h shared/command.h shared/glexts.h
engine/command.o: shared/glemu.h shared/iengine.h shared/igame.h
engine/command.o: engine/world.h engine/octa.h engine/light.h
engine/command.o: engine/texture.h engine/bih.h engine/model.h
engine/console.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h shared/geom.h
engine/console.o: shared/ents.h shared/command.h shared/glexts.h
engine/console.o: shared/glemu.h shared/iengine.h shared/igame.h
engine/console.o: engine/world.h engine/octa.h engine/light.h
engine/console.o: engine/texture.h engine/bih.h engine/model.h
engine/dynlight.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h shared/geom.h
engine/dynlight.o: shared/ents.h shared/command.h shared/glexts.h
engine/dynlight.o: shared/glemu.h shared/iengine.h shared/igame.h
engine/dynlight.o: engine/world.h engine/octa.h engine/light.h
engine/dynlight.o: engine/texture.h engine/bih.h engine/model.h
engine/grass.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h shared/geom.h
engine/grass.o: shared/ents.h shared/command.h shared/glexts.h shared/glemu.h
engine/grass.o: shared/iengine.h shared/igame.h engine/world.h engine/octa.h
engine/grass.o: engine/light.h engine/texture.h engine/bih.h engine/model.h
engine/light.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h shared/geom.h
engine/light.o: shared/ents.h shared/command.h shared/glexts.h shared/glemu.h
engine/light.o: shared/iengine.h shared/igame.h engine/world.h engine/octa.h
engine/light.o: engine/light.h engine/texture.h engine/bih.h engine/model.h
engine/main.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h shared/geom.h
engine/main.o: shared/ents.h shared/command.h shared/glexts.h shared/glemu.h
engine/main.o: shared/iengine.h shared/igame.h engine/world.h engine/octa.h
engine/main.o: engine/light.h engine/texture.h engine/bih.h engine/model.h
engine/material.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h shared/geom.h
engine/material.o: shared/ents.h shared/command.h shared/glexts.h
engine/material.o: shared/glemu.h shared/iengine.h shared/igame.h
engine/material.o: engine/world.h engine/octa.h engine/light.h
engine/material.o: engine/texture.h engine/bih.h engine/model.h
engine/menus.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h shared/geom.h
engine/menus.o: shared/ents.h shared/command.h shared/glexts.h shared/glemu.h
engine/menus.o: shared/iengine.h shared/igame.h engine/world.h engine/octa.h
engine/menus.o: engine/light.h engine/texture.h engine/bih.h engine/model.h
engine/movie.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h shared/geom.h
engine/movie.o: shared/ents.h shared/command.h shared/glexts.h shared/glemu.h
engine/movie.o: shared/iengine.h shared/igame.h engine/world.h engine/octa.h
engine/movie.o: engine/light.h engine/texture.h engine/bih.h engine/model.h
engine/normal.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h shared/geom.h
engine/normal.o: shared/ents.h shared/command.h shared/glexts.h
engine/normal.o: shared/glemu.h shared/iengine.h shared/igame.h
engine/normal.o: engine/world.h engine/octa.h engine/light.h engine/texture.h
engine/normal.o: engine/bih.h engine/model.h
engine/octa.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h shared/geom.h
engine/octa.o: shared/ents.h shared/command.h shared/glexts.h shared/glemu.h
engine/octa.o: shared/iengine.h shared/igame.h engine/world.h engine/octa.h
engine/octa.o: engine/light.h engine/texture.h engine/bih.h engine/model.h
engine/octaedit.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h shared/geom.h
engine/octaedit.o: shared/ents.h shared/command.h shared/glexts.h
engine/octaedit.o: shared/glemu.h shared/iengine.h shared/igame.h
engine/octaedit.o: engine/world.h engine/octa.h engine/light.h
engine/octaedit.o: engine/texture.h engine/bih.h engine/model.h
engine/octarender.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h
engine/octarender.o: shared/geom.h shared/ents.h shared/command.h
engine/octarender.o: shared/glexts.h shared/glemu.h shared/iengine.h
engine/octarender.o: shared/igame.h engine/world.h engine/octa.h
engine/octarender.o: engine/light.h engine/texture.h engine/bih.h
engine/octarender.o: engine/model.h
engine/physics.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h shared/geom.h
engine/physics.o: shared/ents.h shared/command.h shared/glexts.h
engine/physics.o: shared/glemu.h shared/iengine.h shared/igame.h
engine/physics.o: engine/world.h engine/octa.h engine/light.h
engine/physics.o: engine/texture.h engine/bih.h engine/model.h engine/mpr.h
engine/pvs.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h shared/geom.h
engine/pvs.o: shared/ents.h shared/command.h shared/glexts.h shared/glemu.h
engine/pvs.o: shared/iengine.h shared/igame.h engine/world.h engine/octa.h
engine/pvs.o: engine/light.h engine/texture.h engine/bih.h engine/model.h
engine/rendergl.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h shared/geom.h
engine/rendergl.o: shared/ents.h shared/command.h shared/glexts.h
engine/rendergl.o: shared/glemu.h shared/iengine.h shared/igame.h
engine/rendergl.o: engine/world.h engine/octa.h engine/light.h
engine/rendergl.o: engine/texture.h engine/bih.h engine/model.h
engine/renderlights.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h
engine/renderlights.o: shared/geom.h shared/ents.h shared/command.h
engine/renderlights.o: shared/glexts.h shared/glemu.h shared/iengine.h
engine/renderlights.o: shared/igame.h engine/world.h engine/octa.h
engine/renderlights.o: engine/light.h engine/texture.h engine/bih.h
engine/renderlights.o: engine/model.h
engine/rendermodel.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h
engine/rendermodel.o: shared/geom.h shared/ents.h shared/command.h
engine/rendermodel.o: shared/glexts.h shared/glemu.h shared/iengine.h
engine/rendermodel.o: shared/igame.h engine/world.h engine/octa.h
engine/rendermodel.o: engine/light.h engine/texture.h engine/bih.h
engine/rendermodel.o: engine/model.h engine/ragdoll.h engine/animmodel.h
engine/rendermodel.o: engine/vertmodel.h engine/skelmodel.h engine/hitzone.h
engine/rendermodel.o: engine/md2.h engine/md3.h engine/md5.h engine/obj.h
engine/rendermodel.o: engine/smd.h engine/iqm.h
engine/renderparticles.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h
engine/renderparticles.o: shared/geom.h shared/ents.h shared/command.h
engine/renderparticles.o: shared/glexts.h shared/glemu.h shared/iengine.h
engine/renderparticles.o: shared/igame.h engine/world.h engine/octa.h
engine/renderparticles.o: engine/light.h engine/texture.h engine/bih.h
engine/renderparticles.o: engine/model.h engine/explosion.h
engine/renderparticles.o: engine/lensflare.h engine/lightning.h
engine/rendersky.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h
engine/rendersky.o: shared/geom.h shared/ents.h shared/command.h
engine/rendersky.o: shared/glexts.h shared/glemu.h shared/iengine.h
engine/rendersky.o: shared/igame.h engine/world.h engine/octa.h
engine/rendersky.o: engine/light.h engine/texture.h engine/bih.h
engine/rendersky.o: engine/model.h
engine/rendertext.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h
engine/rendertext.o: shared/geom.h shared/ents.h shared/command.h
engine/rendertext.o: shared/glexts.h shared/glemu.h shared/iengine.h
engine/rendertext.o: shared/igame.h engine/world.h engine/octa.h
engine/rendertext.o: engine/light.h engine/texture.h engine/bih.h
engine/rendertext.o: engine/model.h
engine/renderva.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h shared/geom.h
engine/renderva.o: shared/ents.h shared/command.h shared/glexts.h
engine/renderva.o: shared/glemu.h shared/iengine.h shared/igame.h
engine/renderva.o: engine/world.h engine/octa.h engine/light.h
engine/renderva.o: engine/texture.h engine/bih.h engine/model.h
engine/server.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h shared/geom.h
engine/server.o: shared/ents.h shared/command.h shared/glexts.h
engine/server.o: shared/glemu.h shared/iengine.h shared/igame.h
engine/server.o: engine/world.h engine/octa.h engine/light.h engine/texture.h
engine/server.o: engine/bih.h engine/model.h
engine/serverbrowser.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h
engine/serverbrowser.o: shared/geom.h shared/ents.h shared/command.h
engine/serverbrowser.o: shared/glexts.h shared/glemu.h shared/iengine.h
engine/serverbrowser.o: shared/igame.h engine/world.h engine/octa.h
engine/serverbrowser.o: engine/light.h engine/texture.h engine/bih.h
engine/serverbrowser.o: engine/model.h
engine/shader.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h shared/geom.h
engine/shader.o: shared/ents.h shared/command.h shared/glexts.h
engine/shader.o: shared/glemu.h shared/iengine.h shared/igame.h
engine/shader.o: engine/world.h engine/octa.h engine/light.h engine/texture.h
engine/shader.o: engine/bih.h engine/model.h
engine/sound.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h shared/geom.h
engine/sound.o: shared/ents.h shared/command.h shared/glexts.h shared/glemu.h
engine/sound.o: shared/iengine.h shared/igame.h engine/world.h engine/octa.h
engine/sound.o: engine/light.h engine/texture.h engine/bih.h engine/model.h
engine/stain.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h shared/geom.h
engine/stain.o: shared/ents.h shared/command.h shared/glexts.h shared/glemu.h
engine/stain.o: shared/iengine.h shared/igame.h engine/world.h engine/octa.h
engine/stain.o: engine/light.h engine/texture.h engine/bih.h engine/model.h
engine/texture.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h shared/geom.h
engine/texture.o: shared/ents.h shared/command.h shared/glexts.h
engine/texture.o: shared/glemu.h shared/iengine.h shared/igame.h
engine/texture.o: engine/world.h engine/octa.h engine/light.h
engine/texture.o: engine/texture.h engine/bih.h engine/model.h engine/scale.h
engine/ui.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h shared/geom.h
engine/ui.o: shared/ents.h shared/command.h shared/glexts.h shared/glemu.h
engine/ui.o: shared/iengine.h shared/igame.h engine/world.h engine/octa.h
engine/ui.o: engine/light.h engine/texture.h engine/bih.h engine/model.h
engine/ui.o: engine/textedit.h
engine/water.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h shared/geom.h
engine/water.o: shared/ents.h shared/command.h shared/glexts.h shared/glemu.h
engine/water.o: shared/iengine.h shared/igame.h engine/world.h engine/octa.h
engine/water.o: engine/light.h engine/texture.h engine/bih.h engine/model.h
engine/world.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h shared/geom.h
engine/world.o: shared/ents.h shared/command.h shared/glexts.h shared/glemu.h
engine/world.o: shared/iengine.h shared/igame.h engine/world.h engine/octa.h
engine/world.o: engine/light.h engine/texture.h engine/bih.h engine/model.h
engine/worldio.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h shared/geom.h
engine/worldio.o: shared/ents.h shared/command.h shared/glexts.h
engine/worldio.o: shared/glemu.h shared/iengine.h shared/igame.h
engine/worldio.o: engine/world.h engine/octa.h engine/light.h
engine/worldio.o: engine/texture.h engine/bih.h engine/model.h
game/ai.o: game/game.h shared/cube.h shared/tools.h shared/geom.h
game/ai.o: shared/ents.h shared/command.h shared/glexts.h shared/glemu.h
game/ai.o: shared/iengine.h shared/igame.h game/ai.h
game/client.o: game/game.h shared/cube.h shared/tools.h shared/geom.h
game/client.o: shared/ents.h shared/command.h shared/glexts.h shared/glemu.h
game/client.o: shared/iengine.h shared/igame.h game/ai.h
game/entities.o: game/game.h shared/cube.h shared/tools.h shared/geom.h
game/entities.o: shared/ents.h shared/command.h shared/glexts.h
game/entities.o: shared/glemu.h shared/iengine.h shared/igame.h game/ai.h
game/game.o: game/game.h shared/cube.h shared/tools.h shared/geom.h
game/game.o: shared/ents.h shared/command.h shared/glexts.h shared/glemu.h
game/game.o: shared/iengine.h shared/igame.h game/ai.h
game/render.o: game/game.h shared/cube.h shared/tools.h shared/geom.h
game/render.o: shared/ents.h shared/command.h shared/glexts.h shared/glemu.h
game/render.o: shared/iengine.h shared/igame.h game/ai.h
game/server.o: game/game.h shared/cube.h shared/tools.h shared/geom.h
game/server.o: shared/ents.h shared/command.h shared/glexts.h shared/glemu.h
game/server.o: shared/iengine.h shared/igame.h game/ai.h
game/server.o: game/aiman.h
game/waypoint.o: game/game.h shared/cube.h shared/tools.h shared/geom.h
game/waypoint.o: shared/ents.h shared/command.h shared/glexts.h
game/waypoint.o: shared/glemu.h shared/iengine.h shared/igame.h game/ai.h
game/weapon.o: game/game.h shared/cube.h shared/tools.h shared/geom.h
game/weapon.o: shared/ents.h shared/command.h shared/glexts.h shared/glemu.h
game/weapon.o: shared/iengine.h shared/igame.h game/ai.h

shared/cube.h.gch: shared/tools.h shared/geom.h shared/ents.h
shared/cube.h.gch: shared/command.h shared/glexts.h shared/glemu.h
shared/cube.h.gch: shared/iengine.h shared/igame.h
engine/engine.h.gch: shared/cube.h shared/tools.h shared/geom.h shared/ents.h
engine/engine.h.gch: shared/command.h shared/glexts.h shared/glemu.h
engine/engine.h.gch: shared/iengine.h shared/igame.h engine/world.h
engine/engine.h.gch: engine/octa.h engine/light.h engine/texture.h
engine/engine.h.gch: engine/bih.h engine/model.h
game/game.h.gch: shared/cube.h shared/tools.h shared/geom.h shared/ents.h
game/game.h.gch: shared/command.h shared/glexts.h shared/glemu.h
game/game.h.gch: shared/iengine.h shared/igame.h game/ai.h

shared/crypto-standalone.o: shared/cube.h shared/tools.h shared/geom.h
shared/crypto-standalone.o: shared/ents.h shared/command.h shared/iengine.h
shared/crypto-standalone.o: shared/igame.h
shared/stream-standalone.o: shared/cube.h shared/tools.h shared/geom.h
shared/stream-standalone.o: shared/ents.h shared/command.h shared/iengine.h
shared/stream-standalone.o: shared/igame.h
shared/tools-standalone.o: shared/cube.h shared/tools.h shared/geom.h
shared/tools-standalone.o: shared/ents.h shared/command.h shared/iengine.h
shared/tools-standalone.o: shared/igame.h
engine/command-standalone.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h
engine/command-standalone.o: shared/geom.h shared/ents.h shared/command.h
engine/command-standalone.o: shared/iengine.h shared/igame.h engine/world.h
engine/server-standalone.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h
engine/server-standalone.o: shared/geom.h shared/ents.h shared/command.h
engine/server-standalone.o: shared/iengine.h shared/igame.h engine/world.h
engine/worldio-standalone.o: engine/engine.h engine/version.h shared/cube.h shared/tools.h
engine/worldio-standalone.o: shared/geom.h shared/ents.h shared/command.h
engine/worldio-standalone.o: shared/iengine.h shared/igame.h engine/world.h
game/entities-standalone.o: game/game.h shared/cube.h shared/tools.h
game/entities-standalone.o: shared/geom.h shared/ents.h shared/command.h
game/entities-standalone.o: shared/iengine.h shared/igame.h game/ai.h
game/server-standalone.o: game/game.h shared/cube.h shared/tools.h
game/server-standalone.o: shared/geom.h shared/ents.h shared/command.h
game/server-standalone.o: shared/iengine.h shared/igame.h game/ai.h
game/server-standalone.o: game/ctf.h game/extinfo.h game/aiman.h

shared/cube-standalone.h.gch: shared/tools.h shared/geom.h shared/ents.h
shared/cube-standalone.h.gch: shared/command.h shared/iengine.h
shared/cube-standalone.h.gch: shared/igame.h
engine/engine-standalone.h.gch: shared/cube.h shared/tools.h shared/geom.h
engine/engine-standalone.h.gch: shared/ents.h shared/command.h
engine/engine-standalone.h.gch: shared/iengine.h shared/igame.h
engine/engine-standalone.h.gch: engine/world.h
game/game-standalone.h.gch: shared/cube.h shared/tools.h shared/geom.h
game/game-standalone.h.gch: shared/ents.h shared/command.h shared/iengine.h
game/game-standalone.h.gch: shared/igame.h game/ai.h
