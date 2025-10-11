#! /usr/bin/env python3
# This file is part of the RKWard project (https://rkward.kde.org).
# SPDX-FileCopyrightText: 2025 by Thomas Friedrichsmeier <thomas.friedrichsmeier@kdemail.net>
# SPDX-FileContributor: The RKWard Team <rkward@kde.org>
# SPDX-License-Identifier: GPL-2.0-or-later
#
# Extracts messages form R packages. (Of course R has its own update_pkg_po(), but
# we don't want to asssume an R installation for the extraction job.

import os
import codecs
import sys
import subprocess
from xml.dom import minidom
import html.parser
import copy
import re

def usage ():
  print("Usage: " + sys.argv[0] + " files")
  exit (1)

# initialize globals, and parse args
sources = []
outfile = sys.stdout
for arg in (list (sys.argv[1:])):
  if (arg.startswith ("--")):
    usage ()
  else:
    sources.append (arg)
if (len (sources) < 1):
  usage ()

def normalize (text):
  lines = text.split ("\n")
  nlines = []
  for line in lines:
    nlines.append (' '.join (line.strip ().split ()))	# remove whitespace at start, end, and simplify whitespace within each line
  return ' '.join (nlines)

def writeouti18n (call):
  if (call.find ("%") >= 0):
    outfile.write ("/* xgettext:no-c-format */ ")
  outfile.write (call + "\n")

# basic R parsing (comments / strings)
class RParseBuffer:
  def __init__ (self, content):
    self.buf = content
    self.comment = ""
    self.nline = 0
    self.nchar = 0
  def atEof (self):
    return (self.nchar >= len (self.buf))
  def currentChar (self):
    if (self.atEof ()):
      return ""
    return (self.buf[self.nchar])
  def advance (self, n=1):
    for step in range (n):
      self.nchar += 1
      if (self.nchar >= (len (self.buf) - 1)):
        return False
      if (self.buf[self.nchar] == "\n"):
        self.nline += 1
    return True
  def skipWhitespace (self):
    while (self.buf[self.nchar].isspace ()):
      if (not self.advance ()):
        break
  def seekUntil (self, needle):
    fromchar = self.nchar
    while (not self.startswith (needle)):
      if (not self.advance ()):
        break
    return self.buf[fromchar:self.nchar]
  # returns True, if the given word is found at the current position, _and_ it this is a full keyword (i.e. preceded and followed by some delimiter), and could be a function call
  def isAtFunctionCall (self, word):
    if (not self.startswith (word)):
      return False
    # return False if previous char is no delimiter
    if (self.nchar < 0 and self.buf[self.nchar-1].isalnum ()):
      return False
    # return False if next char is no delimiter
    nextchar = self.buf[self.nchar + len (word)]
    if (not (nextchar.isspace () or nextchar == '(')):
      return False
    return True
  def seek_line_comment_end (self):
    comment = ""
    while True:
      comment += self.seekUntil ("\n")
      self.skipWhitespace ()    
      if (self.startswith ("#")):
        self.advance (1)
      else:
        break
    return comment
  def nibbleCallParameters (self):
    fromchar = self.nchar
    self.nibble_until ('(')
    self.advance ()
    self.nibble_until (')', True)
    self.advance ()
    return self.buf[fromchar:self.nchar]

  def nibble_until (self, string, skip_over_parentheses=False):
    fromchar = self.nchar
    while (not self.startswith (string)):
      if (self.atEof ()):
        break
      if (self.buf[self.nchar] in ['"', '\'', '`']):
        ochar = self.buf[self.nchar]
        while (self.advance ()):
          if (self.startswith ('\\')):
            self.advance ()	# skip next char
          elif (self.startswith (ochar)):
            break
      elif (self.startswith ("#")):
        self.comment = self.seek_line_comment_end ()
      elif (skip_over_parentheses and (self.startswith ("("))):	# skip over nested parentheses
        self.advance ()
        self.nibble_until (")", True)
      elif (not self.buf[self.nchar].isspace ()):
        self.comment = ""
      if (not self.advance ()):
        break
    return (self.buf[fromchar:self.nchar])
  def startswith (self, string):
    return self.buf.startswith (string, self.nchar)

def handleRChunk (filename):
  global outfile
  keywords = ("gettext", "ngettext", "stop", "warning", "message", ".rk.i18n") # TODO change me!

  # Convert single quoted and backtick quoted strings in the input chunk to double quotes (so xgettext can work in C-style).
  def normalizeQuotes (chunk):
    pos = 0
    current_quote_symbol = ""
    output = ""
    strip_closing_parentheses = 0
    while (pos < len (chunk)):
      c = chunk[pos]
      if c == "\\":
        nc = chunk[pos+1]
        if ((nc != current_quote_symbol) or (nc == '"')):
          output += c
        output += nc
        pos += 1
      elif c in ['"', '\'', '`']:
        if (current_quote_symbol == ""):
          current_quote_symbol = c
          output += '"'
        elif current_quote_symbol == c:
          current_quote_symbol = ""
          output += '"'
        elif c == '"':
          output += '\\\"'
        else:
          output += c
      else:
        output += c
      pos += 1
    return output

  buf = ""
  with codecs.open(filename, 'r', 'utf-8') as f:
    buf = f.read()
  rbuf = RParseBuffer (buf)
  while (True):
    call = ""
    junk = rbuf.nibble_until (keywords)
    if (rbuf.atEof ()):
      break
    for candidate in keywords:
      if (rbuf.isAtFunctionCall (candidate)):
        call = candidate
        break
    if (call == ""):
      # skip over somethingelsei18nsomethingelse identifiers, i.e. those not matched, above
      rbuf.advance ()
      continue

    rbuf.advance (len (call))
    comment = normalize (rbuf.comment)
    line = rbuf.nline
    parameters = rbuf.nibbleCallParameters ()
    text = "/* "
    if (comment.lower ().startswith ("i18n:") or comment.lower ().startswith ("translators:")):
      text += "i18n: " + comment + "\n"
    text += "i18n: file: " + filename + ":" + str(line)
    text += " */\n" + call + normalizeQuotes (parameters) + ";\n"
    writeouti18n (text)

#######
# Loop over toplevel_sources (specified on command line.
for source in sources:
  handleRChunk(source)
