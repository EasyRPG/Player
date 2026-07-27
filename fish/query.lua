local query = {}

---@generic K
---@generic V
---@generic K2
---@generic V2
---@param t table<K, V>
---@param f fun(k: K, v: V):K2,V2
---@param it function?
---@return table<K2, V2>
function query.foreach(t, f, it)
  local r = {}
  local it = it or pairs

  for k, v in it(t) do
    K, V = f(k, v)
    r[K] = V
  end

  return r
end

---@generic K
---@generic V
---@param t table<K, V>
---@param pred fun(k: K, v: V):boolean
---@return table<K, V>
function query.where(t, pred)
  local selection = {}
  for key, value in pairs(t) do
    if (pred(key, value)) then
      selection[key] = value
    end
  end

  return selection
end

---@generic K, V, K2, V2
---@param t table<K, V>
---@param mapper fun(k:K,v:V):K2?,V2?
---@return table<K2, V2>
function query.map(t, mapper)
  local mapped = {}
  for key, value in pairs(t) do
    local k, v = mapper(key, value)
    if k and v then
      mapped[k] = v
    end
  end

  return mapped
end

return query