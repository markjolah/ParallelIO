# -*- python -*-
import os, string
def module(command, *arguments):
  commands = os.popen('/usr/bin/modulecmd python %s %s'\
                      % (command, string.join(arguments))).read()
  exec commands
  
