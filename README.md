<h1>cleansh</h1>
<h2> Starts a shell with sanitized environment</h2>
<p>
Cleansh copies the environment variables specified in the code
from the parent process to environment of the shell (/bin/sh)
and set some mandatory variables before execve(2)ing the shell.
</p>
<p>
Any arguments passed to cleansh are passed onto the new shell.
</p>
<h2>Examples</h2>
<pre>
	xx % ./cleansh
	cleansh 1 $ exit
	xx %
</pre>
<pre>
	xx % ./cleansh -c /usr/bin/printenv
	SHELL=/bin/sh
	TERM=xterm
	USER=pellucida
	PATH=/sbin:/bin:/usr/sbin:/usr/bin
	LC_COLLATE=C
	PWD=/people/pellucida
	LANG=en_US.UTF-8
	SHLVL=1
	HOME=/people/pellucida
	LOGNAME=pellucida
	DISPLAY=:0
	_=/usr/bin/printenv
</pre>
<p>As it really just /bin/sh can also be used as an interpretor.
</p>
<pre>
	#! /usr/local/bin/cleansh
	#
	# @(#) myfile.sh - runs with cleansh (bash)
	#
	/usr/bin/printenv
</pre>
<h2>Notes</h2>
<p>
Environment variables passed "as is" to child process:
	USER, LOGNAME, HOME, TERM, LANG, DISPLAY
</p>
<p>
SHELL is set to "/bin/sh"
</p>
<p>
PATH is set to "/sbin:/bin:/usr/sbin:/usr/bin"
</p>
<p>
LC_COLLATE is set to "C"
</p>
<p>
PS1 is set to "cleansh ! $ "
</p>
