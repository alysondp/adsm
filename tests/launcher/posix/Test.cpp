#include <sys/types.h>
#include <sys/wait.h>

#include "../Test.h"
    
void
Test::TestCase::run(std::string exec)
{
    pid_t pid = fork();
    if (pid == 0) {
        setEnvironment();

        printf("Launching: %s - %s\n", exec.c_str(), name_.c_str());
        int execReturn = execlp(exec.c_str(), exec.c_str(), (char *)NULL);
        printf("Failure! execve error code %d\n", execReturn);
        abort();
    }
    else if(pid < 0) {
        printf("Failure! failed to fork %d\n", pid);
        abort();
    } else {
        run_ = true;
        int ret = ::wait(&status_); 
    }
}

void
Test::TestCase::setEnvironment()
{
    std::vector<KeyVal>::const_iterator it;
    for (it = keyvals_.begin(); it != keyvals_.end(); it++) {
        ::setenv(it->first.c_str(), it->second.c_str(), 1);
    }
    ::setenv("PATH", ".", 1);
}

/* vim:set backspace=2 tabstop=4 shiftwidth=4 textwidth=120 foldmethod=marker expandtab: */
