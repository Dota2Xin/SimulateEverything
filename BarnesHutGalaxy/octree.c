#include <stdlib.h>
#include <math.h>
#include <stdint.h>

/*
This is going to contain the octree, we'll construc the octree in terms of some "Pools" where a pool is some depth down the tree
the idea is that if we get down to say depth 5 in the tree we can allocate everything below depth 5 easily in just a single pool
and everything above depth 5 can be stored in a static tree that isn't really that big  (8^5=32768), note that 5 is just a typical
placeholder depth we could test other ones, we'll define a global for this setup that controls that. (realistically 4-4 will prob
be the split)

What this means is the octree is essentially stored as a few big chunks of memory (a few kB per Chunk) and then we use it to 
calculate forces. This force calculation will also take place on this file. 
*/

#define startDepth 4
#define leafDepth 4

typedef struct node node;
typedef struct particle particle;

struct particle {
    double mass;
    double x1;
    double x2;
    double x3;
};

struct node {
    particle p;
    node* children[8];
};
//everything here is 8 bytes so we have (4+8)*8=96 bytes per struct. This means an 8-depth tree could hypothetically take up 
//13GB of memory. Realistically it will stay quite a bit below this for the duration of the simulation. The lower bound is going
//to be 1GB as that's how much it takes to represent 10^7 particles. Do most testing on 10^4 or 10^5 particles probably. 

node* root;
double x1Width;
double x2Width;
double x3Width;

void treeInit(double* widths) {
    x1Width=widths[0];
    x2Width=widths[1];
    x3Width=widths[2];
}

/*
the optimization to make is that the bottom pools should store just particles with no children because their children can be
easily inferred at runtime from their array index. 
Consider a particle at index i, i=\sum_{i=0}^d c_k 8^k, if c_d!=0 then it has no children as it's a leaf, on the other hand if
c_d=0 we look at the first c_k that isn't 0, 
*/

//particles is an array of every particle we use to construct the tree
node* createTree(node* rootPass,particle* particles, long particleCount) {
    int done=0;
    for(int i=0; i<8; i++) {
        root->children[i]=makePoolNode(startDepth);
    }
    while(done==0) {
        for(int i=1; i<=particleCount; i++) {

        }
    }
    return root;
}
// want to somehow handle tree construction layer by layer efficiently.  
node* handleTreeLayer(node* parent,particle* subParticles) {
    return root;
}

node* makePoolNode(int size) {
    //bit shift is 8^n, multiplication by 3 gives shift by power of 8 rather than 2
    long poolSize=(1-(1<<(size*3)))/(1-8);
    node* pool=calloc(poolSize, sizeof(node));
    return pool;
}

//assume it to be depth 4
particle* makePoolParticle() {
    long poolSize=585;
    particle* pool=calloc(poolSize, sizeof(particle));
    return pool;
}

long getChildParticle(long index, int child) {
    return 8*index+child;
}