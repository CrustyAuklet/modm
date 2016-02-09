#!/usr/bin/env python2
# -*- coding: utf-8 -*-
# Copyright (c) 2016, Roboterclub Aachen e.V.
# All rights reserved.
#
# The file is part of the xpcc library and is released under the 3-clause BSD
# license. See the file `LICENSE` for the full license governing this code.
# -----------------------------------------------------------------------------

"""
# xpcc QtCreator Project Tool

This tool makes it possible to automatically generate a QtCreator project for
applications using xpcc.

You need to add something like this to your application's `SConstruct`:

~~~{.py}

files = env.FindFiles('.')

# [...]

env.Alias('qtcreator', env.QtCreatorProject(files))

# [...]

env.Alias('all', ['build', 'qtcreator', 'size'])

~~~
"""


import os

def collect_defines(defines, source):
	if isinstance(source, str):
		defines[source] = None
	elif isinstance(source, dict):
		defines.update(source)
	elif isinstance(source, list):
		[collect_defines(defines, mm) for mm in source]


def qt_creator_project_method(env, file_scanner):
	# collect values
	project_name = env['XPCC_PROJECT_NAME']
	defines = {}
	collect_defines(defines, env['CPPDEFINES'])
	collect_defines(defines, file_scanner.defines)
	files = [str(ff) for ff in file_scanner.header + file_scanner.sources]
	includes = env['CPPPATH']

	# generate project files
	proj_path = os.path.join(env['XPCC_BASEPATH'], project_name)
	temp_path = os.path.join(env['XPCC_ROOTPATH'], 'templates', 'qtcreator', 'project')
	return [
		env.Jinja2Template(
			target = proj_path + '.creator', source = temp_path + '.creator.in',
			substitutions = {}),
		env.Jinja2Template(
			target = proj_path + '.config', source = temp_path + '.config.in',
			substitutions = {'defines': defines}),
		env.Jinja2Template(
			target = proj_path + '.files', source = temp_path + '.files.in',
			substitutions = {'files': files}),
		env.Jinja2Template(
			target = proj_path + '.includes', source = temp_path + '.includes.in',
			substitutions = {'includes': includes})
	]


def generate(env):
	env.AddMethod(qt_creator_project_method, 'QtCreatorProject')

def exists(env):
	return 1