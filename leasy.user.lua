local rpg = require('assembly.appdomain')
local node = EasyRPGPlayer.Assembly.leasy.meta2.node;
local std = EasyRPGPlayer.Assembly.std;

function leasy.User.ready() -- Called when the engine boots
  sprite = node.Sprite2D.new('/Users/wys/Documents/leasy/icon.png', true) -- transparent? true
  node.addChildToMain(sprite)
end

function leasy.User.process(delta)
  local x, y = sprite:pos()

  if x >= 400 then
    x = 0
  end

  if y  >= 300 then
    y = 0
  end

  sprite:move((x + 2), (y + 2));
end
