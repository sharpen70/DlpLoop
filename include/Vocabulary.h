#ifndef VOCABULARY_H
#define VOCABULARY_H

#include "structs.h"
#include <cstdio>
#include <vector>

using namespace std;
/*
 * 字符表类，保存各种信息
 */
class Vocabulary {
private:
    unsigned int atom_id;
    vector<char*> atom_list;
private:
    Vocabulary();
    Vocabulary(const Vocabulary&);
    Vocabulary& operator =(const Vocabulary&);
    ~Vocabulary();
    
public:
    static Vocabulary& instance();
    void dumpVocabulary(FILE* _out);
    int addAtom(char*);
    int apSize();
    char* getAtom(int id);
    int queryAtom(char*);
};

#endif
