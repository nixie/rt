CXXFLAGS=-Wall -pedantic -W -ggdb -std=c++0x
#CFLAGS=-Wall -pedantic -W -g -DNDEBUG
LDFLAGS=-D_GNU_SOURCE=1 -D_REENTRANT -I/usr/include/SDL -lSDL -lpthread -lm
DEMO_NAME=testing
SOURCES=*.cpp *.h *.sh *.doc *.py Eigen
PKG_NAME=xferra00

all: rt

rt: main.cpp common.o raymarch.o params.o formulae.o light.o math.o
	$(CXX) $(CXXFLAGS) -o $@ $^ $(LDFLAGS)

%.o: %.cpp %.h
	$(CXX) -c $(CXXFLAGS) $< -o $@

inter:
	./interpolation.py demo_$(DEMO_NAME) demo_$(DEMO_NAME)


render: demo_$(DEMO_NAME)/keyframe_*.txt
	rm -f demo_$(DEMO_NAME)/config_*.txt
	rm -f demo_$(DEMO_NAME)/*.bmp
	./interpolation.py demo_$(DEMO_NAME) demo_$(DEMO_NAME)
	time "./render_frames.sh" "demo_$(DEMO_NAME)"

demo_$(DEMO_NAME).mp4: demo_$(DEMO_NAME)/*.bmp
	echo 'Joining frames into movie ...'
	ffmpeg -r 30 -i demo_$(DEMO_NAME)/config_%05d.txt.bmp $@ 

clean:
	rm -f *.o
	rm -f rt
	rm -f demo_$(DEMO_NAME)/*
	rm -f $(PKG_NAME).zip


run: rt
	./rt

pack:
	mkdir $(PKG_NAME);
	cp -r $(SOURCES) $(PKG_NAME);
	zip -r $(PKG_NAME).zip $(PKG_NAME);
	rm -rf $(PKG_NAME);

.PHONY: clean render interpolate all run pack
