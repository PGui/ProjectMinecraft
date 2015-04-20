#ifndef __GRANULAR_SOUND__
#define __GRANULAR_SOUND__

#include <ostream>
#include <sstream>
/**
* Synthé granulaire.
* Lui charger un fichier de base ou il pioche les grains
* utiliser setGrainParam pour modifier les params d'extraction et d'ajout des grains
*/

/*
Fonctionnement du buffer tournant :
Le pointeur de lecteur est un echantillon avant le pointeur d'écriture
*/

#define GRAIN_CARD_BUFFER_LENGTH 0.1f //On prend 100ms de buffer sur la carte
#define GRAIN_SYNTHE_BUFFER_LENGTH 1.0f //On prend 1s de buffer en memoire

#include "sound.h"


/**
  * Utilise plusieurs buffers :
  * un buffer qui contient le fichier de base
  * un buffer dans lequel on fait la synthèse : on y ajoute les grains les un a la suite des autres
  *   en tenant compte de params de synthèse. C'est un buffer tournant.
  * un buffer temporaire qui permet de créer
  */
class SoundGrain : public Sound
{
private:

	/**
	  * Histoire de vous inspirer, je vous ai laissé les attrbuts qu'utilise la classe une fois codée.
	  * vous pouvez les modifier a loisir.
	  */

	//Datas du fichier de base
	void * _BaseBuffer;
	float _DureeBase;
	uint8 * _PtRdFinBaseBuffer;
	ALenum _Format;
	ALsizei _Size;
	ALfloat _Frequency;
	int _NbPistes;
	int _NbBits;
	int _TailleEchantillon;

	//Buffers pour queuing openal
	ALuint _Buffers[3]; ///<id des buffers
	ALuint _PlayingBuffer; ///< indice du buffer en cours de lecture par la carte
	float _DureeBuffer; ///< Duree d'un buffer openal
	int _TailleBuffer; ///< Taille d'un buffer openal
	void * _DatasToLoad; ///< buffer temporaire pour balancer les datas à openal

	//Buffer pour faire la synthèse (buffer tournant)
	//necessaire car overlap des grains et sinon artefacts entre buffers
	float _DureeBufferSynthe; //Duree du buffer de synthe
	void * _DatasSynthe; 
	uint8 * _PtRdSynthe; ///< Pointeur de lecture du buffer de syntthe vers les buffers openal
	uint8 * _PtWrSynthe; ///< Pointeur d'ecriture dans le buffer de synthe
	uint8 * _PtFinBufferSynthe; ///< Pointeur sur la fin du buffer de synthe
	size_t _TailleBufferSynthe = 44100; ///< taille du buffer de synthé

	//Paramètres de la generation de grains
	float _PosGrainf; ///< Position de prise du grain entre 0 et 1
	float _DureeGrain; ///< Durée du grain en secondes 
	int _TailleGrain; ///< Taille du grain en octets
	int _PosGraini; ///< Position du grain en octets
	int _RandomWidth; //Taille du random quand on chope un grain
	int _SizeOverlap; //Taille d'overlapp entre les grains

	//Pour la lecture / ecriture des grains. 
	//On doit l'avoir au niveau de l'objet pour garder les params entre deux buffers successifs
	uint8 * _PtRdGrain;
	uint8 * _PtRdFinGrain;
	int _TailleLastGrain;

	//Buffer circulaire
	uint8 * buffCirc = new uint8[_TailleBufferSynthe];


private :

	/**
	  * EXERCICE 2
	  * Cette méthode vous est donnée. Elle se contente de charger le buffer principal à la suite des buffers openal.
	  */
	void addBaseDataToQueue(void)
	{
		alBufferData(_Buffers[0],_Format,_BaseBuffer,_Size,(ALsizei)_Frequency);

		ALuint error = alGetError ();
		if (error != ALUT_ERROR_NO_ERROR)
		{
			fprintf (stderr, "%s\n", alGetString (error));
		}

		alSourceQueueBuffers(_Source,1,_Buffers);
		error = alGetError ();
		if (error != ALUT_ERROR_NO_ERROR)
		{
			fprintf (stderr, "%s\n", alGetString (error));
		}
	}

	/**
	  * EXERCICE 2 :
	  * TODO : inspirez vous de la méthode addBaseDataToQueue() et générez un buffer qui contient un signal sinusoidal,
	  * puis ajoutez le a openal
	  */
	void addSinusoidalDataToQueue(float freqSignal)
	{
		float pi = 3.1415;
		float deuxPi = pi * 2;

		short int * myBuffer = new short int[44100];

		for (int i = 0; i < 44100; ++i)
		{
			myBuffer[i] = sin(i * (deuxPi / (_Frequency/freqSignal))) * 32767; //32767 car 65535/2 :
		}

		alBufferData(_Buffers[0], _Format, myBuffer, 44100 * 2, _Frequency);
		alSourceQueueBuffers(_Source, 1, _Buffers);
	}

	/**
	  * EXERCICE 3 : Phase de synthèse
	  * TODO : Cette méthode va lire le buffer qui contient le fichier de base pour en extraire de grains
	  * qu'elle va copier au fur et à mesure dans le buffer de synthèse
	  */
	void fillBufferWithGrains(void)
	{			
		//Log::log(Log::ENGINE_ERROR, "fillBufferWithGrains pas encore codee !!");
		
		while (_PtWrSynthe != _PtRdSynthe)
		{
			_PtWrSynthe = &(((uint8*)_BaseBuffer)[rand() % _Size]);
			_PtWrSynthe += _SizeOverlap;

		}
		

		/*_PtWrSynthe = 

		

		_PosGraini += rand() % 530;
		if (_PosGraini >= _TailleBufferSynthe)
		{
			_PosGraini = 0;
		}*/
	}

	/**
	  * EXERCICE 3 : Phase de copie
	  * TODO : Cette méthode rempli un buffer mémoire avec une partie du buffer de synthèse.
	  * elle copie ensuite ces données vers un buffer opanal et l'ajoute à la suite de a liste de buffers.
	  * on utilise 3 buffers openal pour gérer la file d'attente.
	  */
	void addBufferToQueue(int numBuffer)
	{
		Log::log(Log::ENGINE_ERROR, "addBufferToQueue pas encore codee !!");

		//alBufferData(_Buffers[0], _Format, myBuffer, 44100 * 2, _Frequency);
		//alSourceQueueBuffers(_Source, 1, _Buffers);
	}

	/**
	  * EXERCICE 3 : 
	  * Cette méthode vous est donnée, elle retire un buffer de la liste des buffers openal
	  */
	void removeBufferFromQueue(int numBuffer)
	{
		alSourceUnqueueBuffers(_Source,1,&(_Buffers[numBuffer]));
		ALuint error = alGetError ();
		if (error != ALUT_ERROR_NO_ERROR)
		{
			fprintf (stderr, "%s\n", alGetString (error));
		}
	}

	/**
	  * EXERCICE 3 : 
	  * Cette méthode vous est donnée, elle récupère le nombre de buffers deja parcourus par openal
	  */
	int getNbBuffersProcessed(void)
	{
		ALint  value;
		alGetSourcei(_Source,AL_BUFFERS_PROCESSED,&value);
		ALuint error = alGetError ();
		if (error != ALUT_ERROR_NO_ERROR)
		{
			fprintf (stderr, "%s\n", alGetString (error));
		}
		return value;
	}

	/**
	  * EXERCICE 3 : 
	  * Cette méthode vous est donnée, elle récupère le nombre de buffers dans la liste de buffers en attente
	  */
	int getNbBuffersQueued(void)
	{
		ALint  value;
		alGetSourcei(_Source,AL_BUFFERS_QUEUED,&value);
		ALuint error = alGetError ();
		if (error != ALUT_ERROR_NO_ERROR)
		{
			fprintf (stderr, "%s\n", alGetString (error));
		}
		return value;
	}


public:

	SoundGrain() : Sound()
	{
		_BaseBuffer = NULL;
		_PlayingBuffer = 0;
		_DureeBuffer = GRAIN_CARD_BUFFER_LENGTH; 
		_DureeBufferSynthe = GRAIN_SYNTHE_BUFFER_LENGTH;
		_PtRdGrain = NULL;
		_DureeBase = 0.0f;

		alGenBuffers(3,_Buffers);
		ALuint error = alGetError ();
		if (error != ALUT_ERROR_NO_ERROR)
		{
			fprintf (stderr, "%s\n", alGetString (error));
		}
	}

	float getDureeBase(void)
	{
		return _DureeBase;
	}

	/**
	  * EXERCICE 1 : 
	  * TODO : Cette méthode charge le fichier de base dans le buffer de base. Utiliser alut, et afficher
	  * sur la console le format du fichier chargé (voir énoncé)
	  */
	void loadBaseFile(char * filename)
	{
		
		_BaseBuffer = alutLoadMemoryFromFile(filename,
			&_Format,
			&_Size,
			&_Frequency);

		cout << "Chargement " << filename <<  endl;
		ostringstream s;
		
		if (_BaseBuffer == NULL)
		{
			cerr << "Erreur chargement" << endl;
		}
		else
		{
			cerr << "Chargement OK" << endl;
			float duree = 0.0f;
			switch (_Format)
			{
			case AL_FORMAT_MONO8:
				
				cout << "Nombre de pistes 1" << endl;
				_NbPistes = 1;
				cout << "Format Mono 8 bits" << endl;
				cout << "Frequence " << _Frequency << " hz" << endl;
				_TailleEchantillon = 1;
				cout << "Taille echantillon " << _TailleEchantillon << " octets." << endl;
				cout << "Nombre total d'echantillons " << _Size << "." << endl;
				cout << "Duree du fichier " << (_Size / (_Frequency * (_TailleEchantillon ))) << "." << endl;
				break;
			case AL_FORMAT_MONO16:
				cout << "Nombre de pistes 1" << endl;
				_NbPistes = 1;
				cout << "Format Mono 16 bits" << endl;
				cout << "Frequence " << _Frequency << " hz" << endl;
				_TailleEchantillon = 2;
				cout << "Taille echantillon " << _TailleEchantillon << " octets." << endl;
				cout << "Nombre total d'echantillons " << _Size << "." << endl;
				cout << "Duree du fichier " << (_Size / (_Frequency * (_TailleEchantillon ))) << "." << endl;
				break;
			case AL_FORMAT_STEREO8:
				cout << "Nombre de pistes 2" << endl;
				_NbPistes = 2;
				cout << "Format Stereo 8 bits" << endl;
				cout << "Frequence " << _Frequency << " hz" << endl;
				_TailleEchantillon = 1;
				cout << "Taille echantillon " << _TailleEchantillon << " octets." << endl;
				cout << "Nombre total d'echantillons " << _Size << "." << endl;
				cout << "Duree du fichier " << (_Size / (_Frequency * (_TailleEchantillon ))) << "." << endl;
				break;
			case AL_FORMAT_STEREO16:
				cout << "Nombre de pistes 2" << endl;
				_NbPistes = 2;
				cout << "Format Stereo 16 bits" << endl;
				cout << "Frequence " << _Frequency << " hz" << endl;
				_TailleEchantillon = 2;
				cout << "Taille echantillon " << _TailleEchantillon << " octets." << endl;
				cout << "Nombre total d'echantillons " << _Size << "." << endl;
				cout << "Duree du fichier " << (_Size / (_Frequency * (_TailleEchantillon ))) << "." << endl;
				break;
			default:
				cout << "Format non reconnu" << endl;
				break;

			}
		}

		
		alGenBuffers(3, &_Buffers[0]);

		//Init du buffer de synthèse où on va récupérer les grains
		_DatasSynthe = new uint8[((int)GRAIN_SYNTHE_BUFFER_LENGTH) * _Frequency];

		_PtRdGrain = _PtWrSynthe = (uint8*)(_DatasSynthe);
		_PtWrSynthe += _TailleEchantillon;
		//_PosGraini = 0;

		//On set des params par defaut au cas ou
		setGrainParam(0.5f,0.2f,0.2f,0.05f);
	}

	/**
	  * EXERCICE 3 : 
	  * TODO : modifiez vos paramètres de synthèse quand cette fonction est appelée
	  **/
	void setGrainParam(float posGrain, float dureeGrain, float randomPos, float partOverlap)
	{
		_PosGrainf = posGrain;
		_DureeGrain = dureeGrain;
		_RandomWidth = randomPos;
		_SizeOverlap = partOverlap;


	}

	

	void play(void) 
	{
		
		while(getNbBuffersQueued() < 2)
		{
			_PlayingBuffer = (_PlayingBuffer + 1)%3;
			removeBufferFromQueue(_PlayingBuffer);
			fillBufferWithGrains();
			addBufferToQueue(_PlayingBuffer);
		}
		//addSinusoidalDataToQueue(440);
		Sound::play();
	}


	void update(float elapsed)
	{
		Sound::update(elapsed);

		if(getNbBuffersProcessed() > 0)
		{
			_PlayingBuffer = (_PlayingBuffer + 1)%3;
			removeBufferFromQueue(_PlayingBuffer);
			fillBufferWithGrains();
			addBufferToQueue(_PlayingBuffer);
		}
	}
};

#endif
