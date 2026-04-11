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
#define basePoolSize 585

typedef struct node node;
typedef struct particle particle;
typedef struct particleArray particleArray;

struct particleArray {
    particle* p;
    long size;
};

struct particle {
    double mass;
    double x;
    double y;
    double z;
};

struct node {
    particle p;
    node* children[8];
};
//everything here is 8 bytes so we have (4+8)*8=96 bytes per struct. This means an 8-depth tree could hypothetically take up 
//13GB of memory. Realistically it will stay quite a bit below this for the duration of the simulation. The lower bound is going
//to be 1GB as that's how much it takes to represent 10^7 particles. Do most testing on 10^4 or 10^5 particles probably. 

double xWidth;
double yWidth;
double zWidth;

void treeInit(double* widths) {
    xWidth=widths[0];
    yWidth=widths[1];
    zWidth=widths[2];
}

/*
the optimization to make is that the bottom pools should store just particles with no children because their children can be
easily inferred at runtime from their array index. 
Consider a particle at index i, i=\sum_{i=0}^d c_k 8^k, if c_d!=0 then it has no children as it's a leaf, on the other hand if
c_d=0 we look at the first c_k that isn't 0, 
*/
//particles is an array of every particle we use to construct the tree
node createTree(particle particles[], long particleCount, double domainSize) {
    double coordinates[3]={-domainSize/2.0, -domainSize/2.0, -domainSize/2.0};
    node root=handleTreeLayer(particles, particleCount, coordinates, domainSize, 1);
}

// want to somehow handle tree construction layer by layer efficiently.  
node handleTreeLayer(particle particles[], long particleCount, double coordinates[], double boxSize, char first) {

    //check leaf case
    if(particleCount==1) {
        particle curr=particles[0];
        node main ={.children={0,0,0,0,0,0,0,0}, .p=curr};
        free(particles);
        return main;
    }

    long baseSize=nextTwoPower((particleCount/8));
    long childLengths[8]={0,0,0,0,0,0,0,0};
    particleArray childParticles[8];

    double totalMass=0;
    double centerOfMassX=0;
    double centerOfMassY=0;
    double centerOfMassZ=0;

    double half=boxSize/2.0;
    for (int i=0; i<particleCount; i++) {
        char child=0;
        double x=particles[i].x;
        double y=particles[i].y;
        double z=particles[i].z;
        double mass=particles[i].mass;

        totalMass+=mass;
        centerOfMassX+=mass*x;
        centerOfMassY+=mass*y;
        centerOfMassZ+=mass*z;

        if (x-coordinates[0]>half) {
            child+=1;
        }
        if (y-coordinates[1]>half) {
            child+=2;
        }
        if (z-coordinates[2]>half) {
            child+=4;
        }

        if (childLengths[child]==0) {
            childParticles[child]=create(baseSize);
            childLengths[child]=1;
            append(childParticles[child], particles[i], 0);
        } else {
            append(childParticles[child], particles[i], childLengths[child]);
            childLengths[child]+=1;
        }
    }
    if(first!=1) {
        free(particles);
    }
    particle mainP={.x=centerOfMassX/totalMass, .y=centerOfMassY/totalMass, .z=centerOfMassZ/totalMass, .mass=totalMass};
    node main ={.children={0,0,0,0,0,0,0,0}, .p=mainP};
    for(int i=0; i<8; i++) {
        if (childLengths[i]!=0) {
            double childCoord[]={(i&1)*half,((i>>1)&1)*half,((i>>2)&1)*half};
            node temp=handleTreeLayer(childParticles[i].p, childLengths[i],childCoord, half, 0);
            main.children[i]=&temp;
        } 
    }

    return main;
}

long nextTwoPower(long input) {
    long v=input;
    v--;
    v|= v>>1;
    v|= v>>2;
    v|= v>>4;
    v|= v>>8;
    v|= v>>16;
    v|= v>>32;
    v++;
    return v;
}

node* makePoolNode(int size) {
    //bit shift is 8^n, multiplication by 3 gives shift by power of 8 rather than 2
    long poolSize=(1-(1<<(size*3)))/(1-8);
    node* pool=calloc(poolSize, sizeof(node));
    return pool;
}

//might want more than this3
particle* makePoolParticle() {
    particle* pool=calloc(basePoolSize, sizeof(particle));
    return pool;
}

long getChildParticle(long index, int child) {
    return 8*index+child;
}


//////DYNAMIC ARRAY FOR PARTICLES METHOD/////////

particleArray create(long length) {
    particle* base=calloc(length, sizeof(particle));
    particleArray main;
    main.p=&base;
    main.size=length;

    return main;
}

void append(particleArray currArr, particle add, long lengthCurr) {
    long lengthTrue=currArr.size;
    particle* curr= currArr.p;
    if(lengthCurr!=lengthTrue) {
        curr[lengthCurr]=add;
    } else {
        particle* newArr=calloc(2*lengthTrue, sizeof(particle));
        for(int i=0; i<lengthCurr; i++) {
            newArr[i]=curr[i];
        }
        free(curr);
        newArr[lengthCurr]=add;
        currArr.p=newArr;
        currArr.size=2*lengthTrue;
        return;
    }
}