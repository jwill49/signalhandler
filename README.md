## Process Creation and Signal Handling ##

#### written by Jordan Williams ####

1. Compile using `make` to create two executables "shell" & "carlo"
2. Start the shell script using `./shell <int>`
	* Input/Output is not redirected
	* The parameter will determine the precision of the monte carlo simulation
	* The shell will ignore SIGTSTP & SIGINT
	* Exit the shell by typing the command `exit`
3. Start the monte carlo simulation by entering `./carlo <float>` within the shell
	* The parameter will determine if an alarm is set as specified in the write up
	* Signal handling behaves as specified in the write up
	* Upon termination, carlo will display its results