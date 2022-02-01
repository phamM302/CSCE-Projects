#include "common.h"
#include "TCPRequestChannel.h"
#include "BoundedBuffer.h"
#include "HistogramCollection.h"
#include <sys/wait.h>
#include <thread>
#include <iostream>

using namespace std;

struct Responce {
	int patient;
	double ecgno;
};

void patient_thread_function(int patient, int n, BoundedBuffer* requestBuffer){
    /* What will the patient threads do? */
	DataRequest d (patient, 0.0, 1);
		for (int i = 0; i < n; i++) {
			vector<char> te ((char*) &d, ((char*)&d + sizeof(d)));
			requestBuffer -> push(te);
			d.seconds += 0.004;
		}
}

void worker_thread_function(BoundedBuffer* requestBuffer, TCPRequestChannel* chan, BoundedBuffer* responceBuffer, int bufferCapacity){
    /*
		Functionality of the worker threads	
    */
   char buff[bufferCapacity];
   while(true) {
	   vector<char> requestArr = requestBuffer -> pop();
	   char* request = requestArr.data();
	   chan ->cwrite(request, requestArr.size());
	   Request* m = (Request*) request;

	   if(m->getType() == DATA_REQ_TYPE) {
		    int patient = ((DataRequest*) request)-> person;
		    double ecgno = 0;
			chan -> cread(&ecgno, sizeof(double));
			Responce r {patient, ecgno};
			vector<char> te ((char*)&r, (char*)&r + sizeof(r));
			responceBuffer -> push(te);
	   }
	   else if (m->getType() == FILE_REQ_TYPE) {
			chan -> cread(buff, sizeof(buff));
			FileRequest* fr = (FileRequest*) request;
			string fileName = fr->getFileName();
			fileName = "received/" + fileName;
			FILE* fd = fopen (fileName.c_str(), "r+");
			fseek(fd, fr->offset, SEEK_SET);
			fwrite(buff, 1, fr->length, fd);
			fclose(fd);
	   }
	   else if (m->getType() == QUIT_REQ_TYPE) {
		   break;
	   }
   }

}

void histogram_thread_function (BoundedBuffer* responceBuffer, HistogramCollection* histogramCollection) {
    /*
		Functionality of the histogram threads	
    */
   while (true) {
	   vector<char> responceArr = responceBuffer -> pop();
	   Responce* r = (Responce*) responceArr.data();
	   if (r->patient == -1) {
		   break;
	   }
		histogramCollection -> update(r -> patient, r -> ecgno);
   }
}

void file_thread_function(string fileName, int m, TCPRequestChannel* control, BoundedBuffer* requestBuffer) {
	string fn = "received/" + fileName;
	FILE* fp = fopen(fn.c_str(), "w");
	fclose(fp);

	int lenFilemsg = sizeof(FileRequest) + sizeof (fileName) +1;
	char fileMsgbuffer [lenFilemsg];
	FileRequest* fm = new (fileMsgbuffer) FileRequest(0,0);
	strcpy (fileMsgbuffer + sizeof (FileRequest), fileName.c_str());

	control->cwrite (fileMsgbuffer, sizeof(fm) + sizeof (fileName) + 1);
	__int64_t fileSize;
	FileRequest* send = (FileRequest*)fileMsgbuffer;
	control ->cread(&fileSize, sizeof(fileSize));
	
	while (fileSize > 0) {
		send->length = min((__int64_t) m, fileSize);
		vector<char> t (fileMsgbuffer, fileMsgbuffer + sizeof (FileRequest) + sizeof(fileName) +1);
		requestBuffer->push(t);
		fileSize -= send-> length;
		send -> offset += fm->length;
	}
}

int main(int argc, char *argv[]){
	string server_name;
	string port;

	int opt;
	int p = 10;
	int e = 1;
	string filename = "";
	int m = MAX_MESSAGE;
	int w = 100;
	int a = 10;
	int n = 10000;
	int b = 10; // size of bounded buffer, note: this is different from another variable buffercapacity/m
	// take all the arguments first because some of these may go to the server

	while ((opt = getopt(argc, argv, "f:p:m:b:w:n:h:r:")) != -1) {
		switch (opt) {
			case 'f':
				filename = optarg;
				a = 0;
				break;
			case 'p':
				p = atoi (optarg);
				break;
			case 'm':
				m = atoi (optarg);
				break;
			case 'b':
				b = atoi (optarg);
				break;
			case 'w':
				w = atoi (optarg);
				break;
			case 'n':
				n = atoi (optarg);
				break;
			case 'h':
				server_name = optarg;
				break;
			case 'r':
				port = optarg;
				break;

		}
	}

	/*int pid = fork ();
	if (pid < 0){
		EXITONERROR ("Could not create a child process for running the server");
	}
	if (!pid){ // The server runs in the child process
		char* args[] = {"./server", "-m", (char*) to_string(m).c_str(), nullptr};
		if (execvp(args[0], args) < 0){
			EXITONERROR ("Could not launch the server");
		}
	}*/
	std::cout << server_name << endl;
	std::cout << port << endl;
	TCPRequestChannel* chan = new TCPRequestChannel (server_name, port);
	BoundedBuffer request_buffer(b);
	BoundedBuffer responce_buffer(b);
	HistogramCollection hc;
	struct timeval start, end;
    gettimeofday (&start, 0);

	TCPRequestChannel* wChan [w];//worker channels
	for (int i = 0; i < w; i++) {
		/*Request nc (NEWCHAN_REQ_TYPE);
		chan->cwrite(&nc, sizeof(nc));
		char chanName [1024];
		chan->cread (chanName, sizeof (chanName));
		TCPRequestChannel newChan (chanName, TCPRequestChannel::CLIENT_SIDE);
		Request q (QUIT_REQ_TYPE);
    	newChan.cwrite (&q, sizeof(Request));*/
		wChan[i] = new TCPRequestChannel (server_name, port);
	}

    /* Start all threads here */
	thread workers [w];
	thread patient[p];
	thread histhreads[a];
	thread fileThread;

	
	/* Join all threads here */
	for (int i = 0; i < w; i++) {
		workers[i] = thread(worker_thread_function, &request_buffer, wChan[i], &responce_buffer, m);
	}
	
	if (filename.empty()) {
		for (int i =0; i < p; i++) {
			Histogram* hg = new Histogram (10, -2.0, 2.0);
			hc.add(hg);
			patient[i] = thread(patient_thread_function, i+1, n, &request_buffer);
	
		}
		for (int i = 0; i < a; i++) {
			histhreads[i] = thread(histogram_thread_function, &responce_buffer, &hc);
		}
		for (int i = 0; i < p; i++) {
			patient[i].join();
		}
		Request qm (QUIT_REQ_TYPE);
		vector<char> te ((char*)&qm, (char*)&qm + sizeof(qm));
		for (int i = 0; i < w; i++) {
			request_buffer.push(te);
		}
		for (int i = 0; i < w; i++) {
			workers[i].join();
		}
		Responce rq {-1, 0.0};
		vector<char> t ((char*) &rq, (char*) &rq + sizeof(rq));
		for (int i = 0; i < a; i++) {
			responce_buffer.push(t);
		}
		for (int i = 0; i < a; i++) {
			histhreads[i].join();
		}
		hc.print ();
	}
	else {
		fileThread = thread(file_thread_function, filename, m, chan, &request_buffer);
		fileThread.join();
		Request quitft (QUIT_REQ_TYPE);
		vector<char> te ((char*)&quitft, (char*)&quitft + sizeof(quitft));
		for (int i = 0; i < w; i++) {
			request_buffer.push(te);
		}
		for (int i = 0; i < w; i++) {
			workers[i].join();
		}
	}

    gettimeofday (&end, 0);

    // print the results and time difference
    int secs = (end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec)/(int) 1e6;
    int usecs = (int)(end.tv_sec * 1e6 + end.tv_usec - start.tv_sec * 1e6 - start.tv_usec)%((int) 1e6);
    std::cout << "Took " << secs << " seconds and " << usecs << " micro seconds" << endl;
	
	// closing the channel    
    Request q (QUIT_REQ_TYPE);
    chan->cwrite (&q, sizeof (Request));
	/*for (int i = 0; i < w; i++) {
		delete wChan[i];
	}
	delete chan;*/
	// client waiting for the server process, which is the child, to terminate
	std::cout << "Client process exited" << endl;
}