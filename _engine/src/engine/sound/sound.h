#ifndef __SOUND_H__
#define __SOUND_H__

/**
  * Permet de se construire un petit moteur son
  * Son simple play pause
  */

#include "al.h"
#include "alut/alut.h"
#include "engine/utils/types.h"
#include "engine/utils/ny_utils.h"
#include "engine/log/log.h"


#include <stdio.h>
#include <vector>


/**
  * Classe de base pour tout objet son
  * gere les play pause etc de base
  * Utiliser une classe fille, celle ci ne charge pas de fichier, juste la gestion de base
  */
class Sound
{
	protected :
		bool _MustPlay;
		ALuint _Source;

	public:
		bool _Loop;  

	public:
		Sound()
		{
			_MustPlay = false;
			_Loop = false;
			alGenSources (1, &_Source);
		}

		static void checkAlError(void)
		{
			ALuint error = alGetError ();
			if (error != ALUT_ERROR_NO_ERROR)
				Log::log(Log::ENGINE_ERROR, ("OpenAl error : " + string(alGetString (error))).c_str());
		}

		static void checkAlutError(void)
		{
			ALuint error = alutGetError ();
			if (error != ALUT_ERROR_NO_ERROR)
				Log::log(Log::ENGINE_ERROR, ("OpenAl error : " + string(alutGetErrorString (error))).c_str());
		}

		virtual void play(void)
		{
			alSourcePlay (_Source);
			_MustPlay = true;
			checkAlError();
		}

		virtual void stop(void)
		{
			alSourceStop (_Source);
			_MustPlay = false;
			checkAlError();
		}

		virtual void setVolume(float volume)
		{
			alSourcef(_Source,AL_GAIN,volume);
			checkAlError();
		}

		virtual void update(float elapsed)
		{
			if(_MustPlay && _Loop && !isPlaying())
				play();
		}

		virtual bool isPlaying(void)
		{
			ALint status;
			alGetSourcei (_Source, AL_SOURCE_STATE, &status);
			return (status == AL_PLAYING);
		}
};


/**
* Un son basique construit a partir d'un fichier
*/
class SoundBasic : public Sound
{
	private:
		ALuint _Buffer;		

	public:	

		SoundBasic() : Sound()
		{
			_Buffer = 0;
		}

		void load(char * fileName)
		{

			//On cree le buffer a partir du fichier directement
			_Buffer = alutCreateBufferFromFile (fileName);
			if (_Buffer == AL_NONE)
			{
				checkAlutError();
				return;
			}

			//On attache a la source
			alSourcei (_Source, AL_BUFFER, _Buffer);
			checkAlError();
		}
};

class SoundEngine
{
private: 
	static SoundEngine * _Instance;
	std::vector<Sound*> _Sounds;

	SoundEngine()
	{
		alutInit(0,NULL);
		Sound::checkAlutError();
	}

	~SoundEngine()
	{
		alutExit ();
	}

public:
	static SoundEngine * getInstance(void)
	{
		if(!_Instance)
			_Instance = new SoundEngine();
		return _Instance;
	}

	void addSound(Sound * snd)
	{
		_Sounds.push_back(snd);
	}

	void update(float elapsed)
	{
		for(uint32 i=0;i<_Sounds.size();i++)
			_Sounds[i]->update(elapsed);
	}
};

#endif
