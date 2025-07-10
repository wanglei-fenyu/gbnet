function call_bcak(reply,num)
      log.Info("square reply num:"..num)
end

local function test_rpc()
    local call = RpcCall.new()
    call:SetSession(connect.session)
    call:SetCallBack(call_bcak)
    --net.Call(call,"lua_rpc_test_args","hello")
   -- net.Call(call, "lua_rpc_test")
    net.Call(call, "square",20)
    net.Call(call,"lua_rpc_test_args","world ")
end


--test_rpc()
