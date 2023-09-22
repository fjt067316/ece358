#include <iostream>
#include <cstdio>   
#include <random>
#include <cmath>
#include <vector>
#include <algorithm>
#include <tuple>
#include <queue> 

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

int main() {

    // q1();
    infinite_buffer(0.55);
    finite_buffer(0.55, 10);


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
    double avg = 75;

    // Generate a random number between 0 and 1
    for(int i=0; i<1000; i++){
        double random_number = dis(gen);
        double x = -(avg)*std::log(1-random_number);
        results.push_back(x);
    }

    double sum = 0;

    for(double &value: results){
        sum += value;
    }

    double experiment_avg = sum / results.size();

    std::cout << "question 1 experiment avg:" << experiment_avg << " expect value: " << avg << std::endl; 
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

    double avg, idle, dropped;
    std::tie(avg, idle, dropped) = get_finite_stats(eventQueue, K);
    std::cout << "K: " << K << " ro: " << ro << " Average # packets in queue: " << avg << " portion spent idle: " << idle << " Dropped " << dropped << " packets" << std::endl;
    // for (const Event& event : events) {
    //     std::cout << "Arrival Time: " << event.arrival_time << "  Departure Time: " << event.departure_time << std::endl;
    // }
}


std::tuple<double, double, double> get_finite_stats(std::priority_queue<Event, std::vector<Event>, EventCompare> eventQueue, int K){
    // departure time is always greater than arrival so we can do this in one pass

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_real_distribution<double> dis(0, 1.0);

    double count, dropped, departure_time, total, observer_count, idle, req_made = 0;

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



    return std::make_tuple(total / observer_count, idle/observer_count, dropped); // (avg_in_q, portion_spent_idle, packets_dropped) 
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