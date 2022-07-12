#ifndef LOCAL_CACHE
#define LOCAL_CACHE
#include <filesystem>
namespace fs = std::filesystem;

#include "stages.hh"
#include <fstream>
#include <iostream>
#include <vector>
#define DEBUG
namespace panosc
{

class local_cache
{

	public:
	local_cache(std::string instrument_name, std::string hash, std::string baseDir = "/dev/shm/")
	{

		// define a temp dir in RAM
		std::string dirName = baseDir + instrument_name + "/";
		_p                  = dirName;
		//	fs::create_directories(_p);
		_p /= hash;
		fs::create_directories(_p);
		//	_p += ".tgz";
	};

	// in the form of baseDir/instrument_name/hash/
	inline fs::path output_dir(void) const { return _p; };
	inline fs::path output_dir(size_t job_index) const { return _p / std::to_string(job_index); };
	fs::path        output_dir_merge(void) const { return _p / "merge"; }

	inline bool isOK(void) const
	{
		return true;
		if (!fs::exists(path_tgz()) && fs::exists(output_dir())) {
			std::cerr << "[ERROR] Sim dir already exists but not TGZ, "
			             "please clean up"
			          << std::endl;
			return false;
		}
		return true;
	}

	inline bool is_done(void) const { return fs::exists(output_dir_merge()); }
	inline bool is_done(size_t job_index) const
	{
		return fs::exists(output_dir(job_index) / "sDETECTOR.mcpl.gz");
	}

	inline fs::path path_tgz(void) const
	{
		auto p = _p;
		p += ".tgz";
		return p;
	};

	inline void save_request(std::string sim_request) const
	{
		// print the request json in the directory
		std::ofstream request_dump_file((output_dir()).string() + "/request.json");
		request_dump_file << sim_request << std::endl;
		request_dump_file.close();
	}

	inline void save_tgz(void) const
	{
		system((std::string("tar -cz -C ") + _p.parent_path().string() + " " + _p.stem().string() +
		        " > " + path_tgz().string())
		           .c_str());
	}

	/** \brief rename and move the MCPL file according to a given stage to a specific path
	 * as defined by this method
	 *
	 *  - the MCPL file is moved to the directory defined by panosc::local_cache::stage_path()
	 *  - the request is saved in the output directory
	 */
	inline void save_stage(stage_t is, std::string hash) const
	{
		auto stage_name = stages.at(is);
		auto l          = output_dir() / stage_name;
		l += ".mcpl.gz";

		// it is possible that the INSTRUMENT does not have implemented a MCPL output for all the
		// stages
		if (fs::exists(l) == false) {
			std::cerr << "[WARNING] file: " << l << " does not exists" << std::endl;
			return;
		}

		// strip the hash, add MCPL, add stage_name, add the hash
		fs::path mcpl_path = stage_path(is, hash);
		fs::create_directories(mcpl_path.parent_path());

		// copy the request json
		mcpl_path += ".json";
		fs::copy(output_dir() / "request.json", mcpl_path);

		// move the mcpl file and rename it
		mcpl_path.replace_extension(".mcpl.gz");
		fs::rename(l, mcpl_path);
	}

	inline std::pair<size_t, std::string> get_stage(std::vector<std::string> hashes) const
	{
		std::string mcpl_filename;
		stage_t     istage = sSOURCE;
		// check here if any MCPL exists for one of the stages
		// stages are ordered from the detector to the source
		for (istage = sSOURCE;
		     istage > sFULL and mcpl_filename.empty();) { // there is no MCPL for the full stage
			const auto &stage_hash = hashes[istage];
			const auto &sp         = stage_path(istage, stage_hash);
#ifdef DEBUG
			std::cout << "[INFO] check if MCPL file for stage " << istage
			          << " exists\n       check existence of file" << sp << std::endl;
#endif
			if (fs::exists(sp)) {
				mcpl_filename = sp.parent_path() / sp.stem();
				mcpl_filename += ".mcpl.gz";
#ifdef DEBUG
				std::cout << "    -> file found" << std::endl;
#endif
			} else
				--istage;
		}
		if (mcpl_filename.empty()) {
#ifdef DEBUG
			std::cout << "    -> NO file found" << std::endl;
#endif
		}
		if (mcpl_filename.empty())
			istage = sFULL;

		return std::pair<size_t, std::string>(istage, mcpl_filename);
	}

	//	void set_job_dir(size_t jobIndex) { fs::create_directories(_p / std::to_string(jobIndex)); }

	void clear_cache(void)
	{
#ifdef DEBUG
		std::cout << "[DEBUG] Clearing path: " << _p << std::endl;
#endif
		_p.clear();
		fs::remove_all(_p);
	}

	private:
	fs::path _p;

	inline fs::path stage_path(stage_t istage, std::string stage_hash) const
	{
		// auto stage_name = stages.at(is);
		fs::path sp(_p.parent_path() / "MCPL" / stages.at(istage) / stage_hash);
		sp += ".json";
		return sp;
	}
};
} // namespace panosc
#endif
