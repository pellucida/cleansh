<h1>cleansh</h1>
<h2> Starts a shell with sanitized environment</h2>
<p>
Cleansh copies the environment variables specified in the code
from the parent process to environment of the shell (/bin/sh)
and set some mandatory variables before exec(2) the shell.
</p>
<p>
An arguments passed to cleansh are passed onto the new shell.
</p>
<h2>EXAMPLES</h2>
<pre>
	xx % ./cleansh
	cleansh 1 $ exit
	xx %
</pre>
<pre>
	xx % ./cleansh -c /bin/printenv
</pre>

