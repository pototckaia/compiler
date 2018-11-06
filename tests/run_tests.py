import os
import glob 
import subprocess
import re
import sys
from itertools import count

testsPath = os.path.dirname(os.path.abspath(__file__))

cmakeDir = 'cmake-build-debug'
programName = 'Compile'
compilePath = testsPath + os.sep + '..' + os.sep + cmakeDir + os.sep + programName

lexPath = testsPath + os.sep + 'lexer'

def compareFiles(pos, receive, expect):
	receiveBaseName = os.path.basename(receive)

	with open(receive, 'r') as rec, open(expect, 'r') as exp:
		allWordsReceive = (word for line in rec for word in re.findall(r'(\w+)', line))
		allWordsExpect = (word for line in exp for word in re.findall(r'(\w+)', line))
		we = ' '
		i = 0
		while(we != ''):
			we = next(allWordsExpect, '')
			wr = next(allWordsReceive, '')
			if we != wr:
				print('____________________________________')
				print('{0} Test "{1}" not pass in line {0}'.format(pos + 1, receiveBaseName, i + 1))
				print("Expect  '{0}'  but find  '{1}' ".format(we, wr))
				return False
			i += 1

	print('____________________________________')
	print('{0} Test "{1}" pass'.format(pos + 1, receiveBaseName))
	return True

def runTests(dirPath):
	print(os.path.basename(dirPath), '...')

	inputFiles = glob.glob(dirPath + os.sep + '*.in')
	inputFiles.sort()

	for pos, finput in zip(count(), inputFiles):
		foutput = os.path.splitext(finput)[0] + '.out'
		fanswer = os.path.splitext(finput)[0] + '.ans'
		
		subprocess.run([compilePath, finput, '-o', foutput])

		if not compareFiles(pos, foutput, fanswer):
			sys.exit(0)


if __name__ == '__main__':
	runTests(lexPath)