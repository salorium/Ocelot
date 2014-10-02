#include "ocelot.h"
#include "config.h"
#include "db.h"
#include "worker.h"
#include "events.h"
#include "schedule.h"
#include "response.h"
#include <cerrno>
#include <mutex>

// Define the connection mother (first half) and connection middlemen (second half)

//TODO Better errors

//---------- Connection mother - spawns middlemen and lets them deal with the connection

connection_mother::connection_mother(worker * worker_obj, config * config_obj, mysql * db_obj, site_comm * sc_obj) : work(worker_obj), conf(config_obj), db(db_obj), sc(sc_obj) {
	memset(&address, 0, sizeof(address));
	addr_len = sizeof(address);

	listen_socket = socket(AF_INET6, SOCK_STREAM, 0);

	// Stop old sockets from hogging the port
	int yes = 1;
	if (setsockopt(listen_socket, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int)) == -1) {
		std::cout << "Could not reuse socket" << std::endl;
	}
    int i =0;
    int status = setsockopt(listen_socket, IPPROTO_IPV6, IPV6_V6ONLY,&i, sizeof i);
    if (status < 0) {
        fprintf(stderr, "Cannot set options on the socket: %s\n",
                strerror(errno));
        abort();
    }
	// Create libev event loop
	ev::io event_loop_watcher;

	event_loop_watcher.set<connection_mother, &connection_mother::handle_connect>(this);
	event_loop_watcher.start(listen_socket, ev::READ);

	// Get ready to bind
/*	address.sin6_family = AF_INET6;
	//address.sin_addr.s_addr = inet_addr(conf->host.c_str()); // htonl(INADDR_ANY)
	address.sin6_addr = htonl(INADDR_ANY);
	address.sin6_port = htons(conf->port);*/
address.sin6_family = AF_INET6;
    address.sin6_addr = in6addr_any;
    address.sin6_port = htons(conf->port);


	// Bind
	if (bind(listen_socket, (sockaddr *) &address, sizeof(address)) == -1) {
		std::cout << "Bind failed " << errno << std::endl;
	}

	// Listen
	if (listen(listen_socket, conf->max_connections) == -1) {
		std::cout << "Listen failed" << std::endl;
	}

	// Set non-blocking
	int flags = fcntl(listen_socket, F_GETFL);
	if (flags == -1) {
		std::cout << "Could not get socket flags" << std::endl;
	}
	if (fcntl(listen_socket, F_SETFL, flags | O_NONBLOCK) == -1) {
		std::cout << "Could not set non-blocking" << std::endl;
	}

	// Create libev timer
	schedule timer(this, worker_obj, conf, db, sc);

	schedule_event.set<schedule, &schedule::handle>(&timer);
	schedule_event.set(conf->schedule_interval, conf->schedule_interval); // After interval, every interval
	schedule_event.start();

	std::cout << "Sockets up, starting event loop!" << std::endl;
	ev_loop(ev_default_loop(0), 0);
}


void connection_mother::handle_connect(ev::io &watcher, int events_flags) {
	// Spawn a new middleman
	if (stats.open_connections < conf->max_middlemen) {
		std::unique_lock<std::mutex> lock(stats.mutex);
		stats.opened_connections++;
		lock.unlock();
        addr_size= sizeof(peers);
		new connection_middleman(listen_socket, peers,addr_size, work, this, conf);
	}
}

connection_mother::~connection_mother()
{
	close(listen_socket);
}







//---------- Connection middlemen - these little guys live until their connection is closed

connection_middleman::connection_middleman(int &listen_socket, sockaddr_storage &address, socklen_t &addr_len, worker * new_work, connection_mother * mother_arg, config * config_obj) :
	conf(config_obj), mother (mother_arg), work(new_work) {

	connect_sock = accept(listen_socket, (sockaddr *) &address, &addr_len);
	if (connect_sock == -1) {
		std::cout << "Accept failed, errno " << errno << ": " << strerror(errno) << std::endl;
		delete this;
		std::unique_lock<std::mutex> lock(stats.mutex);
		stats.open_connections++; // destructor decrements open connections
		return;
	}

	// Set non-blocking
	int flags = fcntl(connect_sock, F_GETFL);
	if (flags == -1) {
		std::cout << "Could not get connect socket flags" << std::endl;
	}
	if (fcntl(connect_sock, F_SETFL, flags | O_NONBLOCK) == -1) {
		std::cout << "Could not set non-blocking" << std::endl;
	}

	// Get their info
	if (getpeername(connect_sock, (sockaddr *) &client_addr, &addr_len) == -1) {
		//std::cout << "Could not get client info" << std::endl;
	}
	request.reserve(conf->max_read_buffer);
	written = 0;

	read_event.set<connection_middleman, &connection_middleman::handle_read>(this);
	read_event.start(connect_sock, ev::READ);

	// Let the socket timeout in timeout_interval seconds
	timeout_event.set<connection_middleman, &connection_middleman::handle_timeout>(this);
	timeout_event.set(conf->timeout_interval, 0);
	timeout_event.start();

	std::unique_lock<std::mutex> lock(stats.mutex);
	stats.open_connections++;
}

connection_middleman::~connection_middleman() {
	close(connect_sock);
	std::unique_lock<std::mutex> lock(stats.mutex);
	stats.open_connections--;
}

// Handler to read data from the socket, called by event loop when socket is readable
void connection_middleman::handle_read(ev::io &watcher, int events_flags) {
	char buffer[conf->max_read_buffer + 1];
	memset(buffer, 0, conf->max_read_buffer + 1);
	int ret = recv(connect_sock, &buffer, conf->max_read_buffer, 0);

	if (ret <= 0) {
		delete this;
		return;
	}
	std::unique_lock<std::mutex> lock(stats.mutex);
	stats.bytes_read += ret;
	lock.unlock();
	request.append(buffer, ret);
	size_t request_size = request.size();
	if (request_size > conf->max_request_size || (request_size >= 4 && request.compare(request_size - 4, std::string::npos, "\r\n\r\n") == 0)) {
		read_event.stop();

		if (request_size > conf->max_request_size) {
			shutdown(connect_sock, SHUT_RD);
			response = error("GET string too long");
		} else {
			char ip[INET6_ADDRSTRLEN];
            struct sockaddr_in6 *address_v6;
            address_v6 =((struct sockaddr_in6 *) ((struct sockaddr *)&client_addr));
			inet_ntop(AF_INET6, &address_v6->sin6_addr, ip, INET6_ADDRSTRLEN);
			std::string ip_str = ip;
            std::cout << ip_str << std::endl;
            bool ipv6= false;

            std::size_t found= ip_str.find('.');
            if (found!=std::string::npos){
                std::cout<<"IPV4"<<std::endl;
                std::size_t f = ip_str.find_last_of(':');
                std::cout<< f <<std::endl;
                ip_str =ip_str.substr (++f);
            }else{
                ipv6 = true;
                unsigned int nbdetoken = 7;
                std::cout<<"IPV6"<<std::endl;
                std::size_t n = std::count(ip_str.begin(), ip_str.end(), ':');
                std::size_t nn = ip_str.find(":");
                std::cout<< "Nb :"<< n << "\n";
                if (n == 2 && nn == 0) {
                    std::cout<< "=============\n";
                    n = 1;
                }

                std::cout<< n<<(n==2 ? "Egale 2":"Pas egale") <<"\n";
                if ( n < nbdetoken){
                    std::string complete = "";
                    for (int i=0, end= nbdetoken - n+1;i < end;i++){
                        complete = complete+ "0000:";
                    }
                    size_t t = ip_str.find("::");
                    if ( t!= 0)complete = ":"+complete;
                    ip_str.replace(t,2,complete);
                }
                if (ip_str.length() == 39){
                    std::cout<<"Good\n";
                }else{
                    unsigned int debut = 4;
                    unsigned int debutsrc = 0;
                    for (int i=0;i < 7 ; i++){
                        if (ip_str.find(":",debutsrc) != debut){
                            std::cout<< "insert 0"<<debutsrc<<"\n";
                            do{
                                ip_str.insert(debutsrc,"0");
                            }while(ip_str.find(":",debutsrc)!= debut);
                        }
                        debutsrc +=5;
                        debut += 5;
                    }
                    do{
                        ip_str.insert(debutsrc,"0");
                    }while(ip_str.length()<39);
                }
                std::cout<<"il y a "<<(nbdetoken- n)<< " : \n";
            }


            std::cout << ip_str << std::endl;

			//--- CALL WORKER
			response = work->work(request, ip_str,ipv6);
		}

		// Find out when the socket is writeable.
		// The loop in connection_mother will call handle_write when it is.
		write_event.set<connection_middleman, &connection_middleman::handle_write>(this);
		write_event.start(connect_sock, ev::WRITE);
	}
}

// Handler to write data to the socket, called by event loop when socket is writeable
void connection_middleman::handle_write(ev::io &watcher, int events_flags) {
	int ret = send(connect_sock, response.c_str()+written, response.size()-written, MSG_NOSIGNAL);
	written += ret;
	std::unique_lock<std::mutex> lock(stats.mutex);
	stats.bytes_written += ret;
	lock.unlock();
	if (written == response.size()) {
		write_event.stop();
		timeout_event.stop();
		delete this;
	}
}

// After a middleman has been alive for timout_interval seconds, this is called
void connection_middleman::handle_timeout(ev::timer &watcher, int events_flags) {
	timeout_event.stop();
	read_event.stop();
	write_event.stop();
	delete this;
}
