import os
import glob 
import subprocess
import re
import sys
from itertools import count
import argparse

testsPath = os.path.dirname(os.path.abspath(__file__))

cmakeDir = 'cmake-build-debug'
programName = 'Compile'
compilePath = testsPath + os.sep + '..' + os.sep + cmakeDir + os.sep + programName

lexPath = testsPath + os.sep + 'lexer'
parserExpressionPath = testsPath + os.sep + 'parserExpression'
parserProgramPath = testsPath + os.sep + 'parserProgram'

options = {'l' : lexPath, 'e' : parserExpressionPath, 'p' : parserProgramPath}


def compareFiles(pos, receive, expect):
	receiveBaseName = os.path.basename(receive)

	with open(receive, 'r') as rec, open(expect, 'r') as exp:
		allWordsReceive	 = (line for line in rec)
		allWordsExpect = (line for line in exp)
		we = ' '
		i = 0
		while(we != ''):
			we = next(allWordsExpect, '')
			wr = next(allWordsReceive, '')
			if we != wr:
				print('____________________________________')
				print('{0} Test "{1}" not pass in line {0}'.format(pos + 1, receiveBaseName, i + 1))
				print("Expect:\n{0}\nBut find:\n{1}".format(we, wr))
				return False
			i += 1

	print('____________________________________')
	print('{0} Test "{1}" pass'.format(pos + 1, receiveBaseName))
	return True

def runTests(dirPath, options):
	print(os.path.basename(dirPath), '...')

	inputFiles = glob.glob(dirPath + os.sep + '*.in')
	inputFiles.sort()

	for pos, finput in zip(count(), inputFiles):
		foutput = os.path.splitext(finput)[0] + '.out'
		fanswer = os.path.splitext(finput)[0] + '.ans'
		
		subprocess.run([compilePath, '-i', finput, '-o', foutput, options])

		if not compareFiles(pos, foutput, fanswer):
			sys.exit(0)


if __name__ == '__main__':

	argsParser = argparse.ArgumentParser()

	argsParser.add_argument('-l', '--lexer', help='Start lexer tests', action='store_true')
	argsParser.add_argument('-e', '--expression', help='Start parser expression tests', action='store_true')
	argsParser.add_argument('-p', '--program', help='Start parser pascal program tests', action='store_true')

	args = argsParser.parse_args()

	if args.lexer:
		runTests(options['l'], '-l')

	if args.expression:
		runTests(options['e'], '-e')

	if args.program:
		runTests(options['p'], '-p')