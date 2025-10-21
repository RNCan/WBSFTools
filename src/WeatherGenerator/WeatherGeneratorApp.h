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

		std::ostream& print(std::ostream& os) const {
			std::copy(v, v + LEN, std::ostream_iterator<T>(os, ", "));
			return os;
		}

	protected:
		// typedef TCLAP::StringLike ValueCategory;
		T v[LEN];

	};
}
namespace WBSF
{
	

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
		//ERMsg ProcessOption(int& i, int argc, char* argv[]);


		//xstd::vector<std::string> m_equations;
		//options;
		
		//std::unique_ptr<TCLAP::ValueArg<std::string>> pGlobal;
		//std::unique_ptr < TCLAP::SwitchArg> pVerbose;

		CLocationVector m_loc;
	};



	typedef std::deque < std::vector<float>> OutputData;

	//typedef std::vector<CGDALDatasetEx> CGDALDatasetExVector;

	class CWeatherGeneratorApp
	{
	public:

		ERMsg Execute();

		//std::string GetDescription()
		//{
		//    return  std::string("WeatherGeneratorImages version ") + VERSION + " (" + __DATE__ + ")";
		//}

		CWeatherGeneratorOption m_options;


	};
}
