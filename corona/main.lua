local widget = require("widget")
local bass = require("plugin.bass")
widget.setTheme("widget_theme_android_holo_dark")

local fileName = "Various Artists - Clair De Lune.mp3"
local filePath = "E:\\Google Drive\\Music"

local loadButton =
	widget.newButton(
	{
		label = "Load",
		onPress = function(event)
			channel = bass.loadStream(fileName, filePath)
			print("audio duration: ", bass.getDuration(channel))
		end
	}
)
loadButton.x = display.contentCenterX
loadButton.y = 50

local playButton =
	widget.newButton(
	{
		label = "Play",
		onPress = function(event)
			bass.play(channel)
			print("is channel playing?: ", bass.isChannelPlaying(channel))
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

			--bass.update(channel, {loop = target.isOn})
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

bass.setVolume(1.0)
