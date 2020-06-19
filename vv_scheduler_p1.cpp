// --------------------------------------
//  A variable voltage EDF scheduler that utilizes the STS algorithm orginally outlined in Mok's research paper. 
//  //Execute instructions: g++ -std=c++11 ./vv_scheduler_p1.cpp -o vv_scheduler_p1; ./vv_scheduler_p1 < yourtests.txt;
// Note: occassionally there may be a malloc warning thrown - there's a bug and I'm positive it involves the way the STL heap is being managed. I should have implemented my own since I have no time to debug. If it's incorrect I'll just have to take it - I unfortunately do not think I stand better chances turning it in at a later date. Another side note - I wasn't clear on how professor wanted the float clock speeds being subtracted during the actual task run time -- so I just left it as is it would be with EDF - with the addition of the clock speeds being updated for each task upon arrival of new tasks. 
//  Created by Zachary Grimes on 4/15/20.
// --------------------------------------

#include <unistd.h>
#include <stdio.h>
#include <iostream>
#include <sstream>
#include <iterator>
#include <string>
#include <regex>
#include <vector>
#include <queue>
#include <fstream>
using namespace std;


struct task_info {
	int task_tag,
	arrv_t,
	tcomp_t,
	rcomp_t,
	dead_t,
	fin_t;
	float clock_i;
};

class cmpr {
public:
	bool operator() (task_info const& t1, task_info const& t2)
	{
			// return "true" if "t1" is ordered before t2
			return t1.dead_t > t2.dead_t;
	}
};

float clock_j; //max total utilization - clock speed coefficient
int curr_max_dead_t;

vector<task_info> inc_ts; /* initial list read in */
priority_queue<task_info, vector<task_info>, cmpr> p_queue; /* priority queue using min heap property */
vector<task_info> fin_sched; /* Rejected tasks for output analysis */

string toString() {
	string finalschedule;
	for(int i =0; i< fin_sched.size(); i++){
		finalschedule += "Task " + to_string(fin_sched[i].task_tag) + " finished at t = " + to_string(fin_sched[i].fin_t) + ", with a clock speed of "+ to_string(fin_sched[i].clock_i)+", with a remaining comp time of " + to_string(fin_sched[i].rcomp_t) + "\n";
	}
	return finalschedule;
}

bool util_test(int sim_t, int tsk_tg) {
	if(p_queue.size()==0) { /* no tasks have been accepted yet */
		float temp_u_i = (inc_ts[tsk_tg].rcomp_t)/(inc_ts[tsk_tg].dead_t - sim_t);
		if( temp_u_i <= 1) {
			inc_ts[tsk_tg].clock_i = temp_u_i;
			clock_j = temp_u_i; //update voltage if utilization <= 1
			return true;
		}
		else
			return false;
	}
	else {
		/* compute sum [i:j] RC of each task*/
		float temp_u_j;
		for(int j = 0; j < p_queue.size(); j++) {
			temp_u_j += (inc_ts[tsk_tg].rcomp_t - (inc_ts[j].clock_i * sim_t));
		}
		/* divide by task with highest util factor minus current time*/
		temp_u_j = temp_u_j/(inc_ts[tsk_tg].dead_t - sim_t);
		if(temp_u_j <= 1) {
			inc_ts[tsk_tg].clock_i = temp_u_j;
			clock_j = temp_u_j; //update voltage if utilization <= 1
			return true;
		}
		else
			return false;
	}
	return 0;
}

void sts_simul() {
	/* Try and simulate dynamic arrival of tasks. Second loop tries to save cycles in this already disfigured WCET. Note the 200t upper bound is the hyper-period, H */
	for(int t = 0; t <= 200; t++) {
		for(int j=0; j < inc_ts.size(); j++) {
			if(inc_ts.empty() && p_queue.empty()) {
				//toString();
				exit(0);
			}
			if (inc_ts[j].arrv_t == t)  { /*  EVENT 1 ------ */
				if (!p_queue.empty()) {/* Check to see if event 2 occurs simultaenously */
					if(p_queue.top().rcomp_t == 0) { /* Remove from the stack and send to output analysis*/
						auto top = p_queue.top();
						p_queue.pop();
						top.fin_t = t;
						fin_sched.push_back(top);
						inc_ts.erase(inc_ts.begin()+(j-1));
					}
					if(util_test(t, j)) { /* Now check if incoming task passes accept. test*/
						if(!p_queue.empty()) {util_test(t, (p_queue.top().task_tag-1)); }// update current before new is queued.
						
						p_queue.push(inc_ts[j]);
						if(p_queue.top().task_tag != inc_ts[j].task_tag) { /* If recently scheduled task isn't scheduled as the current running task, decr. current task comp time by one ts */
							auto top = p_queue.top(); /* execute current task for a time slice */
							p_queue.pop();
							(top.rcomp_t)--;
							p_queue.push(top);
						}
						
					}
					else { /* Incoming task fails accept. test*/
						fin_sched.push_back(inc_ts[j]);
						inc_ts.erase(inc_ts.begin()+(j-1));
						auto top = p_queue.top(); /* execute current task for a time slice */
						p_queue.pop();
						(top.rcomp_t)--;
						p_queue.push(top);
					}
					
				}
				else { /* First task */
					if(util_test(t, j)) { /* Incoming task passes accept. test*/
						if(!p_queue.empty()) { util_test(t, (p_queue.top().task_tag-1)); }// update current before new is queued.
						p_queue.push(inc_ts[j]);
					}
					else { /* Incoming task fails accept. test*/
						fin_sched.push_back(inc_ts[j]);
						inc_ts.erase(inc_ts.begin()+(j-1));
					}
				}
			}
			else if(p_queue.top().rcomp_t == 0) { /* JUST EVENT 2 --------*/
				auto top = p_queue.top();
				p_queue.pop();
				top.fin_t = t;
				fin_sched.push_back(top);
				inc_ts.erase(inc_ts.begin()+(j-1));
			}
			else if(inc_ts[j].task_tag == p_queue.top().task_tag){ /* STANDARD EX: No heap changes are being made, continue executing task with earliest deadline.*/
				auto top = p_queue.top(); /* execute current task for a time slice */
				p_queue.pop();
				(top.rcomp_t)--;
				p_queue.push(top);
			}
		}
	}
	
	cout << toString() << endl;

}

int main(int argc,  char *argv[]) {
	int ts_size=0; // starts at zero and increments as each line is read in
	
	string line; // for reading file input

/* Read in data into list vector, sorted by arrival time */
	getline(cin, line);
	//ts_size = stoi(regex_replace(line, regex(R"([^0-9]+)"), ""));
	int i=0;
	regex reg("[^0-9]+");
	string temp;
	while(getline(cin,line)) {
		inc_ts.push_back(task_info());
		string temp;
		sregex_token_iterator it(line.begin(), line.end(), reg, -1);
		++it;
		temp = it->str();
		inc_ts[i].task_tag = stoi(temp);
		temp = (++it)->str();
		inc_ts[i].arrv_t = stoi(temp);
		temp = (++it)->str();
		inc_ts[i].tcomp_t = stoi(temp);
		inc_ts[i].rcomp_t = inc_ts[i].tcomp_t;
		temp = (++it)->str();
		inc_ts[i].dead_t = stoi(temp);

		//test 1
		cout << "Task: " << inc_ts[i].task_tag << ", arrival " << inc_ts[i].arrv_t << ", comp: " << inc_ts[i].tcomp_t << ", deadline: " << inc_ts[i].dead_t <<endl;
		i++;
	
	}

	sts_simul();
	return 0;
}
