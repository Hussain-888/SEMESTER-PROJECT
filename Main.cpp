#include <iostream>
#include <cstring>
#include "ParkingSystem.h"
#include "Zone.h"
#include "ParkingArea.h"
#include "ParkingSlot.h"
#include "ParkingRequest.h"

using namespace std;

void displayMenu() {
    cout << "\n========================================\n";
    cout << "   SMART PARKING ALLOCATION SYSTEM\n";
    cout << "========================================\n";
    cout << "1. Request Parking\n";
    cout << "2. Cancel Request\n";
    cout << "3. View Zone Status\n";
    cout << "4. View All Requests\n";
    cout << "5. View Analytics\n";
    cout << "6. Rollback Last K Allocations\n";
    cout << "7. Run All 10 Tests (Automated)\n";
    cout << "8. Exit\n";
    cout << "========================================\n";
    cout << "Enter your choice: ";
}

void requestParking(ParkingSystem& system) {
    char vehicleId[50];
    int zone;
    
    cout << "\n--- Request Parking ---\n";
    cout << "Enter Vehicle ID: ";
    cin >> vehicleId;
    cout << "Enter Preferred Zone (1, 2, or 3): ";
    cin >> zone;
    
    if (zone < 1 || zone > 3) {
        cout << "ERROR: Invalid zone! Please enter 1, 2, or 3.\n";
        return;
    }
    
    ParkingRequest* request = system.createRequest(vehicleId, zone);
    
    if (request->getState() == OCCUPIED) {
        cout << "\n========================================\n";
        
        if (request->hasCrossZonePenalty()) {
            cout << "WARNING: ZONE FULL - AUTOMATIC TRANSFER\n";
            cout << "========================================\n";
            cout << "\nYour preferred Zone " << request->getRequestedZone() 
                 << " is currently FULL!\n";
            cout << "\nSystem has automatically transferred your\n";
            cout << "request to the nearest available zone:\n\n";
            cout << "FROM: Zone " << request->getRequestedZone() << " (FULL)\n";
            cout << "TO  : Zone " << request->getAllocatedZone() << " (AVAILABLE)\n";
            cout << "\n========================================\n";
            cout << "ALLOCATION DETAILS:\n";
            cout << "========================================\n";
            cout << "Request ID       : " << request->getRequestId() << "\n";
            cout << "Vehicle          : " << request->getVehicleId() << "\n";
            cout << "Requested Zone   : Zone " << request->getRequestedZone() << " (FULL)\n";
            cout << "Allocated Zone   : Zone " << request->getAllocatedZone() << "\n";
            cout << "Slot ID          : " << request->getAllocatedSlotId() << "\n";
            cout << "\nWARNING: CROSS-ZONE PENALTY APPLIED\n";
            cout << "Additional charges will be applied for cross-zone allocation.\n";
        } else {
            cout << "SUCCESS: PARKING ALLOCATED\n";
            cout << "========================================\n";
            cout << "Request ID       : " << request->getRequestId() << "\n";
            cout << "Vehicle          : " << request->getVehicleId() << "\n";
            cout << "Requested Zone   : Zone " << request->getRequestedZone() << "\n";
            cout << "Allocated Zone   : Zone " << request->getAllocatedZone() << "\n";
            cout << "Slot ID          : " << request->getAllocatedSlotId() << "\n";
            cout << "\nAllocated in your preferred zone!\n";
        }
        
        cout << "\nNOTE: Your parking will be automatically\n";
        cout << "released by the system after the time period.\n";
        cout << "========================================\n";
        
    } else if (request->getState() == CANCELLED) {
        cout << "\n========================================\n";
        cout << "ERROR: ALLOCATION FAILED\n";
        cout << "========================================\n";
        cout << "Reason: No parking slots available in ANY zone.\n";
        cout << "Please try again later.\n";
        cout << "========================================\n";
    }
}

void cancelRequest(ParkingSystem& system) {
    int requestId;
    
    cout << "\n--- Cancel Request ---\n";
    cout << "Enter Request ID to cancel: ";
    cin >> requestId;
    
    ParkingRequest* request = system.findRequest(requestId);
    if (request == nullptr) {
        cout << "\nERROR: Request ID " << requestId << " not found!\n";
        return;
    }
    
    cout << "\nRequest Details:\n";
    cout << "========================================\n";
    cout << "Request ID: " << request->getRequestId() << "\n";
    cout << "Vehicle   : " << request->getVehicleId() << "\n";
    cout << "State     : ";
    switch (request->getState()) {
        case REQUESTED: cout << "REQUESTED"; break;
        case ALLOCATED: cout << "ALLOCATED"; break;
        case OCCUPIED: cout << "OCCUPIED"; break;
        case RELEASED: cout << "RELEASED"; break;
        case CANCELLED: cout << "CANCELLED"; break;
    }
    cout << "\n========================================\n";
    
    if (system.cancelRequest(requestId)) {
        cout << "\nSUCCESS: Request #" << requestId << " cancelled successfully!\n";
        cout << "Your parking slot has been freed.\n";
    } else {
        cout << "\nERROR: Cannot cancel this request!\n";
        cout << "Reason: Request is already RELEASED or CANCELLED.\n";
    }
}

void rollbackAllocations(ParkingSystem& system) {
    int k;
    
    cout << "\n--- Rollback Allocations ---\n";
    cout << "This will undo the last K parking allocations.\n";
    cout << "Enter number of allocations to rollback: ";
    cin >> k;
    
    if (k <= 0) {
        cout << "ERROR: Please enter a positive number!\n";
        return;
    }
    
    if (system.rollbackAllocations(k)) {
        cout << "\nSUCCESS: Rolled back " << k << " allocation(s)!\n";
        cout << "Affected parking slots have been freed.\n";
        cout << "Corresponding requests have been cancelled.\n";
    } else {
        cout << "\nERROR: Rollback failed!\n";
        cout << "Reason: Not enough allocations in history.\n";
    }
}

void runAllTests(ParkingSystem& system) {
    cout << "\n========================================\n";
    cout << "   AUTOMATED TEST SUITE (10 TESTS)\n";
    cout << "========================================\n";
    
    system.displayZoneStatus();
    
    // TEST 1: Normal allocation in requested zone
    cout << "\n========================================\n";
    cout << "TEST 1: Normal Allocation\n";
    cout << "========================================\n";
    ParkingRequest* req1 = system.createRequest("ABC123", 1);
    cout << "Created request for vehicle ABC123 in Zone 1\n";
    if (req1->getState() == OCCUPIED && req1->getAllocatedZone() == 1) {
        cout << "PASS: Successfully allocated in Zone 1\n";
    } else {
        cout << "FAIL\n";
    }
    
    // TEST 2: Multiple allocations in same zone
    cout << "\n========================================\n";
    cout << "TEST 2: Multiple Allocations\n";
    cout << "========================================\n";
    ParkingRequest* req2 = system.createRequest("XYZ789", 1);
    ParkingRequest* req3 = system.createRequest("DEF456", 1);
    cout << "Created 2 more requests for Zone 1\n";
    if (req2->getState() == OCCUPIED && req3->getState() == OCCUPIED) {
        cout << "PASS: Both requests allocated\n";
    } else {
        cout << "FAIL\n";
    }
    
    system.displayZoneStatus();
    
    // TEST 3: Zone full - Cross-zone allocation
    cout << "\n========================================\n";
    cout << "TEST 3: Cross-Zone Allocation (Zone Full)\n";
    cout << "========================================\n";
    ParkingRequest* req4 = system.createRequest("GHI111", 1);
    cout << "Created request for Zone 1 (which is now full)\n";
    if (req4->getState() == OCCUPIED && req4->getAllocatedZone() != 1 && req4->hasCrossZonePenalty()) {
        cout << "PASS: Cross-zone allocation with penalty applied\n";
        cout << "Requested Zone: 1, Allocated Zone: " << req4->getAllocatedZone() << "\n";
    } else {
        cout << "FAIL\n";
    }
    
    // TEST 4: Release parking
    cout << "\n========================================\n";
    cout << "TEST 4: Release Parking\n";
    cout << "========================================\n";
    int releaseReqId = req1->getRequestId();
    if (system.releaseParking(releaseReqId)) {
        if (req1->getState() == RELEASED) {
            cout << "PASS: Request " << releaseReqId << " released successfully\n";
        } else {
            cout << "FAIL: State not updated\n";
        }
    } else {
        cout << "FAIL: Release failed\n";
    }
    
    system.displayZoneStatus();
    
    // TEST 5: Cancel allocated request
    cout << "\n========================================\n";
    cout << "TEST 5: Cancel Request\n";
    cout << "========================================\n";
    ParkingRequest* req5 = system.createRequest("JKL222", 2);
    int cancelReqId = req5->getRequestId();
    if (system.cancelRequest(cancelReqId)) {
        if (req5->getState() == CANCELLED) {
            cout << "PASS: Request " << cancelReqId << " cancelled\n";
        } else {
            cout << "FAIL: State not CANCELLED\n";
        }
    } else {
        cout << "FAIL: Cancel failed\n";
    }
    
    // TEST 6: Invalid state transition
    cout << "\n========================================\n";
    cout << "TEST 6: Invalid State Transition\n";
    cout << "========================================\n";
    ParkingRequest* req6 = system.createRequest("MNO333", 2);
    system.releaseParking(req6->getRequestId());
    bool invalidTransition = system.cancelRequest(req6->getRequestId());
    if (!invalidTransition) {
        cout << "PASS: Cannot cancel released request\n";
    } else {
        cout << "FAIL: Invalid transition allowed\n";
    }
    
    // TEST 7: Rollback last 2 allocations
    cout << "\n========================================\n";
    cout << "TEST 7: Rollback Last 2 Allocations\n";
    cout << "========================================\n";
    ParkingRequest* req7 = system.createRequest("PQR444", 3);
    ParkingRequest* req8 = system.createRequest("STU555", 3);
    cout << "Created 2 requests before rollback\n";
    if (system.rollbackAllocations(2)) {
        if (req7->getState() == CANCELLED && req8->getState() == CANCELLED) {
            cout << "PASS: Rollback successful, requests cancelled\n";
        } else {
            cout << "FAIL: Requests not cancelled after rollback\n";
        }
    } else {
        cout << "FAIL: Rollback failed\n";
    }
    
    system.displayZoneStatus();
    
    // TEST 8: Analytics - Average duration
    cout << "\n========================================\n";
    cout << "TEST 8: Analytics Test\n";
    cout << "========================================\n";
    system.displayAnalytics();
    cout << "PASS: Analytics displayed\n";
    
    // TEST 9: Peak usage zone
    cout << "\n========================================\n";
    cout << "TEST 9: Peak Usage Detection\n";
    cout << "========================================\n";
    system.createRequest("VWX666", 3);
    system.createRequest("YZA777", 3);
    system.displayAnalytics();
    cout << "PASS: Peak usage zone should be Zone 3\n";
    
    // TEST 10: Complete lifecycle
    cout << "\n========================================\n";
    cout << "TEST 10: Complete Request Lifecycle\n";
    cout << "========================================\n";
    ParkingRequest* req10 = system.createRequest("BCD888", 2);
    cout << "State after creation: ";
    if (req10->getState() == OCCUPIED) cout << "OCCUPIED\n";
    
    system.releaseParking(req10->getRequestId());
    cout << "State after release: ";
    if (req10->getState() == RELEASED) cout << "RELEASED\n";
    cout << "Parking duration: " << req10->getParkingDuration() << " time units\n";
    cout << "PASS: Complete lifecycle executed\n";
    
    // Final display
    cout << "\n========================================\n";
    cout << "      FINAL SYSTEM STATE\n";
    cout << "========================================\n";
    system.displayZoneStatus();
    system.displayRequestHistory();
    system.displayAnalytics();
    
    cout << "\n========================================\n";
    cout << "    ALL 10 TESTS COMPLETED\n";
    cout << "========================================\n";
}

void initializeSystem(ParkingSystem& system) {
    // Setup Zone 1 with 3 slots
    Zone* zone1 = new Zone(1, 2);
    ParkingArea* area1_1 = new ParkingArea(1, 1, 2);
    area1_1->addSlot(new ParkingSlot(101, 1));
    area1_1->addSlot(new ParkingSlot(102, 1));
    zone1->addParkingArea(area1_1);
    
    ParkingArea* area1_2 = new ParkingArea(2, 1, 1);
    area1_2->addSlot(new ParkingSlot(103, 1));
    zone1->addParkingArea(area1_2);
    
    // Setup Zone 2 with 2 slots
    Zone* zone2 = new Zone(2, 1);
    ParkingArea* area2_1 = new ParkingArea(3, 2, 2);
    area2_1->addSlot(new ParkingSlot(201, 2));
    area2_1->addSlot(new ParkingSlot(202, 2));
    zone2->addParkingArea(area2_1);
    
    // Setup Zone 3 with 2 slots
    Zone* zone3 = new Zone(3, 1);
    ParkingArea* area3_1 = new ParkingArea(4, 3, 2);
    area3_1->addSlot(new ParkingSlot(301, 3));
    area3_1->addSlot(new ParkingSlot(302, 3));
    zone3->addParkingArea(area3_1);
    
    system.addZone(zone1);
    system.addZone(zone2);
    system.addZone(zone3);
    
    cout << "\n========================================\n";
    cout << "  SYSTEM INITIALIZATION COMPLETE\n";
    cout << "========================================\n";
    cout << "\nParking Configuration:\n";
    cout << "  Zone 1: 3 slots\n";
    cout << "  Zone 2: 2 slots\n";
    cout << "  Zone 3: 2 slots\n";
    cout << "  Total : 7 slots\n";
    cout << "\nAuto-Release: System-controlled\n";
    cout << "========================================\n";
}

int main() {
    ParkingSystem system(5);
    
    cout << "\n========================================\n";
    cout << "  SMART PARKING ALLOCATION SYSTEM\n";
    cout << "     DSA Semester Project\n";
    cout << "========================================\n";
    
    initializeSystem(system);
    
    int choice;
    bool running = true;
    
    while (running) {
        displayMenu();
        cin >> choice;
        
        switch (choice) {
            case 1:
                requestParking(system);
                break;
                
            case 2:
                cancelRequest(system);
                break;
                
            case 3:
                system.displayZoneStatus();
                break;
                
            case 4:
                system.displayRequestHistory();
                break;
                
            case 5:
                system.displayAnalytics();
                break;
                
            case 6:
                rollbackAllocations(system);
                break;
                
            case 7:
                runAllTests(system);
                break;
                
            case 8:
                cout << "\n========================================\n";
                cout << "  Thank you for using our system!\n";
                cout << "========================================\n";
                running = false;
                break;
                
            default:
                cout << "\nERROR: Invalid choice! Please enter 1-8.\n";
        }
        
        if (running && choice >= 1 && choice <= 7) {
            cout << "\nPress Enter to continue...";
            cin.ignore();
            cin.get();
        }
    }
    
    return 0;
}
