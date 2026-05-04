#include <iostream>
#include <thread>
#include <getopt.h>
#include <string>
#include <signal.h>

#include "ProcGpu.cpp"

int main(int argc, char* argv[]) {
    
    int opt; 
    static struct option long_options[] { 
        {"device", required_argument, 0, 'd'}, 
        {"version", optional_argument, 0, 'v'},
        {0,0,0,0}
    }; 

    bool get_version {false};

    std::string device_name {"/dev/ttyS6"};
    while ((opt = getopt_long(argc, argv, "d", long_options, nullptr)) != -1) { 
        switch (opt) { 
            case 'd': 
                device_name = optarg;
                break; 
            case 'v':
                get_version = true;
#if defined(GIT_COMMIT_HASH) && defined(GIT_COMMIT_DATE)
                std::cout << "Version 1.0.0\n";
                std::cout << "Commit: " << GIT_COMMIT_HASH << " " << GIT_COMMIT_DATE << std::endl;
#else
                std::cout << "No git info\n";
#endif
                break;
            default: 
                break; 
        } 
    }

    if (!get_version) {
            TI_Gpu ti(device_name);
        if (!ti.Init()) {
            std::cerr << "Init failed\n";
            return 1;
        }
    
        sigset_t mask;
        sigemptyset(&mask);
        sigaddset(&mask, SIGINT);
        sigaddset(&mask, SIGTERM);
        sigaddset(&mask, SIGHUP);
        sigaddset(&mask, SIGQUIT);

        if (pthread_sigmask(SIG_BLOCK, &mask, nullptr) != 0) {
            perror("pthread_sigmask");
            return 1;
        }

        ti.Start();

        int signo = 0;
        int r = sigwait(&mask, &signo);
        if (r != 0) {
            std::cerr << "sigwait failed: " << strerror(r) << std::endl;
        } else {
            std::cout << "\nGot signal " << signo << ", shutting down\n";

            ti.Stop(); 
        }
    }

    return 0;
}
