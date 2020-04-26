#include "bass.h"
#include "CoronaAssert.h"
#include "CoronaEvent.h"
#include "CoronaLua.h"
#include "CoronaLibrary.h"
#include <string.h>
#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <string>

// ----------------------------------------------------------------------------

namespace Corona
{
	// ----------------------------------------------------------------------------

	class BassLibrary
	{
	public:
		typedef BassLibrary Self;

	public:
		static const char kName[];

	public:
		static int Open(lua_State* L);
		static int Finalizer(lua_State* L);
		static Self* ToLibrary(lua_State* L);

	protected:
		BassLibrary();
		bool Initialize(void* platformContext);

	public:

		static int dispose(lua_State* L);
		static int getDuration(lua_State* L);
		static int getVolume(lua_State* L);
		static int isChannelActive(lua_State* L);
		static int isChannelPaused(lua_State* L);
		static int isChannelPlaying(lua_State* L);
		static int loadStream(lua_State* L);
		static int loadSound(lua_State* L);
		static int pause(lua_State* L);
		static int play(lua_State* L);
		static int resume(lua_State* L);
		static int rewind(lua_State* L);
		static int setVolume(lua_State* L);
		static int stop(lua_State* L);
		static int update(lua_State* L);
	};

	// ----------------------------------------------------------------------------

	// This corresponds to the name of the library, e.g. [Lua] require "plugin.library"
	const char BassLibrary::kName[] = "plugin.bass";

	int BassLibrary::Open(lua_State* L)
	{
		// Register __gc callback
		const char kMetatableName[] = __FILE__; // Globally unique string to prevent collision
		CoronaLuaInitializeGCMetatable(L, kMetatableName, Finalizer);

		//CoronaLuaInitializeGCMetatable( L, kMetatableName, Finalizer );
		void* platformContext = CoronaLuaGetContext(L);

		// Set library as upvalue for each library function
		Self* library = new Self;

		if (library->Initialize(platformContext))
		{
			// Functions in library
			static const luaL_Reg kFunctions[] =
			{
				{ "dispose", dispose },
				{ "getDuration", getDuration },
				{ "getVolume", getVolume },
				{ "isChannelActive", isChannelActive },
				{ "isChannelPaused", isChannelPaused },
				{ "isChannelPlaying", isChannelPlaying },
				{ "loadSound", loadSound },
				{ "loadStream", loadStream },
				{ "pause", pause },
				{ "play", play },
				{ "resume", resume },
				{ "rewind", rewind },
				{ "setVolume", setVolume },
				{ "stop", stop },
				{ "update", update },
				{ NULL, NULL }
			};

			// Register functions as closures, giving each access to the
			// 'library' instance via ToLibrary()
			{
				CoronaLuaPushUserdata(L, library, kMetatableName);
				luaL_openlib(L, kName, kFunctions, 1); // leave "library" on top of stack
			}
		}

		return 1;
	}

	int BassLibrary::Finalizer(lua_State* L)
	{
		Self* library = (Self*)CoronaLuaToUserdata(L, 1);
		if (library)
		{
			delete library;
		}

		BASS_Free();

		return 0;
	}

	BassLibrary* BassLibrary::ToLibrary(lua_State* L)
	{
		// library is pushed as part of the closure
		Self* library = (Self*)CoronaLuaToUserdata(L, lua_upvalueindex(1));
		return library;
	}

	BassLibrary::BassLibrary()
	{
	}

	bool BassLibrary::Initialize(void* platformContext)
	{
		if (HIWORD(BASS_GetVersion()) != BASSVERSION)
		{
			printf("ERROR: plugin.bass - An incorrect version of BASS.DLL was loaded\n");
			return 0;
		}

		BASS_SetConfig(BASS_CONFIG_DEV_DEFAULT, 1);

		if (!BASS_Init(1, 44100, 0, 0, NULL))
		{
			printf("ERROR: plugin.bass - Can't initialize device\n");
		}

		return 1;
	}

	// ---------------------- HELPER FUNCTIONS -------------------------------------

	HSTREAM getChannel(lua_State* L, int index, const char* errorMessage)
	{
		HSTREAM channel;

		if (lua_isnumber(L, index))
		{
			channel = lua_tonumber(L, index);
		}
		else
		{
			CoronaLuaError(L, "%s, got: %s", errorMessage, lua_typename(L, index));
		}

		return channel;
	}

	// ----------------------- LUA FUNCTIONS ---------------------------------------

	int BassLibrary::dispose(lua_State* L)
	{
		HSTREAM channel = getChannel(L, 1, "bass.dispose() channel expected");
		BASS_StreamFree(channel);

		return 0;
	}

	int BassLibrary::getDuration(lua_State* L)
	{
		HSTREAM channel = getChannel(L, 1, "bass.getDuration() channel expected");
		QWORD lengthInBytes = BASS_ChannelGetLength(channel, BASS_POS_BYTE);
		double durationInSeconds = BASS_ChannelBytes2Seconds(channel, lengthInBytes);

		// match the corona audio api and return this number in miliseconds
		lua_pushnumber(L, durationInSeconds / 1000);

		return 1;
	}

	int BassLibrary::getVolume(lua_State* L)
	{
		float volume = 0;
		HSTREAM channel;

		if (lua_istable(L, 1))
		{
			lua_getfield(L, 1, "channel");
			if (lua_isnumber(L, -1))
			{
				channel = getChannel(L, -1, "bass.getVolume() options.channel expected");
				BASS_ChannelGetAttribute(channel, BASS_ATTRIB_VOL, &volume);
			}
			lua_pop(L, 1);
		}
		else
		{
			// multiply the number by 10 to match the range used in channel volume control
			volume = BASS_GetVolume() * 10;
		}

		lua_pushnumber(L, volume);

		return 1;
	}

	int BassLibrary::isChannelActive(lua_State* L)
	{
		HSTREAM channel = getChannel(L, 1, "bass.isChannelActive() channel expected");
		DWORD status = BASS_ChannelIsActive(channel);
		bool isActive = (status == BASS_ACTIVE_STOPPED ? false : true);

		lua_pushboolean(L, isActive);

		return 1;
	}

	int BassLibrary::isChannelPaused(lua_State* L)
	{
		HSTREAM channel = getChannel(L, 1, "bass.isChannelPaused() channel expected");
		DWORD status = BASS_ChannelIsActive(channel);
		bool isPaused = (status == BASS_ACTIVE_PAUSED ? true : false);

		lua_pushboolean(L, isPaused);

		return 1;
	}

	int BassLibrary::isChannelPlaying(lua_State* L)
	{
		HSTREAM channel = getChannel(L, 1, "bass.isChannelPlaying() channel expected");
		DWORD status = BASS_ChannelIsActive(channel);
		bool isPlaying = (status == BASS_ACTIVE_PLAYING ? true : false);

		lua_pushboolean(L, isPlaying);

		return 1;
	}

	int BassLibrary::loadSound(lua_State* L)
	{
		HSTREAM channel;
		const char* fileName;
		const char* filePath;

		if (lua_isstring(L, 1))
		{
			fileName = lua_tostring(L, 1);
		}
		else
		{
			CoronaLuaError(L, "bass.loadSound() filename (string) expected, got: %s", lua_typename(L, 1));
		}

		if (lua_isstring(L, 2))
		{
			filePath = lua_tostring(L, 2);
		}
		else
		{
			CoronaLuaError(L, "bass.loadSound() filePath (string) expected, got: %s", lua_typename(L, 2));
		}

		std::string fullPath = filePath;
		fullPath.append("\\");
		fullPath.append(fileName);

		//BASS_SampleLoad ?

		if (channel = BASS_StreamCreateFile(FALSE, fullPath.c_str(), 0, 0, 0))
		{
			lua_pushnumber(L, channel);
			return 1;
		}
		else
		{
			CoronaLuaError(L, "bass.loadSound() couldn't create sound for file: %s", fullPath.c_str());
		}

		return 0;
	}

	int BassLibrary::loadStream(lua_State* L)
	{
		HSTREAM channel;
		const char* fileName;
		const char* filePath;

		if (lua_isstring(L, 1))
		{
			fileName = lua_tostring(L, 1);
		}
		else
		{
			CoronaLuaError(L, "bass.loadStream() filename (string) expected, got: %s", lua_typename(L, 1));
		}

		if (lua_isstring(L, 2))
		{
			filePath = lua_tostring(L, 2);
		}
		else
		{
			CoronaLuaError(L, "bass.loadStream() filePath (string) expected, got: %s", lua_typename(L, 2));
		}

		std::string fullPath = filePath;
		fullPath.append("\\");
		fullPath.append(fileName);

		if (channel = BASS_StreamCreateFile(TRUE, fullPath.c_str(), 0, 0, 0))
		{
			lua_pushnumber(L, channel);
			return 1;
		}
		else
		{
			CoronaLuaError(L, "bass.loadStream() couldn't create stream for file: %s", fullPath.c_str());
		}

		return 0;
	}

	int BassLibrary::pause(lua_State* L)
	{
		HSTREAM channel = getChannel(L, 1, "bass.pause() channel expected");
		BASS_ChannelPause(channel);

		return 0;
	}

	int BassLibrary::play(lua_State* L)
	{
		HSTREAM channel = getChannel(L, 1, "bass.play() channel expected");
		// TODO: get onComplete listener and execute it when audio playback completes

		if (lua_istable(L, 2))
		{
			lua_getfield(L, 2, "loop");
			if (lua_isboolean(L, -1))
			{
				bool loop = lua_toboolean(L, -1);
				BASS_ChannelFlags(channel, (loop ? BASS_SAMPLE_LOOP : 0), BASS_SAMPLE_LOOP);
			}
			lua_pop(L, 1);
		}

		// play the stream (continue from current position)
		if (!BASS_ChannelPlay(channel, FALSE))
		{
			CoronaLuaError(L, "bass.play() couldn't play audio for channel: %lu", channel);
		}

		return 0;
	}

	int BassLibrary::resume(lua_State* L)
	{
		HSTREAM channel = getChannel(L, 1, "bass.resume() channel expected");
		BASS_ChannelPlay(channel, FALSE);
		return 0;
	}

	int BassLibrary::rewind(lua_State* L)
	{
		HSTREAM channel = getChannel(L, 1, "bass.rewind() channel expected");
		// TODO: 

		return 0;
	}

	int BassLibrary::setVolume(lua_State* L)
	{
		float volume = 0;
		HSTREAM channel;

		if (lua_isnumber(L, 1))
		{
			volume = (float)lua_tonumber(L, 1);
		}

		if (lua_istable(L, 2))
		{
			lua_getfield(L, 2, "channel");
			if (lua_isnumber(L, -1))
			{
				channel = getChannel(L, -1, "bass.setVolume() options.channel expected");
				BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, volume);
			}
			lua_pop(L, 1);
		}
		else
		{
			// divide the number by 10 to match the range used in channel volume control
			BASS_SetVolume((float)volume / 10);
		}

		return 0;
	}

	int BassLibrary::stop(lua_State* L)
	{
		HSTREAM channel = getChannel(L, 1, "bass.stop() channel expected");
		BASS_ChannelStop(channel);

		return 0;
	}

	int BassLibrary::update(lua_State* L)
	{
		HSTREAM channel = getChannel(L, 1, "bass.update() channel expected");

		if (lua_istable(L, 2))
		{
			lua_getfield(L, 2, "loop");
			if (lua_isboolean(L, -1))
			{
				bool loop = lua_toboolean(L, -1);
				BASS_ChannelFlags(channel, (loop ? BASS_SAMPLE_LOOP : 0), BASS_SAMPLE_LOOP);
			}
			lua_pop(L, 1);
		}

		return 0;
	}

	// ----------------------------------------------------------------------------

} // namespace Corona

// ----------------------------------------------------------------------------

CORONA_EXPORT
int luaopen_plugin_bass(lua_State* L)
{
	return Corona::BassLibrary::Open(L);
}

