/* 
 toptoken  
*/

#include "general.h"

namespace toptoken {
	std::string GetDataSecuretKey() {
		//The raw key must be a string and end with 0. The key length must be 32 + 1.
		char key[] = { 'H', 'C', 'P', 'w', 'z', '!', 'H', '1', 'Y', '3', 'j', 'a', 'J', '*', '|', 'q', 'w', '8', 'K', '<', 'e', 'o', '7', '>', 'Q', 'i', 'h', ')', 'r', 'P', 'q', '1', 0 };		
		return key;
	}
}
