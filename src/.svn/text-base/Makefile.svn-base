#----------------------------------
# example Makefile used to generate
# binary .so file
#----------------------------------

XX			= g++
LIBS		= -L. -L../../common-without-thrift/  -L../../libaccount -lcommon  -laccount -lmysqlclient
INCLUDES	= -I. -I../../common-without-thrift -I../../async-server/include  -I../../libaccount/include -I/usr/include/libaccount
FLAGS  		= -std=c++0x -Wall -pipe -D_GNU_SOURCE -fPIC -shared -export-dynamic -ggdb -Wno-non-virtual-dtor
SOURCE		= $(wildcard *.cpp)
OBJS		= $(patsubst %.cpp, %.o, $(SOURCE))
TARGET		= ../bin/sync-handler-center.so

all: $(TARGET)

$(TARGET): $(OBJS) $(SUBOBJS)
	$(XX) $(FLAGS) $(OBJS) $(SUBOBJS) -o $(TARGET) $(LIBS)

%.o: %.cpp Makefile
	$(XX) $(FLAGS) $(INCLUDES) -c $< -o $@

$(SUBOBJS):
	 @make_all_sub_dir()\
    {\
        for dir in `ls`; do \
            if [ -d $$dir ]; then \
                cd $$dir;\
                make_all_sub_dir;\
                if [ -f makefile -o -f Makefile ]; then\
                    make;\
                fi;\
                cd ..;\
            fi;\
        done;\
    };\
    make_all_sub_dir

clean:
	rm -rf *.o *.d $(TARGET) $(SUBOBJS) *.so*
	@make_clean_sub_dir()\
    {\
        for dir in `ls`; do \
            if [ -d $$dir ]; then \
                cd $$dir;\
                make_clean_sub_dir;\
                if [ -f makefile -o -f Makefile ]; then\
                    make clean;\
                fi;\
                cd ..;\
            fi;\
        done;\
    };\
    make_clean_sub_dir


#clean:
#	rm -rf *.o .dep.* $(TARGET)

c: clean

-include /dev/null $(wildcard .dep.*)
