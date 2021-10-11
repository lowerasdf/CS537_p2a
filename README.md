# CS537 P2A: Unix Shell

##### Table of Contents
* [Installation](#installation)
* [Implementation](#implementation)
  * [Path](#path)
  * [Redirection](#redirection)
  * [Loop Built-In Command](#loop-built-in-command)
* [Acknowledgement](#acknowledgement)

## Installation
To compile the program, run:
<pre><code>gcc -o wish wish.c</code></pre>
To run the shell in interactive mode, run:
<pre><code>./wish</code></pre>
To run the shell in batch mode from a file called <code>batch.txt</code>, run:
<pre><code>./wish batch.txt</code></pre>
Inside the shell, there are some built-in commands that can be executed:
* <code>cd [PATH]</code>: Change the current directory.
* <code>path [PATH_VARIABLE_1] [PATH_VARIABLE_2] ...</code>: Specify the path variables to run the user program.
* <code>exit</code>: Exit the shell.
* <code>loop [N_TIMES] [USER PROGRAM]</code>: Execute a program n times. A loop variable <code>$loop</code> can also be used. Try running <code>loop 5 echo hello $loop</code>.

## Implementation
### Basic Command
The shell runs an infinite loop until the user execute the built-in <code>exit</code> command or when it hits the EOF of a file if it is running on the batch mode. In each loop, the shell checks if there is a loop counter. If there is not, it parses a line using the redirection delimiter <code>></code> and whitespaces, while also keeping track of the redirected output file. If there is a loop counter, it simply gets the arguments from tbe cacbed data structure. Then it tries to find the matching command, checks the arguments, and executes the command (calling fork if it is not a built-in command).

### Path
The user can tell the shell where to find the user program by calling the built-in command <code>path</code> described above. A dynamic array is used to keep track all the paths.

### Redirection
The redirection is implemented by first noting where to find the corresponding file when parsing, and checking whether it is a valid redirection. Then when the shell executes the program, it will change the <code>STDOUT_FILENO</code> and <code>STDERR_FILENO</code> to the redirected file.

### Loop Built-In Command
The loop built-in command will set up a loop counter and cache the current argument. It will be used in the next iteration of the infinite loop. The loop counter is used to implement the loop variable.

## Acknowledgement
This is an assignment for a class [Comp Sci. 537: Introduction to Operating Systems](https://pages.cs.wisc.edu/~remzi/Classes/537/Fall2021/) by [Remzi Arpaci-Dusseau](https://pages.cs.wisc.edu/~remzi/). Please refer to [this repo](https://github.com/remzi-arpacidusseau/ostep-projects/tree/master/processes-shell) for more details about the assignment.
