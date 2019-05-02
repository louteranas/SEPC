#ifndef ENSISHELL_H
#define ENSISHELL_H

void q1();

struct Pid{
  pid_t pid;
  char * cmd;
  struct Pid *next;
  int show;
};

int question6_executer(char *line);

void terminate(char *line);

void jobsDebug(struct Pid **listPids);

void jobs(struct Pid **listPids);

#endif
