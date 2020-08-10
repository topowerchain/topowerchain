/* 
 toptoken  
*/

#ifndef CONSENSUS_MANAGER_H_
#define CONSENSUS_MANAGER_H_

#include <utils/singleton.h>
#include <utils/net.h>
#include <common/general.h>
#include <main/configure.h>
#include "bft.h"

namespace toptoken {

	class ConsensusManager : public utils::Singleton<ConsensusManager>,
		public TimerNotify,
		public StatusModule {
		friend class utils::Singleton<ConsensusManager>;
	private:
		ConsensusManager();
		~ConsensusManager();

		std::shared_ptr<Consensus> consensus_;
	public:
		bool Initialize(const std::string &validation_type);
		bool Exit();
		std::shared_ptr<Consensus> GetConsensus();

		virtual void OnTimer(int64_t current_time);
		virtual void OnSlowTimer(int64_t current_time);
		virtual void GetModuleStatus(Json::Value &data);
	};

}

#endif