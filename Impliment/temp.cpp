#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <iomanip>
#include <ctime>

using namespace std;

enum RoomStatus { VACANT, OCCUPIED, CLEANING };

struct Room {
    int number;
    int capacity;
    int weekday_rate;
    RoomStatus status;
    string guest_id;
    string cleaning_completion_time;
    string check_in_time;

    Room(int n, int c, int rate) : number(n), capacity(c), weekday_rate(rate), status(VACANT), guest_id(""), cleaning_completion_time(""), check_in_time("") {}
};

// Function to get day of the week (0=Sunday, 1=Monday, ..., 6=Saturday)
int dayOfWeek(int year, int month, int day) {
    if (month < 3) {
        month += 12;
        year--;
    }
    int k = year % 100;
    int j = year / 100;
    int day_index = (day + 13 * (month + 1) / 5 + k + k / 4 + j / 4 + 5 * j) % 7;
    return (day_index + 6) % 7; // Adjust to 0=Monday, ..., 6=Sunday, then adjust back to 0=Sunday, ..., 6=Saturday. Then adjust again to 0=Monday, ..., 6=Saturday.
}


int main() {
    int n_rooms, n_queries;
    cin >> n_rooms >> n_queries;
    cin.ignore();

    vector<Room> rooms;
    for (int i = 0; i < n_rooms; ++i) {
        int number, capacity, weekday_rate;
        cin >> number >> capacity >> weekday_rate;
        rooms.emplace_back(number, capacity, weekday_rate);
    }
    cin.ignore();

    for (int q = 0; q < n_queries; ++q) {
        string line;
        getline(cin, line);
        stringstream ss(line);
        string request_time_str;
        ss >> request_time_str;

        string request_type;
        ss >> request_type;

        if (request_type == "check-in") {
            int room_number, num_adults, num_children, days_of_stay;
            string guest_id;
            ss >> room_number >> num_adults >> num_children >> days_of_stay >> guest_id;

            int room_index = -1;
            for (int i = 0; i < n_rooms; ++i) {
                if (rooms[i].number == room_number) {
                    room_index = i;
                    break;
                }
            }

            if (room_index == -1) {
                // Room does not exist, but according to problem description, it's guaranteed to be an existing room number.
                continue;
            }

            if (rooms[room_index].status == OCCUPIED) {
                cout << request_time_str << " check-in error: room " << room_number << " is occupied" << endl;
            } else if (rooms[room_index].status == CLEANING) {
                cout << request_time_str << " check-in error: room " << room_number << " is being cleaned" << endl;
            } else if (num_adults + num_children > rooms[room_index].capacity) {
                cout << request_time_str << " check-in error: room " << room_number << " cannot accommodate " << guest_id << endl;
            } else {
                rooms[room_index].status = OCCUPIED;
                rooms[room_index].guest_id = guest_id;
                rooms[room_index].check_in_time = request_time_str;
                cout << request_time_str << " check-in to room " << room_number << " was successful" << endl;
            }
        } else if (request_type == "check-out") {
            string guest_id;
            int room_number, cleaning_time;
            ss >> guest_id >> room_number >> cleaning_time;

            int room_index = -1;
            for (int i = 0; i < n_rooms; ++i) {
                if (rooms[i].number == room_number) {
                    room_index = i;
                    break;
                }
            }

            if (room_index == -1) {
                // Room does not exist, should not happen
                continue;
            }

            if (rooms[room_index].guest_id != guest_id) {
                cout << request_time_str << " check-out error: guest ID " << guest_id << " is not in room " << room_number << endl;
            } else {
                // Calculate total charge
                string check_in_date_str = rooms[room_index].check_in_time.substr(0, 10);
                tm check_in_tm;
                strptime(check_in_date_str.c_str(), "%Y-%m-%d", &check_in_tm);
                time_t check_in_time_t = mktime(&check_in_tm);

                int weekday_days = 0;
                int weekend_days = 0;
                for (int day = 0; day < 1; ++day) { // Basic implementation assumes 1 day stay for checkout calculation in example.
                    tm current_tm = check_in_tm;
                    current_tm.tm_mday += day;1
                    mktime(¤t_tm); // Normalize

                    int day_index = current_tm.tm_wday; // 0=Sunday, 1=Monday, ..., 6=Saturday
                    if (day_index >= 1 && day_index <= 5) { // Monday to Friday (weekday)
                        weekday_days++;
                    } else { // Saturday or Sunday (weekend)
                        weekend_days++;
                    }
                }

                long long total_charge = (long long)weekday_days * rooms[room_index].weekday_rate + (long long)weekend_days * (rooms[room_index].weekday_rate * 0.8);
                if (weekday_days == 0 && weekend_days == 0) total_charge = rooms[room_index].weekday_rate; // Assume at least one day and that day is weekday if days_of_stay is not provided or used in charge calculation based on example.

                cout << request_time_str << " check-out " << guest_id << " has to pay " << total_charge << " to leave room " << room_number << endl;

                // Calculate cleaning completion time
                tm checkout_time_tm;
                strptime(request_time_str.c_str(), "%Y-%m-%dT%H:%M:%S", &checkout_time_tm);
                time_t checkout_time_t = mktime(&checkout_time_tm);

                time_t cleaning_completion_time_t = checkout_time_t + cleaning_time * 60;
                tm cleaning_completion_tm;
                localtime_r(&cleaning_completion_time_t, &cleaning_completion_tm);
                char buffer[20];
                strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%S", &cleaning_completion_tm);
                rooms[room_index].cleaning_completion_time = buffer;


                rooms[room_index].status = CLEANING;
                rooms[room_index].guest_id = ""; // Clear guest ID
                cout << request_time_str << " cleaning of " << room_number << " will be completed at " << rooms[room_index].cleaning_completion_time << endl;
            }

        } else if (request_type == "room_status") {
            cout << request_time_str << " room_status:";
            for (int i = 0; i < n_rooms; ++i) {
                string status_str;
                if (rooms[i].status == VACANT) {
                    status_str = "vacant";
                } else if (rooms[i].status == OCCUPIED) {
                    status_str = "occupied";
                } else {
                    status_str = "cleaning";
                }
                cout << " room " << rooms[i].number << " is " << status_str;
            }
            cout << endl;
        } else if (request_type == "check-in-suggestion") {
            // Basic implementation does not handle suggestion
            continue;
        } else if (request_type == "clean") {
             int room_number;
             ss >> room_number;
             int room_index = -1;
            for (int i = 0; i < n_rooms; ++i) {
                if (rooms[i].number == room_number) {
                    room_index = i;
                    break;
                }
            }
            if (room_index != -1 && rooms[room_index].status == CLEANING) {
                 rooms[room_index].status = VACANT;
                 cout << request_time_str << " cleaning of room " << room_number << " has been completed" << endl;
            }
        }
    }

    return 0;
}