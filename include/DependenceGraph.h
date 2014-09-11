/* 
 * File:   DependenceGraph.h
 * Author: sharpen
 *
 * Created on September 3, 2013, 11:34 PM
 */

#ifndef DEPENDENCEGRAPH_H
#define	DEPENDENCEGRAPH_H

#include <algorithm>
#include "Vocabulary.h"
#include <vector>
#include <map>
#include <set>
#include <stack>
#include "Rule.h"

using namespace std;
extern vector<Rule> G_NLP;

struct Loop {
    set<int> loopNodes;
    set<Rule*> ESRules;
    set<int> ESHead;
    bool element;
    bool element_a;
    bool proper;
    bool proper_a;
    bool w_element;
    bool w_proper;

    Loop() {
        loopNodes.clear();
        ESRules.clear();
        ESHead.clear();
        
        element = true;
        proper = true;
        w_element = true;
        w_proper = true;
        element_a = false;
        proper_a = false;
    }
    Loop(set<int> s) {
        loopNodes = s;
        ESRules.clear();
        ESHead.clear();
    }

    bool operator<(const Loop& l) const {
        return this->loopNodes < l.loopNodes;
    }
    
    bool operator==(const Loop& l) const{
        return this->loopNodes == l.loopNodes;
    }
    
    void print(FILE* out) {
        fprintf(out, "Loop: ");
        for(set<int>::iterator it = this->loopNodes.begin(); it != this->loopNodes.end(); it++) {
            fprintf(out, "%s ", Vocabulary::instance().getAtom(*it));
        }
        //fprintf(out, "\n");
        fprintf(out, "ES: ");
        for(set<Rule*>::iterator it = this->ESRules.begin(); it != this->ESRules.end(); it++) {            
            if(it != this->ESRules.begin()) fprintf(out, ", ");
            (*it)->output(out);            
            //fprintf(out, "%d ", *it);
        }
        fprintf(out, "\n");
    }
    
    bool cmp_include(Loop* l) {
        return includes(this->loopNodes.begin(), this->loopNodes.end(), l->loopNodes.begin(),
                l->loopNodes.end()) && this->loopNodes.size() > l->loopNodes.size();
    }
    
    int cmp_support(Loop* l) {
        if(this->ESRules.size() == 0) return 0;
        if(includes(l->ESRules.begin(), l->ESRules.end(), this->ESRules.begin(), this->ESRules.end()) 
                && l->ESRules.size() > this->ESRules.size())
            return 1;
        return 2;
    }

    int cmp_el_support(Loop* l) {
        if(this->ESRules.size() == 0) return 0;
        
        set<int> diff;
        set_difference(l->loopNodes.begin(), l->loopNodes.end(), this->loopNodes.begin(),
               this->loopNodes.end(), inserter(diff, diff.begin()));
        
        set<Rule*> el_supp;
        for(set<Rule*>::iterator it = this->ESRules.begin(); it != this->ESRules.end(); it++) {
            set<int>::iterator it1 = (*it)->head.begin(), it2 = diff.begin();
            while(it1 != (*it)->head.end() && it2 != diff.end()) {
                if(*it1 < *it2) it1++;
                else if(*it1 == *it2) break;
                else it2++;
            } 
            if(it1 == (*it)->head.end() || it2 == diff.end()) el_supp.insert(*it);            
        }
        
        if(includes(l->ESRules.begin(), l->ESRules.end(), el_supp.begin(), el_supp.end())
                && l->ESRules.size() > el_supp.size())
            return 1;
        
        return 2;
    }
    
    int cmp_el_support(Loop* l, set<Rule*>& r) {
        set<int> diff;
        set_difference(l->loopNodes.begin(), l->loopNodes.end(), this->loopNodes.begin(),
               this->loopNodes.end(), inserter(diff, diff.begin()));
        
        set<Rule*> el_supp;
        for(set<Rule*>::iterator it = this->ESRules.begin(); it != this->ESRules.end(); it++) {
            set<int>::iterator it1 = (*it)->head.begin(), it2 = diff.begin();
            while(it1 != (*it)->head.end() && it2 != diff.end()) {
                if(*it1 < *it2) it1++;
                else if(*it1 == *it2) break;
                else it2++;
            } 
            if(it1 == (*it)->head.end() || it2 == diff.end()) el_supp.insert(*it);            
        }
        //printf("el_supp size %d l size %d\n", el_supp.size(), l->ESRules.size());
        set_difference(el_supp.begin(), el_supp.end(), l->ESRules.begin(), l->ESRules.end(),
        inserter(r, r.begin()));

        if(el_supp.size() == l->ESRules.size()) return 0;
        else return 1;
    }
        
    bool cmp_weak_proper_head(Loop* l) {
        for(set<Rule*>::iterator it = this->ESRules.begin(); it != this->ESRules.end(); it++) {
            set<int> s1, s2;
            set_intersection((*it)->head.begin(), (*it)->head.end(), this->loopNodes.begin(),
                    this->loopNodes.end(), inserter(s1, s1.begin()));
            set_intersection((*it)->head.begin(), (*it)->head.end(), l->loopNodes.begin(),
                    l->loopNodes.end(), inserter(s2, s2.begin()));
            if(!includes(s2.begin(), s2.end(), s1.begin(), s1.end())) return false;
        }        
        
        return true;
    }
    
    bool cmp_proper_head(Loop* l) {
        set<int> nodes, itrs1, itrs2;
        set_union(this->loopNodes.begin(), this->loopNodes.end(), l->loopNodes.begin(), 
                l->loopNodes.end(), inserter(nodes, nodes.begin()));
        set_intersection(this->ESHead.begin(), this->ESHead.end(), nodes.begin(), 
                nodes.end(), inserter(itrs1, itrs1.begin()));
        set_intersection(l->ESHead.begin(), l->ESHead.end(), l->loopNodes.begin(), 
                l->loopNodes.end(), inserter(itrs2, itrs2.begin()));
        
        return includes(itrs2.begin(), itrs2.end(), itrs1.begin(), itrs1.end());
    }
    
    void cmp_ES_head(set<int>& s, set<int>& re) {
        for(set<Rule*>::iterator it = this->ESRules.begin(); it != this->ESRules.end(); it++) {
            for(set<int>::iterator rit = (*it)->head.begin(); rit != (*it)->head.end(); rit++) {
                if(s.find(*rit) != s.end()) re.insert(*rit);
            }
        }
    }
    
    bool has_common(Loop* l) {
        set<int>::iterator it1 = this->loopNodes.begin(), it2 = l->loopNodes.begin();
        while(it1 != this->loopNodes.end() && it2 != l->loopNodes.end()) {
            if(*it1 < *it2) it1++;
            else if(*it1 == *it2) return true;
            else it2++;
        }
        return false;
    }
};

class DependenceGraph {
  public:
    DependenceGraph(bool _test, int _interval);
    DependenceGraph(const DependenceGraph& orig);
    ~DependenceGraph();
    
    void findSCC();
    void findSCC(vector<Loop*>& loops);
    void findChildSCC(set<int>& s, vector<Loop*>& loops, bool single);  
    bool getParentSCC(set<int>& s, set<int>& result);

    void findESRules(Loop& loop);
    void ep_check(Loop* loop);
    
    bool EL_(Loop* l);
    bool Proper_(Loop* l, vector<Loop*> sccs);
    bool WEL(set<int>& s);
    int HWEF();

    void cmp_loop();
    void cmp_loop(Loop* l);
    
    void print_ep_loop();
    void print_ep_loop_test();
    void add_atom_loops();
    
    bool is_two_atom_loop(int a, int b);
    
    set<Loop> loops_map;
    vector<Loop*> all_loops;
    set< set<int> > head_pair; 
    
    vector<Loop*> _sccs;
    
 private:
    vector<Rule> nlp;
    map<int, set<int> > dpdGraph;
    map<int, set<Rule*> > dpdRules;
    
    bool test;
    int interval;
    int nodeNumber;
    int maxNode;
    
    void tarjan(int u, vector<Loop*>& loops);
    void tarjan_s(int u, vector<Loop*>& loops);
    //whether s is in the same scc
    bool tarjan_a(int u, set<int>& s, set<int>& result);
    //SCC
    bool *visit;
    bool *involved;
    int *DFN;
    int *Low;
    int Index;
    stack<int> vs;  
};

#endif	/* DEPENDENCEGRAPH_H */
