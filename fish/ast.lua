local ast = {}

---@class ast.node
---@field raw table
---@field parent ast.node|nil
ast.node = {}
ast.node.__index = ast.node


---@param raw table
---@param parent ast.node|nil
---@return ast.node
function ast.node.new(raw, parent)
  return setmetatable({
    raw = raw,
    parent = parent,
  }, ast.node)
end


---@return string|nil
function ast.node:kind()
  return self.raw.kind
end


---@return string|nil
function ast.node:name()
  return self.raw.name
end


---@return string|nil
function ast.node:type()
  if not self.raw.type then
    return nil
  end

  return self.raw.type.qualType
end


---@return ast.node[]
function ast.node:children()
  local children = {}

  if type(self.raw.inner) ~= "table" then
    return children
  end

  for _, child in ipairs(self.raw.inner) do
    table.insert(children, ast.node.new(child, self))
  end

  return children
end

---@return string|nil
function ast.node:fullname()
  local names = {}
  local current = self

  while current do
    if current:name() then
      table.insert(names, 1, current:name())
    end

    current = current.parent
  end

  if #names == 0 then
    return nil
  end

  return table.concat(names, "::")
end

---@return boolean
function ast.node:is_deleted()
  return self.raw.explicitlyDeleted == true
end

---@param node ast.node
---@param fn fun(node: ast.node)
function ast.walk(node, fn)
  fn(node)

  for _, child in ipairs(node:children()) do
    ast.walk(child, fn)
  end
end

---@param raw table
---@return ast.node
function ast.load(raw)
  return ast.node.new(raw, nil)
end
local query = {}


local function has_kind(node, ...)
  local kind = node:kind()

  for _, value in ipairs({...}) do
    if kind == value then
      return true
    end
  end

  return false
end


---@param node ast.node
---@return boolean
function query.is_function(node)
  return has_kind(
    node,
    "FunctionDecl",
    "CXXMethodDecl",
    "CXXConstructorDecl",
    "CXXDestructorDecl"
  )
end


---@param node ast.node
---@return boolean
function query.is_record(node)
  return has_kind(
    node,
    "RecordDecl",
    "CXXRecordDecl"
  )
end


---@param node ast.node
---@return boolean
function query.is_namespace(node)
  return has_kind(
    node,
    "NamespaceDecl"
  )
end

---@param node ast.node
---@return boolean
function query.is_template(node)
  return has_kind(
    node,
    "FunctionTemplateDecl",
    "ClassTemplateDecl",
    "ClassTemplateSpecializationDecl"
  )
end

---@param node ast.node
---@return boolean
function query.is_constructor(node)
  return node:kind() == "CXXConstructorDecl"
end

---@return boolean
function ast.node:inside_template()
  local current = self.parent

  while current do
    if query.is_template(current) then
      return true
    end

    current = current.parent
  end

  return false
end

---@param root ast.node
---@return ast.node[]
function ast.functions(root)
  local result = {}

  ast.walk(root, function(node)
    if query.is_function(node) then
      table.insert(result, node)
    end
  end)

  return result
end


---@param root ast.node
---@return ast.node[]
function ast.records(root)
  local result = {}

  ast.walk(root, function(node)
    if query.is_record(node) then
      table.insert(result, node)
    end
  end)

  return result
end

---@region uhm things location etc (this code gets ugly)

---@class ast.location
---@field raw table
ast.location = {}
ast.location.__index = ast.location

---@param raw table
---@return ast.location
function ast.location.new(raw)
  return setmetatable({
    raw = raw,
  }, ast.location)
end

---@return string?
function ast.location:file()
  return self.raw.file
end

---@return number?
function ast.location:line()
  return self.raw.line
end

---@return number?
function ast.location:column()
  return self.raw.col
end

---@return number?
function ast.location:offset()
  return self.raw.offset
end

---@return ast.location?
function ast.node:location()
  if type(self.raw.loc) ~= "table" then
    return nil
  end

  return ast.location.new(self.raw.loc)
end

---@endregion

ast.query = query

return ast