/*
 * storage.h
 *
 *  Created on: Apr 10, 2012
 *      Author: asanoki
 */

#ifndef STORAGE_H_
#define STORAGE_H_

#include <cstddef>
#include <fstream>
#include <iostream>
#include <vector>
#include <limits>
#include <string>
#include <cstring>
#include <boost/interprocess/sync/scoped_lock.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/unordered_map.hpp>
#include "../common/log.h"
#include "../common/encoding.h"
#include "../common/boost_extras/wprogress.h"
#include <map>

#ifdef HAVE_GOOGLE_SPARSE_HASH_MAP
#include <google/sparse_hash_map>
#endif

#include <boost/array.hpp>
#include <boost/functional/hash/extensions.hpp>

namespace boost {
template<class T, std::size_t Size>
inline std::size_t hash_value(boost::array<T, Size> const &a) {
	return hash_range(a.begin(), a.end());
}
}

#ifdef HAVE_GOOGLE_SPARSE_HASH_MAP
namespace std {
namespace tr1 {
template<std::size_t Size>
struct hash<boost::array<char, Size> > {
	std::size_t operator()(boost::array<char, Size> const &a) const {
		return boost::hash_range(a.begin(), a.end());
	}
};
}
}
#endif

#ifdef HAVE_GOOGLE_SPARSE_HASH_MAP
	#define GRAMDTOOLSMAKE_MAP_TYPE google::sparse_hash_map
#else
#	define GRAMDTOOLSMAKE_MAP_TYPE boost::unordered_map
#endif

namespace jovislab {
namespace gramd {
namespace tools {
namespace make {

template<class Type>
void storageFromString(const std::string &input, std::basic_string<Type> &output) {
	output = input;
}

template<class Type, size_t Capacity>
void storageFromString(const std::string &input, boost::array<Type, Capacity> &output) {
	output.assign(0);
	std::copy(input.begin(), input.end(), output.begin());
}

template<class Type, size_t Capacity>
std::size_t storageLength(const boost::array<Type, Capacity> &holder) {
	return Capacity;
}

template<class Type>
std::size_t storageLength(const std::basic_string<Type> &holder) {
	return holder.length();
}

template<class Type, size_t Capacity>
std::string storageToString(const boost::array<Type, Capacity> &holder) {
	std::string result;
	if (holder[Capacity - 1] != 0) {
		result.assign(&holder.elems[0], Capacity);
	} else {
		result.assign(&holder.elems[0]);
	}
	return result;
}

template<class Type>
std::string storageToString(const std::basic_string<Type> &holder) {
	return holder;
}

class StorageEssentialBase {
public:
	virtual int saveText(const std::string filename) = 0;
	virtual void add(std::vector<std::string> &grams) = 0;
	virtual ~StorageEssentialBase() {
	}
};

class StorageBase: public StorageEssentialBase {
public:
	virtual int initialize(int rank) = 0;
	virtual int initialize() = 0;
	virtual ~StorageBase() {
	}
};

template<class HolderType, std::size_t MaximumCapacity = 0>
class BasicStorage: public StorageBase {
public:
	typedef GRAMDTOOLSMAKE_MAP_TYPE<HolderType, unsigned int> MapType;
private:
	boost::mutex m_mutex;
	int m_rank;
	MapType m_map;

	void mapInsert(const std::string &gram) {
		size_t len = gram.length();
		HolderType tmp;
		storageFromString(gram, tmp);
		m_map[tmp]++;
	}

	void radixSort(std::vector<HolderType> &vector) {
		INFO("Sorting keys: " << vector.size());
		boost::shared_ptr<boost::wprogress_display> progress_display;
		int ls_counts[256];
		int ls_sums[256];
		int local_counter = 0;
		size_t max_size = 0;
		for (typename std::vector<HolderType>::iterator it = vector.begin(); it != vector.end(); it++) {
			const HolderType &value = *it;
			std::size_t size = storageLength(value);
			if (size > max_size) {
				max_size = size;
			}
		}
		INFO("Maximum length of entry: " << max_size);
		progress_display.reset(new boost::wprogress_display(max_size * vector.size() / 10000));
		unsigned int ls_indexes[256];
		std::vector<HolderType> result;
		result.resize(vector.size());
		for (int index = max_size - 1; index >= 0; index--) {
			for (int k = 0; k < 256; k++) {
				ls_counts[k] = 0;
				ls_sums[k] = 0;
			}
			for (typename std::vector<HolderType>::iterator it = vector.begin(); it != vector.end(); it++) {
				const HolderType &value = *it;
				unsigned char code = 0;
				if (index < storageLength(value)) {
					code = value[index];
				}
				ls_counts[code]++;
			}
			for (int k = 1; k < 256; k++) {
				ls_sums[k] = ls_sums[k - 1] + ls_counts[k - 1];
			}
			for (typename std::vector<HolderType>::iterator it = vector.begin(); it != vector.end(); it++) {
				const HolderType &value = *it;
				unsigned char code = 0;
				if (index < storageLength(value)) {
					code = value[index];
				}
				result[ls_sums[code]] = value;
				ls_sums[code]++;
				local_counter++;
				if (local_counter == 10000) {
					local_counter = 0;
					progress_display.get()->operator++();
				}
			}
			result.swap(vector);
		}
		progress_display.reset();
		INFO("Sorted.");
	}
public:
	BasicStorage() {
		m_rank = 0;
	}

	BasicStorage(int rank) : m_rank(rank) {
	}

	virtual ~BasicStorage() {
	}

	int initialize(int rank) {
		m_rank = rank;
		return initialize();
	}

	int initialize() {
		return 0;
	}

	MapType &map() {
		return *m_map;
	}

	void add(std::vector<std::string> &grams) {
		// TODO: Consider using critical sections inside the hashmap.
		boost::mutex::scoped_lock lock(m_mutex);
		DEBUG("Storage::add: Begin (after lock). Adding bunch of pairs.");
		for (std::vector<std::string>::iterator it = grams.begin(); it != grams.end(); it++) {
			if (MaximumCapacity > 0 && it->length() > MaximumCapacity) {
				WARNING("Storage::add: Specified node size is too small to store n-gram of length: " << (it->length()) << " bytes.");
			}
			mapInsert(*it);
		}
		DEBUG("Storage::add: End.");
	}

	int saveText(const std::string filename) {
		INFO("Saving as text...");
		std::ofstream output(filename.c_str(),
				std::ios::out);
		output.imbue(std::locale("C"));
		if (!output.good())
			return -1;
		output << m_rank << "\n";
		std::vector<HolderType> keys;
		for (typename MapType::iterator it = m_map.begin(); it != m_map.end(); it++) {
			keys.push_back(it->first);
		}
		radixSort(keys);
		for (typename std::vector<HolderType>::iterator it = keys.begin(); it != keys.end(); it++) {
			HolderType &key = *it;
			unsigned int value = m_map[key];
			output << storageToString(key) << "\t" << value << "\n";
		}
		INFO("Saved.");
		return 0;
	}
};

template<class Type, size_t Capacity>
class ConstantStorage: public BasicStorage<boost::array<Type, Capacity>
		, Capacity> {
public:
	ConstantStorage() :
			BasicStorage<boost::array<Type, Capacity>, Capacity>() {
	}

	ConstantStorage(int rank) :
			BasicStorage<boost::array<Type, Capacity>, Capacity>(rank) {
	}

	~ConstantStorage() {

	}
};

template<class Type>
class DynamicStorage: public BasicStorage<std::basic_string<Type>, 0> {
public:
	DynamicStorage() :
			BasicStorage<std::basic_string<Type>, 0>() {
	}

	DynamicStorage(int rank) :
			BasicStorage<std::basic_string<Type>, 0>(rank) {
	}

	~DynamicStorage() {

	}
};

}
}
}
}

#endif /* STORAGE_H_ */
