#include <iostream>
#include <stdio.h>
#include <unistd.h>
#include <vector>
#include <string>
#include <cstring>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <ctype.h>
#include <chrono>

using namespace std;

string trim(string input) {
    string ret;
    size_t first = input.find_first_not_of(" ");//first space
    size_t last = input.find_last_not_of(" ");//last space
    input = input.substr(first);
    ret = input.substr(0, last + 1);
    if (ret.find("\"") != string::npos) {
        // if we find quotation marks
        string quotes;
        size_t first = ret.find_first_of("\"");
        if (first > 0)
        {
            quotes = ret.substr(0, first);
        }
        ret = ret.substr(first + 1);
        size_t second = ret.find_last_of("\""); 
        quotes.append(ret.substr(0, second));

        return quotes;; // return string with no quotation marks
    }
    if (ret.find("\'") != string::npos) {//single quote
        string quotations1;
        size_t first = ret.find_first_of("\'"); 
        if (first > 0)
        {
            quotations1 = ret.substr(0, first);
        }
        ret = ret.substr(first + 1);
        size_t second = ret.find_last_of("\'");
        quotations1.append(ret.substr(0, second));

        return quotations1; // returns ret string with no quote mark
    }
    return ret; // returns ret string
}

char** vec_to_char_array(vector<string> parts) {//converts the vector of string to char
    char** array = new char* [parts.size()+1];
    for (int i = 0; i < parts.size(); i++) {
        array[i] = new char[parts[i].size()];
        strcpy(array[i], parts[i].c_str());
    }
    array[parts.size()] = NULL;
    return array;
}

vector<string> split(string line, string seperator = " ") {//splits string into a vector by spaces
    vector<string> space;
    size_t start;
    size_t end = 0;
    while ((start = line.find_first_not_of(seperator, end)) != string::npos) {
        end = line.find(seperator, start);
        space.push_back(line.substr(start, end - start));
    }
    return space;
}

int main() {
    vector<int> bgs; //list of bgs
    char* name = new char[50];
    getlogin_r(name, 50);// getting username
    dup2(0,10);
    while(true) {
        dup2(10,0);
        for (int i=0; i < bgs.size(); i++) {//zombie handler
            if (waitpid (bgs [i], 0, WNOHANG)) {
                cout << "Process: " << bgs[i] << "ended" << endl;
                bgs.erase(bgs.begin() + i);
                i--;
            }
        }
        time_t currtime = time(0);
        string currt = ctime(&currtime);
        currt.erase(currt.end()-1);
        currt.erase(currt.end()-1);
        currt.erase(currt.end()-1);
        currt.erase(currt.end()-1);
        currt.erase(currt.end()-1);//didnt want to have year
        cout << currt << name << "$ "; //prints a prompt with username
        string inputline;
        getline (cin, inputline);
        inputline = trim(inputline);
        char dirarray[1000];
        string currDir = getcwd(dirarray, sizeof(dirarray));//current directory
        string prevDir = currDir;
        if (inputline == string("exit")) {
            cout << "Bye!! End of shell" << endl;
            break;
        }
        if (inputline[inputline.size()-1] == '&') {
                cout << "Bg process found" << endl;
                bool bg = true;
                inputline = inputline.substr (0, inputline.size()-1);
        }
        if (inputline.find("cd") == 0) {
                    vector<string> direc = split(inputline);
                    string dirname = trim(direc[1]);
                    if (dirname == "-") {//checking for previous
                        chdir(prevDir.c_str());
                    }
                    else if (dirname[0] == '/') {
                        chdir(dirname.c_str());
                    }
                    else {
                        dirname = currDir + "/" + dirname;
                        chdir(dirname.c_str()); 
                    }
                    prevDir = currDir;
                    currDir = getcwd(dirarray, sizeof(dirarray));
                    continue;
            }
        int echocheck = inputline.find("echo");
        vector<string> pipeline;
        if (echocheck >= 0) {
            pipeline.push_back(inputline);
        }
        else {
            pipeline = split(inputline, "|");//creating pipe
        }
        for(int i = 0; i < pipeline.size(); i++) {
            int fd[2];
            pipe(fd);
            bool bg = false;
            int pid = fork();//creates the parent and child
            if (pid == 0) {//child process
                pipeline[i] = trim(pipeline[i]);
                if (pipeline[i].find("awk") == 0) {// fixes awk function
                    pipeline[i] = pipeline[i].substr(3);
                    while (pipeline[i].find(" ") != -1) {
                        size_t space = pipeline[i].find_last_of(" ");
                        pipeline[i] = pipeline[i].substr(0, space) + pipeline[i].substr(space + 1);
                    }
                    pipeline[i] = "awk " + pipeline[i];

                }
                if (echocheck < 0) {
                    int pos1 = inputline.find('>');//writing
                    if(pos1 >= 0) {
                        string command = inputline.substr(0, pos1);
                        string filename = trim(inputline.substr(pos1+1));
                        pipeline[i] = command;
                        int fd = open(filename.c_str(), O_WRONLY | O_CREAT, S_IWUSR | S_IRUSR);
                        dup2(fd, 1);
                        close(fd);
                    }
                    int pos2 = inputline.find('<');//reading
                    if(pos2 >= 0) {
                        string command = inputline.substr(0, pos2);
                        string filename = trim(inputline.substr(pos2+1));
                        pipeline[i] = command;
                        int fd = open(filename.c_str(), O_RDONLY | O_CREAT, S_IWUSR | S_IRUSR);
                        dup2(fd, 0);
                        close(fd);
                    }
                }     
                if(i < pipeline.size()-1) {
                    dup2(fd[1], 1);
                }
                pipeline = split(pipeline[i]);
                char** args = vec_to_char_array(pipeline);
                execvp (args[0], args);
            }
            else {// parent process
                if (!bg) {
                    if (i ==  pipeline.size()-1) {
                        waitpid (pid, 0, 0);//wait for child process
                    }
                }
                else {
                    bgs.push_back(pid);
                }
                dup2(fd[0], 0);
                close(fd[1]);
            }
        }
    }
}