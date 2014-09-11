/* 
 * File:   main.cpp
 * Author: yzf
 *
 * Created on July 10, 2013, 2:17 PM
 */

#include <cstdlib>
#include <cstdio>
#include <assert.h>
#include "Vocabulary.h"
#include "structs.h"
#include "DependenceGraph.h"
#include <set>
#include <iostream>
#include <unistd.h>
#include <fstream>
#include <cstring>
#include <sstream>
#include <ctime>

using namespace std;

extern FILE* yyin;
FILE* out;
extern vector<Rule> G_NLP;
extern int yyparse();

int main(int argc, char** argv) {
    bool test = false;
    int interval = 500; 
    
    if(argc > 1) {
        yyin = fopen(argv[1], "r");
        if(argc > 2) {
            if(argv[2][0] != '-') out = fopen(argv[2], "a+");
            else out = stdout;
            
            for(int i = 2; i < argc; i++) {
                if(strcmp(argv[i], "-t") == 0) {
                    printf("test mode\n");
                    test = true;
                }
                if(strcmp(argv[i], "-n") == 0) {
                    if(argc > i + 1) interval = atoi(argv[i+1]);
                    else {
                        printf("-n needs argument\n");
                        return 0;
                    }
                }
            }
        }
        else out = stdout;
    }
    else {
//        yyin = fopen("res/Benchmarks/DLV/non-hcf/qbf.gw/x6.16.ground", "r");
        yyin = fopen("res/ham5_5.lp", "r");
        out = stdout;
        test = true;
    }
    
    yyparse();

    fclose(yyin);
    
    DependenceGraph dpg(test, interval);
    dpg.cmp_loop();
//    int result = dpg.HWEF();
//    
//    switch(result) {
//        case 0: fprintf(out, "  Definitely not HWEF\n");break;
//        case 1: fprintf(out, "  Possibly not HWEF\n");break;
//        case 2: fprintf(out, "  Yes, HWEF\n");break;
//    }

    return 0;
}