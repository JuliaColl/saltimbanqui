#ifndef AUDIO_H
#define AUDIO_H

#include <map>
#include <string>
#include <bass.h>

class Audio
{
public:
	static std::map<std::string, Audio*> sLoadedAudios; //para nuestro manager
	HSAMPLE sample; //aqui guardamos el handler del sample que retorna BASS_SampleLoad
	std::string filename;

	Audio(); 
	~Audio(); 

	HCHANNEL play(float volume); //lanza el audio y retorna el channel donde suena
	HSAMPLE LoadSample(const char* filename, DWORD flag = 0);

	static void Stop(HCHANNEL channel); //para parar un audio si necesitamos su channel
	static Audio* Get(const char* filename, DWORD flag = 0);
	void setName(const char* name) {
		filename = name;
		sLoadedAudios[filename] = this;
	}
	static Audio* Find(const char* filename);
	
	static HCHANNEL* Play(const char* filename, DWORD flag = 0); //version estática para ir mas rapido

};

#endif