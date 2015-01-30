/*
 * main.cc
 *
 *      Author: fengzishiren
 */

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>

#include "tool.h"
#include "interp.h"

#define NAME        "SonarXS"
#define VERSION     NAME "1.0"
#define COPYRIGHT    VERSION "  Copyright (C) 2014-2015  Lunatic Zheng, ZH"

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

static int str2level(std::string lv) {
    Script::Log::Level level;
    if (Script::upper(lv) == "DEBUG")
        level = Script::Log::DEBUG;
    else if (Script::upper(lv) == "INFO")
        level = Script::Log::INFO;
    else if (Script::upper(lv) == "WARN")
        level = Script::Log::WARN;
    else if (Script::upper(lv) == "ERROR")
        level = Script::Log::ERROR;
    else
        level = Script::Log::CLOSE;
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
                //locate -name
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

static std::string read_code(const std::string &file) {
    std::string code;
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
    int i = doargs(argc, argv);
    argc -= i;
    argv += i;
    if (argc <= 0)
        usage("no input files given");
    size_t start = Script::now();
    Script::Log::info(NAME, "Start!");
    Script::Log::debug(NAME, "reading file...%s ", *argv);
    std::string text = read_code(*argv);
    Script::Engine engine;
    engine.parse(text);
    engine.launch();
    Script::Log::info(NAME, "The End! time elapsed %zus", Script::now() - start);
    return 0;
}

