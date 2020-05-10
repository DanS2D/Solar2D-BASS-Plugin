#include "bass.h"
#include "CoronaAssert.h"
#include "CoronaEvent.h"
#include "CoronaLua.h"
#include "CoronaLibrary.h"
#include <windows.h>
#include <math.h>
#include <stdio.h>
#include <string.h>
#include <map>
#include <string>
#include <codecvt> 
#include <vector>

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
		static const char kEventName[];

	public:
		static int Open(lua_State* L);
		static int Finalizer(lua_State* L);
		static Self* ToLibrary(lua_State* L);

	protected:
		BassLibrary();
		bool Initialize(void* platformContext);

	public:
		static int dispose(lua_State* L);
		static int fadeIn(lua_State* L);
		static int fadeOut(lua_State* L);
		static int getDevices(lua_State* L);
		static int getDuration(lua_State* L);
		static int getLevel(lua_State* L);
		static int getPlaybackTime(lua_State* L);
		static int getVolume(lua_State* L);
		static int isChannelActive(lua_State* L);
		static int isChannelPaused(lua_State* L);
		static int isChannelPlaying(lua_State* L);
		static int load(lua_State* L);
		static int pause(lua_State* L);
		static int play(lua_State* L);
		static int resume(lua_State* L);
		static int rewind(lua_State* L);
		static int seek(lua_State* L);
		static int setDevice(lua_State* L);
		static int setVolume(lua_State* L);
		static int stop(lua_State* L);
	};

	// ----------------------------------------------------------------------------

	// This corresponds to the name of the library, e.g. [Lua] require "plugin.library"
	const char BassLibrary::kName[] = "plugin.bass";
	const char BassLibrary::kEventName[] = "bass";
	HPLUGIN ac3;
	HPLUGIN ape;
	HPLUGIN mpc;
	HPLUGIN spx;
	HPLUGIN flac;
	HPLUGIN opus;
	HPLUGIN zxtune;

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
				{ "fadeIn", fadeIn },
				{ "fadeOut", fadeOut },
				{ "getDevices", getDevices },
				{ "getDuration", getDuration },
				{ "getLevel", getLevel },
				{ "getPlaybackTime", getPlaybackTime },
				{ "getVolume", getVolume },
				{ "isChannelActive", isChannelActive },
				{ "isChannelPaused", isChannelPaused },
				{ "isChannelPlaying", isChannelPlaying },
				{ "load", load },
				{ "pause", pause },
				{ "play", play },
				{ "resume", resume },
				{ "rewind", rewind },
				{ "seek", seek },
				{ "setDevice", setDevice },
				{ "setVolume", setVolume },
				{ "stop", stop },
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
			BASS_PluginFree(ac3);
			BASS_PluginFree(ape);
			BASS_PluginFree(mpc);
			BASS_PluginFree(spx);
			BASS_PluginFree(flac);
			BASS_PluginFree(opus);
			BASS_PluginFree(zxtune);

			ac3 = NULL;
			ape = NULL;
			mpc = NULL;
			spx = NULL;
			flac = NULL;
			opus = NULL;
			zxtune = NULL;
			BASS_Free();

			delete library;
		}

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
		BASS_SetConfig(BASS_CONFIG_UNICODE, TRUE);

		if (!BASS_Init(1, 44100, 0, 0, NULL))
		{
			printf("ERROR: plugin.bass - Can't initialize device\n");
		}

#ifdef _WIN32
		ac3 = BASS_PluginLoad("bass_ac3.dll", 0);
		ape = BASS_PluginLoad("bass_ape.dll", 0);
		mpc = BASS_PluginLoad("bass_mpc.dll", 0);
		spx = BASS_PluginLoad("bass_spx.dll", 0);
		flac = BASS_PluginLoad("bassflac.dll", 0);
		opus = BASS_PluginLoad("bassopus.dll", 0);
		zxtune = BASS_PluginLoad("basszxtune.dll", 0);
#endif

		return 1;
	}

	// ---------------------- HELPER FUNCTIONS -------------------------------------

	static std::wstring utf8_to_utf16(const std::string& utf8)
	{
		std::vector<unsigned long> unicode;
		size_t i = 0;
		while (i < utf8.size())
		{
			unsigned long uni;
			size_t todo;
			bool error = false;
			unsigned char ch = utf8[i++];
			if (ch <= 0x7F)
			{
				uni = ch;
				todo = 0;
			}
			else if (ch <= 0xBF)
			{
				throw std::logic_error("not a UTF-8 string");
			}
			else if (ch <= 0xDF)
			{
				uni = ch & 0x1F;
				todo = 1;
			}
			else if (ch <= 0xEF)
			{
				uni = ch & 0x0F;
				todo = 2;
			}
			else if (ch <= 0xF7)
			{
				uni = ch & 0x07;
				todo = 3;
			}
			else
			{
				throw std::logic_error("not a UTF-8 string");
			}
			for (size_t j = 0; j < todo; ++j)
			{
				if (i == utf8.size())
					throw std::logic_error("not a UTF-8 string");
				unsigned char ch = utf8[i++];
				if (ch < 0x80 || ch > 0xBF)
					throw std::logic_error("not a UTF-8 string");
				uni <<= 6;
				uni += ch & 0x3F;
			}
			if (uni >= 0xD800 && uni <= 0xDFFF)
				throw std::logic_error("not a UTF-8 string");
			if (uni > 0x10FFFF)
				throw std::logic_error("not a UTF-8 string");
			unicode.push_back(uni);
		}
		std::wstring utf16;
		for (size_t i = 0; i < unicode.size(); ++i)
		{
			unsigned long uni = unicode[i];
			if (uni <= 0xFFFF)
			{
				utf16 += (wchar_t)uni;
			}
			else
			{
				uni -= 0x10000;
				utf16 += (wchar_t)((uni >> 10) + 0xD800);
				utf16 += (wchar_t)((uni & 0x3FF) + 0xDC00);
			}
		}
		return utf16;
	}

	static DWORD GetChannel(lua_State* L, int index, const char* errorMessage)
	{
		DWORD channel;

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
		DWORD channel = GetChannel(L, 1, "bass.dispose() channel (number) expected");
		BASS_StreamFree(channel);

		return 0;
	}

	int BassLibrary::fadeIn(lua_State* L)
	{
		DWORD channel = GetChannel(L, 1, "bass.fadeIn() channel (number) expected");
		float time = 0;

		if (lua_isnumber(L, 2))
		{
			time = (float)lua_tonumber(L, 2);
			BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, 0);
			BASS_ChannelSlideAttribute(channel, BASS_ATTRIB_VOL | BASS_SLIDE_LOG, 1.0, time);
		}
		else
		{
			CoronaLuaError(L, "bass.fadeIn() time (number) expected, got: ", lua_typename(L, 2));
		}

		return 0;
	}

	int BassLibrary::fadeOut(lua_State* L)
	{
		DWORD channel = GetChannel(L, 1, "bass.fadeOut() channel (number) expected");
		float time = 0;

		if (lua_isnumber(L, 2))
		{
			time = (float)lua_tonumber(L, 2);
			BASS_ChannelSlideAttribute(channel, BASS_ATTRIB_VOL | BASS_SLIDE_LOG, 0, time);
		}
		else
		{
			CoronaLuaError(L, "bass.fadeOut() time (number) expected, got: ", lua_typename(L, 2));
		}

		return 0;
	}

	int BassLibrary::getDevices(lua_State* L)
	{
		BASS_DEVICEINFO deviceInfo;
		lua_newtable(L);

		for (int i = 0; BASS_GetDeviceInfo(i, &deviceInfo); i++)
		{
			if (deviceInfo.flags & BASS_DEVICE_ENABLED)
			{
				lua_newtable(L);
				lua_pushnumber(L, i);
				lua_setfield(L, -2, "index");
				lua_pushstring(L, deviceInfo.name);
				lua_setfield(L, -2, "name");
				lua_rawseti(L, -2, i);
			}
		}

		return 1;
	}

	int BassLibrary::getDuration(lua_State* L)
	{
		// TODO: change duration output to seconds
		DWORD channel = GetChannel(L, 1, "bass.getDuration() channel (number) expected");
		QWORD lengthInBytes = BASS_ChannelGetLength(channel, BASS_POS_BYTE);
		double durationInSeconds = BASS_ChannelBytes2Seconds(channel, lengthInBytes);

		// match the corona audio api and return this number in miliseconds
		lua_pushnumber(L, durationInSeconds / 1000);

		return 1;
	}

	int BassLibrary::getLevel(lua_State* L)
	{
		DWORD channel = GetChannel(L, 1, "bass.getLevel() channel (number) expected");
		DWORD level = BASS_ChannelGetLevel(channel);

		lua_pushnumber(L, LOWORD(level));
		lua_pushnumber(L, HIWORD(level));

		return 2;
	}

	int BassLibrary::getPlaybackTime(lua_State* L)
	{
		DWORD channel = GetChannel(L, 1, "bass.getPlaybackTime() channel (number) expected");
		DWORD length = BASS_ChannelGetLength(channel, BASS_POS_BYTE);
		DWORD position = BASS_ChannelGetPosition(channel, BASS_POS_BYTE);
		DWORD duration = BASS_ChannelBytes2Seconds(channel, length);
		DWORD elapsed = BASS_ChannelBytes2Seconds(channel, position);

		lua_newtable(L);
		lua_newtable(L);
		lua_pushnumber(L, elapsed / 60);
		lua_setfield(L, -2, "minutes");
		lua_pushnumber(L, elapsed % 60);
		lua_setfield(L, -2, "seconds");
		lua_setfield(L, -2, "elapsed");
		lua_newtable(L);
		lua_pushnumber(L, duration / 60);
		lua_setfield(L, -2, "minutes");
		lua_pushnumber(L, duration % 60);
		lua_setfield(L, -2, "seconds");
		lua_setfield(L, -2, "duration");

		return 1;
	}

	int BassLibrary::getVolume(lua_State* L)
	{
		float volume = 0;
		DWORD channel;

		if (lua_istable(L, 1))
		{
			lua_getfield(L, 1, "channel");
			if (lua_isnumber(L, -1))
			{
				channel = GetChannel(L, -1, "bass.getVolume() options.channel (number) expected");
				BASS_ChannelGetAttribute(channel, BASS_ATTRIB_VOL, &volume);
			}
			lua_pop(L, 1);
		}
		else
		{
			volume = (float)(BASS_GetConfig(BASS_CONFIG_GVOL_STREAM));
			volume /= 10000;
		}

		lua_pushnumber(L, volume);

		return 1;
	}

	int BassLibrary::isChannelActive(lua_State* L)
	{
		DWORD channel = GetChannel(L, 1, "bass.isChannelActive() channel (number) expected");
		DWORD status = BASS_ChannelIsActive(channel);
		bool isActive = (status == BASS_ACTIVE_STOPPED ? false : true);

		lua_pushboolean(L, isActive);

		return 1;
	}

	int BassLibrary::isChannelPaused(lua_State* L)
	{
		DWORD channel = GetChannel(L, 1, "bass.isChannelPaused() channel (number) expected");
		DWORD status = BASS_ChannelIsActive(channel);
		bool isPaused = (status == BASS_ACTIVE_PAUSED ? true : false);

		lua_pushboolean(L, isPaused);

		return 1;
	}

	int BassLibrary::isChannelPlaying(lua_State* L)
	{
		DWORD channel = GetChannel(L, 1, "bass.isChannelPlaying() channel (number) expected");
		DWORD status = BASS_ChannelIsActive(channel);
		bool isPlaying = (status == BASS_ACTIVE_PLAYING ? true : false);

		lua_pushboolean(L, isPlaying);

		return 1;
	}

	int BassLibrary::load(lua_State* L)
	{
		DWORD channel;
		const char* fileName;
		const char* filePath;
		std::string fullPath;

		if (lua_isstring(L, 1))
		{
			fileName = lua_tostring(L, 1);
		}
		else
		{
			CoronaLuaError(L, "bass.load() filename (string) expected, got: %s", lua_typename(L, 1));
			lua_pushnil(L);
			return 1;
		}

		if (lua_isstring(L, 2))
		{
			filePath = lua_tostring(L, 2);
		}
		else
		{
			CoronaLuaError(L, "bass.load() filePath (string) expected, got: %s", lua_typename(L, 2));
			lua_pushnil(L);
			return 1;
		}

		fullPath = filePath;
		fullPath.append("\\");
		fullPath.append(fileName);
		std::wstring utf16Path = utf8_to_utf16(fullPath);

		if (channel = BASS_StreamCreateFile(FALSE, utf16Path.c_str(), 0, 0, BASS_ASYNCFILE))
		{
			lua_pushnumber(L, channel);
			return 1;
		}
		else
		{
			CoronaLuaWarning(L, "bass.load() couldn't create stream for file: %s", fullPath.c_str());
			lua_pushnil(L);
			return 1;
		}

		lua_pushnil(L);
		return 1;
	}

	int BassLibrary::pause(lua_State* L)
	{
		DWORD channel = GetChannel(L, 1, "bass.pause() channel (number) expected");
		BASS_ChannelPause(channel);

		return 0;
	}

	int BassLibrary::play(lua_State* L)
	{
		DWORD channel = GetChannel(L, 1, "bass.play() channel (number) expected");

		// play the stream (continue from current position)
		if (!BASS_ChannelPlay(channel, FALSE))
		{
			CoronaLuaWarning(L, "bass.play() couldn't play audio for channel: %lu", channel);
		}

		lua_pushnumber(L, channel);
		return 1;
	}

	int BassLibrary::resume(lua_State* L)
	{
		DWORD channel = GetChannel(L, 1, "bass.resume() channel (number) expected");
		BASS_ChannelPlay(channel, FALSE);
		return 0;
	}

	int BassLibrary::rewind(lua_State* L)
	{
		// TODO: shouldn't try and set position on a potentially NULL channel. Check for that
		DWORD channel = GetChannel(L, 1, "bass.rewind() channel (number) expected");
		BASS_ChannelSetPosition(channel, 0, BASS_POS_RESET);

		return 0;
	}

	int BassLibrary::seek(lua_State* L)
	{
		DWORD channel = GetChannel(L, 1, "bass.seek() channel expected");
		float time = 0;

		if (lua_isnumber(L, 2))
		{
			time = (float)lua_tonumber(L, 2) * 1000;
			BASS_ChannelSetPosition(channel, BASS_ChannelSeconds2Bytes(channel, time), BASS_POS_BYTE);
		}
		else
		{
			CoronaLuaError(L, "bass.seek() time (number) expected, got: ", lua_typename(L, 2));
		}

		return 0;
	}

	int BassLibrary::setDevice(lua_State* L)
	{
		int deviceIndex = 1;

		if (lua_isnumber(L, 1))
		{
			deviceIndex = lua_tonumber(L, 1);
			BASS_Free();

			if (!BASS_Init(deviceIndex, 44100, 0, 0, NULL))
			{
				CoronaLuaError(L, "bass.setDevice() couldn't initialize device at index %d\n", deviceIndex);
			}
		}
		else
		{
			CoronaLuaError(L, "bass.setDevice() device index (number) expected, got: ", lua_typename(L, 1));
		}

		return 0;
	}

	int BassLibrary::setVolume(lua_State* L)
	{
		float volume = 0;
		DWORD channel;

		if (lua_isnumber(L, 1))
		{
			volume = (float)lua_tonumber(L, 1);
		}

		if (lua_istable(L, 2))
		{
			lua_getfield(L, 2, "channel");
			if (lua_isnumber(L, -1))
			{
				channel = GetChannel(L, -1, "bass.setVolume() options.channel (number) expected");
				BASS_ChannelSetAttribute(channel, BASS_ATTRIB_VOL, volume);
			}
			lua_pop(L, 1);
		}
		else
		{
			BASS_SetConfig(BASS_CONFIG_GVOL_STREAM, volume * 10000);
		}

		return 0;
	}

	int BassLibrary::stop(lua_State* L)
	{
		DWORD channel = GetChannel(L, 1, "bass.stop() channel (number) expected");

		BASS_ChannelStop(channel);

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

