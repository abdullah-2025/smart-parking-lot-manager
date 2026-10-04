// ============================================================================
// File: smart_parking_lot_manager.cpp
// Student Name: M Abdullah Khan
// Registration No: 2312280
// Course: Data Structures & Algorithms
// Assignment: Smart Parking Lot Manager
// Platform: Portable C++11 (macOS / Linux / Windows Dev-C++ / Code::Blocks / VS)
// Constraints: Fixed-size arrays only; no <string>, <vector>, <algorithm>, <list>.
// ============================================================================

#include <iostream>
#include <iomanip>
#include <cstring>
#include <cctype>
#include <limits>

// ----------------------------------------------------------------------------
// CONSTANTS & STRUCTURES
// ----------------------------------------------------------------------------
const int MAX_LOT = 60;
const int MAX_HISTORY = 200;
const int MAX_LEVEL2 = 60;
const int MAX_MERGED = 120;

// Vehicle record representation
struct Vehicle {
    int slotNo;        // Physical slot number (1..60)
    char plate[11];    // Vehicle license plate (normalized uppercase, null-terminated)
    int entryTime;     // Entry time in 24-hr format (HHMM, e.g. 1415 = 2:15 PM)
    int type;          // 1 = Bike, 2 = Car, 3 = Truck
};

// Exit history record for Bonus 3 analytics
struct ExitRecord {
    char plate[11];
    int type;
    int entryTime;
    int exitTime;
    int fee;
};

// ----------------------------------------------------------------------------
// GLOBAL DATA STORAGE
// ----------------------------------------------------------------------------
// Main parking lot array (Task requirement: fixed array Vehicle lot[60])
Vehicle lot[MAX_LOT];
int lotCount = 0;

// Bonus 1: Free slot tracking array kept strictly sorted in ascending order.
// When allocating, lot[i] gets freeSlots[0] (lowest available slot).
int freeSlots[MAX_LOT];
int freeCount = 0;

// Total revenue collected across successful exits
int totalRevenue = 0;

// Bonus 2: Level 2 parking lot array
Vehicle level2[MAX_LEVEL2];
int level2Count = 0;

// Bonus 3: Exit history log
ExitRecord history[MAX_HISTORY];
int historyCount = 0;

// ----------------------------------------------------------------------------
// BANNER & DISPLAY UTILITIES
// ----------------------------------------------------------------------------

// Task 6 Helper - Student banner displayed at launch and above every main menu
// Time Complexity: O(1)
void printBanner() {
    std::cout << "\n============================================================\n";
    std::cout << "               SMART PARKING LOT MANAGER\n";
    std::cout << "             Author: M Abdullah Khan\n";
    std::cout << "             Registration No: 2312280\n";
    std::cout << "============================================================\n";
}

// Helper - Clear input stream after errors or invalid inputs
// Time Complexity: O(1)
void clearInputBuffer() {
    std::cin.clear();
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
}

// Helper - Formats and prints time as HH:MM
// Time Complexity: O(1)
void printFormattedTime(int t) {
    int hh = t / 100;
    int mm = t % 100;
    if (hh < 10) std::cout << '0';
    std::cout << hh << ':';
    if (mm < 10) std::cout << '0';
    std::cout << mm;
}

// Helper - Returns readable vehicle type name
// Time Complexity: O(1)
const char* getTypeName(int type) {
    if (type == 1) return "Bike";
    if (type == 2) return "Car";
    if (type == 3) return "Truck";
    return "Unknown";
}

// ----------------------------------------------------------------------------
// VALIDATION & INPUT HELPERS
// ----------------------------------------------------------------------------

// Helper - Validate time in HHMM format (HH: 00-23, MM: 00-59)
// Time Complexity: O(1)
bool isValidTime(int t) {
    if (t < 0 || t > 2359) return false;
    int hh = t / 100;
    int mm = t % 100;
    return (hh >= 0 && hh <= 23 && mm >= 0 && mm <= 59);
}

// Helper - Convert HHMM to total minutes from start of day
// Time Complexity: O(1)
int timeToMinutes(int t) {
    return (t / 100) * 60 + (t % 100);
}

// Helper - Read integer safely within [minVal, maxVal]
// Time Complexity: O(1) per valid attempt
int readInt(const char* prompt, int minVal, int maxVal) {
    int val;
    while (true) {
        if (prompt != nullptr && prompt[0] != '\0') {
            std::cout << prompt;
        }
        if (std::cin >> val) {
            clearInputBuffer();
            if (val >= minVal && val <= maxVal) {
                return val;
            }
            std::cout << "  [Error] Value must be between " << minVal << " and " << maxVal << ".\n";
        } else {
            std::cout << "  [Error] Invalid input. Please enter an integer.\n";
            clearInputBuffer();
        }
    }
}

// Helper - Read and validate license plate (4 to 10 alphanumeric chars, normalized uppercase)
// Time Complexity: O(length)
bool readPlate(char dest[11], const char* prompt) {
    char buffer[128];
    std::cout << prompt;
    if (!std::cin.getline(buffer, sizeof(buffer))) {
        clearInputBuffer();
        return false;
    }
    // Trim leading whitespace
    int start = 0;
    while (buffer[start] == ' ' || buffer[start] == '\t') start++;
    // Trim trailing whitespace
    int end = static_cast<int>(std::strlen(buffer)) - 1;
    while (end >= start && (buffer[end] == ' ' || buffer[end] == '\t' || buffer[end] == '\r' || buffer[end] == '\n')) {
        end--;
    }
    int len = end - start + 1;
    if (len < 4 || len > 10) {
        std::cout << "  [Error] Plate must be 4 to 10 alphanumeric characters (no spaces).\n";
        return false;
    }
    for (int i = 0; i < len; ++i) {
        char ch = buffer[start + i];
        if (!std::isalnum(static_cast<unsigned char>(ch))) {
            std::cout << "  [Error] Plate contains invalid character '" << ch << "'. Only letters and digits allowed.\n";
            return false;
        }
        dest[i] = static_cast<char>(std::toupper(static_cast<unsigned char>(ch)));
    }
    dest[len] = '\0';
    return true;
}

// Helper - Read and validate 24-hr time in HHMM format
// Time Complexity: O(1)
bool readTime(int &timeVal, const char* prompt) {
    char buffer[64];
    std::cout << prompt;
    if (!std::cin.getline(buffer, sizeof(buffer))) {
        clearInputBuffer();
        return false;
    }
    int start = 0;
    while (buffer[start] == ' ' || buffer[start] == '\t') start++;
    int end = static_cast<int>(std::strlen(buffer)) - 1;
    while (end >= start && (buffer[end] == ' ' || buffer[end] == '\t' || buffer[end] == '\r' || buffer[end] == '\n')) {
        end--;
    }
    int len = end - start + 1;
    if (len < 3 || len > 4) {
        std::cout << "  [Error] Time must be 3 or 4 digits in HHMM format (e.g. 0945 or 1430).\n";
        return false;
    }
    int val = 0;
    for (int i = 0; i < len; ++i) {
        char ch = buffer[start + i];
        if (!std::isdigit(static_cast<unsigned char>(ch))) {
            std::cout << "  [Error] Time must contain digits only.\n";
            return false;
        }
        val = val * 10 + (ch - '0');
    }
    if (!isValidTime(val)) {
        std::cout << "  [Error] Invalid time (" << val << "). Hours must be 00-23 and minutes 00-59.\n";
        return false;
    }
    timeVal = val;
    return true;
}

// Helper - Search if plate already exists in the lot (case-insensitive duplicate check)
// Time Complexity: O(n)
int findPlateIndex(const Vehicle arr[], int n, const char* plate) {
    for (int i = 0; i < n; ++i) {
        if (std::strcmp(arr[i].plate, plate) == 0) {
            return i;
        }
    }
    return -1;
}

// Helper - Check if lot array is currently sorted by plate ascending
// Time Complexity: O(n)
bool isSortedByPlate(const Vehicle arr[], int n) {
    for (int i = 0; i < n - 1; ++i) {
        if (std::strcmp(arr[i].plate, arr[i + 1].plate) > 0) {
            return false;
        }
    }
    return true;
}

// ----------------------------------------------------------------------------
// BONUS 1: LOWEST FREE SLOT ALLOCATOR & RELEASE
// ----------------------------------------------------------------------------

// Bonus 1 - Allocate the lowest free slot (index 0 of sorted freeSlots array)
// Time Complexity: O(freeCount) to shift remaining elements left
int allocateLowestSlot() {
    if (freeCount <= 0) {
        return -1;
    }
    int allocated = freeSlots[0];
    for (int i = 0; i < freeCount - 1; ++i) {
        freeSlots[i] = freeSlots[i + 1];
    }
    freeCount--;
    return allocated;
}

// Bonus 1 - Return a vacated slot back into freeSlots keeping it sorted
// Time Complexity: O(freeCount)
void releaseSlot(int slot) {
    int idx = 0;
    while (idx < freeCount && freeSlots[idx] < slot) {
        idx++;
    }
    for (int i = freeCount; i > idx; --i) {
        freeSlots[i] = freeSlots[i - 1];
    }
    freeSlots[idx] = slot;
    freeCount++;
}

// Bonus 1 - Display all currently available free slot numbers
// Time Complexity: O(freeCount)
void showFreeSlots() {
    std::cout << "\n--- Available Free Slots (" << freeCount << " total) ---\n";
    if (freeCount == 0) {
        std::cout << "No free slots available (Parking lot is 100% full).\n";
        return;
    }
    for (int i = 0; i < freeCount; ++i) {
        std::cout << std::setw(3) << freeSlots[i] << " ";
        if ((i + 1) % 15 == 0) std::cout << "\n";
    }
    std::cout << "\n";
}

// ----------------------------------------------------------------------------
// TASK 5: FEE CALCULATION
// ----------------------------------------------------------------------------

// Task 5a - Fee calculation based on duration rounded up to next whole hour
// Formula: minutes = exitMin - entryMin; hours = (minutes + 59) / 60; minimum 1 hour.
// Rates: Bike = 10/hr, Car = 20/hr, Truck = 40/hr.
// Assumption: All parking events occur on the same calendar day.
// Time Complexity: O(1)
bool calculateFee(int entryTime, int exitTime, int type, int &durationHours, int &fee) {
    int entryMin = timeToMinutes(entryTime);
    int exitMin = timeToMinutes(exitTime);
    if (exitMin < entryMin) {
        return false; // Exit time earlier than entry time
    }
    int diffMin = exitMin - entryMin;
    durationHours = (diffMin + 59) / 60;
    if (durationHours < 1) {
        durationHours = 1; // Minimum 1-hour charge
    }
    int rate = 0;
    if (type == 1) rate = 10;
    else if (type == 2) rate = 20;
    else if (type == 3) rate = 40;
    fee = durationHours * rate;
    return true;
}

// ----------------------------------------------------------------------------
// TASK 6: DISPLAY PARKING LOT TABLE
// ----------------------------------------------------------------------------

// Task 6 - Show parking lot contents in an aligned ASCII table
// Time Complexity: O(n)
void showLot(const Vehicle arr[], int n, const char* title = "MAIN PARKING LOT") {
    std::cout << "\n============================================================\n";
    std::cout << "                " << title << "\n";
    std::cout << "============================================================\n";
    std::cout << "+-----+------+------------+------------+---------+\n";
    std::cout << "| Pos | Slot | Plate      | Entry Time | Type    |\n";
    std::cout << "+-----+------+------------+------------+---------+\n";
    if (n == 0) {
        std::cout << "|              Parking lot is currently empty.             |\n";
        std::cout << "+-----+------+------------+------------+---------+\n";
    } else {
        for (int i = 0; i < n; ++i) {
            std::cout << "| " << std::setw(3) << (i + 1) << " | "
                      << std::setw(4) << arr[i].slotNo << " | "
                      << std::left << std::setw(10) << arr[i].plate << std::right << " | ";
            printFormattedTime(arr[i].entryTime);
            std::cout << "      | "
                      << std::left << std::setw(7) << getTypeName(arr[i].type) << std::right << " |\n";
        }
        std::cout << "+-----+------+------------+------------+---------+\n";
    }
    std::cout << "Occupancy: " << n << " / " << MAX_LOT << " occupied | Free slots: " << (MAX_LOT - n) << "\n";
}

// ----------------------------------------------------------------------------
// TASK 4: SORTING ALGORITHMS
// ----------------------------------------------------------------------------

// Task 4a - Insertion Sort by plate number ascending
// Time Complexity: Best O(n), Average/Worst O(n^2). Space: O(1).
void insertionSortByPlate(Vehicle arr[], int n, bool printDetails = true) {
    int comparisons = 0;
    int shifts = 0;
    for (int i = 1; i < n; ++i) {
        Vehicle key = arr[i];
        int j = i - 1;
        while (j >= 0) {
            comparisons++;
            if (std::strcmp(arr[j].plate, key.plate) > 0) {
                arr[j + 1] = arr[j];
                shifts++;
                j--;
            } else {
                break;
            }
        }
        arr[j + 1] = key;
    }
    if (printDetails) {
        std::cout << "\n[Insertion Sort Completed]\n";
        std::cout << "Sorted by: License Plate (A-Z)\n";
        std::cout << "Comparisons: " << comparisons << " | Shifts: " << shifts << "\n";
        showLot(arr, n, "LOT AFTER INSERTION SORT (BY PLATE)");
    }
}

// Task 4b - Selection Sort by entry time ascending
// Time Complexity: O(n^2) comparisons, O(n) swaps. Space: O(1).
void selectionSortByEntryTime(Vehicle arr[], int n) {
    int comparisons = 0;
    int swaps = 0;
    for (int i = 0; i < n - 1; ++i) {
        int minIdx = i;
        for (int j = i + 1; j < n; ++j) {
            comparisons++;
            if (arr[j].entryTime < arr[minIdx].entryTime) {
                minIdx = j;
            }
        }
        if (minIdx != i) {
            Vehicle temp = arr[i];
            arr[i] = arr[minIdx];
            arr[minIdx] = temp;
            swaps++;
        }
    }
    std::cout << "\n[Selection Sort Completed]\n";
    std::cout << "Sorted by: Entry Time (Earliest to Latest)\n";
    std::cout << "Comparisons: " << comparisons << " | Swaps: " << swaps << "\n";
    showLot(arr, n, "LOT AFTER SELECTION SORT (BY ENTRY TIME)");
}

// Task 4c - Stable Bubble Sort by vehicle type then entry time, with early-exit flag
// Time Complexity: Best O(n), Average/Worst O(n^2). Space: O(n) for snapshot verification.
void stableBubbleSortByTypeAndTime(Vehicle arr[], int n) {
    // Snapshot original order to perform rigorous stability verification
    Vehicle snapshot[MAX_LOT];
    int snapCount = n;
    for (int i = 0; i < n; ++i) {
        snapshot[i] = arr[i];
    }

    int comparisons = 0;
    int swaps = 0;
    for (int i = 0; i < n - 1; ++i) {
        bool swapped = false;
        for (int j = 0; j < n - 1 - i; ++j) {
            comparisons++;
            bool shouldSwap = false;
            // Primary key: type ascending; Secondary key: entryTime ascending.
            // STABILITY RULE: Swap ONLY when strictly greater. Never swap when keys are equal!
            if (arr[j].type > arr[j + 1].type) {
                shouldSwap = true;
            } else if (arr[j].type == arr[j + 1].type && arr[j].entryTime > arr[j + 1].entryTime) {
                shouldSwap = true;
            }
            if (shouldSwap) {
                Vehicle temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
                swaps++;
                swapped = true;
            }
        }
        if (!swapped) {
            break; // Early-exit optimization
        }
    }

    // Verify ordering correctness
    bool sortedCorrectly = true;
    for (int i = 0; i < n - 1; ++i) {
        if (arr[i].type > arr[i + 1].type) {
            sortedCorrectly = false;
            break;
        }
        if (arr[i].type == arr[i + 1].type && arr[i].entryTime > arr[i + 1].entryTime) {
            sortedCorrectly = false;
            break;
        }
    }

    // Verify stability: records with identical (type, entryTime) must maintain their original relative order
    bool isStable = true;
    for (int p = 0; p < n; ++p) {
        for (int q = p + 1; q < n; ++q) {
            if (arr[p].type == arr[q].type && arr[p].entryTime == arr[q].entryTime) {
                int origP = -1, origQ = -1;
                for (int k = 0; k < snapCount; ++k) {
                    if (origP == -1 && std::strcmp(snapshot[k].plate, arr[p].plate) == 0) origP = k;
                    if (origQ == -1 && std::strcmp(snapshot[k].plate, arr[q].plate) == 0) origQ = k;
                }
                if (origP > origQ) {
                    isStable = false;
                }
            }
        }
    }

    std::cout << "\n[Stable Bubble Sort Completed]\n";
    std::cout << "Sorted by: Vehicle Type (1->2->3), then Entry Time (HHMM)\n";
    std::cout << "Comparisons: " << comparisons << " | Swaps: " << swaps << "\n";
    if (sortedCorrectly && isStable) {
        std::cout << "Stability check: PASS\n";
    } else {
        std::cout << "Stability check: FAIL\n";
    }
    showLot(arr, n, "LOT AFTER BUBBLE SORT (TYPE THEN TIME)");
}

// ----------------------------------------------------------------------------
// TASK 1: INSERTION (PARK SUBMENU)
// ----------------------------------------------------------------------------

// Common insertion pre-check: validates capacity and reads common vehicle details
// Time Complexity: O(n) for duplicate plate check
bool prepareNewVehicle(Vehicle &v) {
    // Task 1e: Reject if lot is full
    if (lotCount >= MAX_LOT) {
        std::cout << "\n[Error] Parking lot is full (60/60 occupied)! Cannot park vehicle.\n";
        return false;
    }
    // Read plate
    char tempPlate[11];
    if (!readPlate(tempPlate, "Enter license plate (4-10 alphanumeric characters): ")) {
        return false;
    }
    // Task 1e: Reject duplicate plate
    int dupIdx = findPlateIndex(lot, lotCount, tempPlate);
    if (dupIdx != -1) {
        std::cout << "\n[Error] Duplicate plate! Vehicle " << tempPlate
                  << " is already parked at position " << (dupIdx + 1)
                  << " (Slot " << lot[dupIdx].slotNo << "). Insertion rejected.\n";
        return false;
    }
    // Read entry time
    int tempTime;
    if (!readTime(tempTime, "Enter entry time (HHMM, 24-hr format, e.g. 0945): ")) {
        return false;
    }
    // Read vehicle type
    std::cout << "Select vehicle type:\n";
    std::cout << "  1. Bike  ($10/hr)\n";
    std::cout << "  2. Car   ($20/hr)\n";
    std::cout << "  3. Truck ($40/hr)\n";
    int tempType = readInt("Enter type (1-3): ", 1, 3);

    // Allocate physical slot from sorted free slots (Bonus 1)
    int allocatedSlot = allocateLowestSlot();
    if (allocatedSlot == -1) {
        std::cout << "\n[Error] No physical slots available.\n";
        return false;
    }

    v.slotNo = allocatedSlot;
    std::strcpy(v.plate, tempPlate);
    v.entryTime = tempTime;
    v.type = tempType;
    return true;
}

// Task 1a - Park at next free position (end of array)
// Time Complexity: O(1) array append
void parkAtEnd() {
    Vehicle v;
    if (!prepareNewVehicle(v)) return;
    lot[lotCount] = v;
    lotCount++;
    std::cout << "\n[Success] Vehicle " << v.plate << " parked at position " << lotCount
              << " in physical Slot " << v.slotNo << ".\n";
}

// Task 1b - Insert VIP/reserved vehicle at the FRONT (shift all records right)
// Time Complexity: O(n) shifts
void parkAtFrontVIP() {
    Vehicle v;
    if (!prepareNewVehicle(v)) return;
    for (int i = lotCount; i > 0; --i) {
        lot[i] = lot[i - 1];
    }
    lot[0] = v;
    lotCount++;
    std::cout << "\n[Success] VIP Vehicle " << v.plate << " inserted at FRONT (position 1) in physical Slot "
              << v.slotNo << ".\n";
}

// Task 1c - Insert at user-chosen position (1 to count+1, shift right from that point)
// Time Complexity: O(n) shifts
void parkAtPosition() {
    if (lotCount >= MAX_LOT) {
        std::cout << "\n[Error] Parking lot is full (60/60 occupied)!\n";
        return;
    }
    std::cout << "Enter target position (1 to " << (lotCount + 1) << "): ";
    int pos = readInt("", 1, lotCount + 1);
    Vehicle v;
    if (!prepareNewVehicle(v)) return;
    int targetIdx = pos - 1;
    for (int i = lotCount; i > targetIdx; --i) {
        lot[i] = lot[i - 1];
    }
    lot[targetIdx] = v;
    lotCount++;
    std::cout << "\n[Success] Vehicle " << v.plate << " parked at position " << pos
              << " in physical Slot " << v.slotNo << ".\n";
}

// Task 1d - Insert while keeping array sorted by plate
// Time Complexity: O(n) comparisons and shifts
void parkSortedByPlate() {
    if (lotCount >= MAX_LOT) {
        std::cout << "\n[Error] Parking lot is full (60/60 occupied)!\n";
        return;
    }
    // Check if array is currently sorted by plate
    if (!isSortedByPlate(lot, lotCount)) {
        std::cout << "\n[Warning] The lot array is not currently sorted by license plate.\n";
        std::cout << "Would you like to run Insertion Sort first? (1 = Yes, 0 = Cancel): ";
        int choice = readInt("", 0, 1);
        if (choice == 1) {
            insertionSortByPlate(lot, lotCount, true);
        } else {
            std::cout << "Sorted insertion cancelled.\n";
            return;
        }
    }
    Vehicle v;
    if (!prepareNewVehicle(v)) return;
    // Find correct insertion index
    int insertIdx = 0;
    while (insertIdx < lotCount && std::strcmp(lot[insertIdx].plate, v.plate) < 0) {
        insertIdx++;
    }
    // Shift elements right
    for (int i = lotCount; i > insertIdx; --i) {
        lot[i] = lot[i - 1];
    }
    lot[insertIdx] = v;
    lotCount++;
    std::cout << "\n[Success] Vehicle " << v.plate << " inserted at sorted position " << (insertIdx + 1)
              << " in physical Slot " << v.slotNo << ".\n";
}

// ----------------------------------------------------------------------------
// TASK 2: DELETION (EXIT VEHICLE SUBMENU)
// ----------------------------------------------------------------------------

// Task 2 Helper - Logs exit to history array for Bonus 3 analytics
// Time Complexity: O(1)
void logExitHistory(const Vehicle &v, int exitTime, int fee) {
    if (historyCount < MAX_HISTORY) {
        std::strcpy(history[historyCount].plate, v.plate);
        history[historyCount].type = v.type;
        history[historyCount].entryTime = v.entryTime;
        history[historyCount].exitTime = exitTime;
        history[historyCount].fee = fee;
        historyCount++;
    } else {
        std::cout << "  [Warning] Exit history log is full (" << MAX_HISTORY << " records). Logging stopped.\n";
    }
}

// Task 2a - Exit vehicle by position (1..count)
// Time Complexity: O(n) shifts
void exitByPosition() {
    if (lotCount == 0) {
        std::cout << "\n[Error] Parking lot is empty. No vehicle to exit.\n";
        return;
    }
    std::cout << "Enter position of exiting vehicle (1 to " << lotCount << "): ";
    int pos = readInt("", 1, lotCount);
    int idx = pos - 1;
    Vehicle target = lot[idx];

    int exitTime;
    if (!readTime(exitTime, "Enter exit time (HHMM): ")) {
        return;
    }
    int hours = 0, fee = 0;
    if (!calculateFee(target.entryTime, exitTime, target.type, hours, fee)) {
        std::cout << "\n[Error] Exit time (";
        printFormattedTime(exitTime);
        std::cout << ") cannot be earlier than entry time (";
        printFormattedTime(target.entryTime);
        std::cout << "). Exit cancelled.\n";
        return;
    }

    // Bill receipt
    std::cout << "\n============================================\n";
    std::cout << "             PARKING RECEIPT\n";
    std::cout << "============================================\n";
    std::cout << "Plate:         " << target.plate << "\n";
    std::cout << "Slot Number:   " << target.slotNo << "\n";
    std::cout << "Vehicle Type:  " << getTypeName(target.type) << "\n";
    std::cout << "Entry Time:    "; printFormattedTime(target.entryTime); std::cout << "\n";
    std::cout << "Exit Time:     "; printFormattedTime(exitTime); std::cout << "\n";
    std::cout << "Billed Hours:  " << hours << " hour(s) (rounded up, min 1 hr)\n";
    std::cout << "Total Fee:     $" << fee << "\n";
    std::cout << "============================================\n";

    // Update revenue & history
    totalRevenue += fee;
    logExitHistory(target, exitTime, fee);

    // Release slot back to freeSlots
    releaseSlot(target.slotNo);

    // Shift left to close the gap
    for (int i = idx; i < lotCount - 1; ++i) {
        lot[i] = lot[i + 1];
    }
    lotCount--;
    std::cout << "[Success] Vehicle exited. Slot " << target.slotNo << " is now free. Total Revenue: $"
              << totalRevenue << "\n";
}

// Task 2b - Exit vehicle by plate number (linear search then shift left)
// Time Complexity: O(n) search + O(n) shifts = O(n)
void exitByPlate() {
    if (lotCount == 0) {
        std::cout << "\n[Error] Parking lot is empty. No vehicle to exit.\n";
        return;
    }
    char searchPlate[11];
    if (!readPlate(searchPlate, "Enter license plate of exiting vehicle: ")) {
        return;
    }
    int idx = findPlateIndex(lot, lotCount, searchPlate);
    if (idx == -1) {
        std::cout << "\n[Error] Vehicle with plate " << searchPlate << " was not found in the lot.\n";
        return;
    }
    Vehicle target = lot[idx];

    int exitTime;
    if (!readTime(exitTime, "Enter exit time (HHMM): ")) {
        return;
    }
    int hours = 0, fee = 0;
    if (!calculateFee(target.entryTime, exitTime, target.type, hours, fee)) {
        std::cout << "\n[Error] Exit time (";
        printFormattedTime(exitTime);
        std::cout << ") cannot be earlier than entry time (";
        printFormattedTime(target.entryTime);
        std::cout << "). Exit cancelled.\n";
        return;
    }

    std::cout << "\n============================================\n";
    std::cout << "             PARKING RECEIPT\n";
    std::cout << "============================================\n";
    std::cout << "Plate:         " << target.plate << "\n";
    std::cout << "Slot Number:   " << target.slotNo << "\n";
    std::cout << "Vehicle Type:  " << getTypeName(target.type) << "\n";
    std::cout << "Entry Time:    "; printFormattedTime(target.entryTime); std::cout << "\n";
    std::cout << "Exit Time:     "; printFormattedTime(exitTime); std::cout << "\n";
    std::cout << "Billed Hours:  " << hours << " hour(s) (rounded up, min 1 hr)\n";
    std::cout << "Total Fee:     $" << fee << "\n";
    std::cout << "============================================\n";

    totalRevenue += fee;
    logExitHistory(target, exitTime, fee);
    releaseSlot(target.slotNo);

    for (int i = idx; i < lotCount - 1; ++i) {
        lot[i] = lot[i + 1];
    }
    lotCount--;
    std::cout << "[Success] Vehicle exited. Slot " << target.slotNo << " is now free. Total Revenue: $"
              << totalRevenue << "\n";
}

// Task 2c - Closing sweep: remove all vehicles that entered before a given time in one O(n) pass.
// Assumption: Swept vehicles are cleared without fee (e.g. overnight sweep / tow / maintenance).
// Time Complexity: O(n) single compaction pass (plus O(k * freeCount) slot releases for k swept vehicles).
void closingSweep() {
    if (lotCount == 0) {
        std::cout << "\n[Error] Parking lot is empty. Nothing to sweep.\n";
        return;
    }
    int sweepTime;
    if (!readTime(sweepTime, "Enter sweep cutoff time (HHMM - vehicles entered BEFORE this time will be removed): ")) {
        return;
    }
    int writeIdx = 0;
    int removedCount = 0;

    std::cout << "\n============================================================\n";
    std::cout << "               CLOSING SWEEP REPORT\n";
    std::cout << "Cutoff Time: "; printFormattedTime(sweepTime); std::cout << " (Vehicles entered strictly before cutoff)\n";
    std::cout << "============================================================\n";
    std::cout << "+------+------------+------------+---------+\n";
    std::cout << "| Slot | Plate      | Entry Time | Type    |\n";
    std::cout << "+------+------------+------------+---------+\n";

    for (int readIdx = 0; readIdx < lotCount; ++readIdx) {
        if (lot[readIdx].entryTime < sweepTime) {
            std::cout << "| " << std::setw(4) << lot[readIdx].slotNo << " | "
                      << std::left << std::setw(10) << lot[readIdx].plate << std::right << " | ";
            printFormattedTime(lot[readIdx].entryTime);
            std::cout << "      | "
                      << std::left << std::setw(7) << getTypeName(lot[readIdx].type) << std::right << " |\n";
            releaseSlot(lot[readIdx].slotNo);
            removedCount++;
        } else {
            lot[writeIdx] = lot[readIdx];
            writeIdx++;
        }
    }
    lotCount = writeIdx;
    std::cout << "+------+------------+------------+---------+\n";
    std::cout << "Compaction complete. Total vehicles swept: " << removedCount << "\n";
    std::cout << "Remaining vehicles in lot: " << lotCount << "\n";
}

// ----------------------------------------------------------------------------
// TASK 3: SEARCHING (SEARCH SUBMENU)
// ----------------------------------------------------------------------------

// Task 3a - Linear search by plate (lost-car query)
// Time Complexity: O(n). Prints exact number of comparisons made.
void linearSearchByPlate() {
    if (lotCount == 0) {
        std::cout << "\n[Error] Parking lot is empty.\n";
        return;
    }
    char target[11];
    if (!readPlate(target, "Enter license plate to search (Linear Search): ")) {
        return;
    }
    int comparisons = 0;
    int foundIdx = -1;
    for (int i = 0; i < lotCount; ++i) {
        comparisons++;
        if (std::strcmp(lot[i].plate, target) == 0) {
            foundIdx = i;
            break;
        }
    }
    std::cout << "\n--- Linear Search Result ---\n";
    if (foundIdx != -1) {
        std::cout << "Status:         FOUND\n";
        std::cout << "Array Position: " << (foundIdx + 1) << "\n";
        std::cout << "Slot Number:    " << lot[foundIdx].slotNo << "\n";
        std::cout << "Plate:          " << lot[foundIdx].plate << "\n";
        std::cout << "Entry Time:     "; printFormattedTime(lot[foundIdx].entryTime); std::cout << "\n";
        std::cout << "Type:           " << getTypeName(lot[foundIdx].type) << "\n";
    } else {
        std::cout << "Status:         NOT FOUND\n";
        std::cout << "Vehicle with plate " << target << " is not in the lot.\n";
    }
    std::cout << "Comparisons:    " << comparisons << "\n";
}

// Task 3b - Binary search by plate with pre-condition check and failure demonstration option
// Time Complexity: O(log n). Exactly 1 strcmp evaluated and counted per iteration.
void binarySearchByPlate() {
    if (lotCount == 0) {
        std::cout << "\n[Error] Parking lot is empty.\n";
        return;
    }
    // Check if sorted
    bool sorted = isSortedByPlate(lot, lotCount);
    if (!sorted) {
        std::cout << "\n============================================================\n";
        std::cout << "  [WARNING] The parking lot array is NOT sorted by plate!\n";
        std::cout << "  Binary search relies on sorted order. Searching now may\n";
        std::cout << "  fail to find an existing record or produce wrong results.\n";
        std::cout << "============================================================\n";
        std::cout << "Options:\n";
        std::cout << "  1. Sort by plate with Insertion Sort, then perform Binary Search\n";
        std::cout << "  2. Run Binary Search anyway (to demonstrate failure on unsorted data)\n";
        std::cout << "  0. Cancel\n";
        int opt = readInt("Enter choice (0-2): ", 0, 2);
        if (opt == 1) {
            insertionSortByPlate(lot, lotCount, true);
        } else if (opt == 2) {
            std::cout << "[Proceeding with Binary Search on unsorted array]\n";
        } else {
            std::cout << "Search cancelled.\n";
            return;
        }
    }

    char target[11];
    if (!readPlate(target, "Enter license plate to search (Binary Search): ")) {
        return;
    }

    int low = 0;
    int high = lotCount - 1;
    int comparisons = 0;
    int foundIdx = -1;

    while (low <= high) {
        int mid = low + (high - low) / 2;
        comparisons++;
        // Single strcmp stored in variable to strictly count 1 comparison per iteration
        int cmp = std::strcmp(target, lot[mid].plate);
        if (cmp == 0) {
            foundIdx = mid;
            break;
        } else if (cmp < 0) {
            high = mid - 1;
        } else {
            low = mid + 1;
        }
    }

    std::cout << "\n--- Binary Search Result ---\n";
    if (foundIdx != -1) {
        std::cout << "Status:         FOUND\n";
        std::cout << "Array Position: " << (foundIdx + 1) << "\n";
        std::cout << "Slot Number:    " << lot[foundIdx].slotNo << "\n";
        std::cout << "Plate:          " << lot[foundIdx].plate << "\n";
        std::cout << "Entry Time:     "; printFormattedTime(lot[foundIdx].entryTime); std::cout << "\n";
        std::cout << "Type:           " << getTypeName(lot[foundIdx].type) << "\n";
    } else {
        std::cout << "Status:         NOT FOUND\n";
        std::cout << "Vehicle with plate " << target << " was not found.\n";
        if (!sorted) {
            std::cout << "Note: Array was unsorted during this search, demonstrating binary search failure!\n";
        }
    }
    std::cout << "Comparisons:    " << comparisons << "\n";
}

// Task 3c - Find all vehicles of a given type
// Time Complexity: O(n). Comparisons counted.
void searchVehiclesByType() {
    if (lotCount == 0) {
        std::cout << "\n[Error] Parking lot is empty.\n";
        return;
    }
    std::cout << "\nSelect vehicle type to search:\n";
    std::cout << "  1. Bike\n";
    std::cout << "  2. Car\n";
    std::cout << "  3. Truck\n";
    int targetType = readInt("Enter type (1-3): ", 1, 3);

    int comparisons = 0;
    int matchCount = 0;

    std::cout << "\n============================================================\n";
    std::cout << "       VEHICLES OF TYPE: " << getTypeName(targetType) << "\n";
    std::cout << "============================================================\n";
    std::cout << "+-----+------+------------+------------+\n";
    std::cout << "| Pos | Slot | Plate      | Entry Time |\n";
    std::cout << "+-----+------+------------+------------+\n";

    for (int i = 0; i < lotCount; ++i) {
        comparisons++;
        if (lot[i].type == targetType) {
            matchCount++;
            std::cout << "| " << std::setw(3) << (i + 1) << " | "
                      << std::setw(4) << lot[i].slotNo << " | "
                      << std::left << std::setw(10) << lot[i].plate << std::right << " | ";
            printFormattedTime(lot[i].entryTime);
            std::cout << "      |\n";
        }
    }
    std::cout << "+-----+------+------------+------------+\n";
    std::cout << "Total matches found: " << matchCount << " / " << lotCount << " vehicles\n";
    std::cout << "Comparisons:         " << comparisons << "\n";
}

// Task 3d - Find the longest-parked vehicle (earliest entry time; reports all ties)
// Time Complexity: O(n). Comparisons counted.
void findLongestParked() {
    if (lotCount == 0) {
        std::cout << "\n[Error] Parking lot is empty.\n";
        return;
    }
    int currentTime;
    if (!readTime(currentTime, "Enter current time (HHMM): ")) {
        return;
    }
    int minTime = lot[0].entryTime;
    int comparisons = 0;
    for (int i = 1; i < lotCount; ++i) {
        comparisons++;
        if (lot[i].entryTime < minTime) {
            minTime = lot[i].entryTime;
        }
    }

    int currentMin = timeToMinutes(currentTime);

    std::cout << "\n============================================================\n";
    std::cout << "             LONGEST PARKED VEHICLE(S)\n";
    std::cout << "Current Time: "; printFormattedTime(currentTime);
    std::cout << " | Earliest Entry: "; printFormattedTime(minTime); std::cout << "\n";
    std::cout << "============================================================\n";

    int tieCount = 0;
    for (int i = 0; i < lotCount; ++i) {
        if (lot[i].entryTime == minTime) {
            tieCount++;
            int entryMin = timeToMinutes(lot[i].entryTime);
            int diffMin = currentMin - entryMin;
            if (diffMin < 0) diffMin = 0;
            int durH = diffMin / 60;
            int durM = diffMin % 60;

            std::cout << "Record #" << tieCount << ":\n";
            std::cout << "  Array Position: " << (i + 1) << "\n";
            std::cout << "  Slot Number:    " << lot[i].slotNo << "\n";
            std::cout << "  Plate:          " << lot[i].plate << "\n";
            std::cout << "  Vehicle Type:   " << getTypeName(lot[i].type) << "\n";
            std::cout << "  Entry Time:     "; printFormattedTime(lot[i].entryTime); std::cout << "\n";
            std::cout << "  Duration:       " << durH << " hours " << durM << " minutes\n";
            std::cout << "--------------------------------------------\n";
        }
    }
    std::cout << "Comparisons: " << comparisons << "\n";
}

// ----------------------------------------------------------------------------
// TASK 5: FEES MENU INTERFACE
// ----------------------------------------------------------------------------

// Task 5a - Preview fee for a parked plate without removing it
// Time Complexity: O(n) search
void previewFee() {
    if (lotCount == 0) {
        std::cout << "\n[Error] Parking lot is empty.\n";
        return;
    }
    char target[11];
    if (!readPlate(target, "Enter license plate to preview fee: ")) {
        return;
    }
    int idx = findPlateIndex(lot, lotCount, target);
    if (idx == -1) {
        std::cout << "\n[Error] Vehicle with plate " << target << " was not found.\n";
        return;
    }
    int simExitTime;
    if (!readTime(simExitTime, "Enter preview exit time (HHMM): ")) {
        return;
    }
    int hours = 0, fee = 0;
    if (!calculateFee(lot[idx].entryTime, simExitTime, lot[idx].type, hours, fee)) {
        std::cout << "\n[Error] Preview exit time (";
        printFormattedTime(simExitTime);
        std::cout << ") cannot be earlier than entry time (";
        printFormattedTime(lot[idx].entryTime);
        std::cout << ").\n";
        return;
    }
    std::cout << "\n============================================\n";
    std::cout << "              FEE PREVIEW\n";
    std::cout << "============================================\n";
    std::cout << "Plate:            " << lot[idx].plate << "\n";
    std::cout << "Slot Number:      " << lot[idx].slotNo << "\n";
    std::cout << "Type:             " << getTypeName(lot[idx].type) << "\n";
    std::cout << "Entry Time:       "; printFormattedTime(lot[idx].entryTime); std::cout << "\n";
    std::cout << "Preview Exit:     "; printFormattedTime(simExitTime); std::cout << "\n";
    std::cout << "Calculated Hours: " << hours << " hr(s) (rounded up, min 1 hr)\n";
    std::cout << "Estimated Fee:    $" << fee << "\n";
    std::cout << "Note: This is a preview. Vehicle remains parked.\n";
    std::cout << "============================================\n";
}

// ----------------------------------------------------------------------------
// BONUS 2: MERGE TWO LEVELS
// ----------------------------------------------------------------------------

// Bonus 2 - Preload Level 2 with sample data
// Time Complexity: O(1)
void initLevel2() {
    level2Count = 5;
    // Slot 101-105 for Level 2
    level2[0].slotNo = 101; std::strcpy(level2[0].plate, "UP14JK"); level2[0].entryTime = 830;  level2[0].type = 2;
    level2[1].slotNo = 102; std::strcpy(level2[1].plate, "RJ14LM"); level2[1].entryTime = 915;  level2[1].type = 1;
    level2[2].slotNo = 103; std::strcpy(level2[2].plate, "HR26NP"); level2[2].entryTime = 1045; level2[2].type = 3;
    level2[3].slotNo = 104; std::strcpy(level2[3].plate, "WB02QR"); level2[3].entryTime = 1100; level2[3].type = 2;
    level2[4].slotNo = 105; std::strcpy(level2[4].plate, "AP09ST"); level2[4].entryTime = 1215; level2[4].type = 1;
}

// Bonus 2 - Two-pointer merge of Level 1 and Level 2 sorted by plate
// Time Complexity: O(n1 log n1 + n2 log n2) for sorting + O(n1 + n2) merge
void mergeTwoLevels() {
    std::cout << "\n[Bonus 2] Merging Level 1 (Main Lot) and Level 2 by License Plate\n";

    // 1. Sort both arrays by plate with Insertion Sort
    insertionSortByPlate(lot, lotCount, false);
    insertionSortByPlate(level2, level2Count, false);

    // 2. Standard two-pointer merge into merged array
    Vehicle merged[MAX_MERGED];
    int mergedCount = 0;
    int p1 = 0;
    int p2 = 0;
    int comparisons = 0;
    int skippedDuplicates = 0;

    while (p1 < lotCount && p2 < level2Count) {
        comparisons++;
        int cmp = std::strcmp(lot[p1].plate, level2[p2].plate);
        if (cmp < 0) {
            merged[mergedCount++] = lot[p1++];
        } else if (cmp > 0) {
            merged[mergedCount++] = level2[p2++];
        } else {
            // Duplicate plate across levels: keep Level 1 record, skip duplicate Level 2
            std::cout << "  [Warning] Duplicate plate '" << lot[p1].plate
                      << "' found in both levels. Level 2 duplicate skipped.\n";
            merged[mergedCount++] = lot[p1++];
            p2++;
            skippedDuplicates++;
        }
    }
    while (p1 < lotCount) {
        merged[mergedCount++] = lot[p1++];
    }
    while (p2 < level2Count) {
        merged[mergedCount++] = level2[p2++];
    }

    // Display merged results
    std::cout << "\n============================================================\n";
    std::cout << "          MERGED MULTI-LEVEL PARKING LOT (BY PLATE)\n";
    std::cout << "============================================================\n";
    std::cout << "+-----+------+------------+------------+---------+\n";
    std::cout << "| Pos | Slot | Plate      | Entry Time | Type    |\n";
    std::cout << "+-----+------+------------+------------+---------+\n";
    for (int i = 0; i < mergedCount; ++i) {
        std::cout << "| " << std::setw(3) << (i + 1) << " | "
                  << std::setw(4) << merged[i].slotNo << " | "
                  << std::left << std::setw(10) << merged[i].plate << std::right << " | ";
        printFormattedTime(merged[i].entryTime);
        std::cout << "      | "
                  << std::left << std::setw(7) << getTypeName(merged[i].type) << std::right << " |\n";
    }
    std::cout << "+-----+------+------------+------------+---------+\n";
    std::cout << "Total merged vehicles:   " << mergedCount << "\n";
    std::cout << "Skipped duplicates:      " << skippedDuplicates << "\n";
    std::cout << "Merge comparisons made:  " << comparisons << "\n";
}

// ----------------------------------------------------------------------------
// BONUS 3: PEAK OCCUPANCY BY HOUR ANALYTICS
// ----------------------------------------------------------------------------

// Bonus 3 - Computes vehicle presence per hour across currently parked + exited vehicles
// For currently parked: present from entryTime until 2359.
// For exited vehicles: present from entryTime until exitTime.
// Time Complexity: O(24 * (lotCount + historyCount))
void peakOccupancyByHour() {
    int hourlyCount[24];
    for (int h = 0; h < 24; ++h) {
        hourlyCount[h] = 0;
    }

    // Check currently parked vehicles
    for (int i = 0; i < lotCount; ++i) {
        int entryMin = timeToMinutes(lot[i].entryTime);
        int exitMin = timeToMinutes(2359); // Present until end of day
        for (int h = 0; h < 24; ++h) {
            int hourStart = h * 60;
            int hourEnd = h * 60 + 59;
            // Overlap condition
            if (entryMin <= hourEnd && exitMin >= hourStart) {
                hourlyCount[h]++;
            }
        }
    }

    // Check exited vehicles from history
    for (int i = 0; i < historyCount; ++i) {
        int entryMin = timeToMinutes(history[i].entryTime);
        int exitMin = timeToMinutes(history[i].exitTime);
        for (int h = 0; h < 24; ++h) {
            int hourStart = h * 60;
            int hourEnd = h * 60 + 59;
            if (entryMin <= hourEnd && exitMin >= hourStart) {
                hourlyCount[h]++;
            }
        }
    }

    // Find peak occupancy
    int peakCount = 0;
    for (int h = 0; h < 24; ++h) {
        if (hourlyCount[h] > peakCount) {
            peakCount = hourlyCount[h];
        }
    }

    std::cout << "\n============================================================\n";
    std::cout << "            PEAK OCCUPANCY ANALYSIS BY HOUR\n";
    std::cout << "============================================================\n";
    std::cout << "Hour  Time Window    Vehicles  Distribution Graph\n";
    std::cout << "------------------------------------------------------------\n";
    for (int h = 0; h < 24; ++h) {
        std::cout << std::setw(2) << std::setfill('0') << h << ":00-"
                  << std::setw(2) << std::setfill('0') << h << ":59  "
                  << std::setfill(' ') << std::setw(3) << hourlyCount[h] << "   ";
        for (int b = 0; b < hourlyCount[h]; ++b) {
            std::cout << '*';
        }
        std::cout << "\n";
    }
    std::cout << "------------------------------------------------------------\n";
    std::cout << "Peak Occupancy Count: " << peakCount << " vehicle(s)\n";
    std::cout << "Peak Hour(s): ";
    bool first = true;
    for (int h = 0; h < 24; ++h) {
        if (hourlyCount[h] == peakCount && peakCount > 0) {
            if (!first) std::cout << ", ";
            std::cout << std::setw(2) << std::setfill('0') << h << ":00";
            first = false;
        }
    }
    std::cout << std::setfill(' ') << "\n";
}

// ----------------------------------------------------------------------------
// TESTING HELPER (MENU OPTION 8)
// ----------------------------------------------------------------------------

// Testing Helper - Auto-fills the lot to 60 vehicles with unique dummy plates
// Time Complexity: O(remaining * freeCount)
void autoFillLotForTesting() {
    std::cout << "\n[TESTING HELPER] Auto-filling lot to 60 vehicles...\n";
    if (lotCount >= MAX_LOT) {
        std::cout << "Lot is already full (60/60 occupied)!\n";
        return;
    }
    int initialCount = lotCount;
    int dummyIndex = 1;
    while (lotCount < MAX_LOT) {
        char dummyPlate[11];
        // Format plate as TST001, TST002, etc.
        dummyPlate[0] = 'T';
        dummyPlate[1] = 'S';
        dummyPlate[2] = 'T';
        dummyPlate[3] = static_cast<char>('0' + (dummyIndex / 100) % 10);
        dummyPlate[4] = static_cast<char>('0' + (dummyIndex / 10) % 10);
        dummyPlate[5] = static_cast<char>('0' + (dummyIndex % 10));
        dummyPlate[6] = '\0';

        // Ensure unique
        if (findPlateIndex(lot, lotCount, dummyPlate) == -1) {
            int slot = allocateLowestSlot();
            if (slot == -1) break;
            lot[lotCount].slotNo = slot;
            std::strcpy(lot[lotCount].plate, dummyPlate);
            // Valid staggered entry times: 0800, 0815, 0830, 0845, 0900, etc.
            int hh = 8 + (dummyIndex / 4);
            if (hh > 22) hh = 22;
            int mm = (dummyIndex % 4) * 15;
            lot[lotCount].entryTime = hh * 100 + mm;
            lot[lotCount].type = (dummyIndex % 3) + 1;
            lotCount++;
        }
        dummyIndex++;
    }
    std::cout << "[TESTING HELPER] Successfully added " << (lotCount - initialCount)
              << " test vehicles. Lot is now at full capacity (60/60 occupied).\n";
}

// ----------------------------------------------------------------------------
// SUBMENUS
// ----------------------------------------------------------------------------

void parkSubmenu() {
    while (true) {
        std::cout << "\n--- 2. PARK VEHICLE SUBMENU ---\n";
        std::cout << "  1. Park at next free position (end of array)\n";
        std::cout << "  2. Park VIP / Reserved vehicle at FRONT (position 1)\n";
        std::cout << "  3. Park at user-chosen position\n";
        std::cout << "  4. Park while maintaining sorted order by plate\n";
        std::cout << "  0. Back to Main Menu\n";
        int choice = readInt("Enter choice (0-4): ", 0, 4);
        if (choice == 1) parkAtEnd();
        else if (choice == 2) parkAtFrontVIP();
        else if (choice == 3) parkAtPosition();
        else if (choice == 4) parkSortedByPlate();
        else if (choice == 0) break;
    }
}

void exitSubmenu() {
    while (true) {
        std::cout << "\n--- 3. EXIT VEHICLE SUBMENU ---\n";
        std::cout << "  1. Exit by array position (1..count)\n";
        std::cout << "  2. Exit by license plate number\n";
        std::cout << "  3. Closing sweep (remove vehicles entered before cutoff time)\n";
        std::cout << "  0. Back to Main Menu\n";
        int choice = readInt("Enter choice (0-3): ", 0, 3);
        if (choice == 1) exitByPosition();
        else if (choice == 2) exitByPlate();
        else if (choice == 3) closingSweep();
        else if (choice == 0) break;
    }
}

void searchSubmenu() {
    while (true) {
        std::cout << "\n--- 4. SEARCH VEHICLE SUBMENU ---\n";
        std::cout << "  1. Linear search by license plate (lost-car query)\n";
        std::cout << "  2. Binary search by license plate (precondition checked)\n";
        std::cout << "  3. Find all vehicles of a given type\n";
        std::cout << "  4. Find longest-parked vehicle(s)\n";
        std::cout << "  0. Back to Main Menu\n";
        int choice = readInt("Enter choice (0-4): ", 0, 4);
        if (choice == 1) linearSearchByPlate();
        else if (choice == 2) binarySearchByPlate();
        else if (choice == 3) searchVehiclesByType();
        else if (choice == 4) findLongestParked();
        else if (choice == 0) break;
    }
}

void sortSubmenu() {
    while (true) {
        std::cout << "\n--- 5. SORT LOT SUBMENU ---\n";
        std::cout << "  1. Insertion Sort by license plate (A-Z)\n";
        std::cout << "  2. Selection Sort by entry time (Earliest first)\n";
        std::cout << "  3. Stable Bubble Sort by type then entry time (with stability check)\n";
        std::cout << "  0. Back to Main Menu\n";
        int choice = readInt("Enter choice (0-3): ", 0, 3);
        if (choice == 1) insertionSortByPlate(lot, lotCount, true);
        else if (choice == 2) selectionSortByEntryTime(lot, lotCount);
        else if (choice == 3) stableBubbleSortByTypeAndTime(lot, lotCount);
        else if (choice == 0) break;
    }
}

void feesSubmenu() {
    while (true) {
        std::cout << "\n--- 6. FEES & REVENUE SUBMENU ---\n";
        std::cout << "  1. Preview parking fee for a parked plate (no removal)\n";
        std::cout << "  2. View total revenue collected so far\n";
        std::cout << "  0. Back to Main Menu\n";
        int choice = readInt("Enter choice (0-2): ", 0, 2);
        if (choice == 1) previewFee();
        else if (choice == 2) {
            std::cout << "\n============================================\n";
            std::cout << "  Total Revenue Collected: $" << totalRevenue << "\n";
            std::cout << "============================================\n";
        } else if (choice == 0) break;
    }
}

void bonusSubmenu() {
    while (true) {
        std::cout << "\n--- 7. BONUS FEATURES SUBMENU ---\n";
        std::cout << "  1. Bonus 1: Show available free slot numbers (lowest allocator)\n";
        std::cout << "  2. Bonus 2: Merge Level 1 and Level 2 parking lots\n";
        std::cout << "  3. Bonus 3: Peak occupancy analysis by hour\n";
        std::cout << "  0. Back to Main Menu\n";
        int choice = readInt("Enter choice (0-3): ", 0, 3);
        if (choice == 1) showFreeSlots();
        else if (choice == 2) mergeTwoLevels();
        else if (choice == 3) peakOccupancyByHour();
        else if (choice == 0) break;
    }
}

// ----------------------------------------------------------------------------
// INITIALIZATION (SAMPLE DATA PRELOAD)
// ----------------------------------------------------------------------------

// Preload the 4 sample vehicles specified in assignment in slots 1 to 4:
// 1: MH12AB  entry 1000  type 2  (Slot 1)
// 2: DL08CD  entry 0900  type 1  (Slot 2)
// 3: KA05EF  entry 1130  type 3  (Slot 3)
// 4: TN09GH  entry 0945  type 2  (Slot 4)
// Time Complexity: O(1)
void initLot() {
    lotCount = 4;
    lot[0].slotNo = 1; std::strcpy(lot[0].plate, "MH12AB"); lot[0].entryTime = 1000; lot[0].type = 2;
    lot[1].slotNo = 2; std::strcpy(lot[1].plate, "DL08CD"); lot[1].entryTime = 900;  lot[1].type = 1;
    lot[2].slotNo = 3; std::strcpy(lot[2].plate, "KA05EF"); lot[2].entryTime = 1130; lot[2].type = 3;
    lot[3].slotNo = 4; std::strcpy(lot[3].plate, "TN09GH"); lot[3].entryTime = 945;  lot[3].type = 2;

    // Remaining slots 5 to 60 are free initially
    freeCount = MAX_LOT - lotCount;
    for (int i = 0; i < freeCount; ++i) {
        freeSlots[i] = i + 5;
    }

    initLevel2();
}

// ----------------------------------------------------------------------------
// MAIN FUNCTION & MAIN MENU
// ----------------------------------------------------------------------------
int main() {
    // Initial banner display
    printBanner();
    std::cout << "System initialized with 4 sample vehicles in slots 1 to 4.\n";

    initLot();

    while (true) {
        // Banner printed above main menu every time it is displayed
        printBanner();
        std::cout << "MAIN MENU:\n";
        std::cout << "  1. Show Lot\n";
        std::cout << "  2. Park Vehicle\n";
        std::cout << "  3. Exit Vehicle\n";
        std::cout << "  4. Search\n";
        std::cout << "  5. Sort\n";
        std::cout << "  6. Fees & Revenue\n";
        std::cout << "  7. Bonus Features\n";
        std::cout << "  8. Testing Helper (Auto-fill lot to 60)\n";
        std::cout << "  0. Quit\n";

        int choice = readInt("Select an option (0-8): ", 0, 8);

        if (choice == 1) {
            showLot(lot, lotCount);
        } else if (choice == 2) {
            parkSubmenu();
        } else if (choice == 3) {
            exitSubmenu();
        } else if (choice == 4) {
            searchSubmenu();
        } else if (choice == 5) {
            sortSubmenu();
        } else if (choice == 6) {
            feesSubmenu();
        } else if (choice == 7) {
            bonusSubmenu();
        } else if (choice == 8) {
            autoFillLotForTesting();
        } else if (choice == 0) {
            std::cout << "\nThank you for using Smart Parking Lot Manager. Goodbye!\n";
            break;
        }
    }

    return 0;
}
