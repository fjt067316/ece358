#include <iostream>
#include <cstdio>   
#include <random>
#include <cmath>
#include <vector>
#include <algorithm>
#include <tuple>

#define ARRIVAL 0
#define DEPARTURE 1
#define OBSERVER 2

struct Event {
    double time;
    int event_type;
};

typedef Event Event;

void q1();
void infinite_buffer(double ro); // ro is "traffic intensity"
std::tuple<double, double>  get_avg_packets_in_q(std::vector<Event> events);

int main() {

    // q1();
    infinite_buffer(0.55);


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
    
    int T = 1000; // simulation time
    double service_time; // time to process packets and time between packets in this case
    int C = 1000000;
    int avg_packet_bits = 2000;
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
    std::cout << "ro: " << ro << " Average # packets in queue: " << avg << "% time spent idle: " << idle << std::endl;
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
    return std::make_tuple(total / events.size(), idle/observer_count);
}



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