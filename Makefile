
TARG=	electro
OBJS=	src/opengl.o \
	src/shared.o \
	src/server.o \
	src/client.o \
	src/entity.o \
	src/camera.o \
	src/sprite.o \
	src/object.o \
	src/light.o  \
	src/script.o \
	src/galaxy.o \
	src/node.o   \
	src/star.o   \
	src/image.o  \
	src/main.o

DEPS= $(OBJS:.o=.d)

#------------------------------------------------------------------------------

CC= mpicc
RM= rm -f

SDL_LIBS= $(shell sdl-config --libs)
LUA_LIBS= -llua50 -llualib50
PNG_LIBS= -lpng -lz -lm

CFLAGS= -O2 -Wall $(shell sdl-config --cflags) -DMPI -DNDEBUG
INCDIR= -I$(HOME)/include -I/usr/include/lua50
LIBDIR= -L$(HOME)/lib

ifeq ($(shell uname), Darwin)
	LIBS= $(SDL_LIBS) $(LUA_LIBS) $(PNG_LIBS) $(MPI_LIBS)
else
	LIBS= $(SDL_LIBS) $(LUA_LIBS) $(PNG_LIBS) $(MPI_LIBS) -lGL -lGLU
endif

#------------------------------------------------------------------------------

%.d : %.c
	$(CC) $(CFLAGS) $(INCDIR) -MM -MF $@ -MT $*.o $<

%.o : %.c
	$(CC) $(CFLAGS) $(INCDIR) -c -o $@ $<

#------------------------------------------------------------------------------

$(TARG) : $(OBJS)
	$(CC) $(CFLAGS) -o $(TARG) $(OBJS) $(LIBDIR) $(LIBS)

clean :
	$(RM) $(TARG) $(OBJS) $(DEPS)

#------------------------------------------------------------------------------

include $(DEPS)
