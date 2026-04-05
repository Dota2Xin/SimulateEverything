<div style="display:none">
$
\newcommand{\La}{\mathcal{L}}
\newcommand{\Ha}{\mathcal{H}}
\newcommand{\Tau}{\mathrm{T}}
\newcommand{\lr}[1]{\left(#1\right)}
\newcommand{\partialD}[2]{\frac{\partial #1}{\partial #2}}
\newcommand{\fullD}[2]{\frac{d #1}{d #2}}
\newcommand{\thermoD}[3]{\left(\frac{\partial #1}{\partial #2}\right)_{#3}}
\newcommand{\lrb}[1]{\left\{ #1\right\}}
\newcommand{\dbar}{d\hspace*{-0.08em}\bar{}\hspace*{0.1em}}
\newcommand{\av}[1]{\langle #1 \rangle }
\newcommand{\Ga}{\mathcal{G}}
\newcommand{\Va}{\mathcal{V}}
\newcommand{\Ea}{\mathcal{E}}
\newcommand{\Na}{\mathcal{N}}
\newcommand{\Rn}{\mathbb{R}}
\newcommand{\Nn}{\mathbb{N}}
\newcommand{\Zn}{\mathbb{Z}}
\newcommand{\Qn}{\mathbb{Q}}
\newcommand{\ang}{ \overset{\circ}{\textrm{A}}}
\newcommand{\tm}[1]{\textrm{#1}}
$
</div>

<h1>Algorithm Explanation</h1>
This discussion is adapted from Many Body Tree Methods in Physics by Pfalzner and Gibbon (who got the idea from Barnes and Hut of course). The key of the Barnes-Hut algorithm is to use the multipole gravitational expression and to approximate long-range interactions as simple monopole interactions to take $O(N^2)$ force calculations and do them in $O(N\tm{log}(N))$ time. The idea is to take your set of masses and break them up into a tree with nodes getting related if they are sufficiently close to each other and each parent storing all the mass of it's children for long range interaction. 

We'll start by going over the construction of the data structure that we use to do our fast approximation of our interactions. This will then be used to create an algorithm for simulating the $N$-body problem for gravity.

<h2>Tree Construction</h2>
The idea is simple, we start with our simulation box and our goal is to break it up, if it's empty or there's just 1 body then there's no simulation so we ignore those cases. The algorithm is called an Octree algorithm because it works by breaking the space into eighths of the main cubic simulation cell. Basically we recursively break up our cube into eighths which each new cell having the question asked, how many particles are there in this cell? If the cell is empty then it is ignored, if it is 1 then it is stored as a leaf node in the tree structure. If there is more than 1 particle in the cell it is stored as a twig node in the tree structure and is further subdivided to create more children. In 2D this would look as follows:
![[Images/im1.png]]
In a more realistic 3D simulation we would have:
![[Images/im2.png]]
The flowchart of this computation is as follows:
![[Images/im3.png]]
Now we note that we have to scan over $N$ elements to do this and we do this for every layer of tree division of which we expect there to be $\log(N)$ of which means this tree construction is an $O(N\log(N))$ operation. While the tree is constructed each leaf node will have it's physical properties such as mass and position stored on it while each twig node will have a total mass of all children and a center of mass stored on it.  

<h2>Force Calculation</h2>
Now we turn to how we use this data structure to calculate forces efficiently. The idea is to divide our interactions into two types based on if they are long-range or short range, short-range interactions are calculated exactly while long-range interactions are calculated approximately. To do this we need a way of grouping particles based on the types of interactions between them. We'll use the standard "s/d" approach by Salmon and Warren (1994) which has some problems but is useful for a lot of cases. We start by iterating through each particle, for each particle we start this calculation at the root of the tree. We can give each node a size $s$ based on the side-length of the cube it encloses, this size gets compared with the distance to the particle $d$ and the ratio considered is:
$$x=\frac{s}{d}$$
The idea here is that $x$ denotes a measure of importance in terms of how important this part of the tree is to our current particle. The farther away it is the less important it is but the larger its size the more likely it is that there might be a particle in there that is closer and thus more important. The general idea is to look at two cases one where $x\leq \theta$ where $\theta$ is some simulation control parameter and the other where $x>\theta$. If $x$ is small enough then that node isn't important and we just use the monopole interaction to that tree node to get our force, if $x$ is larger than $\theta$ then we subdivide and repeat the calculation for the new boxes, if we reach a leaf node then we just calculate the force exactly. For each particle this only requires one tree traversal with some nodes presumably being cutoff, this tree-traversal is going to be $O(N\tm{log}(N))$ so long as we assume most of the nodes in our problem get cutoff at some point. For large simulations this is generally correct so you lose fairly little. This can actually be proven for something like uniform density but for a general problem it can mostly be seen from numerical experiments. The general force algorithm is laid out below:
![[im4.png]]
 