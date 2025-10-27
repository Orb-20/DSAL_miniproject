#include <bits/stdc++.h>
using namespace std;

class Booking {
public:
    int roomNo;
    string name;
    string checkInDate;
    string checkOutDate;
    double amount;
    Booking* next;

    Booking(){
        roomNo=0;
        amount=0;
        next=nullptr;
    }
};

//HOTEL SYSTEM CLASS
class HotelSystem {
    private:
    Booking* head;
    stack<Booking> recentCheckouts;
    queue<Booking> waitingList;
    unordered_map<int, Booking*> bookingMap;

    void printLine() {
        cout << "------------------------------------------------------------\n";
    }

    bool isRoomAvailable(int roomNo) {
        return bookingMap.find(roomNo) == bookingMap.end();
    }

    void insertBooking(Booking* newBooking) {
        if (!head) {
            head = newBooking;
            return;
        }
        Booking* temp = head;
        while (temp->next){
            temp = temp->next;
        }
        temp->next = newBooking;
    }

    void removeBooking(int roomNo) {
        Booking* temp = head;
        Booking* prev = nullptr;

        while (temp && temp->roomNo != roomNo) {
            prev = temp;
            temp = temp->next;
        }

        if (!temp){
            return;
        }

        if (!prev){
            head = temp->next;
        }else{
            prev->next = temp->next;
        }
        delete temp;
    }

    public:
    HotelSystem(){
        head=nullptr;
    }

    //Core Functionalities
    void addBooking() {
        Booking* newBooking = new Booking;
        cout << "\nEnter Customer Name: ";
        cin.ignore();
        getline(cin, newBooking->name);
        cout << "Enter Room Number: ";
        cin >> newBooking->roomNo;
        cout << "Enter Check-in Date (DD/MM/YYYY): ";
        cin >> newBooking->checkInDate;
        cout << "Enter Check-out Date (DD/MM/YYYY): ";
        cin >> newBooking->checkOutDate;
        cout << "Enter Amount: ₹";
        cin >> newBooking->amount;
        newBooking->next = nullptr;

        if (isRoomAvailable(newBooking->roomNo)) {
            insertBooking(newBooking);
            bookingMap[newBooking->roomNo] = newBooking;
            cout << "\nBooking confirmed successfully for " << newBooking->name << "!\n";
        } else {
            cout << "\nRoom not available! Added to waiting list.\n";
            waitingList.push(*newBooking);
            delete newBooking;
        }
    }

    void displayBookings() {
        if (!head) {
            cout << "\nNo active bookings found.\n";
            return;
        }

        printLine();
        cout << left << setw(10) << "Room No" << setw(20) << "Customer Name"
             << setw(15) << "Check-in" << setw(15) << "Check-out" << setw(10)
             << "Amount\n";
        printLine();

        Booking* temp = head;
        while (temp) {
            cout << left << setw(10) << temp->roomNo << setw(20) << temp->name
                 << setw(15) << temp->checkInDate << setw(15) << temp->checkOutDate
                 << "₹" << temp->amount << "\n";
            temp = temp->next;
        }
        printLine();
    }

    void searchBooking() {
        int roomNo;
        cout << "\nEnter Room Number to search: ";
        cin >> roomNo;

        if (bookingMap.find(roomNo) != bookingMap.end()) {
            Booking* b = bookingMap[roomNo];
            printLine();
            cout << "Booking Found!\n";
            cout << "Name: " << b->name << "\nRoom No: " << b->roomNo
                 << "\nCheck-in: " << b->checkInDate
                 << "\nCheck-out: " << b->checkOutDate
                 << "\nAmount: ₹" << b->amount << "\n";
            printLine();
        } else {
            cout << "\nNo booking found for this room number.\n";
        }
    }

    void sortBookings() {}

    void cancelBooking() {}

    void checkOutCustomer() 
    {
        int roomNo;
        cout << "\nEnter Room Number for checkout: ";
        cin >> roomNo;

        if (bookingMap.find(roomNo) != bookingMap.end()) 
        {
            Booking* b = bookingMap[roomNo];
            recentCheckouts.push(*b);
            bookingMap.erase(roomNo);
            removeBooking(roomNo);
            cout << "\n Checkout completed for " << b->name << "!\n";
        } 
        else
        {
            cout << "\n Room not found in active bookings.\n";
        }
    }


    void showRecentCheckouts() 
    {
        if (recentCheckouts.empty()) 
        {
            cout << "\nNo recent check-outs.\n";
            return;
        }

        printLine();
        cout << "Recent Checkouts:\n";
        printLine();

        stack<Booking> temp = recentCheckouts;
        while (!temp.empty()) 
        {
            Booking b = temp.top();
            temp.pop();
            cout << "Name: " << b.name << " | Room: " << b.roomNo
                 << " | Amount: ₹" << b.amount << endl;
        }
        printLine();
    }

    void showWaitingList() {}

};

//MAIN FUNCTION
int main() {
    HotelSystem hotel;
    int choice;

    do {
        cout << "\n=========== HOTEL BOOKING SYSTEM ===========\n";
        cout << "1. Add New Booking\n";
        cout << "2. Display All Bookings\n";
        cout << "3. Search Booking\n";
        cout << "4. Sort Bookings\n";
        cout << "5. Cancel Booking\n";
        cout << "6. Check-out Customer\n";
        cout << "7. Show Recent Check-outs\n";
        cout << "8. Show Waiting List\n";
        cout << "9. Exit\n";
        cout << "============================================\n";
        cout << "Enter your choice: ";
        cin >> choice;

        switch (choice) {
            case 1: hotel.addBooking(); break;
            case 2: hotel.displayBookings(); break;
            case 3: hotel.searchBooking(); break;
            case 4: hotel.sortBookings(); break;
            case 5: hotel.cancelBooking(); break;
            case 6: hotel.checkOutCustomer(); break;
            case 7: hotel.showRecentCheckouts(); break;
            case 8: hotel.showWaitingList(); break;
            case 9: cout << "\nExiting system... Thank you!\n"; break;
            default: cout << "\nInvalid choice. Try again!\n";
        }
    } while (choice != 9);

    return 0;
}
