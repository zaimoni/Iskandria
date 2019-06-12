#!/usr/bin/python
# designed for Python 3.7.0, may work with other versions
# (C)2019 Kenneth Boyd, license: MIT.txt

# this implements a simplistic pre-preprocessor exploiting the requirement that C++ compilers not error on unrecognized #pragma directives

from sys import argv;

copy_buffers = {}

if __name__ == "__main__":
	src = open(argv[1],'r')		# likely parameter
	# read in src; obtain all enum identifiers and prepare functions based on that
	in_copy = ''
	in_substitute = ()
	for line in src:
		if line.startswith('#pragma start_copy '):
			if in_copy or in_substitute:
				continue
			in_copy = line[19:].strip()
			print(line.rstrip())
			continue
		elif line.startswith('#pragma end_copy'):
			if in_substitute or not in_copy:
				continue
			in_copy = ''
			print(line.rstrip())
			continue
		elif line.startswith('#pragma substitute '):
			# reserved keywords: for, in
			working = line[19:].strip()
			n = working.find(' for ')
			if -1>=n:
				continue
			target = working[:n].strip()
			working = working[n+5:].strip()
			n = working.find(' in ')
			if -1>=n:
				continue
			source = working[:n].strip()
			buffer = working[n+4:].strip()
			if buffer not in copy_buffers:
				continue
			in_substitute = (source,target,buffer)
			print(line.rstrip())
			continue
		elif line.startswith('#pragma end_substitute'):
			if in_copy or not in_substitute:
				continue
			for x in copy_buffers[in_substitute[2]]:
				print(x.replace(in_substitute[0],in_substitute[1]))
			in_substitute = ()
			print(line.rstrip())
			continue
		elif in_copy:
			if in_copy in copy_buffers:
				copy_buffers[in_copy].append(line.rstrip())
			else:
				copy_buffers[in_copy] = [line.rstrip()]
		elif in_substitute:
			continue	# work done on ending
		print(line.rstrip())