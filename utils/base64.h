/* 
 toptoken  
*/

#ifndef UTILS_BASE64_H_
#define UTILS_BASE64_H_

#include "modp_b64.h"

namespace utils {
	/*
		Encodes the input string in base64.  
		Returns true if successful and false otherwise.  
		The output string is only modified if successful.
	*/
	bool Base64Encode(const std::string &input, std::string &output);

	/* 
		Decodes the base64 input string.  
		Returns true if successful and false otherwise.  
		The output string is only modified if successful.
	 */
	bool Base64Decode(const std::string &input, std::string &output);
}

#endif
