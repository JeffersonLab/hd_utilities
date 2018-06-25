
#
# Mac OS X specific settings
#

def InitENV(env):

	# This is needed to allow plugins to have their
	# global variables linked to those in the running
	# executable.
	env.AppendUnique(LINKFLAGS='-flat_namespace')

	# For plugins that don't have everything when they are linked
	env.AppendUnique(SHLINKFLAGS=['-undefined', 'suppress'])

