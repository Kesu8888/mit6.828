# HW2 report

## Executing simple commands

Firstly, we need to know what should be pass to the execv. execv has two parameters execv(path, argv). Path is the location of the file to be executed and argv are the parameters to be pass to the executed file e.g., "ls -l", ls is the path and -l is the argument. In runcmd case' '.
case ' ':
    ecmd = (struct execcmd*)cmd;
    for (int i = 0; i < MAXARGS; i++) {
        printf("The %d'th argument is %s\n", i, ecmd->argv[i]); <- I use this code to examine the content of each argument in ecmd->argv.
    }
    if(ecmd->argv[0] == 0) {
        _exit(0);
    }
    break;
Then I pass /bin/ls to the shell
6.828$ /usr/bin/ls
The 0'th argument is /usr/bin/ls
The 1'th argument is (null)
The 2'th argument is (null)
The 3'th argument is (null)
The 4'th argument is (null)
The 5'th argument is (null)
The 6'th argument is (null)
The 7'th argument is (null)
The 8'th argument is (null)
The 9'th argument is (null)
So I know that the file path is in ecmd->argv[0]. So my code for case ' ' is:
case ' ':
    if(ecmd->argv[0] == 0) {
        _exit(0);
    }
    if (execv(ecmd->argv[0], &ecmd->argv[0]) == -1) // <- The execv returns -1 if fails to execute.
        printf("Something went wrong %s\n", strerror(errno)); 
    break;

You may want to change the 6.828 shell to always try /bin, if the program doesn't exist in the current working directory, so that below you don't have to type "/bin" for each program. If you are ambitious you can implement support for a PATH variable. My code for this is:
case ' ':
    ecmd = (struct execcmd*)cmd;
    if(ecmd->argv[0] == 0) {
        _exit(0);
    }
    if (execv(ecmd->argv[0], &ecmd->argv[0]) == -1) { <- If the system cannot find the command file in the current directory.
        int i = fork();
        if (i == 0) {
            if (execv(strComb("/bin/", ecmd->argv[0]), &ecmd->argv[1]) == -1) <-strComb is a function I write to add "/bin/" infront of ecmd->argc[0].
                printf("Error: %s\n", strerror(errno));
        } else {
            wait(&i);
        }
    }
  break;

Ambitious approach: Adding path !!!Not written yet


## I/O redirection

Fills in the runcmd function for "<" and ">" to pass the below test.
echo "6.828 is cool" > x.txt
cat < x.txt
This is my code.

  case '>':
  case '<':
    rcmd = (struct redircmd*)cmd;
    ecmd = (struct execcmd*)rcmd->cmd;
    int fd1;
    // Your code here ...
    if (rcmd->type == '<') { 
        if ((fd1 = open(rcmd->file, O_RDONLY)) == -1) {// Since stdin redirection file should exist, the mode is O_RDONLY
            printf("Error: %s\n", strerror(errno));
        }
        dup2(fd1, rcmd->fd); // replace the current input with the open file.
    } else {
        if ((fd1 = open(rcmd->file, O_WRONLY | O_CREAT, S_IRWXU)) == -1) {/* Since the stdout refirection should create the file if not exist, the mode is O_WRONLY and O_CREAT. The S_IRWXU set the creating file to read, write and exe by the user. If not, the created file will not be accessible to the user or the programming afterwards.*/
            printf("Error: %s\n", strerror(errno));
        }
        dup2(fd1, rcmd->fd); // Replace the current output with the open file
    }
    runcmd(rcmd->cmd);
    break;


## implement pipes

My code for this question is 
case '|':
    pcmd = (struct pipecmd*)cmd;
    // Your code here ...
    int fd[2]; 
    if (pipe(fd) == -1)// create a pipe
        printf("ERROR: %s", strerror(errno));
    int i = fork1();
    if (i > 0) {
        wait(&r);//mother process wait for the child and read from the pipe[1]
        close(fd[1]); // remember to close the write pipe, if not data may leak
        dup2(fd[0], STDIN_FILENO);
        runcmd(pcmd->right);
    } else {
        close(fd[0]);// child process write to the pipe[0], remember to close the read pipe, if not data may read incorrectly
        dup2(fd[1], STDOUT_FILENO);
        runcmd(pcmd->left);
    }
    break;
  } 


































