/*
 * storage.h
 *
 *  Created on: Apr 10, 2012
 *      Author: asanoki
 */

#ifndef STORAGE_H_
#define STORAGE_H_

#include <fstream>
#include <iostream>
#include <vector>
#include <limits>
#include <string>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/unordered_map.hpp>
#include "../common/log.h"
#include "../common/encoding.h"

#include <google/sparse_hash_map>

#define GRAMDTOOLSMAKE_MAP_TYPE google::sparse_hash_map
//#define GRAMDTOOLSMAKE_MAP_TYPE boost::unordered_map

class Storage {
private:
	boost::mutex m_mutex;
	GRAMDTOOLSMAKE_MAP_TYPE<std::string, unsigned int> m_map;
	
	char *m_add_char_buffer;
	int m_rank;
public:
	Storage() {
		m_rank = 0;
		m_add_char_buffer = 0;
	}
	Storage(int rank) :
			m_rank(rank) {
		m_add_char_buffer = 0;
	}
	~Storage() {
		if (m_add_char_buffer != 0)
			delete [] m_add_char_buffer;
	}
	int initialize(int rank) {
		m_rank = rank;
		return initialize();
	}
	int initialize() {
		m_add_char_buffer = new char[m_rank * 16 + 1];
		return 0;
	}
	void add(const std::wstring gram, int count) {
		boost::mutex::scoped_lock lock(m_mutex);
		encoding::exportAsUtf8(m_add_char_buffer, gram.c_str(), m_rank * 16 + 1, gram.length() + 1);
		m_map[m_add_char_buffer] = m_map[m_add_char_buffer] + count;
	}
	int saveBinary(const std::string filename) {
		std::ofstream output(filename.c_str(),
				std::ios::out | std::ios::binary);
		if (!output.good())
			return -1;

		output.write((char*) &m_rank, sizeof(int));

		std::vector<std::string> keys;
		for (GRAMDTOOLSMAKE_MAP_TYPE<std::string, unsigned int>::iterator it = m_map.begin(); it != m_map.end(); it++) {
		    keys.push_back(it->first);
		}
		std::sort(keys.begin(), keys.end());

		char *char_buffer_current = new char[m_rank * 16 + 1];
		const char *char_buffer;

		for (std::vector<std::string>::iterator it = keys.begin(); it != keys.end(); it++) {
			std::string key = *it;
			unsigned int value = m_map[key];
			unsigned char packet_flags = 0;
			char_buffer = key.c_str();

			int key_byte_size = key.length();
			int to_update = key.length();

			if (char_buffer_current[0] != 0) {
				int index = 0;
				while (to_update > 0) {
					if (char_buffer_current[index] != char_buffer[index]) {
						break;
					}
					to_update--;
					index++;
				}
			}

			assert(to_update <= 63);
			packet_flags = to_update << 2;

			if (value <= std::numeric_limits<unsigned char>::max()) {
				packet_flags |= 0x1;
			}
			else if (value <= std::numeric_limits<unsigned short>::max()) {
				packet_flags |= 0x2;
			}
			else if (value <= std::numeric_limits<unsigned int>::max()) {
				packet_flags |= 0x3;
			}

			output.write((char*) &packet_flags, sizeof(unsigned char));
			output.write((char*) (char_buffer + key_byte_size - to_update), sizeof(char) * to_update);

			if (value <= std::numeric_limits<unsigned char>::max()) {
				unsigned char packet_value = value;
				output.write((char*) &packet_value, sizeof(unsigned char));
			}
			else if (value <= std::numeric_limits<unsigned short>::max()) {
				unsigned short packet_value = value;
				output.write((char*) &packet_value, sizeof(unsigned short));
			}
			else {
				unsigned int packet_value = value;
				output.write((char*) &packet_value, sizeof(unsigned int));
			}

			strcpy(char_buffer_current, char_buffer);
		}
		delete [] char_buffer_current;

		output.close();
		return 0;
	}
	int saveText(const std::string filename) {
		std::ofstream output(filename.c_str(),
				std::ios::out);
		output.imbue(std::locale("C"));

		if (!output.good())
			return -1;

		output << m_rank << "\n";

		std::vector<std::string> keys;
		for (GRAMDTOOLSMAKE_MAP_TYPE<std::string, unsigned int>::iterator it = m_map.begin(); it != m_map.end(); it++) {
		    keys.push_back(it->first);
		}
		std::sort(keys.begin(), keys.end());

		for (std::vector<std::string>::iterator it = keys.begin(); it != keys.end(); it++) {
			const std::string &key = *it;
			long long value = m_map[key];
			output << key.c_str() << "\t" << value << "\n";
		}

		return 0;
	}
};

#endif /* STORAGE_H_ */
