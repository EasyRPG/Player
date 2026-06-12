---@class leasy
---@field leasy.System.version.full any
---@field leasy.System.version.major any
---@field leasy.System.version.minor any
---@field leasy.System.version.fix any
leasy = leasy or {}
leasy.Image = leasy.Image or {}
leasy.User = {}
leasy.Engine = leasy.Engine or {}

---deletes an image
---@param id integer
function leasy.Image.delete(id)end


---sends your image in the drawqueue.
---@param id integer
---@param x integer
---@param y integer
function leasy.Image.draw(id, x, y)end

---creates a new image, and loads it from the given path.
---@param path string
---@return integer id An internal identifier for your image. This identifier will die and may not be the same after deleting the image. 
function leasy.Image.new(path) return -1 end
