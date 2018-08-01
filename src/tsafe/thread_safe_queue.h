/*
 * Copyright (c) 2018 Qwant Research
 * The source code is licenced under MIT Licence that can be found in the 
 * LICENCE file in the root directory of Mermoz
 *
 * Author:
 * Noel Martin <n.martin@qwantresearch.com>
 *
 * Copyright (c) 2010 Shadow7789
 * Use of this source code is governed by a BSD 3-Clause license that can be
 * found in the LICENSE file in this directory.
 */

#ifndef THREAD_SAFE_QUEUE_INCLUDED
#define THREAD_SAFE_QUEUE_INCLUDED

#include <queue>
#include <vector>
#include <deque>

#include <boost/thread.hpp>

namespace thread_safe {

template < class T, class Container = std::deque<T> >
class queue {
public:
    explicit queue( const Container & ctnr = Container() ) : storage( ctnr ) { }
    bool empty( void ) const { boost::lock_guard<boost::mutex> lock( mutex ); return storage.empty(); }

    size_t size( void ) const { boost::lock_guard<boost::mutex> lock( mutex ); return storage.size(); }

    T & back( void ) { boost::lock_guard<boost::mutex> lock( mutex ); return storage.back(); }
    const T & back( void ) const { boost::lock_guard<boost::mutex> lock( mutex ); return storage.back(); }

    T & front( void ) { boost::lock_guard<boost::mutex> lock( mutex ); return storage.front(); }
    const T & front( void ) const { boost::lock_guard<boost::mutex> lock( mutex ); return storage.front(); }

    void push( const T & u ) { boost::lock_guard<boost::mutex> lock( mutex ); storage.push( u ); }

    void pop( void ) { boost::lock_guard<boost::mutex> lock( mutex ); storage.pop(); }
    void pop( T & u ) { boost::lock_guard<boost::mutex> lock( mutex ); u = storage.front(); storage.pop(); }
private:
    std::queue<T, Container> storage;
    mutable boost::mutex mutex;
};

template < class T, class Container = std::vector<T>, class Compare = std::less<typename Container::value_type> >
class priority_queue {
public:
    explicit priority_queue ( const Compare& x = Compare(), const Container& y = Container() ) : storage( x, y ) { }
    template <class InputIterator> priority_queue ( InputIterator first, InputIterator last, const Compare& x = Compare(), const Container& y = Container() ) : storage( first, last, x, y ) { }

    bool empty( void ) const { boost::lock_guard<boost::mutex> lock( mutex ); return storage.empty(); }

    size_t size( void ) const { boost::lock_guard<boost::mutex> lock( mutex ); return storage.size(); }

    T & top( void ) { boost::lock_guard<boost::mutex> lock( mutex ); return storage.top(); }
    const T & top( void ) const { boost::lock_guard<boost::mutex> lock( mutex ); return storage.top(); }

    void push( const T & u ) { boost::lock_guard<boost::mutex> lock( mutex ); storage.push(); }

    void pop( void ) { boost::lock_guard<boost::mutex> lock( mutex ); storage.pop(); }
private:
    std::priority_queue< T, Container, Compare > storage;
    mutable boost::mutex mutex;
};

}

#endif // THREAD_SAFE_QUEUE_INCLUDED
