//***********************************************************************
#pragma once

#include <deque>
#include <set>
#include <boost/dynamic_bitset.hpp>

#include "Basic/ERMsg.h"
#include "Basic/tclap/CmdLine.h"
#include "Basic/Location.h"


namespace TCLAP
{
	//    class TCLAP::CmdLine;
	template<typename T>
	class BoundedConstraint : public TCLAP::Constraint<T>
	{
	public:

		BoundedConstraint(T lower, T upper) :
			m_lower(lower),
			m_upper(upper)
		{
		}


		virtual std::string description() const override {
			return "Number between " + std::to_string(m_lower) + " and " + std::to_string(m_upper);
		}

		virtual std::string shortID() const override {
			return "("+ std::to_string(m_lower) + "-" + std::to_string(m_upper)+")";
		}

		bool check(const T& value) const override {
			return (value >= m_lower && value <= m_upper) || value == m_missing;
		}

	protected:

		T m_lower;
		T m_upper;
		T m_missing;
	};

	// Define a simple array type
	template <typename T, size_t LEN>
	struct Array : public TCLAP::StringLikeTrait
	{
		Array(const std::string& str = "")
		{
			operator=(str);
		}

		// operator= will be used to assign to the vector
		Array& operator=(const std::string& str)
		{
			std::istringstream iss(str);
			for (size_t n = 0; n < LEN; n++)
			{
				if (!(iss >> v[n]))
				{
					std::ostringstream oss;
					oss << " is not a vector of size " << LEN;
					throw TCLAP::ArgParseException(str + oss.str());
				}
			}

			if (!iss.eof())
			{
				std::ostringstream oss;
				oss << " is not a vector of size " << LEN;
				throw TCLAP::ArgParseException(str + oss.str());
			}

			return *this;
		}

		const T& operator[](size_t i) const
		{
			assert(i < LEN);
			return v[i];
		}


		std::ostream& print(std::ostream& os) const 
		{
			std::copy(v, v + LEN, std::ostream_iterator<T>(os, ", "));
			return os;
		}

	protected:
		// typedef TCLAP::StringLike ValueCategory;
		std::array<T, LEN> v;

	};
}
namespace WBSF
{
	class CGDALDatasetEx;
	class CGlobalData;

	class CWeatherGeneratorOption : public std::map < std::string, std::unique_ptr<TCLAP::Arg>>
	{
	public:

		static const char* VERSION;

		//enum TFilePath { INPUT_FILE_PATH, OUTPUT_FILE_PATH, NB_FILE_PATH };


		CWeatherGeneratorOption();

		virtual void failure(TCLAP::CmdLineInterface& c, TCLAP::ArgException& e) {
			static_cast<void>(c);  // Ignore input, don't warn
			std::cerr << "my failure message: " << std::endl << e.what() << std::endl;
			exit(1);
		}

		virtual void usage(TCLAP::CmdLineInterface& c) {
			std::cout << "my usage message:" << std::endl;
			std::list<TCLAP::Arg*> args = c.getArgList();
			for (TCLAP::ArgListIterator it = args.begin(); it != args.end(); it++)
				std::cout << (*it)->longID() << "  (" << (*it)->getDescription() << ")" << std::endl;
		}

		virtual void version(TCLAP::CmdLineInterface& c) {
			static_cast<void>(c);  // Ignore input, don't warn
			std::cout << "WeatherGenerator version " << VERSION << " (" << __DATE__ << ")" << std::endl;
		}


		void Init(TCLAP::CmdLine& cmd);
		ERMsg ParseOption(int argc, char* argv[]);


		/*template <class T>
		ValueArg<T>* ValueArg(const std::string& name)const 
		{
			return static_cast<ValueArg<T>*> this->at(name);
		}*/


		
		TCLAP::ValueArg<std::string>* GetValueArg(const std::string& name)const
		{
			return static_cast<TCLAP::ValueArg<std::string>*> (this->at(name).get());
		}

		template<typename T>
		const T& Get(const std::string& name)const
		{
			return static_cast<TCLAP::ValueArg<T>*> (this->at(name).get())->getValue();
		}

		const std::vector<std::string>& GetFilesArg()const
		{
			return static_cast<TCLAP::UnlabeledMultiArg<std::string>*> (this->at("Files").get())->getValue();
		}


		ERMsg GetLoc(const CGlobalData& global, CLocationVector& loc)const;
		ERMsg GetWGInput(const CGlobalData& global, CWGInput& WGInput)const;
		
		//CLocationVector m_loc;
	};



	class CGlobalData
	{
	public:

		enum TParam { MODELS_PATH, NORMALS_PATH, DAILY_PATH, HOURLY_PATH, SHORE, DEM, DAILY_CACHE_SIZE, NB_PAPAMS };
		static const char* NAME[NB_PAPAMS];


		CGlobalData() { clear(); }
		void clear();

		ERMsg Init(const std::string& str_init);
		ERMsg parse(const std::string& str_init);



		std::string m_model_path;
		std::string m_normal_path;
		std::string m_daily_path;
		std::string m_hourly_path;
		std::string m_shore_file_path;
		std::string m_DEM_file_path;
		size_t m_daily_cache_size;


		ERMsg ComputeElevation(double latitude, double longitude, double& elevation)const;
		ERMsg ComputeShoreDistance(double latitude, double longitude, double& shore_distance)const;
		//ERMsg ComputeHorizon(double latitude, double longitude, double& elevation);

		std::shared_ptr<CGDALDatasetEx> m_pDEM;
	};

	//typedef std::deque < std::vector<float>> OutputData;
	//typedef std::vector<CGDALDatasetEx> CGDALDatasetExVector;
	class CWeatherGenerator;
	typedef std::shared_ptr<CWeatherGenerator> CWeatherGeneratorPtr;

	class CWeatherGeneratorApp
	{
	public:

		ERMsg Execute();


		ERMsg InitializeWG(CWeatherGeneratorPtr& pWG)const;
		


		CWeatherGeneratorOption m_options;

		CGlobalData m_global;
	};
}
