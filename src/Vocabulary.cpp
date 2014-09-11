#include "Vocabulary.h"
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <assert.h>

Vocabulary& Vocabulary::instance() {
    static Vocabulary vocabulary;
    return vocabulary;
}

Vocabulary::Vocabulary() : atom_id(1) {
    atom_list.clear();
}

Vocabulary::~Vocabulary() {
    atom_list.clear();
}

int Vocabulary::addAtom(char* atom) {
    atom_list.push_back(atom);
    return atom_id++;
}

int Vocabulary::queryAtom(char* atom) {
    for(int i = 0; i < atom_list.size(); i++) {
        if(strcmp(atom, atom_list.at(i)) == 0) {
            return i + 1;
        }
    }
    
    return -1;
}

void Vocabulary::dumpVocabulary(FILE* _out) {
    for(int i = 0; i < atom_list.size(); i++) {
        fprintf(_out, "%s\n", atom_list.at(i));
    }
}

char* Vocabulary::getAtom(int id) {
    return atom_list.at(id - 1);
}

int Vocabulary::apSize() {
    return atom_list.size();
}