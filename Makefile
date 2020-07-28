# make命令可以根据目标文件和依赖文件的修改时间，判断是否需要重新编译。
# 定义伪目标：不会生成同名文件
.PHONY: all clean

CFLAGS = -D_GNU_SOURCE

all: tts robot snowboy

# $^ 代表所有依赖文件
# $@ 代表目标文件
tts: tts.c cJSON.c auth.c
	cc -Wall -D_GNU_SOURCE $^ -o $@ -lcurl

robot: robot.c cJSON.c
	cc -Wall $^ -o $@ -lcurl

stt: stt.c cJSON.c auth.c
	cc -Wall -D_GNU_SOURCE $^ -o $@ -lcurl

snowboy: snowboy.o stt.o cJSON.o auth.o snowboy-detect-c-wrapper.o libsnowboy-detect.a
	cc $^ -lstdc++ -lm -lcblas -lasound -lcurl -o $@

snowboy.o: snowboy.c
	cc -c $^

snowboy-detect-c-wrapper.o: snowboy-detect-c-wrapper.cc
	c++ -D_GLIBCXX_USE_CXX11_ABI=0 -c $^

clean:
	rm -f *.o tts robot snowboy
