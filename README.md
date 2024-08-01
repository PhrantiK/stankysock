```
       __            __                     __
  ___ / /____ ____  / /____ _____ ___  ____/ /__
 (_-</ __/ _ `/ _ \/  '_/ // (_-</ _ \/ __/  '_/
/___/\__/\_,_/_//_/_/\_\\_, /___/\___/\__/_/\_\
                       /___/
```
### A stinky bit of code for lua -> yabai socket calls

``brew install lua``

``make test``

``make install``

#### Usage 

```lua
package.cpath = package.cpath
  .. ";" .. os.getenv("HOME") .. "/.local/share/stankysock/?.so"

local yabai = require("stankysock")
```
```lua
-- yabai.cmd("cmd string") - fire & forget - no response, silent failure
yabai.cmd("-m space --focus 1")
```
```lua
-- yabai.query("query string") - returns table
local windows = yabai.query("-m query --windows --space")

for i, window in ipairs(windows) do
  print(string.format("%d: %s", i, window.app))
end
```
```lua
-- yabai.window|space|display({prop table}) - returns table for active item
local win = yabai.window()

local f = win["is-floating"] and "yep" or "nope"

print("floating? - " .. f)

-- optional table of properties to restrict response:
local s = yabai.space({"index", "label"})

print("Active Space Index: " .. s.index .. " Label: " .. s.label)

```
**CC0 -** `⌘c⌘v`
