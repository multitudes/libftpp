# Mathematics

## IVector2 & IVector3

For this project, I implemented templated 2D and 3D vector classes (`IVector2` and `IVector3`). They heavily utilize operator overloading to make the syntax natural, and include standard mathematical vector operations like normalization, dot product, and cross product.

## Random2DCoordinateGenerator

I needed a pseudo-random generator where the combination of a Seed and Coordinates always returns the same result. Change the seed or the coordinates, and the result changes.

*True* randomness is completely unpredictable (like measuring radioactive decay). But a computer algorithm is purely mathematical, so it can only simulate randomness. Because it is just math, this generator is **deterministic**: if you feed it the exact same inputs (seed + coordinates), it will give you the exact same output every single time. It just *looks* random.

### A Stateless Spatial Hash Function

Given a specific 2D coordinate `(x, y)` and a `_seed`, this function predictably returns the exact same pseudo-random number. This is the backbone of procedural generation (like terrain in *Minecraft*)—you need the world to look random, but remain perfectly consistent every time a player visits the same spot.

Here is how the algorithm works under the hood.

First, we map the 2D coordinates and the seed into a single 64-bit integer:

```cpp
const long long PRIME_X = 668265263LL;
// ...
long long hash = (x * PRIME_X) ^ (y * PRIME_Y) ^ (_seed * PRIME_SEED);

```

Multiplying the inputs by large prime numbers ensures the bits wrap around the 64-bit limit in highly irregular ways, breaking up linear patterns. Without this, adjacent coordinates like `(1, 1)` and `(2, 2)` might produce visible diagonal stripes in the generated terrain. The XOR (`^`) operator then merges the `x`, `y`, and `seed` values without losing entropy, creating our starting `hash`.

Next, we run it through the `MurmurHash3` Finalizer:

```cpp
hash ^= hash >> 33;
hash *= 0xff51afd7ed558ccdLL; 
hash ^= hash >> 33;
hash *= 0xc4ceb9fe1a85ec53LL;
hash ^= hash >> 33;

```

If we stopped at the first step, adjacent coordinates (like `x=1` and `x=2`) would still share too many similar bits. This second half fixes that by running the initial hash through an **avalanche function**.

These specific bit-shifts and hex constants (`0xff51afd7ed558ccdLL` and `0xc4ceb9fe1a85ec53LL`) aren't just random—they are famous magic numbers. They make up the 64-bit finalization mix (`fmix64`) from **MurmurHash3** (created by Austin Appleby) and are also used in the **SplitMix64** random number generator.

**The Avalanche Effect:** This specific sequence guarantees that flipping just **one single bit** in the input (e.g., moving from coordinate `x=100` to `x=101`) results in a ~50% probability of *every single bit* in the output flipping.

Here is a quick breakdown of why this approach is highly effective for the project:

| Feature | Description |
| --- | --- |
| **Thread-Safe** | Because it relies only on its inputs, multiple CPU threads can generate chunks of a map simultaneously without needing any locks. |
| **Speed** | Bitwise shifts (`>>`), XORs (`^`), and multiplications are executed extremely fast by modern CPUs. |
| **Artifacts** | Simple coordinate hashing can sometimes exhibit minor axial biases compared to more complex gradient noise (like Perlin or Simplex), but the Murmur3 finalizer cleans up the vast majority of it. |
