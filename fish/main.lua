package.cpath = package.cpath .. ";/opt/homebrew/opt/luv/lib/lua/5.5/?.so"
package.path = package.path .. ";/opt/homebrew/opt/luv/lib/lua/5.5/?.lua"

local luv = require('luv')

local start = luv.hrtime()

local function seconds_since(t)
  return (luv.hrtime() - t) / 1e9
end

local compilation = require('fish.compilation')
local spawn = require('fish.spawn')
local argsp = require('fish.argsp')
local ast = require('fish.ast')
local json = require('fish.json')
local settings = require('fish.settings')
local query = require('fish.query')
local fs = require('fish.fs')

---@alias cmd { file: string, command: string, output: string, directory: string }

for key, value in pairs(settings) do
  print(key, value)
end

print(fs.realpath(settings.root))

io.write('load: loading build database ...')
io.flush()

local compile_file = io.open('build/compile_commands.json', 'r')
if not compile_file then
  error('cannot open build/compile_commands.json')
end

---@type cmd[]
local cpbase = json.decode(compile_file:read('a'))
compile_file:close()

io.write('done\n')

local MAX_JOBS = settings.jobs
local running = 0
local index = 1
local finished = 0

local report = {}
local includes = {}

---@param name string?
---@return boolean
local function is_name_interresting(name)
  if not name then
    return false
  end

  return not name:find("[^%w_:%.]")
end

---@param file string?
local function include(file)
  if not file then
    return
  end

  if fs.childof(file, settings.root) then
    includes[#includes + 1] = file
  end
end


local function spawn_next()
  while running < MAX_JOBS and index <= #cpbase do
    local task = cpbase[index]
    local current = index

    index = index + 1
    running = running + 1

    local pstart = luv.hrtime()

    spawn(
      'clang++',
      argsp(compilation.makecmd('', task.file, task.command)),
      function(code, signal, stdout, stderr)

        if code ~= 0 then
          print(
            'clang failed:',
            task.file,
            'exit code:',
            code,
            stderr
          )
        else
          local ok, node = pcall(function()
            return ast.load(json.decode(stdout))
          end)

          if ok then
            ---@param v ast.node
            local pred = function(_, v)
              local location = v:location()

              if location then
                include(location:file())
              end

              local nice =
                not v:is_deleted()
                and not v:inside_template()
                and is_name_interresting(v:fullname())

              if nice then
                return v:fullname(), v
              end
            end

            local funcs = query.map(ast.functions(node), pred)
            local records = query.map(ast.records(node), pred)

            report[current] = {
              funcs = funcs,
              records = records,
              file = task.file,
              index = current
            }
          else
            print('failed parsing AST for', task.file)
          end
        end

        print(
          ('file[ %d/%d]'):format(current, #cpbase),
          task.file,
          'treated in',
          string.format('%.3f', seconds_since(pstart)),
          's'
        )

        running = running - 1
        finished = finished + 1

        spawn_next()
      end
    )
  end
end


spawn_next()

while finished < #cpbase do
  luv.run('once')
end

print(
  ('compilation finished, took %.3f s')
  :format(seconds_since(start))
)

print('generating assembly glue...')

local file = io.open('src/leasy/dump.hpp', 'w')
if not file then
  error('cannot open dump file!')
end

for _, inc in ipairs(includes) do
  print('inc', inc)
  file:write('#include "', inc, '"\n')
end

file:close()

print(
  ("Total time: %.3f s")
  :format(seconds_since(start))
)