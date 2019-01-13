import os
import glob 
import subprocess
import re
import sys
from itertools import count
import argparse
import difflib
import filecmp

testsPath = os.path.dirname(os.path.abspath(__file__))

cmakeDir = 'cmake-build-debug'
programName = 'Compile'
compilePath = testsPath + os.sep + '..' + os.sep + cmakeDir + os.sep + programName
runNasmPath = testsPath + os.sep + '..' + os.sep + 'run-gen.sh'

lexPath = testsPath + os.sep + 'lexer'
parserExpressionPath = testsPath + os.sep + 'parserExpression'
parserProgramPath = testsPath + os.sep + 'parserProgram'
asmPath = testsPath + os.sep + 'asm'

options = {'l' : lexPath, 
		   'e' : parserExpressionPath, 
		   'p' : parserProgramPath,
		   'a' : asmPath}


def compareFiles(pos, receive, expect):
	receiveBaseName = os.path.basename(receive)
	print('____________________________________')
	if (filecmp.cmp(receive, expect)):
		print('{0} Test "{1}" pass'.format(pos + 1, receiveBaseName))
		return True

	print('{0} Test "{1}" not pass'.format(pos + 1, receiveBaseName))
	with open(receive) as f1, open(expect) as f2:
		for line in difflib.context_diff(f1.readlines(), f2.readlines(), 
				fromfile=receive, 
				tofile=expect, lineterm=''):
			print(line)
	return False

def runTests(dirPath, options):
	print(os.path.basename(dirPath), '...')

	inputFiles = glob.glob(dirPath + os.sep + '*.in')
	inputFiles.sort()
	for pos, finput in zip(count(), inputFiles):
		finput_a = os.path.splitext(finput)[0]
		foutput = finput_a + '.out'
		fanswer = finput_a + '.ans'
		if options == '-a':
			subprocess.run([compilePath, '-i', finput, options])
			subprocess.run(['sh', runNasmPath, finput_a])
		else: 
			subprocess.run([compilePath, '-i', finput, '-o', foutput, options])

		if not compareFiles(pos, foutput, fanswer):
			sys.exit(0)


if __name__ == '__main__':

	argsParser = argparse.ArgumentParser()

	argsParser.add_argument('-l', '--lexer', help='Start lexer tests', action='store_true')
	argsParser.add_argument('-e', '--expression', help='Start parser expression tests', action='store_true')
	argsParser.add_argument('-p', '--program', help='Start parser pascal program tests', action='store_true')
	argsParser.add_argument('-a', '--asm', help='Start run asm tests', action='store_true')

	args = argsParser.parse_args()

	if args.lexer:
		runTests(options['l'], '-l')

	if args.expression:
		runTests(options['e'], '-e')

	if args.program:
		runTests(options['p'], '-p')

	if args.asm:
		runTests(options['a'], '-a')