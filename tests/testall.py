import os
import subprocess
import sys

__author__ = 'fengzishiren'

main = lambda prog, path, debug="debug": \
    map(lambda f: subprocess.Popen('%s -name %s -d %s' % (prog, os.path.join(path, f), debug), shell=True).wait(), os.listdir(path))
main(*sys.argv[1:])