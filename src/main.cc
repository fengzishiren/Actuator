//============================================================================
// Name        : Actuator.cpp
// Author      : Lunatic
// Version     :
// Copyright   : Your copyright notice
// Description : Hello World in C++, Ansi-style
//============================================================================

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>

#include "alarm.h"
#include "engine.h"

#define NAME        "SonarXS"
#define VERSION     NAME "1.0"
#define COPYRIGHT    VERSION "  Copyright (C) 1990-2014  Lunatic Zheng, ZH"

static const char *progname = NAME;

static void help() {
    fprintf(stderr, "usage: %s [options] [filenames]\n"
            "  -name           read file named name(omit this argument)\n"
            "  -d <level>      set the logging level\n"
            "  -v              show version information\n"
            "  --version       show version information\n"
            "  -h              help information\n"
            "  --help          help information\n", progname);
}

static void usage(const char *message) {
    if (*message == '-')
        fprintf(stderr, "%s: unrecognized option %s\n", progname, message);
    else
        fprintf(stderr, "%s: %s\n", progname, message);
    help();
    exit(EXIT_FAILURE);
}

//not found return -1
static int str2level(const std::string &lv) {
    int level = -1;
    if (lv == "DEBUG")
        level = Script::Log::DEBUG;
    else if (lv == "INFO")
        level = Script::Log::INFO;
    else if (lv == "WARN")
        level = Script::Log::WARN;
    else if (lv == "ERROR")
        level = Script::Log::ERROR;
    return level;

}

static int doargs(int argc, char *argv[]) {
#define IS(s) (strcmp(argv[i],s)==0)

    int i, name = 0, version = 0, helpp = 0, log = 0;

    if (argv[0] != NULL && *argv[0] != 0)
        progname = argv[0];

    for (i = 1; i < argc; i++) {
        if (*argv[i] != '-')
            break;
        else if (IS("-name")) {
            if (i + 1 < argc) {
                name = ++i;
            } else
                usage("Illegal args!");
        } else if (IS("-d")) {
            if (i + 1 < argc) {
                log = str2level(argv[i + 1]);
                if (log == -1)
                    usage(argv[i + 1]);
                i++;
            } else
                usage("Illegal args!");

        } else if (IS("-h") || IS("--help"))
            helpp++;
        else if (IS("-v") || IS("--version"))
            ++version;
        else
            /* unknown option */
            usage(argv[i]);
    }
    if (version) {
        printf("%s\n", COPYRIGHT);
        if (version == argc - 1)
            exit(EXIT_SUCCESS);
    }
    if (helpp) {
        help();
        if (helpp == argc - 1)
            exit(EXIT_SUCCESS);
    }
    if (log) {
        Script::Log::level = (Script::Log::Level) log;
    }
#undef IS
    return name == 0 ? i : name;
}

static std::string &get_code(const std::string &file, std::string &code) {
    std::ifstream is(file.c_str());
    if (!is) {
        fprintf(stderr, "%s: not found this file '%s'.\n", progname, file.c_str());
        exit(EXIT_FAILURE);
    }
    std::string s;
    while (std::getline(is, s)) {
        code.append(s) += '\n';//注意： 各平台的换行符不能统一 在读取的时候统一用'\n'替换
    }
    return code;
}

int main(int argc, char **argv) {
    Script::Log::level = Script::Log::WARN;

    int i = doargs(argc, argv);
    argc -= i;
    argv += i;
    if (argc <= 0)
        usage("no input files given");

    Script::Log::debug("Start:");

    std::string text;
    Script::Log::debug("读入文件：%s ", *argv);
    get_code(*argv, text);

//    Script::Env env; //执行环境
//    env.load(text);
//
//    Script::Engine engine; //执行器
//    try {
//        engine.launch(env); //执行
//    } catch (const char *s) {
//        std::cout << s << std::endl;
//    }
//
//    Script::Log::debug("End!");

    return 0;
}

