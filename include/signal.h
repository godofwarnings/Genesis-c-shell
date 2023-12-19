#ifndef _SIGNAL_H_
#define _SIGNAL_H_

int process_exists(pid_t pid);


void reset_signals();
void bg_process_handler();
void install_signal_handlers();
void handle_sigchild();

#endif