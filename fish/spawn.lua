local uv = require("luv")

---@alias uv_process_t metatable

---@alias SpawnCallback fun(
---  code: integer,      -- Exit code (-1 if spawning failed)
---  signal: integer,    -- Signal that terminated the process (0 if none)
---  stdout: string,     -- Complete stdout output
---  stderr: string,     -- Complete stderr output
---)

---Spawns a child process and collects all stdout/stderr before invoking the callback.
---
---The callback is called exactly once:
---  - On success, after the process exits.
---  - On failure to spawn, immediately with `code = -1`.
---
---@param cmd string Executable to run.
---@param args string[] Command-line arguments.
---@param callback SpawnCallback Called when the process finishes.
---@return uv_process_t? handle Process handle, or nil if spawning failed.
local function spawn(cmd, args, callback)
  local stdout = uv.new_pipe(false)
  local stderr = uv.new_pipe(false)

  local mout = {}
  local merr = {}

  local handle

  handle = uv.spawn(cmd, {
    args = args,
    stdio = {nil, stdout, stderr}
  }, function(code, signal)
    stdout:close()
    stderr:close()
    handle:close()

    callback(code, signal, table.concat(mout), table.concat(merr))
  end)

  if not handle then
    callback(-1, 0, "", "failed to spawn " .. cmd)
    return
  end

  stdout:read_start(function(err, chunk)
    if chunk then
      mout[#mout + 1] = chunk
    end
  end)

  stderr:read_start(function(err, chunk)
    if chunk then
      merr[#merr + 1] = chunk
    end
  end)

  return handle
end

return spawn