About "rbidescript"

This is a simple command-line tool which lets you send script commands
to the IDE, allowing you to integrate IDE control with shell scripts and
other command-line tools.

The basic interface is simple: it reads input from stdin, until
end-of-input, and then sends this to the IDE as a script.  That means
that you could make a stand-alone IDE shell script like this:

#!/path/to/rbidescript/executable
Speak "Hello world!"

You would of course replace the path on the first line with the full
path to the rbidescript executable (which is inside the rbidescript
folder).  Then just set the executable bit on the  text file, and you
can execute it from the command line like any other shell script, with
its contents going directly to the IDE.

You can also give the rbidescript tool the name of a file to send, or
directly give it a one-line command using the "-i" option.  Use
"rbidescript -?" to get more help.
