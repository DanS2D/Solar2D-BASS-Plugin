local widget = require("widget")
local bass = require("plugin.bass")
widget.setTheme("widget_theme_android_holo_dark")

local resourcePath = system.pathForFile(nil, system.ResourceDirectory)
--print(package.config)
bass.loadChipTunesPlugin()

local musicSamples = {
	{channel = 0, fileName = "Thunder.wav", loop = false, fileType = "wav"},
	{channel = 0, fileName = "Gentle - Rain.mp3", loop = false, fileType = "mp3"},
	{channel = 0, fileName = "Streets of Rage - The Street of Rage.vgz", loop = false, fileType = "vgz"},
	{channel = 0, fileName = "Sample_BeeMoved_96kHz24bit.flac", loop = false, fileType = "flac"},
	{channel = 0, fileName = "Sample_BeeMoved_48kHz16bit.m4a", loop = false, fileType = "aac"}
}

local currentAudioIndex = 1
local toggleLoopSwitch = nil
local volumeSlider = nil

local function onAudioComplete(event)
	if (type(event) == "table") then
		for k, v in pairs(event) do
			print(k, v)
		end
	end
end

Runtime:addEventListener("bass", onAudioComplete)

local typeSegmentedControl =
	widget.newSegmentedControl(
	{
		segmentWidth = 50,
		segments = {
			musicSamples[1].fileType,
			musicSamples[2].fileType,
			musicSamples[3].fileType,
			musicSamples[4].fileType,
			musicSamples[5].fileType
		},
		defaultSegment = 1,
		onPress = function(event)
			currentAudioIndex = event.target.segmentNumber
			local currentVolume = bass.getVolume()
			--bass.getVolume({channel = musicSamples[currentAudioIndex].channel})
			volumeSlider:setValue(currentVolume * 100)
			toggleLoopSwitch:setState({isOn = musicSamples[currentAudioIndex].loop, isAnimated = false})
		end
	}
)
typeSegmentedControl.x = display.contentCenterX
typeSegmentedControl.y = 50

local loadButton =
	widget.newButton(
	{
		label = "Load",
		onPress = function(event)
			print("path to loaded file: ", resourcePath .. "/" .. musicSamples[currentAudioIndex].fileName)
			musicSamples[currentAudioIndex].channel = bass.load(musicSamples[currentAudioIndex].fileName, resourcePath)
			local currentVolume = bass.getVolume()
			--bass.getVolume({channel = musicSamples[currentAudioIndex].channel})
			volumeSlider:setValue(currentVolume * 100)

			--for k, v in pairs(bass.getTags(musicSamples[currentAudioIndex].channel)) do
			--	print(k, v)
			--end

			print("audio duration: ", bass.getDuration(musicSamples[currentAudioIndex].channel))
		end
	}
)
loadButton.x = display.contentCenterX
loadButton.y = typeSegmentedControl.y + typeSegmentedControl.contentHeight + loadButton.contentHeight

local playButton =
	widget.newButton(
	{
		label = "Play",
		onPress = function(event)
			bass.play(musicSamples[currentAudioIndex].channel)
			--bass.fadeOut(channel, 5000)

			print("is channel playing?: ", bass.isChannelPlaying(musicSamples[currentAudioIndex].channel))
		end
	}
)
playButton.x = display.contentCenterX
playButton.y = loadButton.y + loadButton.contentHeight

local pauseButton =
	widget.newButton(
	{
		label = "Pause",
		onPress = function(event)
			bass.pause(musicSamples[currentAudioIndex].channel)
			print("is channel playing?: ", bass.isChannelPlaying(musicSamples[currentAudioIndex].channel))
		end
	}
)
pauseButton.x = display.contentCenterX
pauseButton.y = playButton.y + playButton.contentHeight

local resumeButton =
	widget.newButton(
	{
		label = "Resume",
		onPress = function(event)
			bass.resume(musicSamples[currentAudioIndex].channel)
			print("is channel playing?: ", bass.isChannelPlaying(musicSamples[currentAudioIndex].channel))
		end
	}
)
resumeButton.x = display.contentCenterX
resumeButton.y = pauseButton.y + pauseButton.contentHeight

local stopButton =
	widget.newButton(
	{
		label = "Stop",
		onPress = function(event)
			bass.stop(musicSamples[currentAudioIndex].channel)
			print("is channel playing?: ", bass.isChannelPlaying(musicSamples[currentAudioIndex].channel))
		end
	}
)
stopButton.x = display.contentCenterX
stopButton.y = resumeButton.y + resumeButton.contentHeight

local rewindButton =
	widget.newButton(
	{
		label = "Rewind",
		onPress = function(event)
			bass.rewind(musicSamples[currentAudioIndex].channel)
		end
	}
)
rewindButton.x = display.contentCenterX
rewindButton.y = stopButton.y + stopButton.contentHeight

local seekButton =
	widget.newButton(
	{
		label = "Seek To Midway",
		onPress = function(event)
			print("seeking")
			bass.seek(musicSamples[currentAudioIndex].channel, bass.getDuration(musicSamples[currentAudioIndex].channel) / 2)
		end
	}
)
seekButton.x = display.contentCenterX
seekButton.y = rewindButton.y + rewindButton.contentHeight

toggleLoopSwitch =
	widget.newSwitch(
	{
		style = "checkbox",
		onPress = function(event)
			local target = event.target

			--bass.update(musicSamples[currentAudioIndex].channel, {loop = target.isOn})
			musicSamples[currentAudioIndex].loop = target.isOn
			print("is channel playing?: ", bass.isChannelPlaying(musicSamples[currentAudioIndex].channel))
		end
	}
)
toggleLoopSwitch.x = display.contentCenterX
toggleLoopSwitch.y = seekButton.y + seekButton.contentHeight

volumeSlider =
	widget.newSlider(
	{
		x = display.contentCenterX,
		width = 180,
		value = 100,
		listener = function(event)
			local value = event.value

			bass.setVolume(value / 100)
			--print("global vol:", bass.getVolume())

			--bass.setVolume(value / 100, {channel = musicSamples[currentAudioIndex].channel})
			--print("channel vol:", bass.getVolume({channel = channel}))
		end
	}
)
volumeSlider.y = toggleLoopSwitch.y + toggleLoopSwitch.contentHeight

--bass.setVolume(1.0)
