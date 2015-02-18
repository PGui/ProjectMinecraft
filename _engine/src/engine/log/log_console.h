#ifndef	__NY_LOG_CONSOLE__
#define	__NY_LOG_CONSOLE__

#include "engine/utils/types.h"
#include "engine/utils/ny_utils.h"
#include "log.h"
#include <string>
#include <vector>

using namespace std;

class LogConsole : public Log
{
	private:
	
		LogConsole() : Log()
		{
			AllocConsole();
			freopen("CON","w",stdout);
			freopen("CON","w",stderr);
		}

		void logMsg(MSG_TYPE type, const char *message, bool intro=true, bool ret=true)
		{
			switch(type)
			{
				case ENGINE_ERROR : SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12); break;
				case ENGINE_INFO : SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7); break;
				case USER_ERROR : SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 12); break;
				case USER_INFO : SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 15); break;
				default: SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 7); break;
			}

			if(intro)
			{
				switch(type)
				{
					case ENGINE_ERROR : fprintf(stdout,"ENGINE_ERROR "); break;
					case ENGINE_INFO : fprintf(stdout,"ENGINE_INFO "); break;
					case USER_ERROR : fprintf(stdout,"USER_ERROR "); break;
					case USER_INFO : fprintf(stdout,"USER_INFO "); break;
					default: fprintf(stdout,"UNKN_TYPE: "); break;
				}
				fprintf(stdout,"[%06ld] ",GetTickCount()-_LastMessageTime);
			}
			
			fprintf(stdout,"%s",message);

			if(ret)
				fprintf(stdout,"\n");
		}
		
	public:
		
		static void createInstance()
		{
			SAFEDELETE(_Instance);
			_Instance = new LogConsole();
		}

		



};

#endif