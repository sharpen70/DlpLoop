#include "Rule.h"
#include "structs.h"
#include "Vocabulary.h"
#include <algorithm>
#include <assert.h>
#include <cstdlib>

Rule::Rule() {
    head_length = 0;
    body_length = 0;
}

Rule::Rule(_rule* r) : 
        head_length(r->head_length), type(r->type), body_length(r->body_length) {
    for(int i = 0; i < (r->head_length); i++) {
        head.insert(r->head[i]);
    }
    for(int i = 0; i < (r->body_length); i++) {
        body_lits.insert(r->body[i]);
    }
}
Rule::Rule(const Rule& _rhs) : 
        head(_rhs.head),
        type(_rhs.type),
        body_lits(_rhs.body_lits),
        body_length(_rhs.body_length),
        head_length(_rhs.head_length) {
}
Rule::~Rule() {
    body_lits.clear();
}
Rule& Rule::operator = (const Rule& _rhs) {
    head = _rhs.head;
    type = _rhs.type;
    head_length = _rhs.head_length;
    body_length = _rhs.body_length;
    body_lits = _rhs.body_lits;
    return *this;
}

bool Rule::cmp_head(set<int>& s) {
    set<int>::iterator it1 = this->head.begin(), it2 = s.begin();
    while(it1 != this->head.end() && it2 != s.end()) {
        if(*it1 < *it2) it1++;
        else if(*it1 == *it2) return true;
        else it2++;
    }

    return false;    
}

bool Rule::cmp_positive_body(set<int>& s) {
    set<int>::iterator it1 = this->body_lits.begin(), it2 = s.begin();
    while(it1 != this->body_lits.end() && it2 != s.end()) {
        if(*it1 < *it2) it1++;
        else if(*it1 == *it2) return true;
        else it2++;
    }

    return false;
}

void Rule::cal_positive_body_m(set<int>& s, set<int>& r) {
    set_difference(this->body_lits.begin(), this->body_lits.end(), s.begin(),
               s.end(), inserter(r, r.begin()));
}

void Rule::output(FILE* _out) const {
    if(head_length > 0)
        for(set<int>::iterator pit = head.begin(); pit != head.end(); pit++) {
            fprintf(_out, "%s", Vocabulary::instance().getAtom(*pit));
            if(pit != (--head.end())) fprintf(_out, "|");
        }
    
    if(type != FACT) {
        fprintf(_out, " :- ");
        for(set<int>::iterator pit = body_lits.begin(); pit != 
                body_lits.end(); pit++) {
            if(*pit < 0) fprintf(_out, "not ");
            int id = (*pit < 0) ? (-1 * (*pit)) : *pit;
            fprintf(_out, "%s", Vocabulary::instance().getAtom(id));
            if(pit != (--body_lits.end())) {
                fprintf(_out, ",");
            }
        }
    }
}