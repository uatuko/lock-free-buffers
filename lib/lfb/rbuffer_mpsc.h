/*
*  lock-free-buffers - A collection of lock free buffer algorithms
*  Copyright (C) 2014 Uditha Atukorala
*
*  This software library is free software; you can redistribute it and/or modify
*  it under the terms of the GNU Lesser General Public License as published by
*  the Free Software Foundation; either version 3 of the License, or
*  (at your option) any later version.
*
*  This software library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU Lesser General Public License for more details.
*
*  You should have received a copy of the GNU Lesser General Public License
*  along with this software library. If not, see <http://www.gnu.org/licenses/>.
*
*/

#ifndef LFB_RBUFFER_MPSC_H
#define LFB_RBUFFER_MPSC_H

#include <atomic>
#include <cstring>


namespace lfb {

	template<class T>
	class rbuffer {
	public:

		typedef T data_t;

		struct rbuffer_t {
			void * s_addr;
			void * e_addr;
			void * t_ptr;

			std::atomic<void *> h_ptr;
			std::atomic<size_t> size;

			rbuffer_t() :
				s_addr( 0 ), e_addr( 0 ),
				h_ptr( 0 ), t_ptr( 0 ),
				size( 0 ) { }
		};



		rbuffer( size_t size ) : _capacity( size ) {

			_rbuffer.s_addr = static_cast<void *>( new data_t[ _capacity ] );
			_rbuffer.e_addr = static_cast<char *>( _rbuffer.s_addr ) + ( _capacity * sizeof( data_t ) );

			_rbuffer.h_ptr = _rbuffer.s_addr;
			_rbuffer.t_ptr = _rbuffer.s_addr;

		}


		virtual ~rbuffer() {

			if ( _rbuffer.s_addr ) {
				delete [] static_cast<char *>( _rbuffer.s_addr );
			}

		}


		size_t push( const data_t &data ) {

			size_t size      = _rbuffer.size.load( std::memory_order_relaxed );
			void * head      = 0;
			void * next_head = 0;

			// reserve space in the buffer
			do {

				if ( size >= _capacity ) {
					return 0;
				}

				// refresh buffer head
				head = _rbuffer.h_ptr.load( std::memory_order_relaxed );

			} while (! _rbuffer.size.compare_exchange_weak( size, ( size + 1 ) ) );


			// update the buffer head
			do {

				// calculate next head
				next_head = static_cast<char *>( head ) + sizeof( data_t );
				if ( next_head == _rbuffer.e_addr ) {
					next_head = _rbuffer.s_addr;
				}

			} while (! _rbuffer.h_ptr.compare_exchange_weak( head, next_head ) );


			// copy data
			memcpy( head, &data, sizeof( data_t ) );

			return size;
		}


		size_t popn( data_t * buffer, size_t n ) {

			size_t size   = committed_read_size();
			size_t offset = 0;

			if ( n == 0 || size == 0 ) {
				return 0;
			}

			if ( n > size ) {
				n = size;
			}


			// calculate the byte size
			size = sizeof( data_t ) * n;

			if ( ( static_cast<char *>( _rbuffer.t_ptr ) + size ) > _rbuffer.e_addr ) {

				offset = static_cast<char *>( _rbuffer.e_addr ) - static_cast<char *>( _rbuffer.t_ptr );
				memcpy( buffer, _rbuffer.t_ptr, offset );
				size -= offset;
				_rbuffer.t_ptr = _rbuffer.s_addr;

			}

			memcpy( buffer + offset, _rbuffer.t_ptr, size );

			_rbuffer.t_ptr = static_cast<char *>( _rbuffer.t_ptr ) + size;
			if ( _rbuffer.t_ptr == _rbuffer.e_addr ) {
				_rbuffer.t_ptr = _rbuffer.s_addr;
			}

			_rbuffer.size -= n;
			return n;

		}


	private:

		rbuffer_t _rbuffer;

		const size_t _capacity;

		size_t committed_read_size() const {

			size_t size = 0;

			if ( _rbuffer.h_ptr >= _rbuffer.t_ptr ) {
				size = static_cast<char *>( _rbuffer.h_ptr.load( std::memory_order_relaxed ) ) - static_cast<char *>( _rbuffer.t_ptr );
			} else {
				size = ( static_cast<char *>( _rbuffer.e_addr ) - static_cast<char *>( _rbuffer.t_ptr ) ) +
						( static_cast<char *>( _rbuffer.h_ptr.load( std::memory_order_relaxed ) ) - static_cast<char *>( _rbuffer.s_addr ) );
			}

			// convert byte size into items
			size = size / sizeof( data_t );

			// if head and tail pointers are at the same location
			// it could be:
			//    1 - buffer is empty
			//    2 - buffer is at full capacity
			if ( size == 0 && _rbuffer.size == _capacity ) {
				size = _capacity;
			}

			return size;

		}

	};

} /* end of namespace lfb */

#endif /* LFB_RBUFFER_MPSC_H */

