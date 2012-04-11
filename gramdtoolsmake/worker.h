/*
 * worker.h
 *
 *  Created on: Apr 5, 2012
 *      Author: asanoki
 */

#ifndef WORKER_H_
#define WORKER_H_

#include <string>
#include <sstream>
#include <boost/unordered_map.hpp>
#include "reader.h"
#include "filter.h"
#include "storage.h"
#include "../common/log.h"

class Worker {
private:
	Reader &m_reader;
	Filter &m_filter;
	Storage &m_storage;
	int m_rank;
	int m_id;
public:
	Worker(int id, int rank, Reader &reader, Storage &storage, Filter &filter) :
			m_id(id), m_rank(rank), m_reader(reader), m_storage(storage), m_filter(
					filter) {
	}
	~Worker() {
	}
	void run() {
		std::wstring raw_line;
		std::wstring line;
		do {
			raw_line = m_reader.readLine();
			line = m_filter.process(raw_line);
			for (int index = 0; index < (int)line.length() - m_rank + 1; index++) {
				m_storage.add(line.substr(index, m_rank), 1);
			}
		} while (!raw_line.empty());
	}
	void operator()() {
		run();
	}
};

#endif /* WORKER_H_ */
