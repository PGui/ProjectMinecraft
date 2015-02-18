#ifndef	__NY_LOG_FILE__
#define	__NY_LOG_FILE__

#include "engine/utils/types.h"
#include "engine/utils/ny_utils.h"
#include "engine/log/log_console.h"
#include "log.h"
#include <stdio.h>
#include <string>
#include <vector>
#include <direct.h>

using namespace std;

/** Interface pour les composants de gestion bas niveau du réseau
	*/
class LogFile : public Log
{
	private:
		string _File;
		
		LogFile() : Log()
		{
			_File = "logudock.txt";
			FILE * fs = fopen(_File.c_str(),"w");
			if(fs)
			   fclose(fs);

			//On redirige stderr et stdout vers du fichier
			freopen( "stdout.log", "w", stdout );
			freopen( "stderr.log", "w", stderr );
			//_cprintf("Cwd : %s\n",_getcwd(NULL,0));
			
		}

		void logMsg(MSG_TYPE type, const char *message, bool intro=true, bool ret=true)
		{
			FILE * fs = fopen(_File.c_str(),"a");

			if(!fs)
			{ 
				LogConsole::createInstance();
				char* buffer = _getcwd(NULL,0);
				Log::log(ENGINE_ERROR,("Unable to open log file in " + toString(buffer) + ", switching to console (lost message following)").c_str());
				Log::log(type,message,intro,ret);
				return;
			}
			
			if(intro)
			{
				switch(type)
				{
					case ENGINE_ERROR : fprintf(fs,"ENGINE_ERROR "); break;
					case ENGINE_INFO : fprintf(fs,"ENGINE_INFO "); break;
					case USER_ERROR : fprintf(fs,"USER_ERROR "); break;
					case USER_INFO : fprintf(fs,"USER_INFO "); break;
					default: fprintf(fs,"UNKN_TYPE: "); break;
				}
				fprintf(fs,"[%06ld] ",GetTickCount()-_LastMessageTime);
			}
			
			fprintf(fs,"%s",message);
			
			if(ret)
				fprintf(fs,"\n");

			fclose(fs);
		}
    	
	public:
		
		static void createInstance()
		{
			SAFEDELETE(_Instance);
			_Instance = new LogFile();
		}

		

};

#endif