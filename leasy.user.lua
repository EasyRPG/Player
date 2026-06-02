leasy = leasy or {}
leasy.image = leasy.image or {}

local img = leasy.image.new("icon.png")

local pos = {
  x = 0,
  y = 0,
}

function leasy.user.draw()
  leasy.image.draw(img, pos.x, pos.y)
  leasy.text.write("hello, world NEGAAAA", 0xFF0000FF, 0, 0, 200, 200)
  pos = {
    x = pos.x + 1,
    y = pos.y + 1
  }

  if pos.x > 400 then
    pos.x = 0
  end
  if pos.y > 300 then
    pos.y = 0
  end
end
