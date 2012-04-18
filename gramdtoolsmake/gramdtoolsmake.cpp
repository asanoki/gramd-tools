/*
 * gramdtoolsmake.cpp
 *
 *  Created on: Apr 5, 2012
 *      Author: asanoki
 */

#include <boost/program_options.hpp>
#include <boost/thread.hpp>
#include <boost/shared_ptr.hpp>
#include <string>

#include "worker.h"
#include "reader.h"
#include "filter.h"
#include "storage.h"
#include "storage_impl.h"
#include "../common/log.h"

namespace po = boost::program_options;

int main(int argc, char **argv) {
	using namespace jovislab;
	using namespace jovislab::gramd::tools::make;

	try {
		Log::error.enable();

		// Initialize
		po::options_description desc;
		desc.add_options()("limit", po::value<int>(), "memory usage threshold, after which cache files will be used, default: 1000000");
		desc.add_options()("rank,r", po::value<int>(), "rank of collected n-grams, default: 3");
		desc.add_options()("entry-size,e", po::value<std::string>(), "maximum length of each n-gram in utf-8 format (ES_ANY, ES_2, ES_4, ES_6, ES_8, ES_10, ES_12, ES_14, ES_16, ES_24, ES_32, ES_40, ES_48, ES_56, ES_64), default: ES_ANY");
		desc.add_options()("threads,j", po::value<int>(), "number of threads, default: 2");
		desc.add_options()("help", "display help message");
		desc.add_options()("verbose,v", "output useful information");
		desc.add_options()("debug,d", "output debug information");
		desc.add_options()("quiet,q", "do not display any startup messages");
		desc.add_options()("filter,F", po::value<std::string>(),
				"filename with regular expressions to be applied on each input line");
		desc.add_options()("locale,l", po::value<std::string>(),
				"define custom locale");
		desc.add_options()("input,I", po::value<std::vector<std::string> >(),
				"input files (--input or -I can be omitted)");
		desc.add_options()("output,O", po::value<std::string>(),
				"Output filename");

		po::positional_options_description p;
		p.add("input", -1);

		po::variables_map vm;
		po::store(
				po::command_line_parser(argc, argv).options(desc).positional(p).run(),
				vm);
		po::notify(vm);

		if (vm.count("help")) {
			std::cout << desc;
			return -1;
		}

		Log::message.enable();
		Log::warning.enable();

		if (vm.count("verbose")) {
			Log::info.enable();
		}

		if (vm.count("debug")) {
			Log::debug.enable();
		}

		if (vm.count("quiet")) {
			Log::debug.disable();
			Log::info.disable();
			Log::message.disable();
			Log::notice.disable();
			Log::warning.disable();
			Log::error.disable();
		}

		// Set dictionary locale
		encoding::setUtf8InternalLocale();

		// Configure global locale
		if (vm.count("locale")) {
			std::string locale_name = vm["locale"].as<std::string>();
			try {
				std::locale::global(std::locale(locale_name.c_str()));
				setlocale(LC_ALL, locale_name.c_str());
			} catch (...) {
				ERROR("Unable to set locale to: " << locale_name.c_str());
				return -1;
			}
		} else {
			// Default locale
			try {
				std::locale::global(std::locale(""));
				setlocale(LC_ALL, "");
			} catch (...) {
				WARNING("Unable to set to system locale. Trying C instead.");
				std::locale::global(std::locale("C"));
				setlocale(LC_ALL, "C");
			}
		}

		// Objects
		Reader reader;
		Filter filter;
		Storage storage;
		std::vector<boost::shared_ptr<Worker> > workers;
		boost::thread_group threads;
		int rank = 3;
		Storage::EntrySize entry_size;
		int threads_count = 2;
		std::string output_filename;

		// Final checks for required fields
		if (!vm.count("input")) {
			std::cout << desc;
			return -1;
		}

		if (!vm.count("output")) {
			std::cout << desc;
			return -1;
		} else {
			output_filename = vm["output"].as<std::string>();
		}

		if (vm.count("rank")) {
			rank = vm["rank"].as<int>();
		}

		if (vm.count("entry-size")) {
			std::string tmp = vm["entry-size"].as<std::string>();
			if (tmp == "ES_ANY")
				entry_size = Storage::ES_ANY;
			else if (tmp == "ES_2")
				entry_size = Storage::ES_2;
			else if (tmp == "ES_4")
				entry_size = Storage::ES_4;
			else if (tmp == "ES_6")
				entry_size = Storage::ES_6;
			else if (tmp == "ES_8")
				entry_size = Storage::ES_8;
			else if (tmp == "ES_10")
				entry_size = Storage::ES_10;
			else if (tmp == "ES_12")
				entry_size = Storage::ES_12;
			else if (tmp == "ES_14")
				entry_size = Storage::ES_14;
			else if (tmp == "ES_16")
				entry_size = Storage::ES_16;
			else if (tmp == "ES_24")
				entry_size = Storage::ES_24;
			else if (tmp == "ES_32")
				entry_size = Storage::ES_32;
			else if (tmp == "ES_40")
				entry_size = Storage::ES_40;
			else if (tmp == "ES_48")
				entry_size = Storage::ES_48;
			else if (tmp == "ES_56")
				entry_size = Storage::ES_56;
			else if (tmp == "ES_64")
				entry_size = Storage::ES_64;
			else {
				ERROR("Unknown entry-size value: " << tmp.c_str());
				return -1;
			}
		}

		storage.initialize(rank, entry_size);

		if (vm.count("threads")) {
			threads_count = vm["threads"].as<int>();
		}

		MESSAGE("[1/3] Initializing...");
		// Read input files
		std::vector<std::string> inputs =
				vm["input"].as<std::vector<std::string> >();
		if (reader.initialize(inputs) < 0) {
			ERROR("Unable to read input files.");
			return -1;
		}

		// Read filter file
		if (vm.count("filter")) {
			if (filter.initialize(vm["filter"].as<std::string>()) < 0) {
				ERROR("Unable to read the filter file.");
				return -1;
			}
		}

		MESSAGE("[2/3] Collecting n-grams...");
		// Construct worker threads
		for (int index = 0; index < threads_count; index++) {
			boost::shared_ptr<Worker> worker(new Worker(index, rank, reader, storage, filter));
			workers.push_back(worker);
			threads.create_thread(*worker);
		}

		// Run threads and wait
		threads.join_all();

		// Close reader (and display summary)
		reader.close();

		MESSAGE("[3/3] Saving output file...");
		// Save storage
		if (storage.saveText(output_filename) < 0) {
			ERROR("Unable to write to output file.");
			return -1;
		}

		MESSAGE("Done.");

	} catch (std::exception &e) {
		ERROR(e.what());
		return -1;
	}

	return 0;
}
