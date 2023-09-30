#include <iostream>
#include <cstdio>   
#include <random>
#include <cmath>
#include <vector>
#include <algorithm>
#include <tuple>
#include <queue> 
#include <fstream>
#include <string>

#define ARRIVAL 0
#define DEPARTURE 1
#define OBSERVER 2

#define T 1000 // simulation time
#define C 1000000
#define avg_packet_bits 2000

struct Event {
    double time;
    int event_type;
};

typedef Event Event;

struct EventCompare {
    bool operator()(const Event& e1, const Event& e2) const {
        return e1.time > e2.time; // Use '>' for min-heap, '<' for max-heap
    }
};

void q1();
void infinite_buffer(double ro); // ro is "traffic intensity"
void finite_buffer(double ro, int K);
std::tuple<double, double>  get_avg_packets_in_q(std::vector<Event> events);
double get_dropped_packets(std::vector<Event> events, int K);
std::tuple<double, double, double> get_finite_stats(std::priority_queue<Event, std::vector<Event>, EventCompare> events, int K);
void writeToCSV(const std::string &data);

int main() {

    // const std::string filename = "output.csv";

    // // Open the file in truncation mode (clears its contents)
    // std::ofstream file(filename, std::ios::trunc);
    //     if (!file.is_open()) {
    //     std::cerr << "Error: Could not open the file" << std::endl;
    //     return 1;
    // }

    // // Close the file after truncating it
    // file.close();

    // // q1();
    // for(double ro = 0.25; ro < 0.95; ro += 0.1 ){
    //     infinite_buffer(ro);
    // }
    // infinite_buffer(1.2); // Q4

    // std::vector<int> K = {10, 25, 50};
    // for(int buff_size: K){
    //     for(double ro = 0.5; ro < 1.5; ro+=0.1){
    //         finite_buffer(ro, buff_size);
    //     }
    // }

    q1();
    


    return 0;
}

bool compareByArrivalTime(Event a, Event b) {
    return a.time < b.time;
}


void q1(){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(0, 1.0);
    std::vector<double> results;
    double lambda = 75; // avg = 1/75

    // Generate a random number between 0 and 1
    for(int i=0; i<1000; i++){
        double random_number = dis(gen);
        double x = -(1/lambda)*std::log(1-random_number);
        results.push_back(x);
    }

    double sum = 0;

    for(double &value: results){
        sum += value;
    }

    double experiment_avg = sum / results.size();
    double variance = 0;

    for (double &value : results) {
        double diff = value - experiment_avg;
        variance += diff * diff;
    }

    variance /= results.size();

    std::cout << "question 1 experiment avg: " << experiment_avg << " expected avg: " << 1/lambda << std::endl 
                << " experiment variance: " << variance << " expected variance: " << 1/(lambda*lambda) << std::endl; 
}


void infinite_buffer(double ro){
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(0, 1.0);
    
    double service_time; // time to process packets and time between packets in this case
    double lambda = ro * C / avg_packet_bits;
    double observer_rate = lambda *5;
    // each time iteration we generate a new rv to determine how many packets came in at that time step

    double time_elapsed = 0;
    double u;
    std::vector<Event> events;

    double last_packet_departure_time = 0;
    
    while(time_elapsed < T){
    
        u = dis(gen); 

        double time_until_next_packet = -(1/lambda)*std::log(1-u);
        
        u = dis(gen); 
        int packet_bits = -(avg_packet_bits)*std::log(1-u);
        service_time = ((double)packet_bits) / ((double)C); // get a random number of packet bits and calculate service time

        time_elapsed += time_until_next_packet;

        if(time_elapsed > last_packet_departure_time){ // buffer is empty
            last_packet_departure_time = time_elapsed + service_time;
        } else {
            last_packet_departure_time += service_time;
        }
        
        events.push_back({time_elapsed, ARRIVAL});
        events.push_back({last_packet_departure_time, DEPARTURE});
    }

    time_elapsed = 0;

    while(time_elapsed < T){
        u = dis(gen); 
        double time_until_next_observer = -(1/observer_rate)*std::log(1-u); // get observer timestamp

        time_elapsed += time_until_next_observer; // increment simulation time

        events.push_back({time_elapsed, OBSERVER}); // add observer event
    }


    std::sort(events.begin(), events.end(), compareByArrivalTime); // sort by arrival time
    double avg, idle;
    std::tie(avg, idle) = get_avg_packets_in_q(events);
    std::cout << "ro: " << ro << " Average # packets in queue: " << avg << " portion spent idle: " << idle << std::endl;
    // for (const Event& event : events) {
    //     std::cout << "Arrival Time: " << event.arrival_time << "  Departure Time: " << event.departure_time << std::endl;
    // }
    std::string output = "infinite," + std::to_string(ro) + "," + std::to_string(avg) + "," + std::to_string(idle);
    writeToCSV(output);
}

std::tuple<double, double> get_avg_packets_in_q(std::vector<Event> events){
    std::vector<int> in_q;

    double count = 0;
    double total = 0;
    double idle = 0;
    double observer_count;
    for(const Event &e: events){
        if(e.event_type == OBSERVER){
            in_q.push_back(count);
            total += count;
            observer_count++;
            if(count == 0){
                idle++;
            }
        }else if (e.event_type == ARRIVAL){
            count++;
        } else if(e.event_type == DEPARTURE){
            count--;
        }
    }

    // std::cout << total << std::endl;
    // std::cout << events.size() << std::endl;
    return std::make_tuple(total / observer_count, idle/observer_count);
}


void writeToCSV( const std::string &data) {
    // Open the file in append mode
    std::ofstream csvFile("output.csv", std::ios::app);

    if (!csvFile.is_open()) {
        std::cerr << "Error: Could not open the CSV file." << std::endl;
        return;
    }

    // Write the data as a new line in the CSV file
    csvFile << data << std::endl;

    // Close the file
    csvFile.close();
}

void finite_buffer(double ro, int K){
    
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(0, 1.0);

    double lambda = ro * C / avg_packet_bits;
    double observer_rate = lambda *5;
    // each time iteration we generate a new rv to determine how many packets came in at that time step

    double time_elapsed = 0;
    double u;
    std::priority_queue<Event, std::vector<Event>, EventCompare> eventQueue;

    while(time_elapsed < T){
    
        u = dis(gen); 

        double time_until_next_packet = -(1/lambda)*std::log(1-u);
        time_elapsed += time_until_next_packet;

        eventQueue.push({time_elapsed, ARRIVAL});
    }

    time_elapsed = 0;

    while(time_elapsed < T){
        u = dis(gen); 
        double time_until_next_observer = -(1/observer_rate)*std::log(1-u); // get observer timestamp

        time_elapsed += time_until_next_observer; // increment simulation time

        eventQueue.push({time_elapsed, OBSERVER}); // add observer event
    }

    double avg, idle, ploss;
    std::tie(avg, idle, ploss) = get_finite_stats(eventQueue, K);
    std::cout << "K: " << K << " ro: " << ro << " Average # packets in queue: " << avg << " portion spent idle: " << idle << " Dropped " << ploss << " packets" << std::endl;
    // for (const Event& event : events) {
    //     std::cout << "Arrival Time: " << event.arrival_time << "  Departure Time: " << event.departure_time << std::endl;
    // }
    std::string output = "finite," + std::to_string(ro) + "," + std::to_string(avg) + "," + std::to_string(idle) + "," + std::to_string(ploss) +","+ std::to_string(K);
    writeToCSV(output);
}


std::tuple<double, double, double> get_finite_stats(std::priority_queue<Event, std::vector<Event>, EventCompare> eventQueue, int K){
    // departure time is always greater than arrival so we can do this in one pass

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(0, 1.0);

    double count = 0, dropped = 0, departure_time = 0, total = 0, observer_count = 0, idle = 0, req_made = 0;

    while (!eventQueue.empty()) {
        Event e = eventQueue.top();
        eventQueue.pop();

        if (e.event_type == ARRIVAL) {
            req_made++;
            if (count < K) { // buffer not full
                double u = dis(gen);
                int packet_bits = -(avg_packet_bits) * std::log(1 - u);
                double service_time = ((double)packet_bits) / ((double)C);

                if (e.time > departure_time) {
                    departure_time = e.time + service_time;
                } else {
                    departure_time = departure_time + service_time; // next packet leaves service time amount after last packet in q
                }

                eventQueue.push({departure_time, DEPARTURE}); // Push departure event
                count++;
            } else {
                dropped++;
            }
        }else if (e.event_type == OBSERVER){
            total += count;
            observer_count++;
            if(count == 0){
                idle++;
            }
        } else if(e.event_type == DEPARTURE){
            count--;
        }
    }


    // std::cout << "DROPPED " << dropped << " REQ " << req_made << std::endl;
    return std::make_tuple(total / observer_count, idle/observer_count, dropped/req_made); // (avg_in_q, portion_spent_idle, packets_dropped) 
}


// std::tuple<double, double> get_avg_packets_in_q(std::vector<Event> events){
//     std::vector<int> in_q;

//     double count = 0;
//     double total = 0;
//     double idle = 0;
//     double observer_count;
//     for(const Event &e: events){
//         if(e.event_type == OBSERVER){
//             in_q.push_back(count);
//             total += count;
//             observer_count++;
//             if(count == 0){
//                 idle++;
//             }
//         }else if (e.event_type == ARRIVAL){
//             count++;
//         } else if(e.event_type == DEPARTURE){
//             count--;
//         }
//     }

//     // std::cout << total << std::endl;
//     // std::cout << events.size() << std::endl;
//     return std::make_tuple(total / events.size(), idle/observer_count);
// }

// class Simulation {
// public:
//     const double C;
//     const double L;
//     const double A;
//     double T;
//     double K;
//     double service_time;
//     double avg_arrival_rate;
//     double prev_departure_time;

//     std::vector<double> buffer;
//     std::vector<Event> events;
    
//     std::mt19937 gen;
//     std::uniform_real_distribution<double> dis;

//     Simulation() : C(0.0), L(0.0), A(0.0), K(-1), T(1000), service_time(0.0), avg_arrival_rate(75), prev_departure_time(0.0) {
//         std::random_device rd;
//         gen = std::mt19937(rd());
//         dis = std::uniform_real_distribution<double>(0.0, 1.0);
//     }

//     double get_time_interval(){
//         double u = dis(gen);
//         double time_until_next_packet = -(1/avg_arrival_rate)*std::log(1-u);

//     }

//     void simulate(){
//         double time_elapsed = 0;
//         // generate arrival event
//         while(time_elapsed < T){
//             double time_until_next_packet = get_time_interval();
//             Event next_event;
//             next_event.arrival_time = time_elapsed + time_until_next_packet;
            
//             if(buffer.size() == 0){
//                 next_event.departure_time = next_event.arrival_time + service_time;
//             }else if (K != -1 && buffer.size() < K){
//                 next_event.departure_time = prev_departure_time + service_time;
//             }

//             prev_departure_time = next_event.departure_time;

//             events.push_back(next_event);
//             time_elapsed += 

//         }



//     }


// };