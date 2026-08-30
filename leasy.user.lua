local rpg = require('assembly.appdomain')

local node = EasyRPGPlayer.Assembly.leasy.meta2.node;
local sounding = EasyRPGPlayer.Assembly.EasyRPGPlayer.Sounding

function leasy.User.ready() -- Called when the engine boots
  sprite = node.Sprite2D.new('/Users/wys/Pictures/pixel_art_large.png', true) -- transparent? true
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
