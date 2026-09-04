local inspect = require('inspect')
local rpg = require('assembly.appdomain')

local node = ep.Assembly.leasy.meta2.node

local image1 = 'icon.png'
local image2 = 'resources/psvita/icon0.png';

function leasy.User.ready() -- Called when the engine boots
  io.open('dump.txt', "w"):write(inspect.inspect(_G)):close()

  sprite = node.Sprite2D.new(image1, true) -- transparent? true
  node.addChildToMain(sprite)
end

function leasy.User.process(delta)
  sprite:move(100 + math.random(0, 3), 100+ math.random(0, 3));
end

function leasy.Engine.onGameLoaded(map)
  print('new game name:', map)

  local sprite2 = node.Sprite2D.new(image2, false) -- no transparent.
  sprite:moveFrom(sprite2)
end