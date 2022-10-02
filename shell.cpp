#include <iostream>

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/param.h>
#include <ctime>
#include <chrono>
#include <time.h>

#include <vector>
#include <string>
#include <string.h>

#include "Tokenizer.h"

// all the basic colours for a shell prompt
#define RED     "\033[1;31m"
#define GREEN	"\033[1;32m"
#define YELLOW  "\033[1;33m"
#define BLUE	"\033[1;34m"
#define WHITE	"\033[1;37m"
#define NC      "\033[0m"

using namespace std;

int main () 
{

    for (;;) 
    {
        // need date/time, username, and absolute path to current dir
        char directory [MAXPATHLEN];
        char *path = getcwd(directory,MAXPATHLEN);
        char buffer[256];

        time_t timer = time(NULL);
        struct tm* timeInfo = localtime(&timer);
        strftime(buffer,256,"%b %d %T",timeInfo);

        string absolutePath = path;
        cout << YELLOW << buffer << " "<< YELLOW << getenv("USER") << ":"<<YELLOW << absolutePath <<"$"<<NC<<" ";
        
        // get user inputted command
        string input;
        getline(cin, input);

        //Save original stdin and stdout 
        int in = dup(0);
        int out = dup(1);
        vector <char*> arguments;
        vector <char*> cdArgumentChecker;

        if (input == "exit" || input == "Exit") 
        {  // print exit message and break out of infinite loop
            cout << RED << "Now exiting shell..." << endl << "Goodbye" << NC << endl;
            break;
        }

        if(input == "")
        {
            continue;
        } 

        // get tokenized commands from user input
        Tokenizer token(input);
        if (token.hasError()) {  // continue to next prompt if input had an error
            continue;
        }
        
        cdArgumentChecker.push_back(const_cast<char*>(token.commands[0]->args[0].c_str()));
        cdArgumentChecker.push_back(const_cast<char*>(token.commands[0]->args[1].c_str()));
        cdArgumentChecker.push_back(NULL);

        char currentDirectory [MAXPATHLEN];
        char *pathDirectory = getcwd(currentDirectory,MAXPATHLEN);

        string currentPathDirectory = pathDirectory;
        string temp;
        string previousDirectory;
        
        if(strcmp(cdArgumentChecker[0], "cd") == 0 && strcmp(cdArgumentChecker[1],"-") != 0)
        {
            
            string newPathDirectory = currentPathDirectory + "/" + cdArgumentChecker[1];
            previousDirectory = currentPathDirectory;

            if (chdir(newPathDirectory.c_str()) < 0) 
            {
                perror("chdir");
            }; 

        } 
        
        else if(strcmp(cdArgumentChecker[0], "cd") == 0 && strcmp(cdArgumentChecker[1],"-") == 0)
        {
            if (chdir(previousDirectory.c_str()) < 0) 
            {
                perror("chdir");
            }; 
        }   
        else
        {
            for(long unsigned int i = 0; i<token.commands.size(); i++)
            {            
                int fd[2];
                if(pipe(fd) == -1)
                {
                    return 0;
                }

                int pid = fork();

                if(pid == 0)
                { 
                    
                    for(long unsigned int j = 0; j<token.commands[i]->args.size();j++)
                    {
                        arguments.push_back(const_cast<char*>(token.commands[i]->args[j].c_str()));
                    }

                    arguments.push_back(NULL); 
                    
                    if(i < token.commands.size() - 1)
                    {
                        dup2(fd[1],STDOUT_FILENO);
                    }
                    
                    close(fd[STDIN_FILENO]);

                    if (execvp(arguments[0], arguments.data()) < 0) 
                    {
                        perror("execvp");
                        exit(1);
                    }; 
                }
                else
                {
                    //Redriect the SHELL(PARENT)'s input to the read end of the pipe 
                    dup2(fd[0],STDIN_FILENO);

                    //Close the write end of the pipe 
                    close(fd[STDOUT_FILENO]);

                    //Wait until the last command finishes 
                    if(i==token.commands.size()-1)
                    {
                        int status;
                        waitpid(pid,&status,0);
                    }
                }
            }

            //use dup(2) to restore the stdin and stdout 
            dup2(in,0);
            dup2(out,1);

        
        
        }


    }

    return 0; 
}

// // print out every command token-by-token on individual lines
        // // prints to cerr to avoid influencing autograder
        // for (auto cmd : token.commands) {
        //     for (auto str : cmd->args) {
        //         cerr << "|" << str << "| ";
        //     }
        //     if (cmd->hasInput()) {
        //         cerr << "in< " << cmd->in_file << " ";
        //     }
        //     if (cmd->hasOutput()) {
        //         cerr << "out> " << cmd->out_file << " ";
        //     }
        //     cerr << endl;
        // }


/*
        // fork to create child
        pid_t pid = fork();
        if (pid < 0) 
        {  // error check
            perror("fork");
            exit(2);
        }

        if (pid == 0) 
        {  // if child, exec to run command
            // run single commands with no arguments
            char* args[] = {(char*) tknr.commands.at(0)->args.at(0).c_str(), nullptr};

            if (execvp(args[0], args) < 0) 
            {  // error check
                perror("execvp");
                exit(2);
            }
        }
        else 
        {  // if parent, wait for child to finish
            int status = 0;
            waitpid(pid, &status, 0);
            if (status > 1) 
            {  // exit if child didn't exec properly
                exit(status);
            }
        } */