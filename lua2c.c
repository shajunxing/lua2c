/*
The MIT License (MIT)

Copyright (C) 2024 shajunxing

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the “Software”), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>
#include <lstate.h>
#include <lundump.h>

// 注意检查%s，如果需要原样输出的必须改为%%s
static const char *template[] = {
    "#include <stdio.h>\n"
    "#include <stdlib.h>\n"
    "#include <assert.h>\n"
    "#include <errno.h>\n"
    "#include <lua.h>\n"
    "#include <lualib.h>\n"
    "#include <lauxlib.h>\n"
    "\n",
    "static const unsigned char %s[] = {",
    "};\n\n",
    "typedef struct {\n"
    "    const char *name;\n"
    "    const unsigned char *buff;\n"
    "    int sz;\n"
    "} embedded_module;\n"
    "\n"
    "static const embedded_module modules[] = {\n",
    "    {\"%s\", %s, sizeof(%s)},\n",
    "};\n"
    "\n"
    "static const int num_modules = sizeof(modules) / sizeof(embedded_module);\n"
    "\n"
    "static int loader_Embed(lua_State *L) {\n"
    "    const char *name;\n"
    "    int i;\n"
    "    embedded_module m;\n"
    "    const char *buff;\n"
    "    size_t sz;\n"
    "    name = luaL_checkstring(L, 1);\n"
    "    for (i = 0; i < num_modules; i++) {\n"
    "        m = modules[i];\n"
    "        if (0 == strcmp(name, m.name)) {\n"
    "            if (luaL_loadbuffer(L, m.buff, m.sz, name) != 0) {\n"
    "                luaL_error(L, \"error loading embedded module \\\"%%s\\\"\", name);\n"
    "            }\n"
    "            return 1; /* library loaded successfully */\n"
    "        }\n"
    "    }\n"
    "    return 1; /* library not found in this path */\n"
    "}\n"
    "\n"
    "int main(int argc, char **argv) {\n"
    "    lua_State *L;\n"
    "    int i;\n"
    "    int status;\n"
    "    char *msg;\n"
    "    L = luaL_newstate();\n"
    "    luaL_openlibs(L);\n"
    "    // setup global variable \"arg\"\n"
    "    lua_newtable(L);\n"
    "    for (i = 1; i < argc; i++) {\n"
    "        lua_pushinteger(L, i - 1);\n"
    "        lua_pushstring(L, argv[i]);\n"
    "        lua_settable(L, -3);\n"
    "    }\n"
    "    lua_setglobal(L, \"arg\");\n"
    "    // add my custom loader\n"
    "    lua_getglobal(L, \"package\");\n"
    "    lua_getfield(L, -1, \"loaders\");\n"
    "    assert(lua_istable(L, -1));\n"
    "    lua_getglobal(L, \"table\");\n"
    "    lua_getfield(L, -1, \"insert\");\n"
    "    assert(lua_isfunction(L, -1));\n"
    "    lua_pushvalue(L, -3);\n"
    "    lua_pushinteger(L, 1);\n"
    "    lua_pushcfunction(L, loader_Embed);\n"
    "    lua_call(L, 3, LUA_MULTRET);\n"
    "    luaL_loadbuffer(L, %s, sizeof(%s), 0);\n"
    "    status = lua_pcall(L, 0, LUA_MULTRET, 0);\n"
    "    if (status && !lua_isnil(L, -1)) {\n"
    "        msg = (char *)lua_tostring(L, -1);\n"
    "        if (msg == 0) {\n"
    "            msg = \"(error object is not a string)\";\n"
    "        }\n"
    "        fprintf(stderr, \"Error: %%s\\n\", msg);\n"
    "        lua_pop(L, 1);\n"
    "    }\n"
    "    lua_close(L);\n"
    "    return status;\n"
    "}\n",
};

typedef struct {
    char *fname;
    char *name;
    unsigned char *buff;
    int sz;
} Chunk;

// 注意，一次luaU_dump调用，writer会多次调用
static int dump_writer(lua_State *L, const void *p, size_t sz, void *ud) {
    Chunk *m;
    int i;
    m = (Chunk *)ud;
    m->buff = (unsigned char *)realloc(m->buff, m->sz + sz);
    memcpy(m->buff + m->sz, p, sz);
    m->sz += sz;
    return 0;
}

#define api_checknelems(L, n) api_check(L, (n) <= (L->top - L->base))

// 复制自lapi.c的lua_dump()
// luac.c里的带有combine()操作，组合多个源，此处用不到，反正我也没看懂
static int strip_dump(lua_State *L, Chunk *m) {
    int status;
    TValue *o;
    lua_lock(L);
    api_checknelems(L, 1);
    o = L->top - 1;
    if (isLfunction(o)) {
        status = luaU_dump(L, clvalue(o)->l.p, dump_writer, m, 1); // 最后的参数改为1，表示strip
    } else {
        status = 1;
    }
    lua_unlock(L);
    return status;
}

#ifdef _WIN32
    #define pathsep '\\'
#else
    #define pathsep '/'
#endif

// 去掉路径和扩展名
static char *strip_filename(char *fname) {
    char *l, *r;
    char *ret;
    l = strrchr(fname, pathsep);
    l = l == 0 ? fname : (l + 1);
    r = strrchr(fname, '.');
    r = r == 0 ? (fname + strlen(fname)) : r;
    if (l > r) {
        return 0;
    }
    ret = (char *)calloc(r - l + 1, 1);
    memcpy(ret, l, r - l);
    return ret;
}

void bin2c(FILE *fp, char *buff, int sz) {
    int i;
    int first = 1;
    for (i = 0; i < sz; i++) {
        if (first) {
            first = 0;
        } else {
            fprintf(fp, ", ");
        }
        fprintf(fp, "%d", buff[i]);
    }
}

int main(int argc, char *argv[]) {
    lua_State *L;
    char *ofname = 0;
    FILE *out;
    Chunk *entrance = 0;
    Chunk *modules = 0;
    int num_modules = 0;
    char *arg;
    char prev_flag = 0;
    char curr_flag = 0;
    int i;
    char *s;
    if (argc < 2) {
        printf("Usage: %s <entry filename> [-o output filename] [-m module filename1 [-m module filename2 ...]]\n", argv[0]);
        printf("Convert single or multiple lua files into one unique compilable C file, which can be compiled to generate single executable program.\n");
        printf("Entry filename is required, that is, the lua file that is directly executed, output filename and module filename are optional, if output filename is not specified, default is stdout, module filename refers to the \"require\" lua file, if there is no require, of course, you don't need to fill it.\n");
        return -1;
    }
    // 解析参数
    for (i = 1; i < argc; i++) {
        arg = argv[i];
        if (0 == strcmp(arg, "-o")) {
            curr_flag = 'o';
        } else if (0 == strcmp(arg, "-m")) {
            curr_flag = 'm';
        } else {
            curr_flag = 0;
        }
        if (prev_flag) {
            if (curr_flag) {
                fprintf(stderr, "Argument \"%s\" cannot be followed by \"%s\".\n", argv[i - 1], arg);
                return -1;
            } else {
                switch (prev_flag) {
                case 'o':
                    if (ofname) {
                        fprintf(stderr, "Cannot specify more than one output file.\n");
                        return -1;
                    }
                    ofname = arg;
                    break;
                case 'm':
                    if (0 == (s = strip_filename(arg))) {
                        fprintf(stderr, "Invalid module file name \"%s\".\n", arg);
                        return -1;
                    }
                    num_modules++;
                    modules = (Chunk *)realloc(modules, sizeof(Chunk) * num_modules);
                    modules[num_modules - 1].fname = arg;
                    modules[num_modules - 1].name = s;
                    modules[num_modules - 1].buff = 0;
                    modules[num_modules - 1].sz = 0;
                    break;
                }
            }
        } else {
            if (!curr_flag) {
                if (entrance) {
                    fprintf(stderr, "Only one entrance is allowed.\n");
                    return -1;
                }
                if (0 == (s = strip_filename(arg))) {
                    fprintf(stderr, "Invalid entrance file name \"%s\".\n", arg);
                    return -1;
                }
                entrance = (Chunk *)calloc(1, sizeof(Chunk));
                entrance->fname = arg;
                entrance->name = s;
            }
        }
        prev_flag = curr_flag;
    }
    if (0 == entrance) {
        fprintf(stderr, "Must specify entrance file.\n");
        return -1;
    }
    // if (ofname) {
    //     printf("ofname=%s\n", ofname);
    // }
    // printf("entrance->fname=%s\n", entrance->fname);
    // printf("entrance->name=%s\n", entrance->name);
    // for (i = 0; i < num_modules; i++) {
    //     printf("modules[%d].fname=%s\n", i, modules[i].fname);
    //     printf("modules[%d].name=%s\n", i, modules[i].name);
    // }
    // 编译源文件
    L = luaL_newstate();
    luaL_openlibs(L);
    luaL_loadfile(L, entrance->fname);
    strip_dump(L, entrance);
    // for (i = 0; i < entrance->sz; i++) {
    //     printf("%d ", entrance->buff[i]);
    // }
    for (i = 0; i < num_modules; i++) {
        luaL_loadfile(L, modules[i].fname);
        strip_dump(L, &modules[i]);
    }
    lua_close(L);
    // 组装输出
    if (ofname) {
        out = fopen(ofname, "w");
        if (0 == out) {
            perror("Failed to open output file");
            return -1;
        }
    } else {
        out = stdout;
    }
    fprintf(out, template[0]);
    fprintf(out, template[1], entrance->name);
    bin2c(out, entrance->buff, entrance->sz);
    fprintf(out, template[2]);
    for (i = 0; i < num_modules; i++) {
        fprintf(out, template[1], modules[i].name);
        bin2c(out, modules[i].buff, modules[i].sz);
        fprintf(out, template[2]);
    }
    fprintf(out, template[3]);
    for (i = 0; i < num_modules; i++) {
        fprintf(out, template[4], modules[i].name, modules[i].name, modules[i].name);
    }
    fprintf(out, template[5], entrance->name, entrance->name);
    fclose(out);
    return 0;
}
