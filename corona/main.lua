local widget = require("widget")
local bass = require("plugin.bass")
widget.setTheme("widget_theme_android_holo_dark")

local resourcePath = system.pathForFile("", system.ResourceDirectory)
local soundFileName = "thunder.wav"
local streamFileName = "Gentle-Rain.mp3"
local loadSoundButton = nil
local loadStreamButton = nil
local soundChannel = 0
local streamChannel = 0
local channel = 0

local function onAudioComplete(event)
	if (type(event) == "table") then
		for k, v in pairs(event) do
			print(k, v)
		end
	end
end

local typeSegmentedControl =
	widget.newSegmentedControl(
	{
		segmentWidth = 150,
		segments = {"Sound", "Stream"},
		defaultSegment = 1,
		onPress = function(event)
			local segmentNumber = event.target.segmentNumber

			loadSoundButton.isVisible = (segmentNumber == 1)
			loadStreamButton.isVisible = (segmentNumber == 2)
			channel = (segmentNumber == 1) and soundChannel or streamChannel
		end
	}
)
typeSegmentedControl.x = display.contentCenterX
typeSegmentedControl.y = 50

loadSoundButton =
	widget.newButton(
	{
		label = "Load Sound",
		onPress = function(event)
			soundChannel = bass.loadSound(soundFileName, resourcePath)
			channel = soundChannel
			print("audio duration: ", bass.getDuration(channel))
		end
	}
)
loadSoundButton.x = display.contentCenterX
loadSoundButton.y = typeSegmentedControl.y + typeSegmentedControl.contentHeight + loadSoundButton.contentHeight

loadStreamButton =
	widget.newButton(
	{
		label = "Load Stream",
		onPress = function(event)
			streamChannel = bass.loadStream(streamFileName, resourcePath)
			channel = streamChannel

			for k, v in pairs(bass.getTags(streamChannel)) do
				print(k, v)
			end

			print("audio duration: ", bass.getDuration(channel))
		end
	}
)
loadStreamButton.x = display.contentCenterX
loadStreamButton.y = typeSegmentedControl.y + typeSegmentedControl.contentHeight + loadStreamButton.contentHeight
loadStreamButton.isVisible = false

local playButton =
	widget.newButton(
	{
		label = "Play",
		onPress = function(event)
			bass.play(channel, {onComplete = onAudioComplete})

			print("is channel playing?: ", bass.isChannelPlaying(channel))
		end
	}
)
playButton.x = display.contentCenterX
playButton.y = loadStreamButton.y + loadStreamButton.contentHeight

local pauseButton =
	widget.newButton(
	{
		label = "Pause",
		onPress = function(event)
			bass.pause(channel)
			print("is channel playing?: ", bass.isChannelPlaying(channel))
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
			bass.resume(channel)
			print("is channel playing?: ", bass.isChannelPlaying(channel))
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
			bass.stop(channel)
			print("is channel playing?: ", bass.isChannelPlaying(channel))
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
			bass.rewind(channel)
		end
	}
)
rewindButton.x = display.contentCenterX
rewindButton.y = stopButton.y + stopButton.contentHeight

local toggleLoopSwitch =
	widget.newSwitch(
	{
		style = "checkbox",
		onPress = function(event)
			local target = event.target

			bass.update(channel, {loop = target.isOn})
			print("is channel playing?: ", bass.isChannelPlaying(channel))
		end
	}
)
toggleLoopSwitch.x = display.contentCenterX
toggleLoopSwitch.y = rewindButton.y + rewindButton.contentHeight

local slider =
	widget.newSlider(
	{
		x = display.contentCenterX,
		width = 180,
		value = 100,
		listener = function(event)
			local value = event.value

			--bass.setVolume(value / 100)
			--print("global vol:", bass.getVolume())

			bass.setVolume(value / 100, {channel = channel})
			--print("channel vol:", bass.getVolume({channel = channel}))
		end
	}
)
slider.y = toggleLoopSwitch.y + toggleLoopSwitch.contentHeight

--bass.setVolume(1.0)
