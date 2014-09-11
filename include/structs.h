#ifndef STRUCTS_H
#define	STRUCTS_H

#define MAX_ATOM_LENGTH 512
#define MAX_HEAD_LENGTH 10
#define MAX_ATOM_NUMBER 1024

#include <map>
#include <set>
#include <vector>
// Type definition
////////////////////////////////////////////////////////////////////////////////

enum BOOL {
    FALSE = 0,
    TRUE = 1
};

enum RULE_TYPE {
    FACT = 0,
    CONSTRANT,
    RULE
};

// Structures
typedef struct __rule {
    int head[MAX_HEAD_LENGTH];
    int body[MAX_ATOM_LENGTH];
    int body_length;
    int head_length;
    RULE_TYPE type;
}_rule;

typedef struct __literals {
    int atoms[MAX_ATOM_LENGTH];
    int length;    
}_literals;

typedef struct __head {
    int atoms[MAX_ATOM_LENGTH];
    int length;    
}_head;


#endif



