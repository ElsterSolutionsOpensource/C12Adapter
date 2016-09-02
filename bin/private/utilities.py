#!/usr/bin/python

import os, sys, subprocess, shutil, fnmatch, re
from private import decorators

verbose = False

def log(s):
    if verbose:
        print(s)

def check_tool_installed(name):
    sys.stdout.write("Path to %s: " % name)
    sys.stdout.flush()
    proc = subprocess.Popen(['which', name])
    if proc.wait() != 0:
        print_and_exit("Tool not present: %s" % " ".join(command))

def split_without_last_empty_line(txt):
    split = txt.split('\n')
    if len(split) > 0 and len(split[-1]) == 0:
        return split[:-1]
    else:
        return split

class Options(object):
    def set(self, name, value):
        setattr(self, name, value)

def print_and_exit(msg, err = 3):
    print(msg)
    exit(err)

def print_usage_and_exit(params, defs = ""):
    txt = "USAGE:\n    " + os.path.basename(sys.argv[0])
    if len(params) > 0:
        txt += params;
    if len(defs) > 0:
        txt += "\n" + defs
    txt += "\n"
    print_and_exit(txt)

@decorators.handle_dry_run
def run_command(command, fl = None):
   fileOut = fl if fl else sys.stdout
   return subprocess.check_call(command,
                                shell=True,
                                stdout=fileOut,
                                stderr=fileOut)  # met freezing problems on Windows when stdout and stderr are diffeernt

@decorators.handle_dry_run
def directory_create(path, clearIfPresent = True):
    if os.path.exists(path):
        if not os.path.isdir(path):
            exit_with_error("Cannot create directory '" + path + "': path exists, but it is not a directory")
        if not clearIfPresent:
            return # done
        shutil.rmtree(path)
    os.makedirs(path)

@decorators.handle_dry_run
def directory_create_current(path, clearIfPresent = True):
    directory_create(path, clearIfPresent)
    os.chdir(path)

@decorators.handle_dry_run
def directory_copy(fromPath, toPath, ignore = None):
    if os.path.exists(toPath):
        shutil.rmtree(toPath)
    try:
        if ignore is not None:
            shutil.copytree(fromPath, toPath, ignore=shutil.ignore_patterns(ignore))
        else:
            shutil.copytree(fromPath, toPath)
    except os.error as e:
        print_and_exit(e)

@decorators.handle_dry_run
def file_copy(fromFile, toFile):
    shutil.copy(fromFile, toFile)

@decorators.handle_dry_run
def file_move(fromFile, toFile):
    shutil.move(fromFile, toFile)

def _recurseLibrary(lib, entryName, subDir):
   for name in os.listdir(subDir):
      if entryName == '':
         subEntryName = name
      else:
         subEntryName = entryName + '/' + name
      fileName = os.path.join(subDir, name)
      if os.path.isdir(fileName):
         recurseLibrary(lib, subEntryName, fileName)
      else:
         print("Adding entry '" + subEntryName + " from  " + fileName)
         lib.AddEntryFromFile(subEntryName, fileName)

@decorators.handle_dry_run
def global_replace(rootDir, filePattern, fromRegexp, toStr, flags = re.IGNORECASE or re.MULTILINE or re.DOTALL):
   """ Replace a pattern in a tree of text files """
   for name in os.listdir(rootDir):
      fileName = os.path.join(rootDir, name)
      if os.path.isdir(fileName):
         global_replace(fileName, filePattern, fromRegexp, toStr, flags)
      elif fnmatch.fnmatch(fileName, filePattern):
         iFile = open(fileName, "r")
         iStr = iFile.read()
         iFile.close() # close explicitly, do ot trust garbage collector as we are possibly going to write into this file later
         oStr = re.sub(fromRegexp, toStr, iStr, 0, flags)
         if oStr != iStr:
            open(fileName, "w").write(oStr)
