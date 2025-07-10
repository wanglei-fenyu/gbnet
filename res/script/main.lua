require("socket.core")
local debuger = require "LuaPanda"
debuger.start("127.0.0.1", 8828)

log.Info("load script ...")

local function test_msg_pack()
    local bin = msgpack.pack(19, "aaa", 100)
    local a, b, c = msgpack.unpack(bin)
    log.Info(""..a)
    log.Info(""..b)
    log.Info(""..c)
end
test_msg_pack()


function world()
    log.Error("xxxxxxxxx")
end



function lua_rpc_test(reply)
    log.Warning("lua_rpc_test")
end

function lua_rpc_test_args(reply,a)
    log.Warning("lua_rpc_test_args"..":"..a)
    reply:Invoke(a..a)        
end

net.Register("lua_rpc_test",lua_rpc_test)
net.Register("lua_rpc_test_args",lua_rpc_test_args)



function hello(session,message)
    log.Error("xxxxxxxxxxxxxxxxxxxxxx")
    log.Error(message:msg())
    log.Info("msg:"..message:msg().." index:"..message:index())
    log.Warning(message:index().."")
    --net.Send(session,1,2,"TestMsg",message)
end
net.Listen(1,2,hello,"TestMsg")