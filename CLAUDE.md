# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Project Overview

Hubert is a header-only C++17 computational geometry library for 3D space. It provides geometry types (points, vectors, lines, planes, rays, segments, triangles, matrices) with comprehensive floating-point safety, degeneracy detection, and intersection algorithms. All types are templated on float/double.

Licensed under MIT.

## Build & Test

Hubert itself is header-only — no build step. Tests use Catch2:

```bash
cd test/hubertTests
mkdir build && cd build
cmake ..
cmake --build . --config Release
./hubertTests   # or .\Release\hubertTests.exe on Windows
```

CI: GitHub Actions (`.github/workflows/hubertTests.yml`) runs tests on Ubuntu (GCC, Clang) and Windows (MSVC).

## Library (`include/hubert.hpp`)

Single header file (~2500 lines) containing all types and algorithms.

### Geometry Types

All inherit from `HubertBase` which tracks validity/degeneracy via bit flags (`cInvalid`, `cDegenerate`, `cSubnormalData`).

| Type | Description |
|------|-------------|
| `Point3<T>` | 3D point (x, y, z) |
| `Vector3<T>` | 3D vector with cached magnitude |
| `UnitVector3<T>` | Normalized vector (magnitude = 1.0) |
| `Matrix3<T>` | 3x3 transformation matrix |
| `MatrixRotation3<T>` | Orthonormal rotation matrix (extends Matrix3) |
| `Line3<T>` | Infinite line through two points |
| `Plane<T>` | Plane defined by base point and normal |
| `Ray3<T>` | Ray from point in direction |
| `Segment3<T>` | Line segment between two points |
| `Triangle3<T>` | Triangle defined by three vertices |

### Key Algorithms

- **Floating-point utilities**: `isEqual()`, `isEqualScaled()`, `isGreaterOrEqual()`, `isLessOrEqual()` — epsilon-aware comparisons
- **Validity**: `isValid()`, `isDegenerate()`, `isSubnormal()` for any geometry entity
- **Vector math**: `dotProduct()`, `crossProduct()`, `magnitude()`, `multiply()`
- **Distances**: `distance(point, point)`, `distance(point, plane)`, `closestPoint(line, point)`, `closestPoint(plane, point)`
- **Triangle**: `area()`, `centroid()`, `unitNormal()`
- **Matrix**: `transpose()`, `multiply(m, m)`, `multiply(v, m)`
- **Intersections**: All return `ResultCode` enum (`eOk`, `eDegenerate`, `eCoplanar`, `eParallel`, `eNoIntersection`, `eOverflow`)
  - `intersect(plane, line/ray/segment)` — line-like vs plane
  - `intersect(triangle, line/ray/segment)` — Moller algorithm
  - `intersect(triangle, plane)`, `intersect(triangle, triangle)` — Moller triangle-triangle

### Construction Helpers

Free functions: `makeVector3()`, `makeUnitVector3()`, `makeLine3()`, `makePlane()`, etc.

## Code Conventions

- Header-only, all implementation inline
- `template <typename T>` throughout (float/double)
- Private members prefixed with `_` (e.g., `_x`, `_y`, `_z`)
- Free functions for operations (not member methods) — keeps types minimal
- No exceptions — error reporting via `ResultCode` enum and invalid/degenerate flags
- Infinity used as invalid value marker
- `std::hypot()` for accurate magnitude computation
- Relative epsilon for normal ranges, absolute for near-zero values

## Test Structure (`test/hubertTests/`)

- **Framework**: Catch2 (amalgamated, embedded in repo)
- **File**: `hubertTests.cpp` (~3200 lines)
- Uses `TEMPLATE_TEST_CASE` to test across both float and double
- `HubertTestSetup` struct provides test fixtures: invalid numbers (NaN, infinity), subnormals, extreme values
- Coverage includes all types, operations, edge cases, and intersection algorithms

## Consumed By

- `tbops3dmath` — wraps hubert types as TheBrute custom types and builds geometry operators on top
- `tbopssdk` — includes hubert (transitive dependency for operator libs)
