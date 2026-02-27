# Contributing to TaskScheduler

First off, thank you for considering contributing to this project! This is a low-level, high-performance task scheduler designed to understand and utilize WinAPI primitives.

To maintain the project's goals, please follow these guidelines.

## The "No STL" Rule
The core philosophy of this project is **zero dependency on the C++ Standard Template Library (STL)**. 
- **NO** `std::vector`, `std::queue`, `std::mutex`, `std::thread`, etc.
- **Why?** This project is built to showcase low-level memory management and WinAPI synchronization.
- **What to use instead:** Use the custom containers already in the repo (`Queue.h`, `PriorityQueue.h`) or implement your own raw-pointer-based structures.

## Coding Standards
1. **Memory Management:** Since we don't use smart pointers (`std::unique_ptr`), you are responsible for manual memory management. Ensure every `new` has a corresponding `delete`.
2. **Naming Convention:** - Classes/Structs: `PascalCase` (e.g., `TaskScheduler`)
   - Functions: `camelCase` (e.g., `enqueueTask`)
   - Variables: `camelCase` (e.g., `workerCount`)
3. **Synchronization:** Use WinAPI primitives (`CRITICAL_SECTION`, `CONDITION_VARIABLE`, `InterlockedXxx`) for thread safety.

## How to Contribute
1. **Pick an Issue:** Check the [Issues](https://github.com/IsxImattI/TaskScheduler/issues) tab. If you have a new idea, open an issue first to discuss it.
2. **Fork and Branch:** Create a branch for your fix/feature (`git checkout -b feature/amazing-feature`).
3. **Keep it Clean:** Small, focused Pull Requests are merged much faster than giant ones.
4. **Documentation:** If you add a feature, update the `README.md` or add comments to your header files.

## Testing
Before submitting a PR, ensure your changes:
- Compile without warnings using the `cl` compiler (MSVC).
- Don't cause deadlocks in the `Benchmark.h` suite.
- Don't introduce memory leaks.

---
By contributing, you agree that your code will be licensed under the project's **MIT License**.
