/* 
 * File:   Rule.h
 * Author: yzf
 *
 * Created on July 30, 2013, 7:59 AM
 */

#ifndef RULE_H
#define	RULE_H

#include <cstdio>
#include <cstdlib>
#include <vector>
#include "structs.h"

using namespace std;
/*
 * 规则类
 */
class Rule {
public:
    int head_length;
    int body_length;
    set<int> head;
    set<int> body_lits;
    RULE_TYPE type;
public:
    Rule();
    Rule(_rule* rule);
    Rule(const Rule& _rhs);
    ~Rule();
    
    void cal_positive_body_m(set<int>& s, set<int>& result);
    bool cmp_head(set<int>& s);
    bool cmp_positive_body(set<int>& s);
    Rule& operator = (const Rule& _rhs);
    void output(FILE* _out) const;
};

#endif	/* RULE_H */

