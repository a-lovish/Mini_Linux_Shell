# Mini_Linux_Shell

•	Please ensure that there is no existing file with name “/tmp/history.txt” as this is my history file.

•	Supported internal commands: cd, echo, pwd, history, exit
For echo command: use echo abc
For exit command: use exit or quit or x (Any will work)

•	For environment variables: 
use setenv TERM = vt100 or setenv TERM=vt100 
(Both case i.e. with two spaces and no space are supported.) 
use printenv SHELL or echo $SHELL

•	Supported external commands: 
ls, cat, man, which, whereis, date, cal, clear, apropos, more, less, touch, find, cp, mv, rm, mkdir, rmdir, sort, wc

•	Multilevel (Upto 2) piping is supported. (No redirection for multilevel piping.)
Ex:  ls -l | grep “.c” | more

•	Single level piping is supported with redirection. (Note that command containing input file should be at left of pipe and output or append file should at right of pipe.)
Ex: ls -a | wc -c ; cat < check.txt | more

•	For redirection, the following cases are supported: -

1.	wc <input.txt or wc < input.txt
2.	ls >output.txt or ls > output.txt
3.	ls >>output.txt or ls >> output.txt
4.	wc <input.txt >output.txt or wc < input.txt > output.txt
5.	wc <input.txt >>output.txt or wc < input.txt >> output.txt

•	I have also included help command which will print the supported internal commands, allowed piping and redirection.
