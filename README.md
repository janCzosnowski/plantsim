# PlantSim

![PlantSim Icon](assets/gfx/icon.png)

**PlantSim** is a cellular automata simulation modeling plant genomes, cell reproduction, mutation, and evolution. Plants grow and adapt in a virtual environment with real-time visualization.

---

# Table of Contents

* [Features](#features)
* [Project Structure](#project-structure)
* [Dependencies](#dependencies)
* [Prebuilt Binaries](#prebuilt-binaries)
* [Build Instructions](#build-instructions)
* [Running the Simulation](#running-the-simulation)
* [Running with Nix](#running-with-nix)
* [Controls](#controls)
* [Genome Basics](#genome-basics)
* [Plant Mechanics](#plant-mechanics)
* [Contributing](#contributing)
* [License](#license)

---
---

## Features

* Cellular Automata Simulation – Cells interact based on genome rules.
* Genetic Mutation & Evolution – Plants reproduce and mutate over generations.
* Real-Time Visualization – Watch growth and interactions live.
* Extensible & Modular – Easily add new behaviors, genome types, or GUI features.

---

## Project Structure

```
PlantSim/
│
├─ assets/            # Graphics and others
│  └─ gfx/
├─ build/             # Build artifacts
│  └─ linux/
│  └─ windows/
├─ dist/              # Output binaries
│  └─ linux/
│  └─ windows/
├─ libs/              # Local libraries
├─ scripts/           # Build scripts
├─ src/               # Source code
├─ tools/             # Utilities and helper binaries
├─ Dockerfile.ci      # CI build setup
```

---
## Prebuilt Binaries

Prebuilt binaries are available in the Releases section of the repository.

Each release includes:

- **Linux (x86_64)** – `PlantSim-x86_64.AppImage`
- **Windows (x86_64)** – `plantsim.exe` with required assets

These builds are automatically generated using the CI Docker pipeline and are ready to run without requiring local compilation.

To download:

1. Navigate to the repository on GitHub.
2. Click on the **Releases** tab.
3. Download the appropriate archive or binary for your platform.

If you encounter issues with a prebuilt binary, consider building from source using the instructions below.

---
## Dependencies

| Library                                       | Purpose                               |
| --------------------------------------------- | --------------------------------------|
| [raylib](https://www.raylib.com/)             | Graphics, window, input handling      |
| [ImGui](https://github.com/ocornut/imgui)     | GUI overlay rendering                 |
| [rlImGui](https://github.com/raylib-extras/rlImGui.git) | Raylib + ImGui integration            |
| [FFmpeg](https://ffmpeg.org/)                 | (LINUX ONLY): video asset processing  |

All libraries are automatically cloned and built by the scripts.


---
## Build Instructions

This project can be built for **Linux (x86_64)** and **Windows (x86_64)**.
You may build locally on your machine or use Docker for fully reproducible CI builds.

---

### Linux Build (x86_64)

#### System Requirements

Tested on Ubuntu 22.04+ (or compatible Debian-based distributions).

#### Install Dependencies

```
sudo apt update
sudo apt install -y \
  build-essential \
  cmake \
  git \
  pkg-config \
  curl \
  libgl1-mesa-dev \
  libx11-dev \
  libxrandr-dev \
  libxinerama-dev \
  libxcursor-dev \
  libxi-dev \
  libdrm-dev \
  zlib1g-dev \
  liblzma-dev \
  nasm \
  file
```

#### Build

```
git clone https://github.com/AlikornSause/plantsim
cd plantsim
./scripts/build-linux.sh
```

#### Output

```
dist/linux/PlantSim-x86_64.AppImage
```

The resulting **AppImage** is a self-contained executable and should run on most modern Linux distributions without requiring additional system libraries.

---

### Windows Build (x86_64 via MinGW-w64)

Cross-compilation from Linux is supported using MinGW-w64.

#### Install Dependencies (Ubuntu/Debian)

```
sudo apt update
sudo apt install -y \
  g++-mingw-w64-x86-64 \
  mingw-w64-tools \
  build-essential \
  cmake \
  git
```

#### Build

```
git clone https://github.com/AlikornSause/plantsim
cd plantsim
./scripts/build-windows.sh
```

#### Output

```
dist/windows/plantsim.exe
dist/windows/assets/
```

The generated directory contains:

* `plantsim.exe`
* All required runtime assets

---

### Docker / CI Build (Recommended)

To ensure consistent, reproducible builds across environments, use the provided CI Docker configuration.

#### Build Docker Image

```
docker build -t plantsim-ci -f Dockerfile.ci .
```

#### Run CI Build

```
docker run --rm -v "$PWD:/workspace" plantsim-ci bash scripts/ci-build.sh
```

This process will:

* Build the Linux AppImage
* Build the Windows executable
* Place outputs in:

  * `dist/linux/`
  * `dist/windows/`

---

## Running the Simulation

* **Linux:**

```
cd dist/linux
./PlantSim-x86_64.AppImage
```

* **Windows:**

```
cd dist/windows
plantsim.exe
```
---

## Running with Nix
### Building
```
  nix build github:AlikornSause/plantsim
```
### Running
```
  nix run github:AlikornSause/plantsim
```

### Adding to environment
  **If you use flakes you can add this repository to your inputs, and then use the package in your environment.systemPackages like this:**
  ```nix
    {
      inputs = {
        ...
        plantsim.url = "github:AlikornSause/plantsim";
      };
      outputs = {nixpkgs, plantsim, ...}: {
        ...
        nixosConfigurations.host = nixpkgs.lib.nixosSystem {
          ...
          environment.systemPackages = [
            ...
            plantsim.packages.${nixpkgs.system}.default
          ];
        };
      };
    }
  ```
  **Then you can rebuild your system**
  ```bash
    sudo nixos-rebuild switch --flake /path/to/flake/directory#host
  ```


## Controls

**Mouse:**

* Right-click: select plant/seed
* Left-click: place seed
* Left-click (Prober mode): inspect cell
* Mouse wheel: zoom
* Middle mouse drag: pan camera

**Keyboard:**

* Space: pause/resume simulation
* S: toggle settings panel
* G: toggle genome panel
* I: toggle info panel
* P: toggle cell prober
* R: toggle rendering
* L: place random-genome seed at cursor
* F: step one frame
* 1: toggle solar overlay
* 2: toggle plant energy overlay

---

## Genome Basics

* Each plant has a **genome**: a list of growth rules.
* **ConditionalGenomeRule** – rules for left, right, up, down.
* **DirectionRule** – defines which cell type to grow and under which condition.
* **ConditionalValue** – condition to check (height, cell count, or none).

### Growth Logic

1. Check each direction rule for a cell.
2. Evaluate the condition (`HEIGHT`, `CELLCOUNT`, or `NONE`).
3. Grow the corresponding cell type.

### Random Genome

* `generateRandomGenome(length)` creates random growth rules.
* Example: grow type 2 left if height > 3, else type 1.

```cpp
DirectionRule left = { 2, 1, {HEIGHT_GREATER, 3} };
```

---

## Plant Mechanics

* Starts with one cell at `(x, y)`.
* Uses **energy** to grow cells.
* Each update:

  1. Collect sunlight.
  2. Apply genome rules to grow cells.
  3. Decrease life steps; die if depleted.

### Cell Growth

* `tryGrow(x, y, type)` checks:

  * Is position free?
  * Enough energy?
* Adds the cell if valid.

### Example Flow

1. Plant has 3 cells.
2. Genome rule: grow left if height > 2, else type 1.
3. Condition is checked; new cell added if possible.
4. Repeat until max cells or energy runs out.

---

## Contributing

1. Fork the repository.
2. Make changes.
3. Test locally using the build scripts.
4. Submit a pull request.

---

## License

PlantSim is licensed under the GNU General Public License v3.0 (GPLv3).

This means:
- You may use, modify, and distribute this software.
- If you distribute modified versions or binaries, you must also provide the source code under the same license.
- No additional restrictions may be added.

See [LICENSE](LICENSE) for the full license text.
