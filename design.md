# Smart Parking Allocation & Zone Management System
## Design Documentation

---

## Table of Contents
1. [System Overview](#system-overview)
2. [Zone and Slot Representation](#zone-and-slot-representation)
3. [Allocation Strategy](#allocation-strategy)
4. [Request Lifecycle State Machine](#request-lifecycle-state-machine)
5. [Rollback Design](#rollback-design)
6. [Data Structures Used](#data-structures-used)
7. [Time and Space Complexity](#time-and-space-complexity)

---

## System Overview

The Smart Parking Allocation System is an in-memory parking management solution designed to handle parking requests across multiple zones in a city. The system enforces strict state transitions, supports cross-zone allocation when preferred zones are full, and maintains complete history for analytics and rollback operations.

### Key Features:
- Multi-zone parking management
- Automatic slot allocation with same-zone preference
- Cross-zone allocation with penalty
- State machine-based request lifecycle
- Stack-based rollback mechanism
- Comprehensive analytics

---

## Zone and Slot Representation

### Hierarchical Structure

```
City (ParkingSystem)
    |
    +-- Zone 1
    |     |
    |     +-- Parking Area 1
    |     |     |
    |     |     +-- Slot 101
    |     |     +-- Slot 102
    |     |
    |     +-- Parking Area 2
    |           |
    |           +-- Slot 103
    |
    +-- Zone 2
    |     |
    |     +-- Parking Area 3
    |           |
    |           +-- Slot 201
    |           +-- Slot 202
    |
    +-- Zone 3
          |
          +-- Parking Area 4
                |
                +-- Slot 301
                +-- Slot 302
```

### Implementation Details

#### Zone (Zone.h/cpp)
- **Purpose**: Represents a geographical zone containing multiple parking areas
- **Data Structure**: Array of ParkingArea pointers
- **Key Attributes**:
  - `zoneId`: Unique identifier
  - `parkingAreas[]`: Array of parking areas
  - `areaCount`: Current number of areas
  - `areaCapacity`: Maximum areas allowed

#### ParkingArea (ParkingArea.h/cpp)
- **Purpose**: Groups multiple parking slots within a zone
- **Data Structure**: Array of ParkingSlot pointers
- **Key Attributes**:
  - `areaId`: Unique identifier
  - `zoneId`: Parent zone identifier
  - `slots[]`: Array of parking slots
  - `slotCount`: Current number of slots

#### ParkingSlot (ParkingSlot.h/cpp)
- **Purpose**: Represents individual parking space
- **Data Structure**: Simple object with state
- **Key Attributes**:
  - `slotId`: Unique identifier
  - `zoneId`: Parent zone identifier
  - `available`: Boolean availability status

### Design Rationale
- **Arrays over maps**: Provides predictable memory layout and iteration performance
- **Hierarchical organization**: Mirrors real-world parking structure
- **Pointer-based relationships**: Enables dynamic allocation and efficient access

---

## Allocation Strategy

### Algorithm: Same-Zone Preference with Cross-Zone Fallback

```
FUNCTION allocateSlot(request, currentTime):
    requestedZone = request.getRequestedZone()
    
    // STEP 1: Try same-zone allocation
    slot = findSlotInZone(requestedZone)
    
    IF slot IS NOT NULL:
        slot.occupy()
        request.allocate(requestedZone, slot.id, currentTime, crossZone=false)
        RETURN true
    
    // STEP 2: Try cross-zone allocation
    FOR each zone IN zones:
        IF zone.id != requestedZone:
            slot = findSlotInZone(zone.id)
            IF slot IS NOT NULL:
                slot.occupy()
                request.allocate(zone.id, slot.id, currentTime, crossZone=true)
                RETURN true
    
    // STEP 3: No slots available
    RETURN false
```

### First-Available Slot Strategy

Within a zone, the system uses a first-available strategy:
```
FUNCTION findSlotInZone(zoneId):
    zone = getZone(zoneId)
    
    FOR each area IN zone.parkingAreas:
        FOR each slot IN area.slots:
            IF slot.isAvailable():
                RETURN slot
    
    RETURN NULL
```

### Cross-Zone Penalty
- When a request cannot be fulfilled in the preferred zone
- System automatically allocates in another available zone
- Sets `crossZonePenalty` flag to true
- Customer is notified of transfer and additional charges

---

## Request Lifecycle State Machine

### State Diagram

```
                    +-------------+
                    |  REQUESTED  |
                    +-------------+
                         |
                         | (allocation attempt)
                         |
              +----------+----------+
              |                     |
              v                     v
       +-----------+          +-----------+
       | ALLOCATED |          | CANCELLED |
       +-----------+          +-----------+
              |
              | (auto-transition)
              |
              v
       +-----------+
       |  OCCUPIED |
       +-----------+
              |
       +------+------+
       |             |
       v             v
  +-----------+  +-----------+
  | RELEASED  |  | CANCELLED |
  +-----------+  +-----------+
```

### Valid Transitions

| From State | To State  | Condition |
|------------|-----------|-----------|
| REQUESTED  | ALLOCATED | Slot found and allocated |
| REQUESTED  | CANCELLED | User cancels or no slots available |
| ALLOCATED  | OCCUPIED  | Automatic (immediate) |
| ALLOCATED  | CANCELLED | User cancels before occupation |
| OCCUPIED   | RELEASED  | Automatic after time period |
| OCCUPIED   | CANCELLED | User cancels parking |

### Invalid Transitions (Blocked)
- RELEASED → CANCELLED
- RELEASED → ALLOCATED
- CANCELLED → any state
- Any backward transition

### Implementation (ParkingRequest.cpp)

```cpp
bool transitionTo(RequestState newState) {
    // Valid transitions only
    if (state == REQUESTED && (newState == ALLOCATED || newState == CANCELLED))
        return true;
    if (state == ALLOCATED && (newState == OCCUPIED || newState == CANCELLED))
        return true;
    if (state == OCCUPIED && (newState == RELEASED || newState == CANCELLED))
        return true;
    
    return false; // Invalid transition blocked
}
```

---

## Rollback Design

### Stack-Based Rollback Mechanism

The system uses a **stack data structure** to maintain allocation history and support undo operations.

### Structure (RollbackManager.h/cpp)

```
Stack (LIFO):
    Top -> [Request #5, Slot 302]
           [Request #4, Slot 201]
           [Request #3, Slot 103]
           [Request #2, Slot 102]
    Bottom [Request #1, Slot 101]
```

### AllocationRecord Structure
```cpp
struct AllocationRecord {
    ParkingRequest* request;  // Pointer to the request
    ParkingSlot* slot;        // Pointer to the allocated slot
    AllocationRecord* next;   // Next record in stack
};
```

### Rollback Algorithm

```
FUNCTION rollback(k):
    IF k > stackSize:
        RETURN false
    
    FOR i = 1 TO k:
        record = stack.pop()
        
        // Restore slot availability
        record.slot.release()
        
        // Cancel the request
        record.request.cancel()
    
    RETURN true
```

### Example Scenario

**Initial State:**
- 3 requests allocated: R1, R2, R3
- Stack: [R3] -> [R2] -> [R1]

**User calls `rollback(2)`:**

1. Pop R3: 
   - Release slot 103
   - Set R3 state to CANCELLED
2. Pop R2:
   - Release slot 102
   - Set R2 state to CANCELLED

**Final State:**
- R1 remains allocated
- R2, R3 cancelled
- Slots 102, 103 available
- Stack: [R1]

### Design Rationale
- **Stack (LIFO)**: Natural fit for undo operations
- **Maintains references**: Direct access to requests and slots
- **Efficient**: O(k) time complexity for k rollbacks
- **Memory-efficient**: Only stores pointers, not copies

---

## Data Structures Used

### Summary Table

| Component | Primary Data Structure | Secondary Structure | Reason |
|-----------|----------------------|-------------------|---------|
| ParkingSystem | Array of Zone pointers | Linked list (request history) | Fixed zones, dynamic requests |
| Zone | Array of ParkingArea pointers | - | Fixed areas per zone |
| ParkingArea | Array of ParkingSlot pointers | - | Fixed slots per area |
| RollbackManager | Stack (linked list) | - | LIFO for undo operations |
| Request History | Singly linked list | - | Sequential access, unlimited growth |
| AllocationEngine | Uses arrays from zones | - | Fast iteration |

### Detailed Analysis

#### 1. Arrays for Zone/Area/Slot Storage
**Justification:**
- Fixed capacity known at initialization
- O(1) random access by index
- Cache-friendly sequential access
- No need for dynamic resizing

**Trade-off:**
- Fixed capacity (acceptable for parking lots)
- vs. flexibility of dynamic structures

#### 2. Linked List for Request History
**Justification:**
- Unbounded growth (unknown number of requests)
- Sequential access pattern (display history)
- Easy append operation
- No need for random access

**Structure:**
```cpp
struct RequestNode {
    ParkingRequest* request;
    RequestNode* next;
};
```

#### 3. Stack for Rollback
**Justification:**
- LIFO matches undo semantics
- O(1) push/pop operations
- Memory efficient (only recent allocations)

**Implementation:**
- Linked list-based stack (no size limit)
- Each node stores request-slot pair

---

## Time and Space Complexity

### Operations Analysis

#### 1. Request Parking (createRequest)

**Time Complexity: O(Z × A × S)**
- Z = number of zones
- A = average areas per zone
- S = average slots per area

**Breakdown:**
```
1. Create request object              : O(1)
2. Try same-zone allocation:
   - Iterate through areas            : O(A)
   - Check slots in each area         : O(S)
   - Total same-zone                  : O(A × S)
3. If fails, try cross-zone:
   - Iterate through other zones      : O(Z)
   - For each zone, find slot         : O(A × S)
   - Total cross-zone                 : O(Z × A × S)
4. Push to rollback stack             : O(1)
5. Add to history                     : O(1)

Worst case: O(Z × A × S)
Average case: O(A × S) (if same-zone succeeds)
```

**Space Complexity: O(1)**
- Only creates one request object
- Uses existing slot references

#### 2. Cancel Request (cancelRequest)

**Time Complexity: O(N + A × S)**
- N = total number of requests

**Breakdown:**
```
1. Find request in history            : O(N)
2. Find slot in zone                  : O(A × S)
3. Release slot                       : O(1)
4. Update request state               : O(1)

Total: O(N + A × S)
```

**Space Complexity: O(1)**

#### 3. Rollback K Allocations (rollbackAllocations)

**Time Complexity: O(k)**
- k = number of allocations to rollback

**Breakdown:**
```
For each of k allocations:
    1. Pop from stack                 : O(1)
    2. Release slot                   : O(1)
    3. Cancel request                 : O(1)

Total: O(k)
```

**Space Complexity: O(1)**
- No additional space needed

#### 4. View Zone Status (displayZoneStatus)

**Time Complexity: O(Z × A × S)**

**Breakdown:**
```
For each zone (Z):
    For each area (A):
        For each slot (S):
            Check availability        : O(1)

Total: O(Z × A × S)
```

**Space Complexity: O(1)**

#### 5. Display Analytics (displayAnalytics)

**Time Complexity: O(N)**
- N = total number of requests

**Breakdown:**
```
Traverse entire request history:
    - Calculate duration              : O(1) per request
    - Update zone usage               : O(1) per request
    - Count states                    : O(1) per request

Total: O(N)
```

**Space Complexity: O(Z)**
- Array to track zone usage counts

### Overall System Complexity

#### Space Complexity: O(Z × A × S + N + H)
Where:
- Z × A × S = Total parking slots
- N = Total requests in history
- H = Allocation history stack size (typically H ≤ N)

**Memory Breakdown:**
```
1. Zone/Area/Slot structure          : O(Z × A × S)
2. Request history (linked list)     : O(N)
3. Rollback stack                    : O(H)
4. Engine and system overhead        : O(1)

Total: O(Z × A × S + N + H)
```

#### Common Operation Summary

| Operation | Time Complexity | Space Complexity |
|-----------|----------------|------------------|
| Request Parking | O(Z × A × S) | O(1) |
| Cancel Request | O(N + A × S) | O(1) |
| Release Parking | O(N + A × S) | O(1) |
| Rollback K | O(k) | O(1) |
| View Zones | O(Z × A × S) | O(1) |
| View Requests | O(N) | O(1) |
| Analytics | O(N) | O(Z) |

### Optimization Opportunities

1. **Request Lookup**: Could use hash table for O(1) lookup instead of O(N)
   - Trade-off: Violates "no STL map" constraint
   
2. **Zone Selection**: Could maintain available slot count per zone
   - Would reduce allocation time to O(A × S) instead of O(Z × A × S)
   
3. **Analytics Caching**: Could cache computed statistics
   - Trade-off: More memory, needs invalidation on updates

### Real-World Performance

With typical values:
- Z = 3 zones
- A = 2 areas per zone
- S = 3 slots per area
- Total slots = 3 × 2 × 3 = 18

**Request Parking:**
- Best case: O(6) = finds slot in first area
- Worst case: O(18) = checks all slots
- Acceptable for small-medium parking systems

---

## Conclusion

This design provides a robust, maintainable parking management system using fundamental data structures. The hierarchical organization mirrors real-world parking infrastructure, the state machine ensures data integrity, and the stack-based rollback mechanism enables reliable undo operations. The system achieves reasonable time complexity for all operations while maintaining clear, modular code organization.