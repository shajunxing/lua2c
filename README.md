The purpose of this tool is to convert a single or multiple LUA files into a unique compilable C file, which can be compiled to generate a single executable program. Compared with other similar tools such as luastatic or same name lua2c, there are two advantages: first is debugging symbol is removed from compiled binary code, which is equivalent to -s parameter of luac, which is equivalent to obfuscating code. The second is code for all modules is loaded on demand, that is, overall behavior is same as that of direct execution of LUA file.

My development environment is Windows WDK 7600.16385.1, I like its link to msvcrt.dll feature, code is standard C, other systems should be able to compile, just modify makefile slightly, in addition to lua2c, it also contains two other gadgets, bin2c and txt2c, which I used to develop lua2c, you may not need it.

How to Use: `lua2c Entry filename -o Output filename -m Module filename1 -m Module filename2 ...` The entry file name is required, which is the lua file directly executed, output file name and module file name are optional, if output file name is not specified, default is stdout, module file name refers to 'require' lua file, if there is no require, of course you don't need to fill it. Makefile also contains a sample test case.

该工具的用途是把单个或者多个lua文件转换为唯一的可编译的c文件，可以编译生成单一的可执行程序。相比其它类似工具比如luastatic或同名的lua2c等，有两个优点：第一是编译后的二进制代码里去除了调试符号，相当于luac的-s参数，也就相当于混淆代码。第二是所有模块代码是按需加载的，也就是整体行为和直接执行lua文件保持一致。

我的开发环境是Windows WDK 7600.16385.1，我喜欢它的链接到msvcrt.dll的特性，代码都是标准C，其它系统应该都能编译，稍微修改一下makefile即可，除了lua2c之外，还包含了另外两个小工具，bin2c和txt2c，是我开发lua2c用的，你们应该不需要。

使用方法：`lua2c 入口文件名 -o 输出文件名 -m 模块文件名1 -m 模块文件名2 ...` 其中入口文件名是必填项，就是直接执行的那个lua文件，输出文件名和模块文件名都是可选项，如果不指定输出文件名，默认是stdout，模块文件名指的是require的lua文件，如果没有require当然就不需要填写了。makefile里面也包含了一个测试样例。