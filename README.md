# N-Body Simulator

A gravitational N-body simulator written in C99, using numerical integration to model the motion of celestial bodies under mutual gravitational attraction.

Built as a teaching project covering the full breadth of C programming — from pointers and dynamic memory to file I/O and numerical methods.

---

## Features

| Feature | Detail |
|---|---|
| Integrators | Euler (1st order), Velocity Verlet (2nd order), RK4 (4th order) |
| Configurations | Solar System (6 bodies), Chaotic 3-body figure-8 |
| Visualization | gnuplot via pipe — dark-background orbital plot |
| Output | `data/output.dat` — trajectory blocks readable by gnuplot |
| Tests | Unity framework — physics and integrator unit tests |
| Memory | Valgrind-clean — zero leaks by design |

---

## How It Works

1. **Input & Configuration (`src/io.c` + `cJSON`)**: Reads a JSON file (such as `config/solar_system.json` or `config/three_body_chaos.json`) containing simulation parameters (`dt`, total time, integrator) and the list of bodies (name, mass, initial position and velocity).
2. **Body Representation (`src/body.c`)**: Each celestial body is stored in a dynamically allocated `Body` struct holding mass, position, velocity, acceleration, and its trajectory trail.
3. **Force Calculation (`src/physics.c`)**: Computes pairwise gravitational forces ($O(N^2)$) using Newton's Law of Universal Gravitation, as well as total system energy (kinetic + potential) to track numerical accuracy.
4. **Numerical Integration (`src/integrator.c`)**: Advances positions and velocities over time using one of three selectable methods: **Euler** (1st order), **Velocity Verlet** (2nd order), or **RK4** (4th order).
5. **Main Loop & Output (`src/main.c`, `src/io.c`, `src/renderer.c`)**: Steps through time by `dt` until the total duration is reached, writes trajectories to `data/output.dat`, and pipes plot commands to `gnuplot` (unless `--no-plot` is passed).

---

## C Concepts Covered

This project is structured to demonstrate every major concept of a C programming curriculum:

| Concept | Where |
|---|---|
| Structs (`Body`, `SimConfig`) | `body.h`, `io.h` |
| Dynamic allocation (`malloc`, `calloc`, `free`) | `body.c`, `integrator.c` |
| Pointers & double indirection | `io.c` (`Body **bodies_out`) |
| Pointer-to-struct, `->` operator | `physics.c`, `renderer.c` |
| Enums as type-safe selectors | `integrator.h` (`IntegratorType`) |
| `switch-case` dispatch | `integrator.c` |
| Nested `for` loops (O(N²)) | `physics.c` |
| `while` loop with time control | `main.c` |
| File I/O (`fopen`/`fclose`/`fprintf`) | `io.c`, `renderer.c` |
| Pipe I/O (`popen`/`pclose`) | `renderer.c` |
| String functions (`strcmp`, `strncpy`, `snprintf`) | `integrator.c`, `io.c` |
| Format specifiers (`%s`, `%e`, `%d`, `%zu`) | Throughout |
| `#define` constants and macros | `physics.h`, `Makefile` |
| `#include` with header guards | All `.h` files |
| `goto` for cleanup on error | `integrator.c` (RK4) |
| `static` functions (file-scoped) | `main.c` |
| `const` correctness | `physics.c`, `renderer.c` |

---

## Project Structure

```
n-body-simulator/
├── README.md
├── Makefile
├── config/
│   ├── solar_system.json       # 6-body solar system (1 year)
│   └── three_body_chaos.json   # chaotic figure-8 (Chenciner & Montgomery)
├── src/
│   ├── main.c                  # simulation loop, argument parsing
│   ├── body.h / body.c         # Body struct, alloc/free, trail recording
│   ├── physics.h / physics.c   # gravitational force, energy calculations
│   ├── integrator.h / integrator.c  # Euler, Verlet, RK4
│   ├── io.h / io.c             # JSON loading (cJSON), trajectory output
│   ├── renderer.h / renderer.c # gnuplot pipe visualization
│   └── cJSON.h / cJSON.c       # single-header JSON parser (MIT)
├── tests/
│   ├── unity.h / unity.c       # Unity test framework (MIT)
│   ├── test_physics.c          # force symmetry, magnitude, energy
│   └── test_integrator.c       # accuracy, reversibility, string parsing
└── data/
    └── output.dat              # generated trajectory file
```

---

## Dependencies

| Tool | Purpose | Install |
|---|---|---|
| `gcc` | C99 compiler | `sudo apt install gcc` |
| `make` | Build system | `sudo apt install make` |
| `gnuplot` | Trajectory visualization | `sudo apt install gnuplot` |
| `valgrind` | Memory leak detection | `sudo apt install valgrind` |
| `cJSON` | JSON parser | Included in `src/` |
| `Unity` | Unit test framework | Included in `tests/` |

---

## Quick Start

```bash
# 1. Clone and enter project
git clone <repo-url>
cd n-body-simulator

# 2. Download cJSON (single-header, MIT license)
curl -o src/cJSON.h https://raw.githubusercontent.com/DaveGamble/cJSON/master/cJSON.h
curl -o src/cJSON.c https://raw.githubusercontent.com/DaveGamble/cJSON/master/cJSON.c

# 3. Download Unity test framework
curl -o tests/unity.h https://raw.githubusercontent.com/ThrowTheSwitch/Unity/master/src/unity.h
curl -o tests/unity.c https://raw.githubusercontent.com/ThrowTheSwitch/Unity/master/src/unity.c

# 4. Build
make

# 5. Run solar system simulation (opens gnuplot window)
./nbody config/solar_system.json

# 6. Run chaotic 3-body (no plot)
./nbody config/three_body_chaos.json --no-plot

# 7. Run unit tests
make test

# 8. Check for memory leaks
make valgrind
```

---

## Integrator Comparison

| Method | Order | Cost per step | Energy drift (1 year) | Recommended for |
|---|---|---|---|---|
| Euler | O(dt) | 1× force eval | ~5% | Quick tests only |
| Velocity Verlet | O(dt²) | 2× force evals | ~0.01% | Long simulations |
| RK4 | O(dt⁴) | 4× force evals | ~0.0001% | High-precision runs |

Velocity Verlet is the default and recommended integrator. It is **symplectic** — it preserves the phase-space volume of the Hamiltonian system, which is why energy is conserved much better than Euler despite the same computational cost.

---

## JSON Configuration

```json
{
  "simulation": {
    "dt": 3600,           /* time step in seconds (1 hour)     */
    "total_time": 31536000, /* simulation duration in seconds   */
    "integrator": "verlet", /* "euler" | "verlet" | "rk4"      */
    "plot_interval": 24   /* write positions every N steps      */
  },
  "bodies": [
    {
      "name": "Earth",
      "mass": 5.972e24,   /* kg                                 */
      "x": 1.496e11,      /* meters from origin                 */
      "y": 0.0,
      "vx": 0.0,          /* m/s                                */
      "vy": 29780.0
    }
  ]
}
```

Initial conditions sourced from [NASA JPL Horizons](https://ssd.jpl.nasa.gov/horizons).

---

## Physics Notes

**Gravitational force** between two bodies:

```
F = G * m1 * m2 / r²
```

**Softening parameter** ε prevents numerical singularities when bodies approach:

```
r_soft = sqrt(dx² + dy² + ε²)
```

**Energy conservation** is the primary diagnostic: total mechanical energy (kinetic + potential) should remain approximately constant. The simulator prints the relative drift at the end of each run.

---

## Academic Reference

Press, W.H., Teukolsky, S.A., Vetterling, W.T., Flannery, B.P.  
*Numerical Recipes in C: The Art of Scientific Computing* (2nd ed.)  
Cambridge University Press, 1992 — Chapters 16–17 (ODE integration).

Chenciner, A., Montgomery, R.  
*A Remarkable Periodic Solution of the Three-Body Problem*  
Annals of Mathematics, 152(3), 881–901 (2000).  
[doi:10.2307/2661357](https://doi.org/10.2307/2661357)

---

## License

MIT. See `LICENSE` for details.  
`cJSON` © Dave Gamble — MIT License.  
`Unity` © ThrowTheSwitch — MIT License.
