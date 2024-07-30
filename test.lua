#!/usr/bin/env lua

os.execute("clear")

package.cpath = package.cpath .. ";./bin/?.so"

local yabai = require("stankysock")
local ansi = require("src.ansicolors")

local function cprint(text, color)
  local function process(line)
    if color then
      return ansi('%{' .. color .. '}' .. line)
    else
      return ansi(line)
    end
  end

  if type(text) == "table" then
    for _, line in ipairs(text) do
      print(process(line))
    end
  else
    print(process(text))
  end
end

local header = {
  "       __            __                     __  ",
  "  ___ / /____ ____  / /____ _____ ___  ____/ /__",
  " (_-</ __/ _ `/ _ \\/  '_/ // (_-</ _ \\/ __/  '_/",
  "/___/\\__/\\_,_/_//_/_/\\_\\\\_, /___/\\___/\\__/_/\\_\\ ",
  "                       /___/                  \n",
}

cprint(header, "magenta")

local display = yabai.display()

cprint("%{cyan}--- Current Display ---%{comment} yabai.display()")

cprint("\n%{green}  " .. string.format("%dx%d", display.frame.w, display.frame.h))

cprint("\n%{cyan}--- Active Window on Space " .. "%{yellow}" .. yabai.space().index
  .. "%{cyan} ---%{comment} yabai.space().index & yabai.window({'app', 'title'})")

local win = yabai.window({"app", "title"})

cprint("\n%{green}  " .. win.app .. "%{green}: %{yellow}" .. win.title)

cprint("\n%{cyan}--- Change Spaces --- %{comment}yabai.query('-m query --spaces')")

local spaces = yabai.query("-m query --spaces")
local space_display = {}
for _, space in ipairs(spaces) do
  local space_str = "%{green}" .. string.format("%d", space.index)
  if space["has-focus"] then
    space_str = "%{red}[" .. space_str .. "%{red}]"
  end
  table.insert(space_display, space_str)
end
cprint("\n " .. table.concat(space_display, " "))

cprint("\n%{cyan}Enter a space number to switch to (or 'q' to quit): %{comment}yabai.cmd('-m space --focus %d')")
local input = io.read()
if input ~= "q" then
  local space_index = tonumber(input)
  if space_index and space_index > 0 and space_index <= #spaces then
    yabai.cmd(string.format("-m space --focus %d", spaces[space_index].index))
  else
    cprint("%{red}Invalid space number! Exiting...")
  end
end
