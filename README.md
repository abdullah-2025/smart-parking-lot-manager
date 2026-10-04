# Smart Parking Lot Manager 🚗

[![Language: C++11](https://img.shields.io/badge/Language-C%2B%2B11-blue.svg)](https://isocpp.org/)
[![Course: DSA](https://img.shields.io/badge/Course-Data%20Structures%20%26%20Algorithms-orange.svg)](#)
[![License: MIT](https://img.shields.io/badge/License-MIT-green.svg)](LICENSE)

A high-performance, console-based **Smart Parking Lot Management System** implemented in **C++11**. Engineered as a Data Structures & Algorithms assignment, it models a real-world multi-tier parking facility with strict memory and architectural constraints—using **fixed-size arrays only** without high-level STL containers or algorithms.

---

## 📌 Project Information

- **Author**: M Abdullah Khan
- **Registration No**: 2312280
- **Course**: Data Structures & Algorithms (DSA)
- **Language & Standard**: C++11 (Standard ISO C++)
- **Platform Support**: macOS, Linux, Windows (MinGW / Dev-C++ / Code::Blocks / Visual Studio)

---

## 🎯 Architectural Constraints

To adhere strictly to algorithmic fundamentals:
- **No Dynamic Containers**: No `<vector>`, `<string>`, `<list>`, or `<algorithm>`.
- **Allowed Headers Only**: `<iostream>`, `<iomanip>`, `<cstring>`, `<cctype>`, `<limits>`.
- **Fixed-Size Data Structures**:
  - Main Lot: `Vehicle lot[60]` (Max capacity: 60 vehicles)
  - Free Slot Tracker: `int freeSlots[60]` (Always maintained in ascending sorted order)
  - Level 2 Lot: `Vehicle level2[60]` (For multi-level merging)
  - Exit History Log: `ExitRecord history[200]` (For analytics and auditing)

---

## ✨ Features & Functionality

### 1. 🅿️ Vehicle Park Management (Task 1)
- **Append to End**: Standard parking at the next free slot ($O(1)$).
- **VIP / Reserved Priority**: Inserts VIP vehicles at the **front (position 1)** via right shifts ($O(n)$).
- **Custom Position Insertion**: Allows parking at any valid user-specified index ($O(n)$).
- **Sorted Insertion**: Inserts vehicle while maintaining lexicographical plate order ($O(n)$).
- **Input Validation**: Prevents duplicate license plates and rejects entries when lot reaches full capacity ($60/60$).

### 2. 🧾 Exit & Billing System (Tasks 2 & 5)
- **Exit by Position**: Removes vehicle at specified array index with left shift compaction ($O(n)$).
- **Exit by Plate**: Linear search followed by removal and compaction ($O(n)$).
- **Closing Sweep**: Single $O(n)$ compaction pass that clears all vehicles that entered before a given cutoff time.
- **Automated Billing Engine**:
  - Computes duration rounded up to the nearest whole hour (minimum 1 hour).
  - Rates:
    - 🏍️ **Bike**: \$10 / hour
    - 🚗 **Car**: \$20 / hour
    - 🚚 **Truck**: \$40 / hour
  - Generates formatted billing receipts and tracks cumulative revenue.
- **Fee Preview**: Check estimated fees for currently parked vehicles without removing them.

### 3. 🔍 Search Suite (Task 3)
- **Linear Search**: Locates any vehicle by license plate with exact comparison metrics.
- **Binary Search ($O(\log n)$)**:
  - Enforces sorted order pre-condition checks.
  - Includes an interactive demonstration mode showing search failures when executed on unsorted arrays.
- **Filter by Type**: Displays all parked Bikes, Cars, or Trucks with match counts.
- **Longest-Parked Detection**: Identifies vehicles with earliest arrival times, computing elapsed durations and handling ties gracefully.

### 4. 📊 Sorting Algorithms (Task 4)
Each sorting routine displays real-time execution metrics:
- **Insertion Sort** (by License Plate A-Z):
  - Tracks total comparisons and element shifts.
- **Selection Sort** (by Entry Time earliest-to-latest):
  - Tracks total comparisons and swaps.
- **Stable Bubble Sort** (Primary key: Vehicle Type, Secondary key: Entry Time):
  - Implements early-exit flag optimization when array becomes sorted.
  - Automatically verifies stability by capturing snapshots and validating relative ordering of equal keys.

### 5. 🌟 Bonus Features
- **Bonus 1 — Lowest Free Slot Allocator**: Always assigns the lowest available physical slot number (e.g., Slot 1 before Slot 5) and returns released slots into sorted order ($O(k)$).
- **Bonus 2 — Multi-Level Two-Pointer Merge**: Merges Level 1 and Level 2 parking lots sorted by plate using the optimal two-pointer technique ($O(n_1 + n_2)$), handling and reporting cross-level duplicate plates.
- **Bonus 3 — Peak Occupancy Analytics**: Generates a 24-hour time-window histogram (ASCII bar chart) combining active parked cars and historical exit logs to identify peak operational hours.
- **Testing Helper (Option 8)**: Rapidly auto-populates the lot to full capacity ($60/60$) with staggered entry times and unique dummy plates for benchmarking.

---

## ⏱️ Time & Space Complexity Reference

| Operation / Feature | Algorithm / Technique | Time Complexity | Auxiliary Space |
| :--- | :--- | :---: | :---: |
| Park at End | Direct Array Indexing | $O(1)$ | $O(1)$ |
| Park at Front (VIP) | Array Right Shift | $O(n)$ | $O(1)$ |
| Park at Position | Array Right Shift from Index | $O(n)$ | $O(1)$ |
| Sorted Park | Linear Scan + Shift | $O(n)$ | $O(1)$ |
| Exit by Position / Plate | Array Left Shift Compaction | $O(n)$ | $O(1)$ |
| Closing Sweep | Single-pass Read/Write Compaction | $O(n)$ | $O(1)$ |
| Linear Search | Sequential Scan | $O(n)$ | $O(1)$ |
| Binary Search | Divide & Conquer | $O(\log n)$ | $O(1)$ |
| Search by Type | Sequential Scan | $O(n)$ | $O(1)$ |
| Insertion Sort (Plate) | In-place Insertion Sort | Best: $O(n)$ / Worst: $O(n^2)$ | $O(1)$ |
| Selection Sort (Time) | In-place Selection Sort | $O(n^2)$ | $O(1)$ |
| Stable Bubble Sort | Early-Exit Bubble Sort | Best: $O(n)$ / Worst: $O(n^2)$ | $O(n)$ (snapshot verification) |
| Multi-Level Merge | Two-Pointer Merge | $O(n_1 + n_2)$ | $O(n_1 + n_2)$ |
| Peak Occupancy Analysis | 24-Hour Range Overlap Check | $O(24 \times (N + M))$ | $O(24)$ |

---

## 💻 Getting Started

### Prerequisites
- A modern C++ compiler supporting **C++11** or later (`g++`, `clang++`, or MSVC).
- Make, CMake, or direct shell/terminal access.

### Compilation
Compile with all strict warnings enabled:

```bash
g++ -std=c++11 -Wall -Wextra -pedantic smart_parking_lot_manager.cpp -o smart_parking_lot_manager
```

### Running the Application
```bash
./smart_parking_lot_manager
```

On Windows (Command Prompt / PowerShell):
```cmd
g++ -std=c++11 -Wall -Wextra -pedantic smart_parking_lot_manager.cpp -o smart_parking_lot_manager.exe
smart_parking_lot_manager.exe
```

---

## 🖥️ Menu Structure

```text
MAIN MENU:
  1. Show Lot                           -> Aligned ASCII table of current occupancy
  2. Park Vehicle                       -> Submenu: End, Front (VIP), Position, Sorted
  3. Exit Vehicle                       -> Submenu: By Position, By Plate, Closing Sweep
  4. Search                             -> Submenu: Linear, Binary, By Type, Longest Parked
  5. Sort                               -> Submenu: Insertion (Plate), Selection (Time), Bubble (Type/Time)
  6. Fees & Revenue                     -> Submenu: Preview Fee, Total Revenue
  7. Bonus Features                     -> Submenu: Free Slots, Multi-Level Merge, Peak Occupancy
  8. Testing Helper (Auto-fill lot)     -> Auto-fills 60 test vehicles
  0. Quit                               -> Exit program
```

---

## 📄 License

This project is licensed under the [MIT License](LICENSE).
