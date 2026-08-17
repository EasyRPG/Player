local easyrpg = require('assembly.appdomain')
local node = EasyRPGPlayer.Assembly.leasy.meta2.node;

function leasy.User.ready()
  local frame = node.Node2D.new();
  sprite = node.Sprite2D.new('/Users/wys/Documents/leasy/icon.png', true)
  frame:addChild(sprite)

  node.addChildToMain(sprite)
end

function leasy.User.process()
  local x, y = sprite:pos()
  sprite:move(x + 1, y + 1)
end