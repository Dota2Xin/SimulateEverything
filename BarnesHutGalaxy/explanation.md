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
 
<h2>Physical Gravity Simulation</h2>
To actually use this to simulate a physical system we need to use a force law. For our case we consider two galaxies colliding. We then have Newton's law which tells us:
$$\vec{F}=m\vec{a}$$
Letting $\vec{r}_i$ be the position of body $i$ we can write Newton's law of gravitation as:
$$\vec{F}_{ij}=-\frac{Gm_im_j}{|r_i-r_j|^2}\hat{r}_{ij}$$
Which states the force on $j$ from particle $i$ is equal to a magnitude term based on the distance between them times the negative of the vector pointing from $i$ to $j$ (i.e. it is attractive along the line connecting the two particles). Plugging this in to our second law we get:
$$\frac{d^2\vec{r}_i}{dt^2}=-\sum_{j\neq i}\frac{m_j}{|r_i-r_j|^2}\hat{r}_{ji}=-\sum_{j\neq i}\frac{m_i (\vec{r}_{j}-\vec{r}_i)}{|r_i-r_j|^3}$$
Where we note that $m_i$ divides out of every term present in the law. Now in practice we often have to make a correction to this equation. The correction comes from the idea that when we simulate we can have systems with collisions or without collisions where a "collision" is two bodies getting very close and then basically bouncing off of each other gravitationally, the problem is that say we have a timestep $\Delta t$ and we simulate a collision during a single time step then the particles will be accelerated by the collision for the entire timestep which would lead to unphysical results. Essentially when particles are too close problems emerge with our simulation. Now whether or not particles get close depends on the type of system we are interested in, for say the collision of two-galaxies it isn't important as we are interested in large scale structure so we can ignore binary star systems and note that both galaxies will be relatively sparse and so things won't get too dense. However, if we are interested in something like the long-time evolution of a single galaxy and the formation of binary star systems (where the stars can get incredibly close relative to the scale of a galaxy) then we would have a "collisionfull" system where collisions are very important. Mathematically the easiest way to fix collisions is through a combination of reduction in timestep and an introduction of a softening parameter $\epsilon$.  It get's used to adjust our force law as follows:
$$\frac{d^2\vec{r}_i}{dt^2}=-\sum_{j\neq i}\frac{m_i (\vec{r}_{j}-\vec{r}_i)}{(|r_i-r_j|^2+\epsilon^2)^{3/2}}$$
With this formulation if $\epsilon=0$ we reproduce our original equation and we let $\epsilon$ have units of distance so we can set it to some fraction of our characteristic inter-particle distance to ensure weak collisions. Because we will be simulating two galaxies colliding we'll neglect really close particles and let $\epsilon$ be fairly large (we'll discuss exact numbers later). With this we have everything we need to calculate the forces now we just want to calculate the dynamics. We will use leapfrog integration because it is better at conserving energy, the idea here is that even though our force is unphysical it is still approximately physical and so leapfrog will remain approximately correct and try to keep us on a nearby constant energy surface (symplectic manifold for ball knowers). The evolution of a time-step follows like this:
$$a_i=A(x_i)$$
$$v_{i+1/2}=v_{i}+\frac{1}{2}a_i\Delta t$$
$$x_{i+1}=x_i+v_{i+1/2}\Delta t$$
$$v_{i+1}=v_{i+1/2}+\frac{1}{2}a_{i+1}\Delta t$$
This is called the drift-kick-drift method, technically it requires two force calculations but because the last force calculation of this one can be re-used for the next one it still works out to be just one force calculation per time-step. With all of this setup we now have the tools we need to create the simulation we just need the initial conditions and setup for our galaxy collision problem.

<h1>Galaxy Physics</h1>
(data from https://arxiv.org/pdf/2603.22863 ?)

This will be a brief discussion of how we construct the initial problem of two galaxies colliding. It will be based on the galactic dynamics textbook by Binney and Tremaine and will mostly be galaxy specific stuff. We will start by discussing the observations and why we think galaxies have to have a certain structure. Then we will build up the equations that will allow us to impose this structure while assuming a lot of pre-existing physics. If you want to learn more about the origins of the equations assumed you would have to study classical mechanics (up to Hamiltonian dynamics and Liouville's theorem especially), orbital dynamics, fluid mechanics, and maybe statistical mechanics and the connection between classical dynamics and statistical distribution functions. I would say that for a general overview if you're not interested in studying lots of physics just understanding that we see certain things and that we can do physics to reproduce this structure and that the code will do this is sufficient.

<h2>Observations</h2>
In the night sky we see stars, as we developed better telescopes we eventually developed the ability to see objects that appear to cluster together a huge amount of stars, we call these objects galaxies. We will now describe some properties of galaxies, to understand how we get these numbers it should be noted that we often come up with models based on known physics like Newton's law of gravity and use it to predict observations from a galaxy such as how bright it is for a given size and what shape it takes. By fitting the free parameters in our theory such as galactic mass, distance, and size to our observations we can obtain estimates for these parameters as well as things like galactic mass distribution. We can also probe chemical composition based on absorption and emission spectra. Through a combination of coming up with a predictive model and comparing it to observed results we can continually refine our measurements of the universe. For now I will cite numbers without a model and we'll eventually build up our model over time.

Galaxies contain between $10^5$ and $10^{12}$ stars and the structure of these objects is described by stellar dynamics. Because the amount of particles is so great we often apply some larger scale statistical theory like statistical mechanics or fluid/plasma dynamics to describe galaxy (plasma because the same way plasmas have long range electromagnetic interactions galaxies have long range gravitational interactions). We'll start by discussing the galaxy you are probably most familiar with, the Milky Way. We'll state the following observations about the Milky Way (which we may also occasionally call the Galaxy):

1. The Galaxy has around $10^{11}$ stars which combine to $\approx 5\times10^{10} M_{\odot}$ where $M_{\odot}$ denotes the solar mass and is equal to the estimated mass of our own sun ($M_{\odot}=1.99\times 10^{30}kg$). Most stars travel on a nearly circular orbit in a thin disk with a radius $\sim10^{4}$ parsecs where $1$ parsec is defined as $1pc=3.086\times 10^{16}m$, we would say our galaxy is 10kpc (kiloparsec) across while it appears to be about 0.5kpc thick, our sun is in the middle of this thickness and is about 8kpc from the center. 
2. The disk also contains gas, mostly molecular and atomic hydrogen concentrated into clouds with a wide range of masses and sizes as well as into small solid particles (dust) which render the gas opaque at visible wavelengths over distances of several kpc. This mix of gas and dust forms the interstellar medium, its mass is only $\approx 10\%$ the mass of the stars so it does not contribute much to the dynamics but it is important for the chemistry of the galaxy as this stuff is what turns into stars and it is what dying stars eject most of their mass into allowing it to contain the heavy nuclei that form things like Earth. 
3. At the center of the disk is a black hole called Sagittarius A* (pronounced Sagittarius A star), you actually don't even need a model for this one as we have a picture of it (look it up!). It has an approximate mass of $\sim 4\times 10^6 M_{\odot}$. 
4. The largest component in size and mass is the dark halo which has a radius of 200kpc and a mass of about $10^{12}M_{\odot}$ (lot's of uncertainty in these estimates). The dark halo is made of dark matter which is what we use to explain observations from galaxies that are consistent from a large amount of mass that we can't see. In theory this could also be due to some other theory but the model of matter we can't see does work for matching the observations we make so it is predictive even if it is not correct. 

Now we'll look at the characteristic time scales here. A typical orbit happens at $200 km/s$ where we note that $1km/s$ is enough to travel $1pc$ in about $1Myr$ (megayear) which means that the time to complete an orbit at a radius of 8pc is about 250 Myr. The age of the galaxy is about 10Gyr (gigayears) which means that most objects have completed around 40 orbits and we can say the galaxy is probably in a steady state. This is just an approximation but it is a powerful one as it will allow us to write down equations describing the steady state with no thought needing to be given to the complex issue of formation. The geometry of a galaxy and our Sun within it are shown below:
![[im5.png]]
Two angles are used to define a galactic coordinate system. Now given that all we can do is look at light emitted from galaxies many of the things we measure will be based on brightness. We define surface brightness as the amount of light a galaxy emits per unit area of disk as a function of galactic position, from surveying many galaxies it seems that surface brightness obeys the following law:
$$I(R)=I_d\exp(R/R_d)$$
Where $R$ is the galactic radius and $R_d$ is the disk length scale. Now given the stars orbit the center we can also define a galactic speed curve $v_c(R)$ for the speed of stars at a given radius from the center. Our sun has $v_c\approx 220km/s$. By looking at how the brightness of disks falls off and using dynamical estimates we seem to observe that galactic density along the direction perpendicular to the galactic plane (the plane of the disk) falls off exponentially as:
$$\rho(R,z)=\rho(R,0)e^{-|z|/z_d(R)}$$
Where $z_d(R)$ is a characteristic thickness for the given radius. We call this the scale height. Our models seem to work better for our galaxy when we combine two populations, one with a smaller scale height around 300pc comprised mostly of new stars and one with a larger scale height around 1kpc comprised of old stars (you can guess star age based on chemical composition which we can get from spectra). 

Often we describe galaxies through averaging over some region of stars to get a rough density that we can use to describe it. For our purposes this is useful because we can only observe a subset of the stars in our galaxy because we can only look at stars in our own stellar neighborhood. We often discuss the mass-to-light ratio of these objects $M/L$. We also see that our galaxy has a bulge in the center which is a bit thicker and whose star population is different from the rest of the galaxy. In a given neighborhood like this bulge or our stellar neighborhood there can be bulk orbital motion and other local essentially random motion. The ratio of bulk motion to random motion is often described in terms of a faux "temperature" in analogy to gasses, where more ordered motion is cold and more random motion is hot. 

Through the observations of many galaxies four major types have been observed. These are elliptical galaxies, spiral galaxies, lenticular galaxies, and irregular galaxies. We will discuss them in that order and then discuss how these different types are related. 