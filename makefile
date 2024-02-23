# 注意这些宏会覆盖环境变量，比如取名LIB就会和环境变量的LIB冲突，导致Link报错
# nmake /e可以倒过来用环境变量覆盖同名宏，但是显然也是问题
# 比如含有中文注释的时候，源文件是utf-8的，而控制台是cp936的，就会：warning C4819: The file contains a character that cannot be represented in the current code page (936). Save the file in Unicode format to prevent data loss
CC=cl.exe /c /nologo /O2 /MD /wd4819
LINK=link.exe /nologo

all: bin2c.exe txt2c.exe lua2c.exe

clean:
    del /q *.dll *.exe *.obj *.lib *.exp *.bin test.c

bin2c.exe: bin2c.c
    $(CC) bin2c.c
    $(LINK) bin2c.obj

txt2c.exe: txt2c.c
    $(CC) txt2c.c
    $(LINK) txt2c.obj

lua2c.exe: lua2c.c
    $(CC) /DLUA_BUILD_AS_DLL lua2c.c
    $(LINK) lua2c.obj lua51.lib

test.exe: lua2c.exe test_require.lua test_module_1.lua test_module_2.lua
    lua2c -o test.c test_require.lua test_module_1.lua test_module_2.lua
    $(CC) test.c
    $(LINK) test.obj lua51.lib

