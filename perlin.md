## PerlinNoise2D

Invented by Ken Perlin (originally to generate realistic textures for the 1982 movie *Tron*), Perlin noise is a way to generate natural-looking randomness.

If we use a standard random generator for a 2D image, we get "white noise"—it looks like harsh TV static because every pixel is completely unrelated to its neighbor. Perlin noise fixes this by creating **smooth, continuous randomness**. Instead of static, it looks like clouds, rolling hills, or swirling smoke. If we sample two points very close to each other, their values will be very similar. It is the mathematical backbone of almost all procedural generation in games today, like generating terrain heights in *Minecraft*.

### Implementation Requirements

* A method `float sample(float x, float y)` (the coordinates need to be floats so we can sample points *between* the whole-number grid lines).
* Overload `operator()` for generating noise, meaning we can call our noise object just like a function (e.g., `myNoise(1.2f, 3.5f)`).
* Store an instance of our `Random2DCoordinateGenerator` inside this class to use for hashing the grid corners.

---

### Overloading the `operator()` (Functors)

By overloading the function call operator `operator()`, we tell the compiler to treat an instance of our class like a function call. This creates a **functor** (or function object).

```cpp
// Behind the scenes, the compiler translates myNoise(x, y) into this:
long long randomNumber = myNoise.operator()(x, y);

```

**Why bother with Functors?**
We use functors instead of standard functions (like `generateRandomNumber(x, y)`) because they offer distinct advantages:

* **Statefulness:** A normal function forgets everything once it finishes running. A functor is an object, so it can have member variables and "remember" its internal state—like a seed value or our random generator instance—across multiple calls.
* **Performance:** The compiler can usually inline the `operator()` code. This makes functors significantly faster than passing traditional function pointers.
* **Cleaner Syntax:** They provide the lightweight syntax of a simple function call.

---

### The Algorithm Step-by-Step

To calculate the noise value for a specific float coordinate (like $x = 1.2$, $y = 3.5$), the algorithm works in distinct mathematical steps:

#### 1. The Grid (The Graph Paper)

We imagine the 2D plane as a grid of whole numbers. A floating-point coordinate sits inside a single square cell on that paper. To determine the value at our specific point, we only need the four integer corners of the square it resides in. For $(1.2, 3.5)$, those corners are $(1,3)$, $(2,3)$, $(1,4)$, and $(2,4)$.

We extract the global corners and the local point coordinates like this:

```cpp
long long X = static_cast<long long>(std::floor(x));
long long Y = static_cast<long long>(std::floor(y));

IVector2<float> orig_point(x, y);

// The 4 integer corners of our grid cell
IVector2<long long> bottom_left(X, Y);
IVector2<long long> bottom_right(X + 1, Y);
IVector2<long long> upper_left(X, Y + 1);
IVector2<long long> upper_right(X + 1, Y + 1);

// Where our point sits inside the cell (values from 0.0 to 1.0)
IVector2<float> local_coord = orig_point - IVector2<float>(X, Y);

```

#### 2. The Angles (The Random Arrows)

For each of those four integer corners, we feed their coordinates into our `Random2DCoordinateGenerator`. Because the generator is deterministic, a specific grid corner will *always* have the exact same random hash.

We convert that random `long long` into an angle between $0$ and $2\pi$ radians, and then use trigonometry to create a normalized 2D gradient vector (a random arrow):

$$angle = (\text{random\_value} \bmod 360) \times \frac{\pi}{180.0}$$

$$gradient = IVector2<float>(\cos(angle), \sin(angle))$$

*Note: We must call the generator instance directly (`_generator`), rather than the type.*

```cpp
// 1. Get the random hash using our internal generator instance
long long random_val = _generator(bottom_left.x, bottom_left.y);

// 2. Turn it into an angle (using 3.14159265f for pi)
float bottom_left_angle = (random_val % 360) * 3.14159265f / 180.0f;

// 3. Turn the angle into a gradient vector
IVector2<float> bottom_left_gradient(std::cos(bottom_left_angle), std::sin(bottom_left_angle));

```

#### 3. Putting it Together (The Dot Product)

Next, we calculate four distance vectors pointing from each of the four corners directly to our local point. We then calculate the **dot product** of those distance vectors and our generated gradient vectors.

* If the distance vector aligns perfectly with the corner's random gradient arrow, the value goes up.
* If it points exactly opposite to the arrow, the value goes down.

```cpp
float bottom_left_dot = bottom_left_gradient.dot(first_distance_vector);
// Repeat for bottom_right, upper_left, and upper_right...

```

#### 4. Smooth Interpolation (Fade and Lerp)
If we just draw straight lines between our random values, the noise will look blocky and jagged (like a 90s video game). Ken Perlin established a **fade function** to smooth out the transition curve of our local coordinates:

$$fade(t) = t^3 \times (t \times (t \times 6 - 15) + 10)$$

We apply this fade function to our local coordinates to get our smooth blending weights, which we call **$u$** and **$v$**:
*   **$u = fade(local\_coord.x)$**
*   **$v = fade(local\_coord.y)$**

Once we have $u$ and $v$, we use Linear Interpolation (**Lerp**) to blend the dot product values. A standard Lerp function takes a weight ($t$) and uses it to blend between two values ($a$ and $b$):

$$lerp(t, a, b) = a + t \times (b - a)$$

**The final blending steps:**
We plug our $u$ and $v$ weights directly into the $t$ parameter of the Lerp function to combine our four dot products into a single final number:

1. Lerp the bottom two dot products together using $u$ for the weight:
   `float x1 = lerp(u, bottom_left_dot, bottom_right_dot);`
2. Lerp the top two dot products together using $u$ for the weight:
   `float x2 = lerp(u, upper_left_dot, upper_right_dot);`
3. Lerp those two resulting values together using $v$ for the weight to get the final smooth noise float:
   `float final_noise = lerp(v, x1, x2);`
