-- Hamster Protocol
-- Author: Andreas Werner
-- Date: 2018-04-26
--
-- Install Plugin to ~/.local/lib/wireshark/plugins

local hamster_proto = Proto("hamster", "Hamster Protocol")
hamster_proto.fields.version = ProtoField.uint8("hamster.version", "Version", base.HEX)
hamster_proto.fields.flags = ProtoField.uint8("hamster.flags", "Flags", base.HEX)
hamster_proto.fields.msgID = ProtoField.uint16("hamster.msgID", "Message ID", base.DEC)
hamster_proto.fields.length = ProtoField.uint16("hamster.length", "Payload Length", base.DEC)
hamster_proto.fields.rpcCallID = ProtoField.uint16("hamster.rpcCallID", "RPC-Call-ID", base.DEC)

hamster_proto.fields.fd = ProtoField.int32("hamster.fd", "File Descriptor", base.DEC)
hamster_proto.fields.owner_name = ProtoField.string("hamster.owner_name", "Name")
hamster_proto.fields.hamster_name = ProtoField.string("hamster.hamster_name", "Hamster Name")
hamster_proto.fields.treats = ProtoField.uint16("hamster.treats", "Treats", base.DEC)

hamster_proto.fields.state_treats_left = ProtoField.uint16("hamster.state.treats_left", "Treats Left", base.DEC)
hamster_proto.fields.state_rounds = ProtoField.uint32("hamster.state.rounds", "Rounds", base.DEC)
hamster_proto.fields.state_cost = ProtoField.int16("hamster.state.cost", "Cost", base.DEC)

hamster_proto.fields.id = ProtoField.int32("hamster.id", "ID", base.DEC)
hamster_proto.fields.ret = ProtoField.int32("hamster.ret", "Return Value", base.DEC)

function hamster_proto.dissector(buffer, pinfo, tree)
	err=""
	pinfo.cols.protocol = "Hamster"
	if (buffer:len() < 8) then
		if (buffer:len() >= 1) then
			local version = buffer(0, 1):uint()
			if (version ~= 0xb6) then
				-- skip message
				local subtree = tree:add(hamster_proto, buffer(), "Hamster: wrong version number: " .. string.format("0x%x", version))
				subtree:add(hamster_proto.fields.version, buffer(0,1))
				subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "Version Number shall be 0xb6")
				pinfo.cols.info = "Hamster wrong version number: " .. version
				return buffer:len()
			end
		end
		-- Message to short Header should >= 8 bytes
		-- wait for it
		pinfo.desegment_len = 8 - buffer:len()
		return buffer:len()
	end

	local version = buffer(0, 1):uint()
	local flags = buffer(1, 1):uint()
	local msgID = buffer(2, 2):uint()
	local length = buffer(4, 2):uint()
	local rpcCallID = buffer(6, 2):uint()

	if (version ~= 0xb6) then
		-- skip message
		local subtree = tree:add(hamster_proto, buffer(), "Hamster: wrong version number: " .. string.format("0x%x", version))
		subtree:add(hamster_proto.fields.version, buffer(0,1))
		subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "Version Number shall be 0xb6")
		pinfo.cols.info = "Hamster wrong version number: " .. string.format("0x%x", version)
		return buffer:len()
	end

	if (buffer:len() < (length + 8)) then
		-- Incomplete Frame
		pinfo.desegment_len = (length + 8) - buffer:len()
		return buffer:len()
	end
	

	local rpcName = ""
	if (rpcCallID == 0) then
		-- none
		rpcName = "0 is not a Call ID"
	elseif (rpcCallID == 1) then
		-- new
		rpcName = "New Hamster"
	elseif (rpcCallID == 2) then
		-- lookup
		rpcName = "Lookup"
	elseif (rpcCallID == 3) then
		-- Directory
		rpcName = "Directory"
	elseif (rpcCallID == 4) then
		-- Hows doing
		rpcName = "Hows doing"
	elseif (rpcCallID == 5) then
		-- read entry
		rpcName = "Read entry"
	elseif (rpcCallID == 6) then
		-- give threads
		rpcName = "Give Threads"
	elseif (rpcCallID == 7) then
		-- collect
		rpcName = "Collect"
	else
		-- ?
		rpcName = "Unknown: " .. rpcCallID
	end

	if (flags == 0x0) then
		request = "Request"
	elseif (flags == 0x1) then
		request = "Response"
	elseif (flags == 0x3) then
		request = "Error"
	else
		request = "Unknown Flag: " .. string.format("0x%x", flags)
	end

	info = request .. ", Call: ".. rpcName ..", Payload Length: " .. length .. ", msgID: " .. msgID
	local subtree = tree:add(hamster_proto, buffer(0, 8), "Hamster Header, " .. info)
	subtree:add(hamster_proto.fields.version, buffer(0,1))
	subtree:add(hamster_proto.fields.flags, buffer(1,1))
	subtree:add(hamster_proto.fields.msgID, buffer(2,2))
	subtree:add(hamster_proto.fields.length, buffer(4,2))
	subtree:add(hamster_proto.fields.rpcCallID, buffer(6,2))
	if (flags ~= 0x0 and flags ~= 0x1 and flags ~= 0x3) then
		subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "Unknown Flag: " .. string.format("0x%x", flags))
	end

	local subtree = tree:add(hamster_proto, buffer(8, length), "Hamster Payload")
	if (flags == 0x0) then
		if (rpcCallID == 1) then
			-- new
			if (length ~= (32 + 32 + 2)) then
				err = err .. " Error: Wrong Payload Length see expert information"
				subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "Payload length to small should be 66 bytes")
				subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "Payload contains owner_name(32 bytes), hamster_name(32 bytes) and treats(2 bytes)")
			else
				subtree:add(hamster_proto.fields.owner_name, buffer(8, 32))
				subtree:add(hamster_proto.fields.hamster_name, buffer(8 + 32, 32))
				subtree:add(hamster_proto.fields.treats, buffer(8 + 32 + 32, 2))
			end
		elseif (rpcCallID == 2) then
			-- lookup
			if (length ~= (32 + 32)) then
				err = err .. " Error: Wrong Payload Length see expert information"
				subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "Payload length to small should be 64 bytes")
				subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "Payload contains owner_name(32 bytes) and hamster_name(32 bytes)")
			else
				subtree:add(hamster_proto.fields.owner_name, buffer(8, 32))
				subtree:add(hamster_proto.fields.hamster_name, buffer(8 + 32, 32))
			end
		elseif (rpcCallID == 3) then
			-- Directory
			if (length ~= (4 + 32 + 32)) then
				err = err .. " Error: Wrong Payload Length see expert information"
				subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "Payload length to small should be 68 bytes")
				subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "Payload contains File dissector(4 bytes), owner_name(32 bytes) and hamster_name(32 bytes)")
			else
				subtree:add(hamster_proto.fields.fd, buffer(8, 4))
				subtree:add(hamster_proto.fields.owner_name, buffer(8 + 4, 32))
				subtree:add(hamster_proto.fields.hamster_name, buffer(8 + 4 + 32, 32))
			end
		elseif (rpcCallID == 4) then
			-- Hows doing
			if (length ~= 4) then
				err = err .. " Error: Wrong Payload Length see expert information"
				subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "Payload length to small should be 4 bytes")
				subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "Payload contains id (4 bytes)")
			else
				subtree:add(hamster_proto.fields.id, buffer(8, 4))
			end
		elseif (rpcCallID == 5) then
			-- read entry
			if (length ~= 4) then
				err = err .. " Error: Wrong Payload Length see expert information"
				subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "Payload length to small should be 4 bytes")
				subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "Payload contains id (4 bytes)")
			else
				subtree:add(hamster_proto.fields.id, buffer(8, 4))
			end
		elseif (rpcCallID == 6) then
			-- give threads
			if (length ~= (4 + 2)) then
				err = err .. " Error: Wrong Payload Length see expert information"
				subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "Payload length to small should be 4 bytes")
				subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "Payload contains id (4 bytes) and treats(2 bytes)")
			else
				subtree:add(hamster_proto.fields.id, buffer(8, 4))
				subtree:add(hamster_proto.fields.treats, buffer(8 + 4, 2))
			end
		elseif (rpcCallID == 7) then
			-- collect
			if (length ~= 32) then
				err = err .. " Error: Wrong Payload Length see expert information"
				subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "Payload length to small should be 32 bytes")
				subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "Payload contains owner_name(32 bytes)")
			else
				subtree:add(hamster_proto.fields.owner_name, buffer(8, 32))
			end

		else
			subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "Unknown CallID: " .. rpcCallID)
		end
	elseif (flags == 0x1) then
		if (length < 4) then
			err = err .. " Error: Wrong Payload Length see expert information"
			subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "Payload length to small should be >= 4 bytes")
			subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "Payload contains return value(4 bytes)")
		else
			subtree:add(hamster_proto.fields.ret, buffer(8, 4))
			if (rpcCallID == 3) then
				if (length ~= (4 + 4)) then
					err = err .. " Error: Wrong Payload Length see expert information"
					subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "Payload length to small should be 8 bytes")
					subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "Payload contains return value(4 bytes) and file dissector(4 bytes)")
				else
					subtree:add(hamster_proto.fields.fd, buffer(8 + 4, 4))
				end
			elseif (rpcCallID == 4) then
				if (length ~= (4 + 2 + 4 + 2)) then
					err = err .. " Error: Wrong Payload Length see expert information"
					subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "Payload length to small should be 12 bytes")
					subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "Payload contains return value(4 bytes), treats_left(2 bytes), rounds(4 bytes) and cost(2 bytes)")
				else
					subtree:add(hamster_proto.fields.state_treats_left, buffer(8 + 4, 2))
					subtree:add(hamster_proto.fields.state_rounds, buffer(8 + 4 + 2, 4))
					subtree:add(hamster_proto.fields.state_cost, buffer(8 + 4 + 2 + 4, 2))
				end
			elseif (rpcCallID == 5) then
				if (length ~= (4 + 32 + 32 + 2)) then
					err = err .. " Error: Wrong Payload Length see expert information"
					subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "Payload length to small should be 70 bytes")
					subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "Payload contains return value(4 bytes), owner name (32 bytes), hamster name (32 bytes) and cost(2 bytes)")
				else
					subtree:add(hamster_proto.fields.owner_name, buffer(8 + 4, 32))
					subtree:add(hamster_proto.fields.hamster_name, buffer(8 + 4 + 32, 32))
					subtree:add(hamster_proto.fields.state_cost, buffer(8 + 4 + 32 + 32, 2))
				end
			elseif (rpcCallID > 7 or rpcCallID == 0) then
				subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "Unknown CallID: " .. rpcCallID)
			end
		end
	elseif (flags == 0x3) then
		if (length ~= 4) then
			err = err .. " Error: Wrong Payload Length see expert information"
			subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "Payload length to small should be 4 bytes")
			subtree:add_expert_info(PI_MALFORMED, PI_ERROR, "Payload contains return value(4 bytes)")
		else
			subtree:add(hamster_proto.fields.ret, buffer(8, 4))
		end
	end
	pinfo.cols.info = "Hamster " .. info .. err
end

tcp_table = DissectorTable.get("tcp.port")
tcp_table:add(2323, hamster_proto)
tcp_table:add(9000, hamster_proto)
tcp_table:add(9001, hamster_proto)
tcp_table:add(9002, hamster_proto)
tcp_table:add(9003, hamster_proto)
