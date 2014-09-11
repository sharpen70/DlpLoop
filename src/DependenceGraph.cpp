/* 
 * File:   DependenceGraph.cpp
 * Author: sharpen
 * 
 * Created on September 3, 2013, 11:34 PM
 */

#include "DependenceGraph.h"
#include <cstring>
#include <assert.h>
#include <vector>
#include "Rule.h"
#include <map>
#include "structs.h"
#include "Vocabulary.h"
#include <string.h>
#include <set>
#include <algorithm>
#include <functional>

extern vector<Rule> G_NLP;
extern FILE* out;

DependenceGraph::DependenceGraph(bool _test, int _interval) {
    set<int> nodeSet;
    
    for(int i = 0; i < G_NLP.size(); i++) {
        Rule* r = &G_NLP[i];
        if(r->type == RULE || r->type == FACT) {
            for(set<int>::iterator sit = r->head.begin(); sit != r->head.end(); sit++) {
                nodeSet.insert(*sit);
                dpdRules[*sit].insert(r);
                for(set<int>::iterator p_it = r->body_lits.begin(); p_it != r->body_lits.end(); p_it++) {
                    if(*p_it > 0) {
                        dpdGraph[*sit].insert(*p_it);
                        nodeSet.insert(*p_it);
                    }
                }
            }
        }
    }
    
    nodeNumber = nodeSet.size();
    maxNode = *(--nodeSet.end());
    visit = new bool[maxNode + 1];
    memset(visit, false, sizeof(bool) * (maxNode + 1));
    DFN = new int[maxNode + 1];
    memset(DFN, 0, sizeof(int) * (maxNode + 1));
    Low = new int[maxNode + 1];
    memset(Low, 0, sizeof(int) * (maxNode + 1));
    involved = new bool[maxNode + 1];
    memset(involved, true, sizeof(bool) * (maxNode + 1));
    
    test = _test;
    interval = _interval;
    
    findSCC(_sccs);
}

DependenceGraph::~DependenceGraph() {
    dpdGraph.clear();
    dpdRules.clear();
    delete[] visit;
    delete[] DFN;
    delete[] Low;
}

void DependenceGraph::findSCC(vector<Loop*>& loops) {
    memset(visit, false, sizeof(bool) * (maxNode + 1));

    for(map<int, set<int> >::iterator it = dpdGraph.begin(); it != dpdGraph.end(); it++) {
        if(!visit[it->first] && involved[it->first]) {
            Index = 0;
            tarjan(it->first, loops);
        }
    }
}

void DependenceGraph::findChildSCC(set<int>& s, vector<Loop*>& loops, bool single) {
   memset(involved, false, sizeof(bool) * (maxNode + 1)); 
   for(set<int>::iterator it = s.begin(); it != s.end(); it++) involved[*it] = true;
   
   for(set<int>::iterator it = s.begin(); it != s.end(); it++) {
       memset(visit, false, sizeof(bool) * (maxNode + 1)); 
       involved[*it] = false;
       for(set<int>::iterator sit = s.begin(); sit != s.end(); sit++) {
            if(!visit[*sit] && involved[*sit]) {
                Index = 0;
                if(single) tarjan_s(*sit, loops);
                else tarjan(*sit, loops);
            }       
       }
       involved[*it] = true;
   }
}

bool DependenceGraph::getParentSCC(set<int>& s, set<int>& result) {
    Index = 0;
    memset(visit, false, sizeof(bool) * (maxNode +1));
    
    return tarjan_a(*(s.begin()), s, result);
}

bool DependenceGraph::is_two_atom_loop(int a, int b) {
    set<int>& _a = dpdGraph[a], _b = dpdGraph[b];
    if(_a.find(b) != _a.end() && _b.find(a) != _b.end()) return true;
    return false;
}

bool DependenceGraph::WEL(set<int>& s) {
    set<int> scc;
    if(!getParentSCC(s, scc)) return false;
    else {
        for(int i = 0; i < G_NLP.size(); i++) {
            if(G_NLP[i].cmp_positive_body(s)) {
                memset(involved, false, sizeof(bool) * (maxNode + 1));
                set<int> tmp, Gr, Cr, tmp1;
                G_NLP[i].cal_positive_body_m(s, tmp);
                set_difference(scc.begin(), scc.end(), tmp.begin(),
                tmp.end(), inserter(Gr, Gr.begin()));
                for(set<int>::iterator it = Gr.begin(); it != Gr.end(); it++) 
                    involved[*it] = true;
                
                if(getParentSCC(s, Cr)) {
                    set_difference(Cr.begin(), Cr.end(), s.begin(),
                        s.end(), inserter(tmp1, tmp1.begin()));
                    if(G_NLP[i].cmp_head(tmp1)) return true;
                }
            }
        }
    }
    
    return false;
}

int DependenceGraph::HWEF() {
  // fprintf(out, "Rule size: %d\n", G_NLP.size());
   for(int i = 0; i < G_NLP.size(); i++) {
       set<int>* head = &(G_NLP[i].head);
       if(head->size() >= 2) {
           set<int> pair_s;
           for(set<int>::iterator it1 = head->begin(); it1 != (--head->end()); it1++) { 
               set<int>::iterator it2 = it1;
               it2++;
               for(; it2 != head->end(); it2++) {
                   pair_s.insert(*it1);
                   pair_s.insert(*it2);
                   
                   if(head_pair.find(pair_s) != head_pair.end()) continue;
                   head_pair.insert(pair_s);
                   
                   if(is_two_atom_loop(*it1, *it2)) return 0;
                   if(WEL(pair_s)) return 1;
                   pair_s.clear();
               }
           }
       }
 //      fprintf(out, "Rule %d\n", i);
   }
   
   return 2;
}

void DependenceGraph::cmp_loop() {
    vector<Loop*> loops;
    
    add_atom_loops();
    
    memset(involved, true, sizeof(bool) * (maxNode + 1));
    findSCC(loops);

    for(int i = 0; i < loops.size(); ++i) {
        cmp_loop(loops[i]);
    }
    
    if(!test) print_ep_loop();
    else print_ep_loop_test();
    fprintf(out, "All Loop found, Done\n");
}

void DependenceGraph::cmp_loop(Loop* l) {       
    if(loops_map.find(*l) != loops_map.end()) {
        delete l;
        return;
    }
    
    loops_map.insert(*l);
    
    vector<Loop*> sccs;
    
    ep_check(l);
    
    if(all_loops.size() % interval == 0) {
        if(!test) print_ep_loop();
        else print_ep_loop_test();
    } 

    if(l->loopNodes.size() > 2) {
        findChildSCC(l->loopNodes, sccs, false);

        for(int i = 0; i < sccs.size(); i++) {
            cmp_loop(sccs[i]);
        }
    }
}

void DependenceGraph::add_atom_loops() {
    int count = 0;
    for(map<int, set<int> >::iterator it = dpdGraph.begin(); it != dpdGraph.end(); it++) {
        Loop* l = new Loop();
        l->loopNodes.insert(it->first);
        findESRules(*l);
        l->element_a = true;
        if(Proper_(l, _sccs)) l->proper_a = true;
        all_loops.push_back(l);
        count++;
    }
    fprintf(out, "AL size: %d\n", count);
    fflush(out);
}

void DependenceGraph::print_ep_loop_test() {
    fprintf(out, "All loop size:%d\n", all_loops.size());
    int counter_e = 0;
    int counter_p = 0;
    int counter_we = 0;
    int counter_wp = 0;
    int counter_ea = 0;
    
    vector<Loop*> we, e, wp, p, ea;
    
    for(int i = 0; i < all_loops.size(); ++i) {
        if(all_loops[i]->element) {counter_e++;e.push_back(all_loops[i]);}
        if(all_loops[i]->proper) {counter_p++;p.push_back(all_loops[i]);}
        if(all_loops[i]->w_element) {counter_we++;we.push_back(all_loops[i]);}
        if(all_loops[i]->w_proper) {counter_wp++;wp.push_back(all_loops[i]);}
        if(all_loops[i]->element_a) {counter_ea++;ea.push_back(all_loops[i]);}
    }

    fprintf(out, "Elementary Loop size:%d\n", counter_e);
    for(int i = 0; i < e.size(); i++) e[i]->print(out);
    fprintf(out, "Elementary Loop* size:%d\n", counter_ea);
    for(int i = 0; i < ea.size(); i++) ea[i]->print(out);
    fprintf(out, "Proper Loop size:%d\n", counter_p);
    for(int i = 0; i < p.size(); i++) p[i]->print(out);
    fprintf(out, "Weak Elementary Loop size:%d\n", counter_we);
    for(int i = 0; i < we.size(); i++) we[i]->print(out);
    fprintf(out, "Weak Proper Loop size:%d\n", counter_wp);
    for(int i = 0; i < wp.size(); i++) wp[i]->print(out);
    
    fflush(out);
}

void DependenceGraph::print_ep_loop() {
    fprintf(out, "All size:%d ", all_loops.size());
    int counter_e = 0;
    int counter_p = 0;
    int counter_we = 0;
    int counter_wp = 0;
    int counter_ea = 0;
    int counter_pa = 0;
    for(int i = 0; i < all_loops.size(); ++i) {
        if(all_loops[i]->element) counter_e++;
        if(all_loops[i]->proper) counter_p++;
        if(all_loops[i]->w_element) counter_we++;
        if(all_loops[i]->w_proper) counter_wp++;
        if(all_loops[i]->element_a) counter_ea++;
        if(all_loops[i]->proper_a) counter_pa++;
    }

    fprintf(out, "EL size:%d ", counter_e);
    fprintf(out, "EL* size:%d ", counter_ea);
    fprintf(out, "WEL size:%d ", counter_we);
    fprintf(out, "PL size:%d ", counter_p); 
    fprintf(out, "PL* size:%d\n", counter_pa);
    fprintf(out, "WPL size:%d\n", counter_wp);
    
    fflush(out);
}

void DependenceGraph::ep_check(Loop* l) { 
    findESRules(*l);
    
    if(EL_(l)) l->element_a = true;
    if(Proper_(l, _sccs)) l->proper_a = true;

   // if(test) l->print(stdout);
    for(vector<Loop*>::iterator it = all_loops.begin(); it != all_loops.end(); ++it) {
        if((*it)->w_element && l->has_common(*it)) {
            int this_el_support_l = (*it)->cmp_el_support(l);
            int l_el_support_this = l->cmp_el_support(*it);
            int this_support_l = (*it)->cmp_support(l);
            int l_support_this = l->cmp_support(*it);
            bool this_include_l = (*it)->cmp_include(l);
            bool l_include_this = l->cmp_include(*it);
            
            if(this_el_support_l != 2) {
                if(this_el_support_l == 0 && this_include_l) l->proper = false;
                if(this_el_support_l == 1 && (*it)->cmp_proper_head(l)) l->proper = false;

                if(this_support_l == 0 && this_include_l) l->w_proper = false;
                if(this_support_l == 1 && (*it)->cmp_weak_proper_head(l)) l->w_proper = false;
                
                if(l_include_this) {
                    l->element = false;
                    if(this_support_l != 2) l->w_element = false;
                }
            }
            if(l_el_support_this != 2) {
                if(l_el_support_this == 0 && l_include_this) (*it)->proper = false;
                if(l_el_support_this == 1 && l->cmp_proper_head(*it)) (*it)->proper = false;

                if(l_support_this == 0 && l_include_this) (*it)->w_proper = false;
                if(l_support_this == 1 && l->cmp_weak_proper_head(*it)) (*it)->w_proper = false;
                
                if(this_include_l) {
                    (*it)->element = false;
                    if(l_support_this != 2) (*it)->w_element = false;
                }
            }
        }
    }
    
    all_loops.push_back(l);
}

bool DependenceGraph::tarjan_a(int u, set<int>& s, set<int>& result) {
    DFN[u] = Low[u] = ++Index;
    vs.push(u);
    visit[u] = true;
    for(set<int>::iterator it = dpdGraph[u].begin(); it != dpdGraph[u].end(); it++) {
        if(!involved[*it]) continue;
        
        if(!visit[*it]) {
            if(tarjan_a(*it, s, result)) return true;
            else {
                if(Low[u] > Low[*it]) Low[u] = Low[*it];
            }
        }
        else {
            if(Low[u] > DFN[*it]) Low[u] = DFN[*it];
        }
    }
    if(Low[u] == DFN[u]) {
        if(vs.top() != u) {
            while(vs.top() != u) {   
                result.insert(vs.top());
                vs.pop();
            }
            result.insert(u);
            vs.pop();
            if(includes(result.begin(), result.end(), s.begin(), s.end())) return true;
            else {
                result.clear();
                return false;
            }
        }
        else {
            vs.pop();
        }
    }    
    
    return false;
}

void DependenceGraph::tarjan_s(int u, vector<Loop*>& loops) {
    DFN[u] = Low[u] = ++Index;
    vs.push(u);
    visit[u] = true;
    for(set<int>::iterator it = dpdGraph[u].begin(); it != dpdGraph[u].end(); it++) {
        if(!involved[*it]) continue;
        
        if(!visit[*it]) {
            tarjan_s(*it, loops);
            if(Low[u] > Low[*it]) Low[u] = Low[*it];
        }
        else {
            if(Low[u] > DFN[*it]) Low[u] = DFN[*it];
        }
    }
    if(Low[u] == DFN[u]) {
        Loop* l = new Loop();
        while(vs.top() != u) {                  
            l->loopNodes.insert(vs.top());
            vs.pop();
        }
        l->loopNodes.insert(u);
        vs.pop();
        loops.push_back(l);
    }
}

void DependenceGraph::tarjan(int u, vector<Loop*>& loops) {
    DFN[u] = Low[u] = ++Index;
    vs.push(u);
    visit[u] = true;
    for(set<int>::iterator it = dpdGraph[u].begin(); it != dpdGraph[u].end(); it++) {
        if(!involved[*it]) continue;
        
        if(!visit[*it]) {
            tarjan(*it, loops);
            if(Low[u] > Low[*it]) Low[u] = Low[*it];
        }
        else {
            if(Low[u] > DFN[*it]) Low[u] = DFN[*it];
        }
    }
    if(Low[u] == DFN[u]) {
        if(vs.top() != u) {
            Loop* l = new Loop();
            while(vs.top() != u) {                  
                l->loopNodes.insert(vs.top());
                vs.pop();
            }
            l->loopNodes.insert(u);
            vs.pop();
            loops.push_back(l);
        }
        else {
            vs.pop();
        }
    }
}

void DependenceGraph::findESRules(Loop& loop) {   
    for(set<int>::iterator it = loop.loopNodes.begin(); it != loop.loopNodes.end();
            it++) {
        map<int, set<Rule*> >::iterator dit = dpdRules.find(*it);
        if(dit != dpdRules.end()) {
            set<Rule*>::iterator rit = dit->second.begin();
            for(; rit != dit->second.end(); rit++) {
                set<int>::iterator ait = (*rit)->body_lits.begin();
                for(; ait != (*rit)->body_lits.end(); ait++) {
                    if(*ait > 0) {
                        if(loop.loopNodes.find(*ait) != loop.loopNodes.end()) break;
                    }
                }
                if(ait == (*rit)->body_lits.end()) {
                    loop.ESHead.insert(*it);
                    loop.ESRules.insert(*rit);
                }
            }
        }
    }
}

bool DependenceGraph::EL_(Loop* l) {
    set<Loop> loop_map;
    vector<Loop*> sccs;
    
    findChildSCC(l->loopNodes, sccs, true);
    for(int i = 0; i < sccs.size(); i++) {
        Loop* _l = sccs[i];
        set<Rule*> r;
        if(loop_map.find(*_l) != loop_map.end()) {
            delete _l;
            continue;
        }
        loop_map.insert(*_l);
        findESRules(*_l);

        _l->cmp_el_support(l, r);
   
        if(r.size() == 0) return false;
        else {
            memset(involved, false, sizeof(bool) * (maxNode + 1));
            for(set<int>::iterator it1 = _l->loopNodes.begin(); it1 != _l->loopNodes.end(); it1++)
                involved[*it1] = true;
            for(set<Rule*>::iterator it2 = r.begin(); it2 != r.end(); it2++) 
                for(set<int>::iterator hit = (*it2)->head.begin(); hit != (*it2)->head.end(); hit++)
                    involved[*hit] = false;
                
            findSCC(sccs);  
        }
        delete _l;
    }
    
    return true;
}

bool DependenceGraph::Proper_(Loop* l, vector<Loop*> sccs) {
    set<Loop> loop_map;
    
    for(int i = 0; i < sccs.size(); i++) {
        Loop* _l = sccs[i];
        set<Rule*> r;
        if(loop_map.find(*_l) != loop_map.end()) {
            if(i >= _sccs.size()) delete _l;
            continue;
        }
        loop_map.insert(*_l);
        findESRules(*_l);
        int p = _l->cmp_el_support(l, r);
        if(l->cmp_include(_l) && r.size() == 0) return false;
        else {
            set<int> h, n, u;
            set_union(_l->loopNodes.begin(), _l->loopNodes.end(), l->loopNodes.begin(),
                    l->loopNodes.end(), inserter(u, u.begin()));
            _l->cmp_ES_head(u, h);
            l->cmp_ES_head(l->loopNodes, n);
            bool t = includes(n.begin(), n.end(), h.begin(), h.end());
            if(t && r.size() == 0 && p == 1) return false;
            else if(r.size() == 0 && p == 0 || !t) {
                findChildSCC(_l->loopNodes, sccs, true);
            }
            else {
                memset(involved, false, sizeof(bool) * (maxNode + 1));
                for(set<int>::iterator it1 = _l->loopNodes.begin(); it1 != _l->loopNodes.end(); it1++)
                    involved[*it1] = true;
                for(set<Rule*>::iterator it2 = r.begin(); it2 != r.end(); it2++) 
                    for(set<int>::iterator hit = (*it2)->head.begin(); hit != (*it2)->head.end(); hit++)
                        involved[*hit] = false;

                findSCC(sccs);                 
            }
        }
        if(i >= _sccs.size()) delete _l;
    }
    
    return true;
}