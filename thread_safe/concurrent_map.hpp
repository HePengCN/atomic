/**
*******************************************************************************
*
*
******************************************************************************
* @file
* @brief
*******************************************************************************
**/
#pragma once


#include <map>
#include <stdint.h>
#include <boost/thread/mutex.hpp>
#include <boost/thread/condition_variable.hpp>

template<class Key, class T>
class concurrent_map
{
private:
    std::map<Key, T> the_map;
    mutable boost::mutex the_mutex;
    boost::condition_variable the_condition_variable;
public:
    void insert(const Key &inputKey, const T &inputValue) {
        boost::mutex::scoped_lock lock(the_mutex);
        the_map.insert(std::pair<Key, T>(inputKey, inputValue));
        lock.unlock();
        the_condition_variable.notify_all();
    }

    bool empty() const {
        boost::mutex::scoped_lock lock(the_mutex);
        return the_map.empty();
    }

    bool try_get(const Key &inputKey, T &outputValue) {
        boost::mutex::scoped_lock lock(the_mutex);
        if(the_map.empty()) {
            return false;
        }

        typename std::map<Key, T>::iterator it;
        it = the_map.find(inputKey);

        if(the_map.end() == it) {
            return false;
        }

        outputValue = it->second;
        return true;
    }

    void wait_and_get(const Key &inputKey, T &outputValue) {
        boost::mutex::scoped_lock lock(the_mutex);
        while(the_map.empty()) {
            the_condition_variable.wait(lock);
        }

        typename std::map<Key, T>::iterator it;
        it = the_map.find(inputKey);

        while(the_map.end() == it) {
            the_condition_variable.wait(lock);
            it = the_map.find(inputKey);
        }

        outputValue = it->second;
    }

    void wait_next_insert() {
        boost::mutex::scoped_lock lock(the_mutex);
        the_condition_variable.wait(lock);
    }

    void erase(const Key &inputKey) {
        boost::mutex::scoped_lock lock(the_mutex);
        the_map.erase(inputKey);
    }

    size_t size() const {
        boost::mutex::scoped_lock lock(the_mutex);
        return the_map.size();
    }
};


