#include "audio.h"
#include "utils.h"
#include <cassert>

std::map<std::string, Audio*> Audio::sLoadedAudios;



Audio::Audio()
{
	sample = NULL;
}

Audio::~Audio()
{
	BASS_SampleFree(sample);
}

HCHANNEL Audio::play(float volume)
{
	//El handler para un canal
	HCHANNEL hSampleChannel;

	//Creamos un canal para el sample
	hSampleChannel = BASS_SampleGetChannel(sample, false);

	//Lanzamos un sample
	BASS_ChannelPlay(hSampleChannel, true);

	return hSampleChannel;
}

HSAMPLE Audio::LoadSample(const char* filename, DWORD flag)
{
	//El handler para un sample
	HSAMPLE hSample;

	//Cargamos un sample del disco duro (memoria, filename, offset, length, max, flags)
	//use BASS_SAMPLE_LOOP in the last param to have a looped sound
	hSample = BASS_SampleLoad(false, filename, 0, 0, 3, flag);  //mem, file, offset (donde empezar), lenght (0 para lenght maxima), max (numero maximo de canales en que se puede reproducir), flags (hay una que es un loop)
	if (hSample == 0)
	{
		std::cout << " ERROR load " << filename << std::endl;
	}
	std::cout << " + AUDIO load " << filename << std::endl;
	this->filename = filename;
	setName(filename);
	this->sample = hSample;

	return hSample;
}


Audio* Audio::Find(const char* filename)
{
	assert(filename);
	auto it = sLoadedAudios.find(filename);
	if (it != sLoadedAudios.end())
		return it->second;
	return NULL;
}

HCHANNEL* Audio::Play(const char* filename, DWORD flag)
{
	Audio* audio = Audio::Get(filename, flag);

	HSAMPLE hSample = audio->sample;

	//El handler para un canal
	HCHANNEL hSampleChannel;

	//Creamos un canal para el sample
	hSampleChannel = BASS_SampleGetChannel(hSample, false);

	//Lanzamos un sample
	BASS_ChannelPlay(hSampleChannel, true);

	return &hSampleChannel; 
}



Audio* Audio::Get(const char* filename, DWORD flag)
{
	//load it
	Audio* audio = Find(filename);
	if (audio) {
		return audio;
	}
		

	audio = new Audio();
	if (!audio->LoadSample(filename, flag))
	{
		delete audio;
		return NULL;
	}

	return audio;
}


