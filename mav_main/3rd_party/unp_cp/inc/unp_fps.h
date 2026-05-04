/*
 * unp_fps.h
 *
 * Вычисление fps
 *
 *  Created on: 19.11.2014
 *      Author: kudr
 */

#ifndef UNP_FPS_H_
#define UNP_FPS_H_

#include <inttypes.h>
#include <string>

namespace UNP {

class Fps {
public:
	enum PrintType {
		PRINT_UNP_LOG, //!< Печать fps через UNP::GetLogger()
		PRINT_STD,     //!< Печать fps на std
	};
	
private:
	int64_t t;
	int64_t n;
	int c;
	
	std::string prefix;
	PrintType printType;
	
public:
	Fps(const char *prefix_ = NULL, PrintType printType_ = PRINT_STD) : t(0), n(0), c(0), prefix(prefix_ ? prefix_ : ""), printType(printType_) {}
	Fps(const std::string &prefix_, PrintType printType_ = PRINT_STD) : t(0), n(0), c(0), prefix(prefix_), printType(printType_) {}
	
	void SetPrefix(const char *prefix_) { prefix = prefix_ ? prefix_ : ""; }
	void SetPrefix(const std::string &prefix_) { prefix = prefix_; }
	void SetPrintType(PrintType printType_) { printType = printType_; }
	void CalcPrint(int deltaFramesCount = 1);
};

}
#endif
