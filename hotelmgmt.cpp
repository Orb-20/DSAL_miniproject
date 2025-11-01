#include <iostream>
#include <string>
#include <iomanip>
#include <queue>
#include <stack>
#include <unordered_map>
#include <vector>
#include <limits>
#include <algorithm>
#include <sstream>
#include <fstream>
#include <ctime>
#include <climits>
#include <cctype>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <sys/ioctl.h>
    #include <unistd.h>
#endif

using namespace std;

// 🎨 Console Color Codes
#define RESET       "\033[0m"
#define RED         "\033[31m"
#define GREEN       "\033[32m"
#define YELLOW      "\033[33m"
#define BLUE        "\033[34m"
#define MAGENTA     "\033[35m"
#define CYAN        "\033[36m"
#define BOLDWHITE   "\033[1m\033[37m"
#define UNDERLINE   "\033[4m"
#define BG_PURPLE   "\033[45m"
#define BG_BLUE     "\033[44m"
#define BG_CYAN     "\033[46m"
#define BG_GREEN    "\033[42m"

// Room pricing structure
const int TOTAL_ROOMS = 10;
const double ROOM_PRICES[TOTAL_ROOMS] = {5000, 5000, 7000, 7000, 10000, 5000, 7000, 10000, 5000, 7000};

// Cross-platform terminal utilities
int getTerminalWidth() {
#ifdef _WIN32
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }
    return 80;
#else
    struct winsize w;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &w) == 0 && w.ws_col > 0) {
        return w.ws_col;
    }
    const char* cols = getenv("COLUMNS");
    if (cols) {
        int c = atoi(cols);
        if (c > 0) return c;
    }
    return 80;
#endif
}

void clearScreen() {
#ifdef _WIN32
    system("cls");
#else
    system("clear");
#endif
}

void centerText(const string& text) {
    int width = getTerminalWidth();
    int pad = max(0, (width - (int)text.size()) / 2);
    cout << string(pad, ' ') << text << endl;
}

// Date utility class
class Date {
public:
    int day, month, year;

    Date() : day(0), month(0), year(0) {}
    Date(int d, int m, int y) : day(d), month(m), year(y) {}

    bool isValid() const {
        if (year < 2024 || year > 2100) return false;
        if (month < 1 || month > 12) return false;
        if (day < 1 || day > 31) return false;

        int daysInMonth[] = {0, 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
        
        if (month == 2 && ((year % 4 == 0 && year % 100 != 0) || (year % 400 == 0))) {
            daysInMonth[2] = 29;
        }

        return day <= daysInMonth[month];
    }

    bool operator<(const Date& other) const {
        if (year != other.year) return year < other.year;
        if (month != other.month) return month < other.month;
        return day < other.day;
    }

    bool operator==(const Date& other) const {
        return day == other.day && month == other.month && year == other.year;
    }

    bool operator<=(const Date& other) const {
        return *this < other || *this == other;
    }

    int daysBetween(const Date& other) const {
        int days1 = year * 365 + month * 30 + day;
        int days2 = other.year * 365 + other.month * 30 + other.day;
        return days2 - days1;
    }

    string toString() const {
        ostringstream oss;
        oss << setfill('0') << setw(2) << day << "/" 
            << setw(2) << month << "/" << year;
        return oss.str();
    }
};

class Booking {
public:
    int roomNo;
    string name;
    Date checkInDate;
    Date checkOutDate;
    double totalAmount;
    int numDays;
    string roomType;
    Booking* next;

    Booking() {
        roomNo = 0;
        totalAmount = 0;
        numDays = 0;
        next = nullptr;
    }
};

class HotelSystem {
private:
    string hotelName;
    int totalRooms;
    int bookedRooms;
    Booking* head;
    stack<Booking> recentCheckouts;
    queue<Booking> waitingList;
    unordered_map<int, Booking*> roomBookingMap;
    unordered_map<string, Booking*> nameBookingMap;
    bool roomOccupied[TOTAL_ROOMS + 1];

    void printLine(string color = CYAN) {
        cout << color << "----------------------------------------------------------------" << RESET << "\n";
    }

    bool isRoomAvailable(int roomNo) {
        return roomNo >= 1 && roomNo <= TOTAL_ROOMS && !roomOccupied[roomNo];
    }

    int findAvailableRoom() {
        for (int i = 1; i <= TOTAL_ROOMS; i++) {
            if (!roomOccupied[i]) {
                return i;
            }
        }
        return -1;
    }

    int findAvailableRoomByPrice(double price) {
        for (int i = 1; i <= TOTAL_ROOMS; i++) {
            if (!roomOccupied[i] && ROOM_PRICES[i - 1] == price) {
                return i;
            }
        }
        return -1;
    }

    void insertBookingNode(Booking* newBooking) {
        if (!head) {
            head = newBooking;
            return;
        }
        Booking* temp = head;
        while (temp->next) {
            temp = temp->next;
        }
        temp->next = newBooking;
    }

    void removeBookingNode(int roomNo) {
        Booking* temp = head;
        Booking* prev = nullptr;

        while (temp && temp->roomNo != roomNo) {
            prev = temp;
            temp = temp->next;
        }

        if (!temp) return;

        if (!prev) {
            head = temp->next;
        } else {
            prev->next = temp->next;
        }
        delete temp;
    }

    bool assignFromWaitingList() {
        if (waitingList.empty()) return false;

        queue<Booking> newQueue;
        bool assigned = false;

        while (!waitingList.empty()) {
            Booking w = waitingList.front();
            waitingList.pop();
            
            int availableRoom = findAvailableRoom();
            if (!assigned && availableRoom != -1) {
                Booking* newNode = new Booking();
                newNode->roomNo = availableRoom;
                newNode->name = w.name;
                newNode->checkInDate = w.checkInDate;
                newNode->checkOutDate = w.checkOutDate;
                newNode->numDays = w.numDays;
                newNode->roomType = w.roomType;
                newNode->totalAmount = ROOM_PRICES[availableRoom - 1] * w.numDays;
                newNode->next = nullptr;
                
                insertBookingNode(newNode);
                roomOccupied[availableRoom] = true;
                roomBookingMap[availableRoom] = newNode;
                nameBookingMap[newNode->name] = newNode;
                bookedRooms++;
                
                cout << GREEN << "\n[Waiting List] Assigned room " << availableRoom 
                     << " to " << newNode->name << " (Rs" << fixed << setprecision(2) 
                     << newNode->totalAmount << " for " << newNode->numDays << " days)" << RESET << "\n";
                assigned = true;
            } else {
                newQueue.push(w);
            }
        }

        waitingList = newQueue;
        saveToFile();
        return assigned;
    }

    Date parseDate(const string& dateStr) {
        Date d;
        char slash1, slash2;
        stringstream ss(dateStr);
        ss >> d.day >> slash1 >> d.month >> slash2 >> d.year;
        
        if (ss.fail() || slash1 != '/' || slash2 != '/') {
            return Date();
        }
        return d;
    }

    Date readDate(const string& prompt) {
        string input;
        Date d;
        while (true) {
            cout << CYAN << prompt << RESET;
            cout.flush();
            if (!getline(cin, input)) {
                if (cin.eof()) {
                    cout << "\n" << YELLOW << "End of input detected. Exiting..." << RESET << "\n";
                    exit(0);
                }
                cin.clear();
                continue;
            }
            
            d = parseDate(input);
            if (d.isValid()) {
                return d;
            }
            cout << RED << "Invalid date format or invalid date. Please use DD/MM/YYYY format." << RESET << "\n";
        }
    }

    Date getCurrentDate() {
        time_t now = time(0);
        tm* ltm = localtime(&now);
        return Date(ltm->tm_mday, 1 + ltm->tm_mon, 1900 + ltm->tm_year);
    }

    int readInt(const string& prompt, int minVal = INT_MIN, int maxVal = INT_MAX) {
        int x;
        while (true) {
            cout << CYAN << prompt << RESET;
            cout.flush();
            if (cin >> x) {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                if (x >= minVal && x <= maxVal) {
                    return x;
                }
                cout << RED << "Please enter a value between " << minVal << " and " << maxVal << "." << RESET << "\n";
            } else {
                if (cin.eof()) {
                    cout << "\n" << YELLOW << "End of input detected. Exiting..." << RESET << "\n";
                    exit(0);
                }
                cout << RED << "Invalid input. Please enter a number." << RESET << "\n";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
            }
        }
    }

    string readNonEmptyString(const string& prompt) {
        string input;
        while (true) {
            cout << CYAN << prompt << RESET;
            cout.flush();
            if (!getline(cin, input)) {
                if (cin.eof()) {
                    cout << "\n" << YELLOW << "End of input detected. Exiting..." << RESET << "\n";
                    exit(0);
                }
                cin.clear();
                continue;
            }
            
            size_t start = input.find_first_not_of(" \t");
            size_t end = input.find_last_not_of(" \t");
            
            if (start != string::npos && end != string::npos) {
                input = input.substr(start, end - start + 1);
                if (!input.empty()) {
                    return input;
                }
            }
            cout << RED << "Input cannot be empty. Please try again." << RESET << "\n";
        }
    }

    string toLowerCase(const string& str) {
        string result = str;
        for (char& c : result) {
            c = tolower(c);
        }
        return result;
    }

    void saveToFile() {
        string filename = hotelName + "_bookings.txt";
        for (char& c : filename) {
            if (c == ' ') c = '_';
        }
        
        ofstream fout(filename, ios::trunc);
        if (!fout) {
            cout << RED << "Error: Could not save to file." << RESET << "\n";
            return;
        }
        
        Booking* temp = head;
        while (temp) {
            fout << temp->name << ","
                 << temp->roomNo << ","
                 << temp->roomType << ","
                 << temp->checkInDate.toString() << ","
                 << temp->checkOutDate.toString() << ","
                 << temp->numDays << ","
                 << temp->totalAmount << endl;
            temp = temp->next;
        }
        fout.close();
    }

    void loadFromFile() {
        string filename = hotelName + "_bookings.txt";
        for (char& c : filename) {
            if (c == ' ') c = '_';
        }
        
        ifstream fin(filename);
        if (!fin) return;
        
        string line;
        while (getline(fin, line)) {
            if (line.empty()) continue;
            
            stringstream ss(line);
            string name, roomType, checkIn, checkOut;
            int roomNo, numDays;
            double amount;
            
            getline(ss, name, ',');
            ss >> roomNo;
            ss.ignore();
            getline(ss, roomType, ',');
            getline(ss, checkIn, ',');
            getline(ss, checkOut, ',');
            ss >> numDays;
            ss.ignore();
            ss >> amount;
            
            if (roomNo >= 1 && roomNo <= TOTAL_ROOMS && !roomOccupied[roomNo]) {
                Booking* b = new Booking();
                b->name = name;
                b->roomNo = roomNo;
                b->roomType = roomType;
                b->checkInDate = parseDate(checkIn);
                b->checkOutDate = parseDate(checkOut);
                b->numDays = numDays;
                b->totalAmount = amount;
                b->next = nullptr;
                
                insertBookingNode(b);
                roomOccupied[roomNo] = true;
                roomBookingMap[roomNo] = b;
                nameBookingMap[b->name] = b;
                bookedRooms++;
            }
        }
        fin.close();
    }

public:
    HotelSystem(const string& name = "Grand Hotel", int rooms = TOTAL_ROOMS) 
        : hotelName(name), totalRooms(rooms), bookedRooms(0) {
        head = nullptr;
        for (int i = 0; i <= TOTAL_ROOMS; i++) {
            roomOccupied[i] = false;
        }
        loadFromFile();
    }

    ~HotelSystem() {
        Booking* temp = head;
        while (temp) {
            Booking* nxt = temp->next;
            delete temp;
            temp = nxt;
        }
    }

    string getHotelName() const {
        return hotelName;
    }

    int getAvailableRooms() const {
        return totalRooms - bookedRooms;
    }

    int getTotalRooms() const {
        return totalRooms;
    }

    void showAvailableRooms() {
        clearScreen();
        centerText("========= AVAILABLE ROOMS =========");
        cout << "\n";
        
        vector<int> standard, deluxe, suite;
        
        for (int i = 1; i <= TOTAL_ROOMS; i++) {
            if (!roomOccupied[i]) {
                if (ROOM_PRICES[i - 1] == 5000) {
                    standard.push_back(i);
                } else if (ROOM_PRICES[i - 1] == 7000) {
                    deluxe.push_back(i);
                } else if (ROOM_PRICES[i - 1] == 10000) {
                    suite.push_back(i);
                }
            }
        }
        
        if (standard.empty() && deluxe.empty() && suite.empty()) {
            cout << RED << "No rooms currently available." << RESET << "\n";
        } else {
            cout << YELLOW << "STANDARD ROOMS (Rs 5,000/night):" << RESET << "\n";
            if (standard.empty()) {
                cout << RED << "  No standard rooms available" << RESET << "\n";
            } else {
                cout << GREEN << "  Available: " << RESET;
                for (size_t i = 0; i < standard.size(); i++) {
                    cout << standard[i];
                    if (i < standard.size() - 1) cout << ", ";
                }
                cout << "\n";
            }
            
            cout << YELLOW << "\nDELUXE ROOMS (Rs 7,000/night):" << RESET << "\n";
            if (deluxe.empty()) {
                cout << RED << "  No deluxe rooms available" << RESET << "\n";
            } else {
                cout << GREEN << "  Available: " << RESET;
                for (size_t i = 0; i < deluxe.size(); i++) {
                    cout << deluxe[i];
                    if (i < deluxe.size() - 1) cout << ", ";
                }
                cout << "\n";
            }
            
            cout << YELLOW << "\nSUITE ROOMS (Rs 10,000/night):" << RESET << "\n";
            if (suite.empty()) {
                cout << RED << "  No suite rooms available" << RESET << "\n";
            } else {
                cout << GREEN << "  Available: " << RESET;
                for (size_t i = 0; i < suite.size(); i++) {
                    cout << suite[i];
                    if (i < suite.size() - 1) cout << ", ";
                }
                cout << "\n";
            }
        }
        printLine(BLUE);
    }

    void addBooking() {
        clearScreen();
        centerText("========= NEW BOOKING =========");
        cout << "\n";
        
        Booking* newBooking = new Booking();

        newBooking->name = readNonEmptyString("Enter Customer Name: ");
        
        Date today = getCurrentDate();
        
        while (true) {
            newBooking->checkInDate = readDate("Enter Check-in Date (DD/MM/YYYY): ");
            if (today <= newBooking->checkInDate) {
                break;
            }
            cout << RED << "Check-in date cannot be in the past. Please enter a future date." << RESET << "\n";
        }

        while (true) {
            newBooking->checkOutDate = readDate("Enter Check-out Date (DD/MM/YYYY): ");
            if (newBooking->checkInDate < newBooking->checkOutDate) {
                break;
            }
            cout << RED << "Check-out date must be after check-in date. Please try again." << RESET << "\n";
        }

        newBooking->numDays = newBooking->checkInDate.daysBetween(newBooking->checkOutDate);
        if (newBooking->numDays <= 0) {
            newBooking->numDays = 1;
        }

        cout << YELLOW << "\n--- Select Room Type ---" << RESET << "\n";
        cout << "1. Standard Room - Rs 5,000 per night\n";
        cout << "2. Deluxe Room - Rs 7,000 per night\n";
        cout << "3. Suite Room - Rs 10,000 per night\n";
        
        int roomChoice;
        double selectedPrice;
        string roomType;
        
        while (true) {
            roomChoice = readInt("Enter your choice (1-3): ", 1, 3);
            
            if (roomChoice == 1) {
                selectedPrice = 5000;
                roomType = "Standard";
            } else if (roomChoice == 2) {
                selectedPrice = 7000;
                roomType = "Deluxe";
            } else {
                selectedPrice = 10000;
                roomType = "Suite";
            }
            
            int availableRoom = findAvailableRoomByPrice(selectedPrice);
            
            if (availableRoom != -1) {
                newBooking->roomNo = availableRoom;
                newBooking->roomType = roomType;
                newBooking->totalAmount = selectedPrice * newBooking->numDays;
                newBooking->next = nullptr;

                insertBookingNode(newBooking);
                roomOccupied[availableRoom] = true;
                roomBookingMap[availableRoom] = newBooking;
                nameBookingMap[newBooking->name] = newBooking;
                bookedRooms++;
                
                printLine(GREEN);
                cout << GREEN << "✓ Booking confirmed successfully!" << RESET << "\n";
                printLine(GREEN);
                cout << "Customer: " << BOLDWHITE << newBooking->name << RESET << "\n";
                cout << "Room Type: " << YELLOW << roomType << RESET << "\n";
                cout << "Room Number: " << CYAN << newBooking->roomNo << RESET << "\n";
                cout << "Room Rate: Rs" << fixed << setprecision(2) << selectedPrice << " per night\n";
                cout << "Number of Days: " << newBooking->numDays << "\n";
                cout << "Total Amount: " << GREEN << "Rs" << newBooking->totalAmount << RESET << "\n";
                cout << "Check-in: " << newBooking->checkInDate.toString() << "\n";
                cout << "Check-out: " << newBooking->checkOutDate.toString() << "\n";
                printLine(GREEN);
                
                saveToFile();
                return;
            } else {
                cout << RED << "\n✗ No " << roomType << " rooms available!" << RESET << "\n";
                cout << "Would you like to:\n";
                cout << "1. Choose a different room type\n";
                cout << "2. Join waiting list for " << roomType << " room\n";
                cout << "3. Cancel booking\n";
                
                int nextChoice = readInt("Enter your choice (1-3): ", 1, 3);
                
                if (nextChoice == 1) {
                    continue;
                } else if (nextChoice == 2) {
                    newBooking->roomNo = 0;
                    newBooking->roomType = roomType;
                    newBooking->totalAmount = selectedPrice * newBooking->numDays;
                    waitingList.push(*newBooking);
                    cout << YELLOW << "\n✓ You have been added to the waiting list for " << roomType << " rooms." << RESET << "\n";
                    cout << "You will be notified when a room becomes available.\n";
                    delete newBooking;
                    return;
                } else {
                    cout << RED << "\nBooking cancelled." << RESET << "\n";
                    delete newBooking;
                    return;
                }
            }
        }
    }

    void displayBookings() {
        clearScreen();
        centerText("========= ALL BOOKINGS =========");
        cout << "\n";
        
        if (!head) {
            cout << RED << "No active bookings found." << RESET << "\n";
            return;
        }

        printLine(BLUE);
        cout << BOLDWHITE << left << setw(8) << "Room" << setw(20) << "Customer"
             << setw(15) << "Type" << setw(12) << "Check-in" << setw(12) << "Check-out" 
             << setw(8) << "Days" << setw(12) << "Amount" << RESET << "\n";
        printLine(BLUE);

        Booking* temp = head;
        while (temp) {
            cout << YELLOW << left << setw(8) << temp->roomNo << RESET
                 << setw(20) << temp->name
                 << setw(15) << temp->roomType
                 << setw(12) << temp->checkInDate.toString() 
                 << setw(12) << temp->checkOutDate.toString()
                 << setw(8) << temp->numDays
                 << GREEN << "Rs" << fixed << setprecision(2) << temp->totalAmount << RESET << "\n";
            temp = temp->next;
        }
        printLine(BLUE);
        
        cout << CYAN << "Occupied Rooms: " << bookedRooms << "/" << totalRooms << RESET << "\n";
        printLine(BLUE);
    }

    void searchBooking() {
        clearScreen();
        centerText("========= SEARCH BOOKING =========");
        cout << "\n";
        
        cout << MAGENTA << "Search Booking By:" << RESET << "\n";
        cout << "1. Room Number\n";
        cout << "2. Customer Name\n";
        int choice = readInt("Enter your choice (1-2): ", 1, 2);

        if (choice == 1) {
            int roomNo = readInt("Enter Room Number (1-" + to_string(TOTAL_ROOMS) + "): ", 1, TOTAL_ROOMS);

            if (roomBookingMap.find(roomNo) != roomBookingMap.end()) {
                Booking* b = roomBookingMap[roomNo];
                printLine(GREEN);
                cout << GREEN << "✓ Booking Found!" << RESET << "\n";
                printLine(GREEN);
                cout << "Customer Name: " << BOLDWHITE << b->name << RESET << "\n";
                cout << "Room Number: " << CYAN << b->roomNo << RESET << "\n";
                cout << "Room Type: " << YELLOW << b->roomType << RESET << "\n";
                cout << "Check-in Date: " << b->checkInDate.toString() << "\n";
                cout << "Check-out Date: " << b->checkOutDate.toString() << "\n";
                cout << "Number of Days: " << b->numDays << "\n";
                cout << "Total Amount: " << GREEN << "Rs" << fixed << setprecision(2) << b->totalAmount << RESET << "\n";
                printLine(GREEN);
            } else {
                cout << RED << "\nNo booking found for room " << roomNo << "." << RESET << "\n";
            }
        } else {
            string cname = readNonEmptyString("Enter Customer Name: ");
            
            Booking* found = nullptr;
            for (auto& pair : nameBookingMap) {
                if (toLowerCase(pair.first) == toLowerCase(cname)) {
                    found = pair.second;
                    break;
                }
            }
            
            if (found) {
                printLine(GREEN);
                cout << GREEN << "✓ Booking Found!" << RESET << "\n";
                printLine(GREEN);
                cout << "Customer Name: " << BOLDWHITE << found->name << RESET << "\n";
                cout << "Room Number: " << CYAN << found->roomNo << RESET << "\n";
                cout << "Room Type: " << YELLOW << found->roomType << RESET << "\n";
                cout << "Check-in Date: " << found->checkInDate.toString() << "\n";
                cout << "Check-out Date: " << found->checkOutDate.toString() << "\n";
                cout << "Number of Days: " << found->numDays << "\n";
                cout << "Total Amount: " << GREEN << "Rs" << fixed << setprecision(2) << found->totalAmount << RESET << "\n";
                printLine(GREEN);
            } else {
                cout << RED << "\nNo booking found for customer: " << cname << RESET << "\n";
            }
        }
    }

    void sortBookings() {
        clearScreen();
        centerText("========= SORT BOOKINGS =========");
        cout << "\n";
        
        if (!head || !head->next) {
            cout << RED << "Not enough bookings to sort." << RESET << "\n";
            return;
        }

        vector<Booking*> bookings;
        Booking* temp = head;
        while (temp) {
            bookings.push_back(temp);
            temp = temp->next;
        }

        cout << MAGENTA << "Sort by:" << RESET << "\n";
        cout << "1. Room Number\n";
        cout << "2. Customer Name\n";
        cout << "3. Check-in Date\n";
        int choice = readInt("Enter your choice (1-3): ", 1, 3);

        if (choice == 1) {
            for (size_t i = 0; i + 1 < bookings.size(); ++i) {
                for (size_t j = 0; j + 1 < bookings.size() - i; ++j) {
                    if (bookings[j]->roomNo > bookings[j + 1]->roomNo) {
                        swap(bookings[j], bookings[j + 1]);
                    }
                }
            }
        } else if (choice == 2) {
            for (size_t i = 0; i + 1 < bookings.size(); ++i) {
                for (size_t j = 0; j + 1 < bookings.size() - i; ++j) {
                    if (bookings[j]->name > bookings[j + 1]->name) {
                        swap(bookings[j], bookings[j + 1]);
                    }
                }
            }
        } else if (choice == 3) {
            for (size_t i = 0; i + 1 < bookings.size(); ++i) {
                for (size_t j = 0; j + 1 < bookings.size() - i; ++j) {
                    if (bookings[j + 1]->checkInDate < bookings[j]->checkInDate) {
                        swap(bookings[j], bookings[j + 1]);
                    }
                }
            }
        }

        printLine(BLUE);
        cout << BOLDWHITE << left << setw(8) << "Room"
             << setw(20) << "Customer"
             << setw(15) << "Type"
             << setw(12) << "Check-in"
             << setw(12) << "Check-out"
             << setw(8) << "Days"
             << setw(12) << "Amount" << RESET << "\n";
        printLine(BLUE);

        for (size_t i = 0; i < bookings.size(); ++i) {
            cout << YELLOW << left << setw(8) << bookings[i]->roomNo << RESET
                 << setw(20) << bookings[i]->name
                 << setw(15) << bookings[i]->roomType
                 << setw(12) << bookings[i]->checkInDate.toString()
                 << setw(12) << bookings[i]->checkOutDate.toString()
                 << setw(8) << bookings[i]->numDays
                 << GREEN << "Rs" << fixed << setprecision(2) << bookings[i]->totalAmount << RESET << "\n";
        }
        printLine(BLUE);
    }

    void cancelBooking() {
        clearScreen();
        centerText("========= CANCEL BOOKING =========");
        cout << "\n";
        
        int roomNo = readInt("Enter Room Number to cancel (1-" + to_string(TOTAL_ROOMS) + "): ", 1, TOTAL_ROOMS);

        if (roomBookingMap.find(roomNo) == roomBookingMap.end()) {
            cout << RED << "\nNo active booking found for room " << roomNo << "." << RESET << "\n";
            return;
        }

        Booking* b = roomBookingMap[roomNo];
        printLine(YELLOW);
        cout << YELLOW << "Booking Details:" << RESET << "\n";
        cout << "Customer: " << BOLDWHITE << b->name << RESET << "\n";
        cout << "Room: " << CYAN << roomNo << RESET << "\n";
        cout << "Room Type: " << YELLOW << b->roomType << RESET << "\n";
        cout << "Amount: " << GREEN << "Rs" << fixed << setprecision(2) << b->totalAmount << RESET << "\n";
        printLine(YELLOW);
        cout << RED << "Are you sure you want to cancel this booking? (y/n): " << RESET;
        
        char ch;
        cin >> ch;
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        
        if (ch == 'y' || ch == 'Y') {
            nameBookingMap.erase(b->name);
            roomBookingMap.erase(roomNo);
            roomOccupied[roomNo] = false;
            removeBookingNode(roomNo);
            bookedRooms--;
            cout << GREEN << "\n✓ Booking for room " << roomNo << " has been cancelled." << RESET << "\n";
            saveToFile();
            assignFromWaitingList();
        } else {
            cout << YELLOW << "\nCancellation aborted." << RESET << "\n";
        }
    }

    void checkOutCustomer() {
        clearScreen();
        centerText("========= CHECKOUT =========");
        cout << "\n";
        
        int roomNo = readInt("Enter Room Number for checkout (1-" + to_string(TOTAL_ROOMS) + "): ", 1, TOTAL_ROOMS);

        if (roomBookingMap.find(roomNo) != roomBookingMap.end()) {
            Booking* b = roomBookingMap[roomNo];
            
            printLine(GREEN);
            cout << BOLDWHITE << "Checkout Details" << RESET << "\n";
            printLine(GREEN);
            cout << "Customer: " << BOLDWHITE << b->name << RESET << "\n";
            cout << "Room: " << CYAN << roomNo << RESET << "\n";
            cout << "Room Type: " << YELLOW << b->roomType << RESET << "\n";
            cout << "Total Stay: " << b->numDays << " days\n";
            cout << "Total Amount: " << GREEN << "Rs" << fixed << setprecision(2) << b->totalAmount << RESET << "\n";
            printLine(GREEN);
            
            cout << YELLOW << "Confirm checkout (Y/N): " << RESET;
            char confirm;
            cin >> confirm;
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            
            if (tolower(confirm) != 'y') {
                cout << CYAN << "Checkout cancelled." << RESET << "\n";
                return;
            }
            
            recentCheckouts.push(*b);
            nameBookingMap.erase(b->name);
            roomBookingMap.erase(roomNo);
            roomOccupied[roomNo] = false;
            removeBookingNode(roomNo);
            bookedRooms--;
            
            cout << GREEN << "\n✓ Checkout completed successfully for " << b->name << "." << RESET << "\n";
            cout << "Thank you for staying with us!\n";
            
            saveToFile();
            assignFromWaitingList();
        } else {
            cout << RED << "\nRoom " << roomNo << " is not currently occupied." << RESET << "\n";
        }
    }

    void showRecentCheckouts() {
        clearScreen();
        centerText("========= RECENT CHECKOUTS =========");
        cout << "\n";
        
        if (recentCheckouts.empty()) {
            cout << RED << "No recent check-outs." << RESET << "\n";
            return;
        }

        printLine(YELLOW);
        cout << BOLDWHITE << "Recent Checkouts (Most Recent First):" << RESET << "\n";
        printLine(YELLOW);

        stack<Booking> temp = recentCheckouts;
        int count = 1;
        while (!temp.empty()) {
            Booking b = temp.top();
            temp.pop();
            cout << CYAN << count++ << ". " << RESET << b.name << " | Room: " << YELLOW << b.roomNo << RESET
                 << " | Type: " << b.roomType
                 << " | Days: " << b.numDays
                 << " | Amount: " << GREEN << "Rs" << fixed << setprecision(2) << b.totalAmount << RESET << "\n";
        }
        printLine(YELLOW);
    }

    void showWaitingList() {
        clearScreen();
        centerText("========= WAITING LIST =========");
        cout << "\n";
        
        if (waitingList.empty()) {
            cout << RED << "No customers in waiting list." << RESET << "\n";
            return;
        }

        printLine(MAGENTA);
        cout << BOLDWHITE << "Waiting List:" << RESET << "\n";
        printLine(MAGENTA);

        queue<Booking> temp = waitingList;
        int idx = 1;
        while (!temp.empty()) {
            Booking b = temp.front();
            temp.pop();
            cout << CYAN << idx++ << ". " << RESET << b.name 
                 << " | Type: " << YELLOW << b.roomType << RESET
                 << " | Check-in: " << b.checkInDate.toString()
                 << " | Days: " << b.numDays << "\n";
        }
        printLine(MAGENTA);
    }
};

// Main multi-hotel system
class MultiHotelSystem {
private:
    vector<pair<string, HotelSystem*>> hotels;
    
public:
    MultiHotelSystem() {
        hotels.push_back(make_pair("The Grand Palace", new HotelSystem("The Grand Palace", 10)));
        hotels.push_back(make_pair("OceanView Resort", new HotelSystem("OceanView Resort", 8)));
        hotels.push_back(make_pair("Mountain Retreat", new HotelSystem("Mountain Retreat", 12)));
        hotels.push_back(make_pair("CityLights Inn", new HotelSystem("CityLights Inn", 10)));
        hotels.push_back(make_pair("Royal Heritage Hotel", new HotelSystem("Royal Heritage Hotel", 15)));
    }
    
    ~MultiHotelSystem() {
        for (auto& hotel : hotels) {
            delete hotel.second;
        }
    }
    
    void displayHotels() {
        clearScreen();
        centerText(BG_PURPLE + string(BOLDWHITE) + "========= AVAILABLE HOTELS =========" + string(RESET));
        cout << "\n";
        
        for (size_t i = 0; i < hotels.size(); i++) {
            cout << CYAN << i + 1 << ". " << BOLDWHITE << hotels[i].first << RESET 
                 << " (" << GREEN << hotels[i].second->getAvailableRooms() << RESET 
                 << "/" << hotels[i].second->getTotalRooms() << " rooms available)\n";
        }
        cout << CYAN << "0. " << YELLOW << "Search hotel by name" << RESET << "\n\n";
    }
    
    int selectHotel() {
        displayHotels();
        
        int choice;
        while (true) {
            cout << MAGENTA << "Enter hotel number (0-" << hotels.size() << "): " << RESET;
            if (cin >> choice) {
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                
                if (choice == 0) {
                    string searchName;
                    cout << CYAN << "Enter hotel name to search: " << RESET;
                    getline(cin, searchName);
                    
                    for (size_t i = 0; i < hotels.size(); i++) {
                        string hotelLower = hotels[i].first;
                        string searchLower = searchName;
                        transform(hotelLower.begin(), hotelLower.end(), hotelLower.begin(), ::tolower);
                        transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
                        
                        if (hotelLower.find(searchLower) != string::npos) {
                            cout << GREEN << "Found: " << hotels[i].first << RESET << "\n";
                            return i;
                        }
                    }
                    cout << RED << "No matching hotel found!" << RESET << "\n";
                    displayHotels();
                } else if (choice >= 1 && choice <= (int)hotels.size()) {
                    return choice - 1;
                } else {
                    cout << RED << "Invalid choice. Please try again." << RESET << "\n";
                }
            } else {
                if (cin.eof()) {
                    cout << "\n" << YELLOW << "End of input detected. Exiting..." << RESET << "\n";
                    exit(0);
                }
                cout << RED << "Invalid input. Please enter a number." << RESET << "\n";
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                displayHotels();
            }
        }
    }
    
    HotelSystem* getHotel(int index) {
        if (index >= 0 && index < (int)hotels.size()) {
            return hotels[index].second;
        }
        return nullptr;
    }
    
    void customerMenu() {
        int hotelIndex = selectHotel();
        HotelSystem* hotel = getHotel(hotelIndex);
        if (!hotel) return;
        
        int choice;
        do {
            clearScreen();
            centerText(BG_CYAN + string(BOLDWHITE) + "========= CUSTOMER MENU =========" + string(RESET));
            cout << "\n" << YELLOW << "Hotel: " << BOLDWHITE << hotel->getHotelName() << RESET << "\n\n";
            cout << CYAN << "1. " << RESET << "View Available Rooms\n";
            cout << CYAN << "2. " << RESET << "Book a Room\n";
            cout << CYAN << "3. " << RESET << "Search My Booking\n";
            cout << CYAN << "4. " << RESET << "Cancel My Booking\n";
            cout << CYAN << "5. " << RESET << "Back to Hotel Selection\n";
            cout << CYAN << "6. " << RESET << "Exit\n\n";
            
            cout << MAGENTA << "Enter your choice: " << RESET;
            if (!(cin >> choice)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << RED << "Invalid input!" << RESET << "\n";
                continue;
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            
            switch (choice) {
                case 1: 
                    hotel->showAvailableRooms(); 
                    break;
                case 2: 
                    hotel->addBooking(); 
                    break;
                case 3: 
                    hotel->searchBooking(); 
                    break;
                case 4: 
                    hotel->cancelBooking(); 
                    break;
                case 5: 
                    cout << GREEN << "Returning to hotel selection..." << RESET << "\n";
                    return;
                case 6: 
                    cout << GREEN << "Thank you for using our system!" << RESET << "\n";
                    break;
                default: 
                    cout << RED << "Invalid choice. Try again!" << RESET << "\n";
            }
            
            if (choice != 5 && choice != 6) {
                cout << "\n" << YELLOW << "Press Enter to continue..." << RESET;
                cin.get();
            }
        } while (choice != 6);
    }
    
    void adminMenu() {
        string username, password;
        clearScreen();
        centerText("========= ADMIN LOGIN =========");
        cout << "\n";
        
        cout << CYAN << "Username: " << RESET;
        getline(cin, username);
        cout << CYAN << "Password: " << RESET;
        getline(cin, password);
        
        if (username != "admin" || password != "1234") {
            cout << RED << "\n✗ Access Denied! Invalid Credentials" << RESET << "\n";
            return;
        }
        
        cout << GREEN << "\n✓ Access Granted! Welcome, Admin!" << RESET << "\n";
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.get();
        
        int hotelIndex = selectHotel();
        HotelSystem* hotel = getHotel(hotelIndex);
        if (!hotel) return;
        
        int choice;
        do {
            clearScreen();
            centerText(BG_BLUE + string(BOLDWHITE) + "========= ADMIN DASHBOARD =========" + string(RESET));
            cout << "\n" << YELLOW << "Hotel: " << BOLDWHITE << hotel->getHotelName() << RESET << "\n\n";
            cout << CYAN << "1. " << RESET << "Display All Bookings\n";
            cout << CYAN << "2. " << RESET << "Search Booking by Room\n";
            cout << CYAN << "3. " << RESET << "Sort Bookings\n";
            cout << CYAN << "4. " << RESET << "Cancel Booking\n";
            cout << CYAN << "5. " << RESET << "Check-out Customer\n";
            cout << CYAN << "6. " << RESET << "Show Recent Checkouts\n";
            cout << CYAN << "7. " << RESET << "Show Waiting List\n";
            cout << CYAN << "8. " << RESET << "View Available Rooms\n";
            cout << CYAN << "9. " << RESET << "Change Hotel\n";
            cout << CYAN << "10. " << RESET << "Logout\n\n";
            
            cout << MAGENTA << "Enter your choice: " << RESET;
            if (!(cin >> choice)) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << RED << "Invalid input!" << RESET << "\n";
                continue;
            }
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            
            switch (choice) {
                case 1: 
                    hotel->displayBookings(); 
                    break;
                case 2: 
                    hotel->searchBooking(); 
                    break;
                case 3: 
                    hotel->sortBookings(); 
                    break;
                case 4: 
                    hotel->cancelBooking(); 
                    break;
                case 5: 
                    hotel->checkOutCustomer(); 
                    break;
                case 6: 
                    hotel->showRecentCheckouts(); 
                    break;
                case 7: 
                    hotel->showWaitingList(); 
                    break;
                case 8: 
                    hotel->showAvailableRooms(); 
                    break;
                case 9:
                    hotelIndex = selectHotel();
                    hotel = getHotel(hotelIndex);
                    if (!hotel) return;
                    break;
                case 10: 
                    cout << GREEN << "Returning to main menu..." << RESET << "\n";
                    break;
                default: 
                    cout << RED << "Invalid choice. Try again!" << RESET << "\n";
            }
            
            if (choice != 9 && choice != 10) {
                cout << "\n" << YELLOW << "Press Enter to continue..." << RESET;
                cin.get();
            }
        } while (choice != 10);
    }
};

int main() {
    MultiHotelSystem system;
    int choice;
    
    do {
        clearScreen();
        centerText("==========================================================");
        centerText("     WELCOME TO NATIONAL HOTEL MANAGEMENT SYSTEM          ");
        centerText("==========================================================");
        cout << "\n";
        
        cout << BG_GREEN << BOLDWHITE << "         MAIN MENU         " << RESET << "\n\n";
        cout << CYAN << "1. " << RESET << "Customer Login\n";
        cout << CYAN << "2. " << RESET << "Admin Login\n";
        cout << CYAN << "3. " << RESET << "Exit\n\n";
        
        cout << MAGENTA << "Enter your choice: " << RESET;
        if (!(cin >> choice)) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << RED << "Invalid input!" << RESET << "\n";
            cout << YELLOW << "Press Enter to continue..." << RESET;
            cin.get();
            continue;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');
        
        switch (choice) {
            case 1:
                system.customerMenu();
                break;
            case 2:
                system.adminMenu();
                break;
            case 3:
                clearScreen();
                centerText("==========================================================");
                centerText("          Thank you for using our system!                 ");
                centerText("               Have a great day!                          ");
                centerText("==========================================================");
                break;
            default:
                cout << RED << "Invalid option! Please try again." << RESET << "\n";
                cout << YELLOW << "Press Enter to continue..." << RESET;
                cin.get();
        }
    } while (choice != 3);
    
    return 0;
}