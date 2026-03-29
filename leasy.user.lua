leasy = leasy or {}
leasy.image = leasy.image or {}

local img = leasy.image.new("icon.png")

local pos = {
  x = 0,
  y = 0,
}

function leasy.user.draw()
  leasy.image.draw(img, pos.x, pos.y)
  leasy.text.write("hello, world", 0xFF0000FF, 0, 0, 200, 200)
end