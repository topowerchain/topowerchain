/* 
 toptoken  
*/

#ifndef  DAEMON_H_
#define  DAEMON_H_

#include <utils/singleton.h>
#include <common/general.h>
#ifdef WIN32
#else 
#include <sys/sem.h>
#include <sys/ipc.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/shm.h>
#include<sys/mman.h>
#endif
#define TEXT_SZ 2048

namespace utils {
	class Daemon : public utils::Singleton<utils::Daemon>, public toptoken::TimerNotify {
		friend class utils::Singleton<utils::Daemon>;
	private:
		Daemon();
		~Daemon();
		pthread_mutex_t *mptr;

		int64_t last_write_time_;

		int running;
		void *shm;
		int64_t* shared;
		int shmid;

	public:
		bool Initialize(int32_t key);
		bool Exit();
		void OnTimer(int64_t current_time);

		virtual void OnSlowTimer(int64_t current_time);
		void GetModuleStatus(Json::Value &data) {

		};
		//virtual void GetModuleStatus(Json::Value &data) const;
	};
}
#endif
