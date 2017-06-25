#!/usr/bin/python
# designed for Python 2.7

# We're simulating Perl's Template::Toolkit syntax.

# -D... option: says template variable ... exists and its value is the next argument, rather than a filename
# default interpretation is a filename to do template substitution on

# example usage: generate a minimum-viable isk:Object subclass that compiles to isk_object.hpp and isk_object.cpp
# \Python27\python TemplateToolkit.py -DCLASS craft -DUC_CLASS_FILE CRAFT -DCLASS_FILE craft -DMODEL_STRUCT model isk_object.hpp.tmpl isk_object.cpp.tmpl

# same, without the model structure
# \Python27\python TemplateToolkit.py -DCLASS craft -DUC_CLASS_FILE CRAFT -DCLASS_FILE craft isk_object.hpp.tmpl isk_object.cpp.tmpl

import sys

template_vars = {}
macro_name = ''

for line in sys.argv[1:]:
	if ''!=macro_name:
		template_vars[macro_name] = line
		macro_name = ''
		continue
	if '-D'==line[0:2]:
		if 2<len(line):
			macro_name = line[2:]
		continue
	
	with open(line,'r') as f:
		src = f.readlines()

	dest_file = line
	if '.tmpl'==dest_file[-5:]:
		dest_file = dest_file[0:-5]
	else:
		dest_file = dest_file+'.out'

	if_stack = []
	if_not_stack = []
	ub = len(src)
	i = 0
	while(i<ub):
		src_line = src[i].rstrip()
		if 0==len(if_stack) and 0==len(if_not_stack):
			if '[% END IF %]'==src_line:
				print line+";"+str(i)+": unbalanced END IF"
				exit(1)
			if '[% IF NOT '==src_line[:10] and ' %]'==src_line[-3:] and 13<len(src_line):
				if_not_stack.append(i)
				i = i+1
				continue
			if '[% IF '==src_line[:6] and ' %]'==src_line[-3:] and 9<len(src_line):
				if_stack.append(i)
				i = i+1
				continue
			for macro_value in template_vars.iteritems():
				target = '[% '+macro_value[0]+' %]'
				if target in src_line:
					src_line = macro_value[1].join(src_line.split(target))
			src[i] = src_line+'\n'
			i = i+1
		elif '[% END IF %]'==src_line.strip():
			j = if_stack[-1:][0] if 0<len(if_stack) else -1
			k = if_not_stack[-1:][0] if 0<len(if_not_stack) else -1
			if j<k:
				if_not_stack.pop()
				test = src[k][10:-3]
				if test not in template_vars:
					src.pop(i)
					src.pop(k)
				else:
					del src[k:i+1]
				i = k
				ub = len(src)
				continue
			else:
				if_stack.pop()
				test = src[j].strip()[6:-3]
				if test in template_vars:
					src.pop(i)
					src.pop(j)
				else:
					del src[j:i+1]
				i = j
				ub = len(src)
				continue
		else:
			i = i+1

	if 0<len(if_stack):
		print "Unhandled IF at lines: "+','.join(if_stack)
	if 0<len(if_not_stack):
		print "Unhandled IF NOT at lines: "+','.join(if_not_stack)
	if 0<len(if_stack) or 0<len(if_not_stack):
		exit(1)

	with open(dest_file,'w') as f:
		f.writelines(src)			


