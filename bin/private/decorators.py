#!/usr/bin/python

import os, sys, string

dry_run = False

def preserve_current_directory(function):
    def callback(*argc, **argv):
        currentDir = os.getcwd()
        try: 
            return function(*argc, **argv)
        finally: 
            os.chdir(currentDir)
    return callback

def step(message):
    def print_step_internal(function):
        def callback(*argc, **argv):
            print(message + '...')
            response = function(*argc, **argv)
            print(message + ' successfully completed')
            return response
        return callback
    return print_step_internal

class handle_dry_run(object):
    def __init__(self, func):
        self.func = func
    def __call__(self, *args):
        def recurseArgs(indent, args):
            shift = "\n" + "    " * indent
            if type(args).__name__ in ('tuple', 'list'):
                txtArgs = shift + "("
                for i in args:
                   txtArgs += recurseArgs(indent + 1, i)
                txtArgs += shift + ")"
            else:
                txtArgs = shift + str(args)
            return txtArgs
        global dry_run
        if dry_run:
            print("  DRY: " + self.func.__name__ + recurseArgs(2, args))
        else:
            return self.func(*args)
