/*
*  lock-free-buffers - A collection of lock free buffer algorithms
*  Copyright (C) 2014 Uditha Atukorala
*
*  This program is free software: you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation, either version 3 of the License, or
*  (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#include "lfb/rbuffer_mpsc.h"

#include <thread>
#include <list>
#include <future>
#include <vector>
#include <iostream>

#define QUEUE_SIZE           1024
#define CONSUMER_BUFFER_SIZE 4
#define MAX_PRODUCERS        4
#define PRODUCER_MSG_LIMIT   100000


// globals
lfb::rbuffer<size_t> g_queue( QUEUE_SIZE );


size_t producer( size_t t_id ) {

	size_t failed = 0;
	for ( size_t i = 0; i < PRODUCER_MSG_LIMIT; i++ ) {
		if (! g_queue.push( i + ( t_id * PRODUCER_MSG_LIMIT ) ) ) {
			failed++;
			std::this_thread::yield();
		}
	}

	return failed;

}


int main() {

	std::vector<std::future<size_t>> thread_futures;
	std::list<size_t> msg_list;

	size_t consumed_total = 0;
	size_t failed_total   = 0;
	size_t delayed_total = 0;
	size_t m_buf[CONSUMER_BUFFER_SIZE];
	size_t m_count;


	// producer threads
	for ( size_t i = 0; i < MAX_PRODUCERS; i++ ) {
		auto task = std::packaged_task<size_t()>( std::bind( producer, ( i + 1 ) ) );
		thread_futures.push_back( std::move( task.get_future() ) );
		std::thread( std::move( task ) ).detach();

	}

	std::this_thread::yield();


	// consume
	size_t msg   = 0;
	while ( ( m_count = g_queue.popn( m_buf, CONSUMER_BUFFER_SIZE ) ) ) {
		for ( size_t i = 0; i < m_count; i++) {
			msg_list.push_back( m_buf[i] );
			consumed_total++;
		}
	}

	// calculate failed msg count from thread futures
	for ( std::future<size_t> &f : thread_futures ) {
		f.wait();
		failed_total += f.get();
	}

	// calculate messages that didn't get consumed before
	while ( g_queue.popn( &msg, 1 ) ) {
		msg_list.push_back( msg );
		delayed_total++;
	}


	std::cout << "consumed total: " << consumed_total << std::endl;
	std::cout << "delayed total: " << delayed_total << std::endl;
	std::cout << "failed total: " << failed_total << std::endl;

	std::cout << "list size: " << msg_list.size() << std::endl;

	msg_list.sort();
	msg_list.unique();
	std::cout << "list unique: " << msg_list.size() << std::endl;

	std::cout << "total: " << ( consumed_total + delayed_total + failed_total ) << std::endl;


	return 0;

}

