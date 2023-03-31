# C++ Carworld

A 3D rendered traffic simulation where cars gradually learn how to navigate the streets using a genetic algorithm on neural networks

## Building and running the project
```sh
# Make build dir if it doesn't exist
mkdir -p build
cd build

# [Build type] can be 'Release' or 'Debug'
cmake -DCMAKE_BUILD_TYPE=[Build type] ..

make

# Run build/carworld from root of repository
cd ..
build/carworld
```

## Interface inside the program
The top left of the screen shows keys you can press to perform learning.
The options change based on weather or not a generation is active.

#### No active generation
The number of the next generation is show, as well as ways of starting it.
Pressing `R` starts a new generation, with one simulation running visibly, in real time.

Pressing `T` toggles automatic starting of new generations.

Pressing `Ctrl`+`O` lets you open an existing gene pool, while `Ctrl`+`S` lets you save.
When loading a gene pool, the generation number of that pool is restored,
and all config files leading up to that generation are loaded.

#### With an active generation running
While a generation is running, the number of unfinished simulations left in the generation,
is printed in the top left corner.
Once all simulations in the active generation are done,
you can press `Enter` to finish the generation.

If you don't want to continue to the next generation, you can at any point press `Backspace` to abort the generation.

#### With an active generation with a realtime simulation
When rendering a realtime simulation, use the mouse and `W`, `A`, `S`, `D` to look around.
Use `Space` and `E` to go up and down, respectively.
Holding `Shift` makes you move faster (`Shift`+`Up` and `Down` to scale speed).

Read the top left of the screen to see extra toggles, for visualization.

## Configuring learning
Before generation `N` starts, the file `res/config/<N>.txt` is read, line by line,
parsing options. The possible options are as follows:

### Brain size
This parameter can only be set once, and has to be set in `0.txt`.

Set the number of hidden layers, and place the size of each hidden layer on the next line.
```
brainLayers 3
40 20 20
```

### Seed
A number used to deterministically initialize several random number generators:
 - The RNG for generating completely new brains
 - `seed + generation_number` is used to evolve the gene pool after each generation
 - The same combo is given to each simulation.
   - Here it is used to slightly randomize the spawn position and speed of cars 

```
seed 123
```

### World
Loads in a world from a file.
Will automatically **clear all routes and spawn times**.
```
world res/maps/simple_intersection.txt
```

### Define Routes
Clears the current set of routes, and defines new ones.
Also clears the list of car spawn times.

Takes the number of routes as a parameter, e.g. 12.
The next 12 lines should contain two integers, which are start and end nodes on the map.
The shortest path between these nodes are added as a route.
```
defineRoutes 12
106 95
106 118
 ... 10 more lines ...
```
**Tip:** Use the "closest node" feature when visualizing a simulation, to get node indecies.

**Tip:** Loading a world clears routes, so write this configuration below any `world` options.

### Manually provide car spawn times
**Tip:** You probably want to use random route picker instead.

Clears the list of spawn times, and fills it with `N` new values.
A spawn time is a pair of `frame number` and `route`.
Do this **after** defining the routes.
```
spawnTimes 28
0 0
100 1
200 2
 ... 25 more lines ...
```

### Automatically pick spawn times and routes for cars
Provide the following:
```
pickRandomRoutes 200 100 300 350 2000
```
This line means that the next generation,
and every future generation that is a multiple of `200`,
a new set of random spawn times is picked.
A spawn time consists of a specific frame number, and the route the spawned car will follow.
This option must be typed after `defineRoutes`.

Only frames that are a multiple of `100` can have a car spawn.

On such a frame, a random route is chosen. If it is ok to spawn
a car at the route's start node, it is added to the list of car spawn times.

To prevent other cars from spawning at the same start node,
a timed lock is applied to the node, lasting between `300` and `350` frames.

Attempting to spawn new cars every `100` frames continues until frame `2000`.

### The rest of the config options
```
# Sets the number of brains in the entire gene pool, must be at least 1
poolSize 100
```

```
# The N best brains of a generation are the elite, and are kept as-is in the next gen
survivorsPerGeneration 10
```

```
# Maximum run time of each simulation 
framesPerSimulation 3000
```

```
# How likely any value in a brain is to be modified, during mutation
mutationChance 0.05
```

```
# The standard deviation used to randomly displace new cars sideways
spawnRandomness 0.1
```

## Configuring the score function
Inside the file `src/carConfig.h`, there are a bunch of parameters,
that define how much score and penalty is given to each car for different
things. The comments there describe each value.

Some fields are remnants from previous attempts at tuning,
such as `SCORE_INITIAL_SCORE`.
