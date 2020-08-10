/* 
 toptoken  
*/

#ifndef CONSENSUS_MSG_
#define CONSENSUS_MSG_

#include <proto/cpp/consensus.pb.h>

namespace toptoken {
	class ConsensusMsg {
		int64_t seq_;
		std::string type_;
		protocol::PbftEnv pbft_env_;
		std::vector<std::string> values_;
		std::string node_address_;
		std::string hash_;
	public:
		ConsensusMsg() {}
		ConsensusMsg(const protocol::PbftEnv &pbft_env);
		~ConsensusMsg();

		bool operator < (const ConsensusMsg &msg) const;
		bool operator == (const ConsensusMsg &value_frm) const;
		int64_t GetSeq() const;
		std::vector<std::string> GetValues() const;
		const char *GetNodeAddress() const;
		std::string GetType() const;
		protocol::PbftEnv  GetPbft() const;
		std::string  GetHash() const;
		size_t GetSize() const;
	};
}

#endif