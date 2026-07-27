local function parse_args(str)
  local args = {}
  local current = {}
  local quote = nil
  local escape = false

  local function push()
    if #current > 0 then
      table.insert(args, table.concat(current))
      current = {}
    end
  end

  for i = 1, #str do
    local c = str:sub(i, i)

    if escape then
      table.insert(current, c)
      escape = false

    elseif c == "\\" then
      escape = true

    elseif quote then
      if c == quote then
        quote = nil
      else
        table.insert(current, c)
      end

    elseif c == '"' or c == "'" then
      quote = c

    elseif c:match("%s") then
      push()

    else
      table.insert(current, c)
    end
  end

  if escape then
    table.insert(current, "\\")
  end

  push()

  return args
end

return parse_args