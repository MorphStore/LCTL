# This script could be used when an 'incoplete type'-error occures during compilation. 
# Given the line, with specifies the template definition (delivered as a single line from the compiler) this script
# prints the given definition while adding line breaks and indentation to improve readability

import sys
import time

indent = " " * 2 # determine indent size

def findNextOccurence(line):
  occurences = [line.find("<"), line.find(", "), line.find(">"), line.find("; ")]
  
  optIndex = -1; optValue = len(line)
  for i in range(0, len(occurences), 1):
    if(occurences[i] > -1 and occurences[i] < optValue):
      optIndex = i
      optValue = occurences[i]
  return optValue, optIndex

def indentLine(line):
  indents = 0
  occurence, occurenceType = findNextOccurence(line);
  newLine = ""
  while(occurenceType >= 0):
    if(occurenceType == 0): # case <
      indents += 1;
      newLine += line[:occurence+1].lstrip() + "\n" + (indent * indents)
    elif(occurenceType == 1): # case ,
      newLine += line[:occurence+1].lstrip() + "\n" + (indent * indents)
    elif(occurenceType == 2): # case >
      indents -= 1
      newLine += line[:occurence].lstrip() + "\n" + (indent * indents) + line[occurence:occurence+1]
    elif(occurenceType == 3): # case ; 
      newLine += line[:occurence+1].lstrip() +"\n\n\n"

    line = line[occurence+1:]
    occurence, occurenceType = findNextOccurence(line);
  return newLine


line = input("Please insert Template definition:\n")
print(indentLine(line))
