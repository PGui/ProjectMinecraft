#ifndef	__NY_LOG__
#define	__NY_LOG__

#include "engine/utils/types.h"
#include "engine/utils/ny_utils.h"
#include <string>
#include <vector>

using namespace std;

class Log	
{

	public:
		
		typedef enum {
			USER_INFO,
			USER_ERROR,
			ENGINE_INFO,
			ENGINE_ERROR,
		}MSG_TYPE;

		
  
	protected:
		bool _HideEngineLog;
		unsigned long _LastMessageTime;

		static Log * _Instance;
		Log()
		{
			_HideEngineLog = false;
			_LastMessageTime = GetTickCount();
		}

		virtual void logMsg(MSG_TYPE type, const char *message, bool intro=true, bool ret=true) = 0;
	public:

		static void log(MSG_TYPE type, const char *message, bool intro=true, bool ret=true)
		{
			if(!_Instance)
				return;
			if(_Instance->_HideEngineLog && (type == ENGINE_ERROR || type == ENGINE_INFO))
				return;
			_Instance->logMsg(type,message,intro,ret);
			_Instance->_LastMessageTime = GetTickCount();
		}

		static void showEngineLog(bool show)
		{
			_Instance->_HideEngineLog = !show;
		}

};

#endif