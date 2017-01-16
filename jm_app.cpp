#include <iostream>
#include <fstream>
#include <string>
#include <map>
#include <list>
#include <sstream>
#include <stdlib.h> 
#include <thread>
#include <mutex>          
#include <cctype>
#include <time.h>       /* time_t, struct tm, time, localtime */
/*
Any one can copy this code to manage Job threads with a bare metal like
job dispatcher:Here it reads a file then parses them to do some set of commands
defined in the document.
*/
using namespace std;
enum FILECMD{
	SORT_ASC = 0,
	SORT_DSC,
	AVG,
	MED,
	STD
};
#define MAX_LINE_LEN 		500
#define MAX_PARAMS 		3
#define MAX_JOBS 		10

class CommQ{
	public:
		CommQ():cmdcnt(0){}
		bool ReadCmd(string &jobline){
			bool ret = false;
			int idx = 0; 
			int p = 0, n;
			mtx.lock();			
			if(cmdcnt > 0 && !CommQ_lns.empty()) {
				jobline = CommQ_lns.front();
				//cout << jobline << endl;
			    CommQ_lns.pop_front();
				cmdcnt--;
				ret = true;
			}
			mtx.unlock();
			return ret;
		} 
		void WriteCmd(string jobline){
			mtx.lock();
			CommQ_lns.push_back(jobline);
			cmdcnt++;
			mtx.unlock();
		}
		int NoofCmd(){return cmdcnt;}
	private:
		list<string> CommQ_lns;
		int cmdcnt;
		std::mutex mtx;           // mutex for critical section
								 //With mutex lock and unlock
};

class jdthreads{
	public:
		jdthreads() {}
		void setstarttm(int cmdcnt, string cmdstr) {
			thdid = cmdcnt;				
			jb = cmdstr;			
			time_t rawtime;  struct tm * timeinfo;
			time (&rawtime);
			timeinfo = localtime (&rawtime);
			stm.assign(asctime(timeinfo));
		}
		void setendtm(void) {
			time_t rawtime;  struct tm * timeinfo;
			time (&rawtime);
			timeinfo = localtime (&rawtime);
			etm.assign(asctime(timeinfo));
		}
	public:	
	int thdid;
	thread thd;
	string jb;
	string outstr;
	string stm;
	string etm;
};


CommQ Commflops;
jdthreads flproc[MAX_JOBS]; 

bool jmdef_fileParser(const string& flname){
	bool ret = false;	
	//Parse the lines within the file
	ifstream infl;
	infl.open (flname.c_str());
	string jobline;
	while (!infl.eof()) {
		std::getline(infl, jobline);
		//Write this into the Comm Queue
		if(jobline.size() > 1) Commflops.WriteCmd(jobline);
		ret = true;
	}
	infl.close();
	return ret;
}

int fileCmdProc(const string Cfmd, string fip, string fop, int cmdcnt) {
	int ret = 0;
	string outstr;
	//Reads and parse the fip
	map<string, int> actcmd;
	actcmd["SORT_ASC"] = SORT_ASC;  actcmd["SORT_DSC"] = SORT_DSC; 
	actcmd["AVG"] = AVG; actcmd["MED"] = MED; actcmd["STD"] = STD; 

	switch(actcmd[Cfmd]){
		case SORT_ASC: {
			list<string> namestrs;
			std::ifstream fld(fip.c_str(), std::ifstream::in);
			std::string data;
			//Sort the array in Ascending order
			while(std::getline(fld, data, ' ')) {namestrs.push_back(data); }
			fld.close();
			namestrs.sort();
			//Write it into the fop  			
			std::ofstream olfd;
			olfd.open (fop.c_str(), std::ofstream::out);
			while(!namestrs.empty()) {			
				olfd << namestrs.front() << " ";
				outstr += namestrs.front() + " ";
				namestrs.pop_front();
			}
			olfd.close();
		}
		break;

		case SORT_DSC: {
			list<string> namestrs;
			std::ifstream fld(fip.c_str(), std::ifstream::in);
			std::string data;
			//Sort the array in Ascending order
			while(std::getline(fld, data, ' ')) {
				for(int i= 0; data[i]; i++) if(data[i] == '\n') data[i] = ' ';
				namestrs.push_back(data); 
			}
			fld.close();			
			namestrs.sort();
			//Write it into the fop  			
			std::ofstream olfd;
			olfd.open (fop.c_str(), std::ofstream::out);
			while(!namestrs.empty()) {			
				olfd << namestrs.back() << " ";
				outstr += namestrs.back() + " ";
				namestrs.pop_back();
			}
			olfd.close();
		}
		break;
		case AVG: {
			list<int> numarr;
			list<int>::iterator it;

			int  sum = 0;float avg;
			std::ifstream fld(fip.c_str(), std::ifstream::in);
			std::string data;
			//Sort the array in Ascending order
			while(std::getline(fld, data, ' ')) {numarr.push_back(atoi(data.c_str()));}
			fld.close();
			//Get the average value
			for (it=numarr.begin(); it!=numarr.end(); ++it)	 sum += *it;
			avg = float(sum) / numarr.size();
			//Write it into the fop
			std::ofstream olfd;
			olfd.open (fop.c_str(), std::ofstream::out);
			olfd << avg << " ";
			std::ostringstream buff;
			buff << avg << " ";
			outstr = buff.str();
			olfd.close();	
		}		
		break;
		case MED: {
				list<int> numarr;
				list<int>::iterator it;

				int  sum = 0;float med;
				std::ifstream fld(fip.c_str(), std::ifstream::in);
				std::string data;
				//Sort the array in Ascending order
				while(std::getline(fld, data, ' ')) {numarr.push_back(atoi(data.c_str()));}
				fld.close();
				numarr.sort();
				//Get the median value
				int lenarr = numarr.size(), k;
				for (it=numarr.begin(), k = 0; k < lenarr/2 - (lenarr%2==0); it++, k++);
				if(lenarr % 2 == 0) med = (*(it) + *(++it)) / 2;
				else med = *it;
				//Write it into the fop
				std::ofstream olfd;
				olfd.open (fop.c_str(), std::ofstream::out);
				olfd << med << " ";
				std::ostringstream buff;
				buff << med << " ";
				outstr = buff.str();
				olfd.close();	
		}		
		break;
		case STD: {
				list<int> numarr;
				list<int>::iterator it;
				float  sum = 0;float avg, std;

				std::ifstream fld(fip.c_str(), std::ifstream::in);
				std::string data;
				//Sort the array in Ascending order
				while(std::getline(fld, data, ' ')) {numarr.push_back(atoi(data.c_str()));}
				fld.close();
				//Get the average value
				for (it=numarr.begin(); it!=numarr.end(); ++it)	 sum += *it;
				avg = sum / float(numarr.size());
				for (it=numarr.begin(), sum = 0; it!=numarr.end(); ++it) sum += (*it - avg)*(*it - avg);
				std = sum / float(numarr.size());
				//Write it into the fop
				std::ofstream olfd;
				olfd.open (fop.c_str(), std::ofstream::out);
				olfd << std << " ";
				std::ostringstream buff;
				buff << std << " ";
				outstr = buff.str();
				olfd.close();
		}
		break;
	}
	flproc[cmdcnt].outstr = outstr;
	return ret;
}

//Thread 

int job_thread(string jb, int cmdcnt){
	int ret = 0;
	const int phs = 2;	
	//Parse the jb commmand to obtain the Job ACTION, file ip, file op name
	string str[MAX_PARAMS];
	int inparams = 0, n, p = 0;
	while(inparams < MAX_PARAMS) {
		n = jb.find(' ', p); str[inparams++] = jb.substr(p, n  - p);
		p = n + 1;
	}
	ret = fileCmdProc(str[0], str[1].substr(phs), str[2].substr(phs), cmdcnt);
	return ret;	
}


int job_dispatcher(void){
	int ret = 0, cmdcnt = 0;
	bool CommQStat = true; 	string jb;

	do{
		//Read the Comm queue cmd with IPC mutex
		if(CommQStat = Commflops.ReadCmd(jb)) {
			flproc[cmdcnt].setstarttm(cmdcnt, jb);	
			flproc[cmdcnt].thd = std::thread(job_thread, jb, cmdcnt);
			cmdcnt++;
		}
		
	}while(CommQStat);
 	for(int i = 0; i < cmdcnt; i++) {
		flproc[i].thd.join();
		flproc[i].setendtm();
		cout << endl << endl << "Job ID: " << flproc[i].thdid << endl << flproc[i].jb << endl
		 << endl << "Start time:" << 
		flproc[i].stm << endl << "End time:" << flproc[i].etm << endl 
		<< "Result:" << flproc[i].outstr;		
	}
	return ret;	
}


int main(int argv, char  **argc){
	if(argv < 2) {
		 cerr << "No Job Definition file mentioned on the command line" << endl;
		return -1;
	}	
	string  flname(argc[1]);
	thread thjd;
    //Read the Job Def file and write into the Comm Queue
	if(jmdef_fileParser(flname))
		//Create and launch the Thread - Job Dispatcher	
		 thjd=std::thread(job_dispatcher);
	else {
		 cerr << "The Job definition file commands are non-existent or a format error occured" << endl;
		return -1;
	}
	// Makes the main thread wait for the new thread to finish execution, therefore blocks its own execution.
	thjd.join();
	
}
