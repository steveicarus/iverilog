import os
import sys
import fileinput
import shutil
import glob
import argparse

textToclass = "class " 
textToendclass = "endclass"
textToVirclass = "virtual class "

parser = argparse.ArgumentParser()
parser.add_argument('-i', required=True)

args = parser.parse_args()

fileToSearch = args.i

File = open( fileToSearch , 'r+' )
tempFile = open ('tmp.svh','w+')

num_c = 0

for line in fileinput.input( fileToSearch ):
  rec = line.strip()
  if rec.startswith(textToclass) :
    #print('Match Found')
    tempFile.write(line)
    tempFile.write("`ifndef IVL_UVM\n")
  elif rec.startswith(textToVirclass) :
    #print('Match Found')
    tempFile.write(line)
    tempFile.write("`ifndef IVL_UVM\n")
  elif textToendclass in line:
    tempFile.write("`endif // IVL_UVM \n")
    tempFile.write(line)
    num_c += 1
  else : 
    tempFile.write(line)

print 'Fixed: ', num_c, ' classes for IVLOG'

File.close()
tempFile.close()
